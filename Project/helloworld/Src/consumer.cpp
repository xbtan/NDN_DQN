#include "consumer.hpp"
#include <qtimer.h>
#include "Src/pipeline-interests-fixed-window.hpp"

namespace ndn {
namespace chunks {
uint64_t Consumer::SegmentNoForTimer=0;
uint64_t Consumer::ReceivedSegment=0;

//Consumer::Consumer(Validator& validator, bool isVerbose, ofstream& os)   //旧版-
Consumer::Consumer(security::v2::Validator& validator, bool isVerbose, std::ostream& os)   //新版+
  : m_validator(validator)
  , m_outputStream(os)
  , m_nextToPrint(0)
  , m_isVerbose(isVerbose)
{
}

void
Consumer::run(unique_ptr<DiscoverVersion> discover, unique_ptr<PipelineInterests> pipeline)
{
  m_discover = std::move(discover);
  m_pipeline = std::move(pipeline);
  m_nextToPrint = 0;
  m_bufferedData.clear();

  m_discover->onDiscoverySuccess.connect(bind(&Consumer::startPipeline, this, _1));   //connect
  m_discover->onDiscoveryFailure.connect([] (const std::string& msg) {
      BOOST_THROW_EXCEPTION(std::runtime_error(msg));
    });

  m_discover->run();   //->DiscoverVersionFixed::run()
}

void
Consumer::startPipeline(const Data& data)
{cout<<"startPipeline: "<<"  "<<data.getName().toUri()<<"  "<<endl;

  this->handleData(data);

  m_pipeline->run(data,
      [this] (const Interest&, const Data& data) { this->handleData(data); },
      [] (const std::string& msg) { BOOST_THROW_EXCEPTION(std::runtime_error(msg)); });


   // m_validator.validate(data,   //新版本函数不匹配
   //                      bind(&Consumer::onDataValidated, this, _1),   //类的对象的函数绑定，需要用到类的对象或指针
   //                      bind(&Consumer::onFailure, this, _2));
 // m_validator.validate(data,   //新版本函数不匹配
  //                     bind(&Consumer::onDataValidated, this, _1),   //类的对象的函数绑定，需要用到类的对象或指针
  //                     bind(&Consumer::onFailure, this, _2));


  //m_pipeline->run(data,
  //                bind(&Consumer::onData, this, _1, _2),
  //                bind(&Consumer::onFailure, this, _1));
}

void
Consumer::handleData(const Data& data)
{
  auto dataPtr = data.shared_from_this();

  m_validator.validate(data,
    [this, dataPtr] (const Data& data) {
      if (data.getContentType() == ndn::tlv::ContentType_Nack) {
        if (m_isVerbose) {
          std::cerr << "Application level NACK: " << data << std::endl;
        }
        m_pipeline->cancel();
        BOOST_THROW_EXCEPTION(ApplicationNackError(data));
      }

      // 'data' passed to callback comes from DataValidationState and was not created with make_shared
      m_bufferedData[getSegmentFromPacket(data)] = dataPtr;

      //cout<<"StoreDataInBuffer:"<<getSegmentFromPacket(data)<<endl;

      DownloadDataBuffer=dataPtr;   //自己加
      writeInOrderData();
    },
    [this] (const Data& data, const security::v2::ValidationError& error) {
      BOOST_THROW_EXCEPTION(DataValidationError(error));
    });
}

/*
void
Consumer::onData(const Interest& interest, const Data& data)
{cout<<"onData: "<<"  "<<data.getName().toUri()<<"  "<<endl;
//  m_validator.validate(data,   //新版函数不匹配
//                       bind(&Consumer::onDataValidated, this, _1),
//                       bind(&Consumer::onFailure, this, _2));
}

void
Consumer::onDataValidated(shared_ptr<const Data> data)
{cout<<"Consumer::onDataValidated"<<endl;
  if (data->getContentType() == ndn::tlv::ContentType_Nack) {
    if (m_isVerbose)
      std::cerr << "Application level NACK: " << *data << std::endl;

    m_pipeline->cancel();
    throw ApplicationNackError(*data);
  }
    DownloadDataBuffer=data;
    writeInOrderData();
  cout<<"****Finish****"<<getSegmentFromPacket(*data)<<endl;
}

void
Consumer::onFailure(const std::string& reason)
{cout<<"onFailure"<<endl;
  throw std::runtime_error(reason);
}
*/

void
Consumer::writeInOrderData()
{
    for (auto it = m_bufferedData.begin();
         it != m_bufferedData.end() && it->first == m_nextToPrint;
         it = m_bufferedData.erase(it), ++m_nextToPrint)
    {
      const Block& content = it->second->getContent();
      m_outputStream.write(reinterpret_cast<const char*>(content.value()), content.value_size());
}
      SegmentNoForTimer++;
      ReceivedSegment++;


    //const Block& content=DownloadDataBuffer->getContent();
    //m_outputStream.write(reinterpret_cast<const char*>(content.value()), content.value_size());
}


} // namespace chunks
} // namespace ndn
