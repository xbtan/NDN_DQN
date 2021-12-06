#ifndef NDN_TOOLS_CHUNKS_CATCHUNKS_OPTIONS_HPP
#define NDN_TOOLS_CHUNKS_CATCHUNKS_OPTIONS_HPP

#include <ndn-cxx/util/time.hpp>

namespace ndn {
namespace chunks {

class Options
{
public:
  Options();

public:
  time::milliseconds interestLifetime;
  int maxRetriesOnTimeoutOrNack;
  bool mustBeFresh;
  bool isVerbose;
};

} // namespace chunks
} // namespace ndn

#endif // NDN_TOOLS_CHUNKS_CATCHUNKS_OPTIONS_HPP
