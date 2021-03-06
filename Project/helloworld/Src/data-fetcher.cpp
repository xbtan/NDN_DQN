#include "data-fetcher.hpp"
#include <cmath>
#include <iostream>
using namespace std;

namespace ndn {
namespace chunks {

const int DataFetcher::MAX_RETRIES_INFINITE = -1;
const time::milliseconds DataFetcher::MAX_CONGESTION_BACKOFF_TIME = time::seconds(10);

shared_ptr<DataFetcher>
DataFetcher::fetch(Face& face, const Interest& interest, int maxNackRetries, int maxTimeoutRetries,
                   DataCallback onData, FailureCallback onNack, FailureCallback onTimeout,
                   bool isVerbose)
{
  auto dataFetcher = shared_ptr<DataFetcher>(new DataFetcher(face,
                                                             maxNackRetries,
                                                             maxTimeoutRetries,
                                                             std::move(onData),
                                                             std::move(onNack),
                                                             std::move(onTimeout),
                                                             isVerbose));
  dataFetcher->expressInterest(interest, dataFetcher);   //->DataFetcher::expressInterest
  return dataFetcher;
}

DataFetcher::DataFetcher(Face& face, int maxNackRetries, int maxTimeoutRetries,
                         DataCallback onData, FailureCallback onNack, FailureCallback onTimeout,
                         bool isVerbose)
  : m_face(face)
  , m_scheduler(m_face.getIoService())
  , m_onData(std::move(onData))
  , m_onNack(std::move(onNack))
  , m_onTimeout(std::move(onTimeout))
  , m_maxNackRetries(maxNackRetries)
  , m_maxTimeoutRetries(maxTimeoutRetries)
  , m_nNacks(0)
  , m_nTimeouts(0)
  , m_nCongestionRetries(0)
  , m_isVerbose(isVerbose)
  , m_isStopped(false)
  , m_hasError(false)
{
  BOOST_ASSERT(m_onData != nullptr);
}

void
DataFetcher::cancel()
{
  if (isRunning()) {
    m_isStopped = true;
    m_face.removePendingInterest(m_interestId);
    m_scheduler.cancelAllEvents();
  }
}

void
DataFetcher::expressInterest(const Interest& interest, const shared_ptr<DataFetcher>& self)
{cout<<"DataFetcher::expressInterest  "<<interest.getName().toUri()<<endl;
  m_nCongestionRetries = 0;
  m_interestId = m_face.expressInterest(interest,   //->face.expressInterest
                                        bind(&DataFetcher::handleData, this, _1, _2, self),   //->DataFetcher::handleData
                                        bind(&DataFetcher::handleNack, this, _1, _2, self),   //->connect
                                        bind(&DataFetcher::handleTimeout, this, _1, self));
}

void
DataFetcher::handleData(const Interest& interest, const Data& data,
                        const shared_ptr<DataFetcher>& self)
{cout<<"DataFetcher::handleData"<<endl;
  if (!isRunning())
    return;

  m_isStopped = true;
  m_onData(interest, data);
}

void
DataFetcher::handleNack(const Interest& interest, const lp::Nack& nack,
                        const shared_ptr<DataFetcher>& self)
{cout<<"DataFetcher::handleNack"<<endl;
  if (!isRunning())
    return;

  if (m_maxNackRetries != MAX_RETRIES_INFINITE)
    ++m_nNacks;

  if (m_isVerbose)
    std::cerr << "Received Nack with reason " << nack.getReason()
              << " for Interest " << interest << std::endl;

  if (m_nNacks <= m_maxNackRetries || m_maxNackRetries == MAX_RETRIES_INFINITE) {
    Interest newInterest(interest);
    newInterest.refreshNonce();

    switch (nack.getReason()) {
      case lp::NackReason::DUPLICATE: {
        expressInterest(newInterest, self);
        break;
      }
      case lp::NackReason::CONGESTION: {
        time::milliseconds backoffTime(static_cast<uint64_t>(std::pow(2, m_nCongestionRetries)));
        if (backoffTime > MAX_CONGESTION_BACKOFF_TIME)
          backoffTime = MAX_CONGESTION_BACKOFF_TIME;
        else
          m_nCongestionRetries++;

        m_scheduler.scheduleEvent(backoffTime, bind(&DataFetcher::expressInterest, this,
                                                    newInterest, self));
        break;
      }
      default: {
        m_hasError = true;
        if (m_onNack)
          m_onNack(interest, "Could not retrieve data for " + interest.getName().toUri() +
                             ", reason: " + boost::lexical_cast<std::string>(nack.getReason()));
        break;
      }
    }
  }
  else {
    m_hasError = true;
    if (m_onNack)
      m_onNack(interest, "Reached the maximum number of nack retries (" + to_string(m_maxNackRetries) +
                         ") while retrieving data for " + interest.getName().toUri());
  }
}

void
DataFetcher::handleTimeout(const Interest& interest, const shared_ptr<DataFetcher>& self)
{cout<<"DataFetcher::handleTimeout"<<endl;
  if (!isRunning())
    return;

  if (m_maxTimeoutRetries != MAX_RETRIES_INFINITE)
    ++m_nTimeouts;

  if (m_isVerbose)
    std::cerr << "Timeout for Interest " << interest << std::endl;

  if (m_nTimeouts <= m_maxTimeoutRetries || m_maxTimeoutRetries == MAX_RETRIES_INFINITE) {
    Interest newInterest(interest);
    newInterest.refreshNonce();
    expressInterest(newInterest, self);
  }
  else {
    m_hasError = true;
    if (m_onTimeout)
      m_onTimeout(interest, "Reached the maximum number of timeout retries (" + to_string(m_maxTimeoutRetries) +
                            ") while retrieving data for " + interest.getName().toUri());
  }
}

} // namespace chunks
} // namespace ndn
