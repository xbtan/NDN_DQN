#ifndef NDN_SEGMENT_PUBLISHER_HPP
#define NDN_SEGMENT_PUBLISHER_HPP

#include <fstream>
#include <iostream>
#include "common.hpp"
#include "filedlg.h"
#include "ui_filedlg.h"

#include<iomanip>
#include <iostream>
#include <string>
#include <stdio.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>

using namespace std;
using std::ofstream;
using std::ifstream;

namespace ndn {
namespace segment {

class Publisher : noncopyable
{
public:
  Publisher(const Name& prefix, Face& face, KeyChain& keyChain,
           const security::SigningInfo& signingInfo, time::milliseconds freshnessPeriod,
           size_t maxSegmentSize, bool isVerbose,
           ifstream& is);

  void run();
  static void SegmentFile();

  static uint64_t SegmentNoForTimer;
  static uint64_t PublishedSegment;
  static uint64_t WholeFileSegmentNo;
  std::vector<shared_ptr<Data>> m_store;   //模式2
  std::vector<shared_ptr<Data>> m_store1;   //模式1

private:
  void UploadFirstPacket();
  void UploadPacketOnInterest();
  void onInterest(const Interest& interest);
  void onRegisterFailed(const Name& prefix, const std::string& reason);
  void StoreWholeFile(std::istream& is);

  Name m_prefix;
  Name m_versionedPrefix;
  Face& m_face;
  KeyChain& m_keyChain;
  security::SigningInfo m_signingInfo;
  time::milliseconds m_freshnessPeriod;
  size_t m_maxSegmentSize;
  bool m_isVerbose;
  ifstream& InFileStream;
  uint64_t UploadPacketNo;
  bool IsFileEnd;
  bool IsFirstInterest;
};

}   //namespace segment
}   //namespace ndn

#endif   //NDN_SEGMENT_PUBLISHER_HPP
