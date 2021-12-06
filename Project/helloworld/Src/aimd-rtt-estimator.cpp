#include "aimd-rtt-estimator.hpp"
#include <cmath>

namespace ndn {
namespace chunks {
namespace aimd {

RttEstimator::RttEstimator(const Options& options)
  : m_options(options)
  , m_sRtt(std::numeric_limits<double>::quiet_NaN())
  , m_rttVar(std::numeric_limits<double>::quiet_NaN())
  , m_rto(m_options.initialRto.count())
  , DQN_RTT(20)
{
  if (m_options.isVerbose) {
    std::cerr << m_options;
  }
}

void
RttEstimator::RTOInit()
{
    m_sRtt = m_options.m_sRttInit;
    m_rttVar = m_options.m_rttVarInit;
    m_rto = m_options.initialRto;
}

void
RttEstimator::addMeasurement(uint64_t segNo, Milliseconds rtt, size_t nExpectedSamples)   //平均偏差？nExpectedSamples所有发出的兴趣包？从而确定权重？
{                                                                                         //这是Rto的计算，和Rtt不一样
  BOOST_ASSERT(nExpectedSamples > 0);

  if (std::isnan(m_sRtt.count())) { // first measurement
    m_sRtt = rtt;   //Rtt    //smoothRtt
    m_rttVar = m_sRtt / 2;
    m_rto = m_sRtt + m_options.k * m_rttVar;   //Rto一般大于Rtt
  }
  else {
    double alpha = m_options.alpha / nExpectedSamples;
    double beta = m_options.beta / nExpectedSamples;
    m_rttVar = (1 - beta) * m_rttVar + beta * time::abs(m_sRtt - rtt);
    m_sRtt = (1 - alpha) * m_sRtt + alpha * rtt;
    m_rto = m_sRtt + m_options.k * m_rttVar;
  }

  DQN_RTT=ndn::clamp(m_rto,m_options.minDQNInterval,m_options.maxDQNInterval);
  BBR_RTT=ndn::clamp(m_rto,m_options.minBBRInterval,m_options.maxBBRInterval);
  m_rto = ndn::clamp(m_rto, m_options.minRto, m_options.maxRto);

  afterRttMeasurement({segNo, rtt, m_sRtt, m_rttVar, m_rto});
}

void
RttEstimator::backoffRto()
{
  m_rto = ndn::clamp(m_rto * m_options.rtoBackoffMultiplier,
                     m_options.minRto, m_options.maxRto);
}

std::ostream&
operator<<(std::ostream& os, const RttEstimator::Options& options)
{
  os << "RttEstimator initial parameters:\n"
     << "\tAlpha = " << options.alpha << "\n"
     << "\tBeta = " << options.beta << "\n"
     << "\tK = " << options.k << "\n"
     << "\tInitial RTO = " << options.initialRto << "\n"
     << "\tMin RTO = " << options.minRto << "\n"
     << "\tMax RTO = " << options.maxRto << "\n";
  return os;
}

} // namespace aimd
} // namespace chunks
} // namespace ndn
