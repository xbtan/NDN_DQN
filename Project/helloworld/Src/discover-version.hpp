#ifndef NDN_TOOLS_CHUNKS_CATCHUNKS_DISCOVER_VERSION_HPP
#define NDN_TOOLS_CHUNKS_CATCHUNKS_DISCOVER_VERSION_HPP

#include "common.hpp"
#include "options.hpp"

namespace ndn {
namespace chunks {

class DataFetcher;

/**
 * @brief Base class of services for discovering the latest Data version
 *
 * DiscoverVersion's user is notified once after identifying the latest retrievable version or
 * on failure to find any Data version.
 */
class DiscoverVersion : virtual protected Options, noncopyable
{
public: // signals
  /**
   * @brief Signal emited when the first segment of a specific version is found.
   */
  signal::Signal<DiscoverVersion, const Data&> onDiscoverySuccess;

  /**
   * @brief Signal emitted when a failure occurs.
   */
  signal::Signal<DiscoverVersion, const std::string&> onDiscoveryFailure;

  DECLARE_SIGNAL_EMIT(onDiscoverySuccess)
  DECLARE_SIGNAL_EMIT(onDiscoveryFailure)

public:
  /**
   * @brief create a DiscoverVersion service
   */
  DiscoverVersion(const Name& prefix, Face& face, const Options& options);

  /**
   * @brief identify the latest Data version published.
   */
  virtual void
  run() = 0;

protected:
  void
  expressInterest(const Interest& interest, int maxRetriesNack, int maxRetriesTimeout);

  virtual void
  handleData(const Interest& interest, const Data& data);

  virtual void
  handleNack(const Interest& interest, const std::string& reason);

  virtual void
  handleTimeout(const Interest& interest, const std::string& reason);

protected:
  const Name m_prefix;
  Face& m_face;
  shared_ptr<DataFetcher> fetcher;
};

} // namespace chunks
} // namespace ndn

#endif // NDN_TOOLS_CHUNKS_CATCHUNKS_DISCOVER_VERSION_HPP
