#ifndef NDN_TOOLS_CHUNKS_CATCHUNKS_AIMD_RTT_ESTIMATOR_HPP
#define NDN_TOOLS_CHUNKS_CATCHUNKS_AIMD_RTT_ESTIMATOR_HPP

#include "common.hpp"

namespace ndn {
namespace chunks {
namespace aimd {

typedef time::duration<double, time::milliseconds::period> Milliseconds;

struct RttRtoSample
{
  uint64_t segNo;
  Milliseconds rtt; ///< measured RTT
  Milliseconds sRtt; ///< smoothed RTT
  Milliseconds rttVar; ///< RTT variation
  Milliseconds rto; ///< retransmission timeout
};

/**
 * @brief RTT Estimator.
 *
 * This class implements the "Mean--Deviation" RTT estimator, as discussed in RFC6298,
 * with the modifications to RTO calculation described in RFC 7323 Appendix G.
 */
class RttEstimator
{
public:
  class Options
  {
  public:
    Options()
      : isVerbose(false)
      , alpha(0.125)
      , beta(0.25)
      , k(4)
      , initialRto(13.0)    //1000.0
      , minRto(13.0)    //200.0    30.0
      , maxRto(4000.0)
      , minDQNInterval(30.0)
      , maxDQNInterval(4000.0)
      , minBBRInterval(10.0)
      , maxBBRInterval(40.0)
      , rtoBackoffMultiplier(2)
      , m_sRttInit(0.0)
      , m_rttVarInit(0.0)
    {
    }

  public:
    bool isVerbose;
    double alpha; ///< parameter for RTT estimation   //平滑因子，保持计算简单的情况尽量考虑历史RTT，alpha越接近于0，则表示SRTT越相信这一次的RTT；越接近于1，则表示SRTT越相信上次统计的RTT
    double beta; ///< parameter for RTT variation calculation   //延迟方差因子
    int k; ///< factor of RTT variation when calculating RTO
    Milliseconds initialRto; ///< initial RTO value
    Milliseconds minRto; ///< lower bound of RTO
    Milliseconds maxRto; ///< upper bound of RTO
    Milliseconds minDQNInterval;
    Milliseconds maxDQNInterval;
    Milliseconds minBBRInterval;
    Milliseconds maxBBRInterval;

    Milliseconds m_sRttInit;
    Milliseconds m_rttVarInit;

    int rtoBackoffMultiplier;
  };

  /**
   * @brief create a RTT Estimator
   *
   * Configures the RTT Estimator with the default parameters if an instance of Options
   * is not passed to the constructor.
   */
  explicit
  RttEstimator(const Options& options = Options());

  /**
   * @brief Add a new RTT measurement to the estimator for the given received segment.
   *
   * @note Don't take RTT measurement for retransmitted segments
   * @param segNo the segment number of the received segmented Data
   * @param rtt the sampled rtt
   * @param nExpectedSamples number of expected samples, must be greater than 0.
   *        It should be set to current number of in-flight Interests. Please
   *        refer to Appendix G of RFC 7323 for details.
   */
  void
  addMeasurement(uint64_t segNo, Milliseconds rtt, size_t nExpectedSamples);

  void
  RTOInit();

  /**
   * @return estimated RTO
   */
  Milliseconds
  getEstimatedRto() const;

  Milliseconds
  getEstimatedDQN_RTT() const;

  Milliseconds
  getEstimatedBBR_RTT() const;

  /**
   * @brief backoff RTO by the factor of RttEstimatorOptions::rtoBackoffMultiplier
   */
  void
  backoffRto();

  /**
   * @brief Signals after rtt is measured
   */
  signal::Signal<RttEstimator, RttRtoSample> afterRttMeasurement;

PUBLIC_WITH_TESTS_ELSE_PRIVATE:
  const Options m_options;
  Milliseconds m_sRtt; ///< smoothed round-trip time
  Milliseconds m_rttVar; ///< round-trip time variation
  Milliseconds m_rto; ///< retransmission timeout
  Milliseconds DQN_RTT;
  Milliseconds BBR_RTT;
};

/**
 * @brief returns the estimated RTO value
 */
inline Milliseconds
RttEstimator::getEstimatedRto() const
{
  return m_rto;
}

inline Milliseconds
RttEstimator::getEstimatedDQN_RTT() const
{
  return DQN_RTT;
}

inline Milliseconds
RttEstimator::getEstimatedBBR_RTT() const
{
  return BBR_RTT;
}

std::ostream&
operator<<(std::ostream& os, const RttEstimator::Options& options);

} // namespace aimd
} // namespace chunks
} // namespace ndn

#endif // NDN_TOOLS_CHUNKS_CATCHUNKS_RTT_ESTIMATOR_HPP
