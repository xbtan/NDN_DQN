#ifndef NDN_TOOLS_CHUNKS_CATCHUNKS_DISCOVER_VERSION_FIXED_HPP
#define NDN_TOOLS_CHUNKS_CATCHUNKS_DISCOVER_VERSION_FIXED_HPP

#include "discover-version.hpp"

namespace ndn {
namespace chunks {

/**
 * @brief Service to retrieve a specific version segment. The version is specified in the prefix
 *
 * Send a request of a specific version and expect to be answered with one segment.
 *
 * The received name component after version can be an invalid segment number, this component will
 * be excluded in the next interests. In the unlikely case that there are too many excluded
 * components such that the Interest cannot fit in ndn::MAX_NDN_PACKET_SIZE, the discovery
 * procedure will throw Face::Error.
 *
 * DiscoverVersionFixed's user is notified once after one segment with the user specified version
 * is found or on failure to find any Data version.
 */
class DiscoverVersionFixed : public DiscoverVersion
{

public:
  /**
   * @brief create a DiscoverVersionSpecified service
   */
  DiscoverVersionFixed(const Name& prefix, Face& face, const Options& options);

  /**
   * @brief identify the latest Data version published.
   */
  void
  run() final;

private:
  void
  handleData(const Interest& interest, const Data& data) final;

private:
  Exclude m_strayExcludes;
};

} // namespace chunks
} // namespace ndn

#endif // NDN_TOOLS_CHUNKS_CATCHUNKS_DISCOVER_VERSION_FIXED_HPP
