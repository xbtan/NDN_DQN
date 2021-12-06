#include "Publisher.hpp"
#include "Add.h"
#include <string>
#include <string.h>
#include <sstream>
#include <qmessagebox.h>
#include <QTextStream>
#include "ui_filedlg.h"
#include <qclipboard.h>
#include <ratio>
#include <chrono>
#include <time.h>
#include <thread>
#include <qdebug.h>
#include "mainwindow.h"

namespace ndn {
namespace segment {
//静态变量初始化
uint64_t Publisher::SegmentNoForTimer=0;    //用于显示速度的分块计数
uint64_t Publisher::PublishedSegment=0;   //已上传的数据包计数
uint64_t Publisher::WholeFileSegmentNo=0;   //整个文件的分快数

Publisher::Publisher(const Name& prefix,   //前缀
                   Face& face,   //抽象接口
                   KeyChain& keyChain,   //秘钥
                   const security::SigningInfo& signingInfo,   //签名信息
                   time::milliseconds freshnessPeriod,   //发布期限
                   size_t maxSegmentSize,   //最大文件分块尺寸
                   bool isVerbose,   //是否冗长
                   ifstream& is)   //输入文件流
  : m_face(face)
  , m_keyChain(keyChain)
  , m_signingInfo(signingInfo)
  , m_freshnessPeriod(freshnessPeriod)
  , m_maxSegmentSize(maxSegmentSize)
  , m_isVerbose(isVerbose)
  , InFileStream(is)
  , UploadPacketNo(1)
  , m_store(3)
  , IsFileEnd(false)
{
  if (prefix.size() > 0 && prefix[-1].isVersion()) {  //前缀最后的内容已经包含版本编码，就用这个版本编码
    m_prefix = prefix.getPrefix(-1);
    m_versionedPrefix = prefix;
  }
  else {
    m_prefix = prefix;
    //m_versionedPrefix = Name(m_prefix).appendVersion();   //使用当前UNIX以毫秒为单位的时间戳生成版本编码
    m_versionedPrefix = Name(m_prefix);   //+
  }

  if (Filedlg::PublishMode==1)   //模式1
  {
      StoreWholeFile(is);   //上传整个文件
  }
  else if (Filedlg::PublishMode==2)   //模式2
  {
      UploadFirstPacket();   //先上传第一个文件分块
  }

  string str1;
  str1=m_versionedPrefix.toUri();   //将前缀转换成字符串
  str1=str1.erase(0,1);
  QString str2;
  str2=QString(QString::fromLocal8Bit(str1.c_str()));
  Filedlg::File_Version=QString(QString::fromLocal8Bit(str1.c_str()));   //将前缀赋给对话框进行显示

  QClipboard *cb = QApplication::clipboard();
  cb->setText(str2);   //将前缀复制到剪切板

  m_face.setInterestFilter(m_prefix,   //设置兴趣包的过滤器
                           bind(&Publisher::onInterest, this, _2),
                           RegisterPrefixSuccessCallback(),
                           bind(&Publisher::onRegisterFailed, this, _1, _2));

  if (m_isVerbose)
    std::cerr << "Data published with name: " << m_versionedPrefix << std::endl;
}

void Publisher::run()
{
   m_face.processEvents(time::seconds::zero(),false);   //开启IO处理，处理所有接收到的数据或者进行超时处理
}

void Publisher::onInterest(const Interest& interest)   //兴趣包的处理函数
{
    ofstream outfile2;
    string str_outfile2;
    str_outfile2="../UpLoad/OnInterest_Log.txt";
    outfile2.open(str_outfile2,ios::out|ios::binary|ios::app);
    std::ostream& log2(outfile2);

  if (Filedlg::PublishMode==1)   //模式1
  {
      BOOST_ASSERT(m_store1.size() > 0);   //确保已有能上传数据包的断言
      if (m_isVerbose)
        std::cerr << "Interest: " << interest << std::endl;

      const Name& name = interest.getName();   //获取兴趣包的命名
      shared_ptr<Data> data;

      //对兴趣包分块号的检索
      if (name.size() == m_versionedPrefix.size() + 1 && m_versionedPrefix.isPrefixOf(name) &&
          name[-1].isSegment()) {
        const auto segmentNo = static_cast<size_t>(interest.getName()[-1].toSegment());
        if (segmentNo < m_store1.size()) {
          data = m_store1[segmentNo];   //将缓存的相应分块赋给data
        }

        //记录时间
        struct timeval tv;
        gettimeofday(&tv,NULL);
        struct tm* pTime;
        pTime = localtime(&tv.tv_sec);
        char sTemp[30] = {0};
        snprintf(sTemp, sizeof(sTemp), "%04d-%02d-%02d %02d:%02d:%02d %03d:%03d", pTime->tm_year+1900, \
               pTime->tm_mon+1, pTime->tm_mday, pTime->tm_hour, pTime->tm_min, pTime->tm_sec, \
               tv.tv_usec/1000,tv.tv_usec%1000);
        log2<<std::left<<setw(10)<<segmentNo<<std::left<<setw(30)<<(string)sTemp;
      }
      else if (interest.matchesData(*m_store1[0])) {
        //兴趣包在寻找第一个分块或兴趣包没有版本编码
        data = m_store1[0];   //将缓存的第一个分块赋给data

        //记录时间
        struct timeval tv;
        gettimeofday(&tv,NULL);
        struct tm* pTime;
        pTime = localtime(&tv.tv_sec);
        char sTemp[30] = {0};
        snprintf(sTemp, sizeof(sTemp), "%04d-%02d-%02d %02d:%02d:%02d %03d:%03d", pTime->tm_year+1900, \
               pTime->tm_mon+1, pTime->tm_mday, pTime->tm_hour, pTime->tm_min, pTime->tm_sec, \
               tv.tv_usec/1000,tv.tv_usec%1000);
        log2<<std::left<<setw(10)<<"0"<<std::left<<setw(30)<<(string)sTemp;
      }

      if (data != nullptr) {
        if (m_isVerbose)
          std::cerr << "Data: " << *data << std::endl;

        m_face.put(*data);   //将data上传至抽象接口

        //记录时间
        struct timeval tv;
        gettimeofday(&tv,NULL);
        struct tm* pTime;
        pTime = localtime(&tv.tv_sec);
        char sTemp[30] = {0};
        snprintf(sTemp, sizeof(sTemp), "%04d-%02d-%02d %02d:%02d:%02d %03d:%03d", pTime->tm_year+1900, \
               pTime->tm_mon+1, pTime->tm_mday, pTime->tm_hour, pTime->tm_min, pTime->tm_sec, \
               tv.tv_usec/1000,tv.tv_usec%1000);
        log2<<std::left<<setw(30)<<(string)sTemp<<endl;
      }
      outfile2.close();
  }
  else if (Filedlg::PublishMode==2)   //模式2
  {
      BOOST_ASSERT(m_store.size() > 0);   //确保已有能上传数据包的断言
      if (m_isVerbose)
         std::cerr << "Interest: " << interest << std::endl;

      const Name& name = interest.getName();   //获取兴趣包命名
      shared_ptr<Data> data;
      //对兴趣包分块号的检索
      if (name.size() == m_versionedPrefix.size() + 1 && m_versionedPrefix.isPrefixOf(name) &&
         name[-1].isSegment()) {
         const auto segmentNo = static_cast<size_t>(interest.getName()[-1].toSegment());
         if (segmentNo==UploadPacketNo) {
            UploadPacketOnInterest();
           if (IsFileEnd)
             data = m_store[2];   //将缓存的相应分块赋给data
           else
             data = m_store[1];
         }
      cout<<"segmentNo:"<<segmentNo<<"UploadPacketNo"<<UploadPacketNo<<endl;
      }
      else if (interest.matchesData(*m_store[0])) {
         //兴趣包在寻找第一个分块或兴趣包没有版本编码
         data = m_store[0];   //将缓存的第一个分块赋给data
      }

      if (data != nullptr) {
        if (m_isVerbose)
           std::cerr << "Data: " << *data << std::endl;
         m_face.put(*data);   //将data上传至抽象接口
         SegmentNoForTimer++;   //用于显示速度的分块计数加一
         if (IsFirstInterest==true)   //避免对显示的第一个再次计数
            IsFirstInterest=false;
         else
            PublishedSegment++;   //已上传的分快数加一
       cout<<data->getName().toUri()<<endl;
      }
  }
}

void Publisher::UploadFirstPacket()
{
  BOOST_ASSERT(m_store.size()==3);   //如果表达式值为true，那么断言成立，程序会继续向下执行，否则断言会引发一个异常，在终端上输出调试信息并终止程序的执行
  IsFirstInterest=false;

  if (m_isVerbose)
    std::cerr << "Loading input ..." << std::endl;

  std::vector<uint8_t> buffer(m_maxSegmentSize);   //m_maxSegmentSize=4400
  while (InFileStream.good()) {
    InFileStream.read(reinterpret_cast<char*>(buffer.data()), buffer.size());     //把输入的is参数换一下即可
    const auto nCharsRead = InFileStream.gcount();
    if (nCharsRead > 0) {
      auto data = make_shared<Data>(Name(m_versionedPrefix).appendSegment(0));   //-1?
      data->setFreshnessPeriod(m_freshnessPeriod);
      data->setContent(&buffer[0], nCharsRead);
      cout<<"Finish: "<<data->getName()[-1].toSegment()<<"  "<<data->getName().toUri()<<endl;
      m_store[0]=data;

      PublishedSegment++;
      IsFirstInterest=true;
    break;
    }
  }

  m_keyChain.sign(*m_store[0], m_signingInfo);

  if (m_isVerbose)
    std::cerr << "Created " << m_store.size() << " chunks for prefix " << m_prefix << std::endl;
}

void Publisher::UploadPacketOnInterest()
{
    std::vector<uint8_t> buffer(m_maxSegmentSize);
    if (InFileStream.good()) {
       InFileStream.read(reinterpret_cast<char*>(buffer.data()), buffer.size());     //把输入的is参数换一下即可
       const auto nCharsRead = InFileStream.gcount();
       if (nCharsRead > 0) {
             auto data = make_shared<Data>(Name(m_versionedPrefix).appendSegment(UploadPacketNo));   //-1?
             data->setFreshnessPeriod(m_freshnessPeriod);
             data->setContent(&buffer[0], nCharsRead);
             cout<<"Finish: "<<data->getName()[-1].toSegment()<<"  "<<data->getName().toUri()<<endl;
             m_store[1]=data;
             m_keyChain.sign(*m_store[1], m_signingInfo);
             UploadPacketNo++;
        }
    }
    else {   //最后多了一个空包,包含文件结尾信息
        IsFileEnd=true;
        auto finalBlockId = name::Component::fromSegment(UploadPacketNo);
        auto data = make_shared<Data>(Name(m_versionedPrefix).appendSegment(UploadPacketNo));
        data->setFreshnessPeriod(m_freshnessPeriod);
        data->setFinalBlockId(finalBlockId);
        m_store[2]=data;
        m_keyChain.sign(*m_store[2], m_signingInfo);
        cout<<"finalBlockId: "<<finalBlockId<<"  "<<data->getName().toUri()<<endl;
    }
}

void Publisher::onRegisterFailed(const Name& prefix, const std::string& reason)
{
  std::cerr << "ERROR: Failed to register prefix '"
            << prefix << "' (" << reason << ")" << std::endl;
  m_face.shutdown();
}

void Publisher::SegmentFile()
{
    WholeFileSegmentNo=0;
    PublishedSegment=0;

    std::vector<uint8_t> buffer(8800>>1);   //m_maxSegmentSize=4400
    fstream InFileStream(string((const char *)Filedlg::Input_File_Path.toLocal8Bit()),ios::in|ios::binary);
    while (InFileStream.good()) {
      InFileStream.read(reinterpret_cast<char*>(buffer.data()), buffer.size());     //把输入的is参数换一下即可
      const auto nCharsRead = InFileStream.gcount();
      if (nCharsRead > 0) {
        WholeFileSegmentNo++;
      }
    }
}

void Publisher::StoreWholeFile(std::istream& is)
{
  BOOST_ASSERT(m_store1.size() == 0);

  if (m_isVerbose)
    std::cerr << "Loading input ..." << std::endl;

  std::vector<uint8_t> buffer(m_maxSegmentSize);
  while (is.good()) {
    is.read(reinterpret_cast<char*>(buffer.data()), buffer.size());
    const auto nCharsRead = is.gcount();
    if (nCharsRead > 0) {
      auto data = make_shared<Data>(Name(m_versionedPrefix).appendSegment(m_store1.size()));
      data->setFreshnessPeriod(m_freshnessPeriod);
      data->setContent(&buffer[0], nCharsRead);
qDebug()<<"appendSegment   "<<m_store1.size();


      data->setFinalBlockId(name::Component::fromSegment(WholeFileSegmentNo-1));
      m_keyChain.sign(*data, m_signingInfo);


      m_store1.push_back(data);

      SegmentNoForTimer++;
      PublishedSegment++;
    }
  }

  if (m_store1.empty()) {
    auto data = make_shared<Data>(Name(m_versionedPrefix).appendSegment(0));
    data->setFreshnessPeriod(m_freshnessPeriod);

    data->setFinalBlockId(name::Component::fromSegment(WholeFileSegmentNo-1));
    m_keyChain.sign(*data, m_signingInfo);

    m_store1.push_back(data);
  }

/*
  auto finalBlockId = name::Component::fromSegment(m_store1.size() - 1);
  for (const auto& data : m_store1) {
    data->setFinalBlockId(finalBlockId);
    m_keyChain.sign(*data, m_signingInfo);
  }*/

  if (m_isVerbose)
    std::cerr << "Created " << m_store1.size() << " chunks for prefix " << m_prefix << std::endl;
}
} // namespace segment
} // namespace ndn
