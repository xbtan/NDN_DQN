#ifndef NDN_TOOLS_CHUNKS_CATCHUNKS_DATA_FETCHER_HPP
#define NDN_TOOLS_CHUNKS_CATCHUNKS_DATA_FETCHER_HPP

#include "common.hpp"

namespace ndn {
namespace chunks {

/**
 * @brief fetch data for a given interest and handle timeout or nack error with retries
 *
 * To instantiate a DataFetcher you need to use the static method fetch, this will also express the
 * interest. After a timeout or nack is received, the same interest with a different nonce will be
 * requested for a maximum number of time specified by the class user. There are separate retry
 * counters for timeouts and nacks.
 *
 * A specified callback is called after the data matching the expressed interest is received. A
 * different callback is called in case one of the retries counter reach the maximum. This callback
 * can be different for timeout and nack. The data callback must be defined but the others callback
 * are optional.
 *
 */
class DataFetcher
{
public:
  /**
   * @brief means that there is no maximum number of retries,
   *        i.e. fetching must be retried indefinitely
   */
  static const int MAX_RETRIES_INFINITE;

  /**
   * @brief ceiling value for backoff time used in congestion handling
   */
  static const time::milliseconds MAX_CONGESTION_BACKOFF_TIME;

  typedef function<void(const Interest& interest, const std::string& reason)> FailureCallback;

  /**
   * @brief instantiate a DataFetcher object and start fetching data
   *
   * @param onData callback for segment correctly received, must not be empty
   */
  static shared_ptr<DataFetcher>
  fetch(Face& face, const Interest& interest, int maxNackRetries, int maxTimeoutRetries,
        DataCallback onData, FailureCallback onTimeout, FailureCallback onNack,
        bool isVerbose);

  /**
   * @brief stop data fetching without error and calling any callback
   */
  void
  cancel();

  bool
  isRunning() const
  {
    return !m_isStopped && !m_hasError;
  }

  bool
  hasError() const
  {
    return m_hasError;
  }

private:
  DataFetcher(Face& face, int maxNackRetries, int maxTimeoutRetries,
              DataCallback onData, FailureCallback onNack, FailureCallback onTimeout,
              bool isVerbose);

  void
  expressInterest(const Interest& interest, const shared_ptr<DataFetcher>& self);

  void
  handleData(const Interest& interest, const Data& data, const shared_ptr<DataFetcher>& self);

  void
  handleNack(const Interest& interest, const lp::Nack& nack, const shared_ptr<DataFetcher>& self);

  void
  handleTimeout(const Interest& interest, const shared_ptr<DataFetcher>& self);

private:
  Face& m_face;
  Scheduler m_scheduler;
  const PendingInterestId* m_interestId;
  DataCallback m_onData;
  FailureCallback m_onNack;
  FailureCallback m_onTimeout;

  int m_maxNackRetries;
  int m_maxTimeoutRetries;
  int m_nNacks;
  int m_nTimeouts;
  uint32_t m_nCongestionRetries;

  bool m_isVerbose;
  bool m_isStopped;
  bool m_hasError;
};

} // namespace chunks
} // namespace ndn

#endif // NDN_TOOLS_CHUNKS_CATCHUNKS_DATA_FETCHER_HPP
