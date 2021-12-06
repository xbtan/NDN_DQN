#include "options.hpp"

#include <ndn-cxx/interest.hpp>

namespace ndn {
namespace chunks {

Options::Options()
  : interestLifetime(ndn::DEFAULT_INTEREST_LIFETIME)
  //: interestLifetime(time::milliseconds(300000))   //300000
  , maxRetriesOnTimeoutOrNack(3000)   //3?  300?
  , mustBeFresh(false)
  , isVerbose(false)
{
}

} // namespace chunks
} // namespace ndn
