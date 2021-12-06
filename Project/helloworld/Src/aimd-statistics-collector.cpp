#include "aimd-statistics-collector.hpp"

namespace ndn {
namespace chunks {
namespace aimd {

StatisticsCollector::StatisticsCollector(PipelineInterestsAimd& pipeline, RttEstimator& rttEstimator,
                                         std::ostream& osCwnd, std::ostream& osRtt)
  : m_osCwnd(osCwnd)
  , m_osRtt(osRtt)
{
  m_osCwnd << "time\tcwndsize\n";
  m_osRtt  << "segment\trtt\trttvar\tsrtt\trto\n";
  pipeline.afterCwndChange.connect(
    [this] (Milliseconds timeElapsed, double cwnd) {
      m_osCwnd << timeElapsed.count() / 1000 << '\t' << cwnd << '\n';
    });
  rttEstimator.afterRttMeasurement.connect(
    [this] (const RttRtoSample& rttSample) {
      m_osRtt << rttSample.segNo << '\t'
              << rttSample.rtt.count() << '\t'
              << rttSample.rttVar.count() << '\t'
              << rttSample.sRtt.count() << '\t'
              << rttSample.rto.count() << '\n';
    });
}

} // namespace aimd
} // namespace chunks
} // namespace ndn
