#include "discover-version.hpp"
#include <iostream>
#include "data-fetcher.hpp"

using namespace std;

namespace ndn {
namespace chunks {

DiscoverVersion::DiscoverVersion(const Name& prefix, Face& face, const Options& options)
  : Options(options)
  , m_prefix(prefix)
  , m_face(face)
{
}

void
DiscoverVersion::expressInterest(const Interest& interest, int maxRetriesNack,
                                 int maxRetriesTimeout)
{//cout<<"DiscoverVersion::expressInterest"<<endl;
  fetcher = DataFetcher::fetch(m_face, interest, maxRetriesNack, maxRetriesTimeout,
                               bind(&DiscoverVersion::handleData, this, _1, _2),
                               bind(&DiscoverVersion::handleNack, this, _1, _2),
                               bind(&DiscoverVersion::handleTimeout, this, _1, _2),
                               isVerbose);   //->DataFetcher
}

void
DiscoverVersion::handleData(const Interest& interest, const Data& data)
{

cout<<"DiscoverVersion::handleData"<<endl;

  onDiscoverySuccess(data);
}

void
DiscoverVersion::handleNack(const Interest& interest, const std::string& reason)
{

cout<<"DiscoverVersion::handleNack"<<endl;

  onDiscoveryFailure(reason);
}

void
DiscoverVersion::handleTimeout(const Interest& interest, const std::string& reason)
{

cout<<"DiscoverVersion::handleTimeout"<<endl;

  onDiscoveryFailure(reason);
}

} // namespace chunks
} // namespace ndn
