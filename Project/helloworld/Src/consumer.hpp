// basic file operations
#include <iostream>
#include <fstream>
using namespace std;

#ifndef NDN_TOOLS_CHUNKS_CATCHUNKS_CONSUMER_HPP
#define NDN_TOOLS_CHUNKS_CATCHUNKS_CONSUMER_HPP

//#include <ndn-cxx/security/validator.hpp>   //旧版-
#include <ndn-cxx/security/v2/validation-error.hpp>   //新版+
#include <ndn-cxx/security/v2/validator.hpp>   //新版+

#include "discover-version.hpp"
#include "discover-version-fixed.hpp"
#include "discover-version-iterative.hpp"
#include "pipeline-interests.hpp"

namespace ndn {
namespace chunks {
/**
 * @brief Segmented version consumer
 *
 * Discover the latest version of the data published under a specified prefix, and retrieve all the
 * segments associated to that version. The segments are fetched in order and written to a
 * user-specified stream in the same order.
 */
class Consumer : noncopyable
{
public:
  static uint64_t SegmentNoForTimer;
  static uint64_t ReceivedSegment;

  class ApplicationNackError : public std::runtime_error
  {
  public:
    explicit
    ApplicationNackError(const Data& data)
      : std::runtime_error("Application generated Nack: " + boost::lexical_cast<std::string>(data))
    {
    }
  };

  //新版+
  class DataValidationError : public std::runtime_error
  {
  public:
    explicit
    DataValidationError(const security::v2::ValidationError& error)
      : std::runtime_error(boost::lexical_cast<std::string>(error))
    {
    }
  };


  /**
   * @brief Create the consumer
   */
  //Consumer(Validator& validator, bool isVerbose, ofstream& os);   //旧版-
  //Consumer(security::v2::Validator& validator, bool isVerbose, std::ostream& os = std::cout);
  Consumer(security::v2::Validator& validator, bool isVerbose, std::ostream& os);

  /**
   * @brief Run the consumer
   */
  void
  run(unique_ptr<DiscoverVersion> discover, unique_ptr<PipelineInterests> pipeline);

private:
  void
  startPipeline(const Data& data);

  void
  handleData(const Data& data);

PUBLIC_WITH_TESTS_ELSE_PRIVATE:
  void
  writeInOrderData();

private:
  //Validator& m_validator;   //旧版-
  ndn::security::v2::Validator& m_validator;   //新版+

  std::ostream& m_outputStream;
  unique_ptr<DiscoverVersion> m_discover;
  unique_ptr<PipelineInterests> m_pipeline;
  uint64_t m_nextToPrint;
  bool m_isVerbose;
  shared_ptr<const Data> DownloadDataBuffer;

PUBLIC_WITH_TESTS_ELSE_PRIVATE:
  std::map<uint64_t, shared_ptr<const Data>> m_bufferedData;   //std::map是一个有序关联容器
};

} // namespace chunks
} // namespace ndn

#endif // NDN_TOOLS_CHUNKS_CATCHUNKS_CONSUMER_HPP
