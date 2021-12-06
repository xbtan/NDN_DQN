#include "discover-version-fixed.hpp"
#include <cmath>
#include <boost/lexical_cast.hpp>
#include <iostream>
using namespace std;

namespace ndn {
namespace chunks {

DiscoverVersionFixed::DiscoverVersionFixed(const Name& prefix, Face& face, const Options& options)
  : Options(options)
  , DiscoverVersion(prefix, face, options)
  , m_strayExcludes()
{
}

void
DiscoverVersionFixed::run()
{cout<<"DiscoverVersionFixed::run"<<endl;
  Interest interest(m_prefix);
  interest.setInterestLifetime(interestLifetime);
  interest.setMustBeFresh(mustBeFresh);
  interest.setMaxSuffixComponents(2);
  interest.setMinSuffixComponents(2);

  expressInterest(interest, maxRetriesOnTimeoutOrNack, maxRetriesOnTimeoutOrNack);   //->DiscoverVersion::expressInterest
}

void
DiscoverVersionFixed::handleData(const Interest& interest, const Data& data)
{cout<<"DiscoverVersionFixed::handleData"<<endl;
  if (isVerbose)
    std::cerr << "Data: " << data << std::endl;

  size_t segmentIndex = interest.getName().size();

cout<<"DiscoverVersionFixed::handleData"<<"  "<<data.getName().toUri()<<"  "<<segmentIndex<<endl;

  if (data.getName()[segmentIndex].isSegment()) {
    if (isVerbose)
      std::cerr << "Found data with the requested version: " << m_prefix[-1] << std::endl;

cout<<"data.getName()[segmentIndex].isSegment"<<endl;

    this->emitSignal(onDiscoverySuccess, data);
  }
  else {
    // data isn't a valid segment, add to the exclude list
    m_strayExcludes.excludeOne(data.getName()[segmentIndex]);
    Interest newInterest(interest);
    newInterest.refreshNonce();
    newInterest.setExclude(m_strayExcludes);

    expressInterest(newInterest, maxRetriesOnTimeoutOrNack, maxRetriesOnTimeoutOrNack);
  }
}

} // namespace chunks
} // namespace ndn
