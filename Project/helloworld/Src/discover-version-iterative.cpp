#include "discover-version-iterative.hpp"

namespace ndn {
namespace chunks {

DiscoverVersionIterative::DiscoverVersionIterative(const Name& prefix, Face& face,
                                                   const Options& options)
  : chunks::Options(options)
  , DiscoverVersion(prefix, face, options)
  , Options(options)
  , m_latestVersion(0)
  , m_latestVersionData(nullptr)
  , m_foundVersion(false)
{
}

void
DiscoverVersionIterative::run()
{
  m_latestVersion = 0;
  m_foundVersion = false;

  Interest interest(m_prefix);
  interest.setInterestLifetime(interestLifetime);   //设置兴趣包有效期
  interest.setMustBeFresh(mustBeFresh);   //兴趣包可以是上一次的
  interest.setMinSuffixComponents(3);   //为兴趣包设置所需文件分块数的最小值
  interest.setMaxSuffixComponents(3);   //为兴趣包设置所需文件分块数的最小值
  interest.setChildSelector(1);   //设置一个子选择器

  expressInterest(interest, maxRetriesOnTimeoutOrNack, maxRetriesOnTimeoutOrNack);   //发出兴趣包，超时或者返回否定应答，可重复发送三次
}

void
DiscoverVersionIterative::handleData(const Interest& interest, const Data& data)
{
  size_t versionindex = m_prefix.size();

  const Name& name = data.getName();
  Exclude exclude;

  if (isVerbose)
    std::cerr << "Data: " << data << std::endl;

  BOOST_ASSERT(name.size() > m_prefix.size());   //数据包名字要长于用户输入的前缀
  if (name[versionindex].isVersion()) {   //在预期的索引中找到版本号
    m_latestVersion = name[versionindex].toVersion();   //将版本赋值给存储最新版本的变量
    m_latestVersionData = make_shared<Data>(data);   //将该版本的数据内容赋给存储变量
    m_foundVersion = true;   //找到了一个版本的数据

    exclude.excludeBefore(name[versionindex]);   //设置过滤器，排除这个版本之前的版本

    if (isVerbose)
      std::cerr << "Discovered version = " << m_latestVersion << std::endl;
  }
  else {   //没有在预期的索引中找到版本号
    m_strayExcludes.excludeOne(name[versionindex]);   //设置过滤器，排除这个版本索引对应的数据包
  }

  for (const Exclude::Range& range : m_strayExcludes) {
    BOOST_ASSERT(range.isSingular());
    exclude.excludeOne(range.from);
  }

  Interest newInterest(interest);   //定义新的兴趣包
  newInterest.refreshNonce();   //刷新当前标志值
  newInterest.setExclude(exclude);   //新的兴趣包设置排除范围

  if (m_foundVersion)   //如果检索到了一个版本，发出新的兴趣包（由检索到一个版本之后的重新请求的次数限定，1次）
    expressInterest(newInterest, maxRetriesOnTimeoutOrNack, maxRetriesAfterVersionFound);
  else   //如果未检索到，再次发出之前的兴趣包
    expressInterest(interest, maxRetriesOnTimeoutOrNack, maxRetriesOnTimeoutOrNack);
}

void
DiscoverVersionIterative::handleTimeout(const Interest& interest, const std::string& reason)
{
  if (m_foundVersion) {   //在时间期限内找到了相应版本的数据包，将目前这个版本定为最新版本
    if (isVerbose)
      std::cerr << "Found data with the latest version: " << m_latestVersion << std::endl;

    this->emitSignal(onDiscoverySuccess, *m_latestVersionData);   //检索到了至少一个版本的数据包，将其定为最新版本
  }
  else {   //超时处理
    DiscoverVersion::handleTimeout(interest, reason);
  }
}

} // namespace chunks
} // namespace ndn
