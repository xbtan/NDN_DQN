#ifndef NDN_TOOLS_CHUNKS_CATCHUNKS_DISCOVER_VERSION_ITERATIVE_HPP
#define NDN_TOOLS_CHUNKS_CATCHUNKS_DISCOVER_VERSION_ITERATIVE_HPP

#include "discover-version.hpp"

namespace ndn {
namespace chunks {

/**
 * @brief Options for discover version iterative DiscoverVersionIterative
 *
 * The canonical name to use is DiscoverVersionIterative::Options
 */
class DiscoverVersionIterativeOptions : public virtual Options
{
public:
  explicit
  DiscoverVersionIterativeOptions(const Options& opt = Options())
    : Options(opt)
    , maxRetriesAfterVersionFound(1)
  {
  }

public:
  int maxRetriesAfterVersionFound;  // used only in timeout handling
};

/**
 * @brief Service for discovering the latest Data version in the iterative way
 *
 * Identifies the latest retrievable version published under the specified namespace
 * (as specified by the Version marker).
 *
 * DiscoverVersionIterative declares the largest discovered version to be the latest after some
 * Interest timeouts (i.e. failed retrieval after exclusion and retransmission). The number of
 * timeouts are specified by the value of maxRetriesAfterVersionFound inside the iterative options.
 *
 * The received name component after version can be an invalid segment number, this component will
 * be excluded in the next interests. In the unlikely case that there are too many excluded
 * components such that the Interest cannot fit in ndn::MAX_NDN_PACKET_SIZE, the discovery
 * procedure will throw Face::Error.
 *
 * DiscoverVersionIterative's user is notified once after identifying the latest retrievable
 * version or on failure to find any version Data.
 */
/*
DiscoverVersionIterative将最大的发现版本声明为在一些兴趣超时之后的最新版本（即排除和重新传输后检索失败）。
 超时次数由迭代选项中的maxRetriesAfterVersionFound值指定。

接收到的版本后的名称组件可以是无效的段号，该组件将被排除在下面的兴趣之中。
在不太可能的情况下，排除的组件太多，使得兴趣不能适用于ndn :: MAX_NDN_PACKET_SIZE，
发现过程将抛出Face :: Error。

DiscoverVersionIterative的用户在识别最新可检索的版本后或在找不到任何版本的数据时被通知一次。
*/
class DiscoverVersionIterative : public DiscoverVersion, protected DiscoverVersionIterativeOptions
{
public:
  typedef DiscoverVersionIterativeOptions Options;

public:
  /**
   * @brief create a DiscoverVersionIterative service
   */
  DiscoverVersionIterative(const Name& prefix, Face& face, const Options& options);

  /**
   * @brief identify the latest Data version published.
   */
  void
  run() final;

private:
  void
  handleData(const Interest& interest, const Data& data) final;

  void
  handleTimeout(const Interest& interest, const std::string& reason) final;

private:
  uint64_t m_latestVersion;
  shared_ptr<const Data> m_latestVersionData;
  Exclude m_strayExcludes;
  bool m_foundVersion;
};

} // namespace chunks
} // namespace ndn

#endif // NDN_TOOLS_CHUNKS_CATCHUNKS_DISCOVER_VERSION_ITERATIVE_HPP
