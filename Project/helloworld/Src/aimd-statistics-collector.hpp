#ifndef NDN_TOOLS_CHUNKS_CATCHUNKS_AIMD_STATISTICS_COLLECTOR_HPP
#define NDN_TOOLS_CHUNKS_CATCHUNKS_AIMD_STATISTICS_COLLECTOR_HPP

#include "pipeline-interests-aimd.hpp"
#include "aimd-rtt-estimator.hpp"

namespace ndn {
namespace chunks {
namespace aimd {

/**
 * @brief Statistics collector for AIMD pipeline
 */
class StatisticsCollector : noncopyable
{
public:
  StatisticsCollector(PipelineInterestsAimd& pipeline, RttEstimator& rttEstimator,
                      std::ostream& osCwnd, std::ostream& osRtt);

private:
  std::ostream& m_osCwnd;
  std::ostream& m_osRtt;
};

} // namespace aimd
} // namespace chunks
} // namespace ndn

#endif // NDN_TOOLS_CHUNKS_CATCHUNKS_AIMD_STATISTICS_COLLECTOR_HPP
