#include "pipeline-interests-fixed-window.hpp"
#include <chrono>
#include <time.h>
#include <thread>
#include <qtimer.h>
#include <qdebug.h>
#include <iostream>
#include "data-fetcher.hpp"
#include "filedlg1.h"
using namespace std;

namespace ndn {
namespace chunks {

PipelineInterestsFixedWindow::PipelineInterestsFixedWindow(Face& face, const Options& options)
  : PipelineInterests(face)
  , m_options(options)
  , m_nextSegmentNo(Filedlg1::BeginSegmentNo)
  , m_hasFailure(false)
{
  m_segmentFetchers.resize(m_options.maxPipelineSize);

  time.start();
}

PipelineInterestsFixedWindow::~PipelineInterestsFixedWindow()
{
  cancel();
}

void
PipelineInterestsFixedWindow::doRun()
{//cout<<"PipelineInterestsFixedWindow::doRun"<<endl;

  // if the FinalBlockId is unknown, this could potentially request non-existent segments
  for (size_t nRequestedSegments = 0;   //维持maxPipelineSize个通道
       nRequestedSegments < m_options.maxPipelineSize;
       ++nRequestedSegments) {
    if (!fetchNextSegment(nRequestedSegments))   //nRequestedSegments表示通道号
      // all segments have been requested
      break;
  }
}

bool
PipelineInterestsFixedWindow::fetchNextSegment(std::size_t pipeNo)
{//cout<<"PipelineInterestsFixedWindow::fetchNextSegment"<<endl;
  if (Filedlg1::DownLoadStatus==2)   //下载状态2
      return false;

  if (m_hasFailure) {
    onFailure("Fetching terminated but no final segment number has been found");
    return false;
  }

  if (m_nextSegmentNo == m_excludedSegmentNo)   //下一个需要提取的分块号属于被过滤的
    m_nextSegmentNo++;   //下一个需要提取的分块号加一

  Filedlg1::TotalTime=time.elapsed();   //总耗时的计时

  if (m_hasFinalBlockId && m_nextSegmentNo > m_lastSegmentNo)   //下一个需要提取的分块号大于文件最大的分块号
   return false;

  if (Filedlg1::FinishSegmentNo>=0){
   if (m_hasFinalBlockId && m_nextSegmentNo > Filedlg1::FinishSegmentNo)
    return false;
  }

  // send interest for next segment
  if (m_options.isVerbose)
    std::cerr << "Requesting segment #" << m_nextSegmentNo << std::endl;

  while (Filedlg1::DownLoadStatus==1)   //下载状态1
  {
  }

  Interest interest(Name(m_prefix).appendSegment(m_nextSegmentNo));   //定义包含下一个分块号的兴趣包
  interest.setInterestLifetime(m_options.interestLifetime);   //设置兴趣包有效期
  interest.setMustBeFresh(m_options.mustBeFresh);
  interest.setMaxSuffixComponents(1);

  auto fetcher = DataFetcher::fetch(m_face, interest,
                                    m_options.maxRetriesOnTimeoutOrNack,
                                    m_options.maxRetriesOnTimeoutOrNack,
                                    bind(&PipelineInterestsFixedWindow::handleData, this, _1, _2, pipeNo),   //多个通道号
                                    bind(&PipelineInterestsFixedWindow::handleFail, this, _2, pipeNo),
                                    bind(&PipelineInterestsFixedWindow::handleFail, this, _2, pipeNo),
                                    m_options.isVerbose);

  BOOST_ASSERT(!m_segmentFetchers[pipeNo].first || !m_segmentFetchers[pipeNo].first->isRunning());   //一直到没有运行
  m_segmentFetchers[pipeNo] = make_pair(fetcher, m_nextSegmentNo);   //将提取变量和通道号配对
  m_nextSegmentNo++;   //下一个需要提取的文件分块号加一

  return true;
}

void
PipelineInterestsFixedWindow::doCancel()
{
  for (auto& fetcher : m_segmentFetchers) {
    if (fetcher.first)
      fetcher.first->cancel();
  }

  m_segmentFetchers.clear();
}

void
PipelineInterestsFixedWindow::handleData(const Interest& interest, const Data& data, size_t pipeNo)
{//cout<<"PipelineInterestsFixedWindow::handleData"<<endl;
  if (Filedlg1::DownLoadStatus==2)   //下载状态2
    return;

  BOOST_ASSERT(data.getName().equals(interest.getName()));   //保证数据包和兴趣包的命名相同

  if (m_options.isVerbose)
    std::cerr << "Received segment #" << getSegmentFromPacket(data) << std::endl;

  onData(interest, data);   //处理数据包

  if (!m_hasFinalBlockId && !data.getFinalBlockId().empty()) {   //获取文件最大的分块号
    m_lastSegmentNo = data.getFinalBlockId().toSegment();
    m_hasFinalBlockId = true;

    for (auto& fetcher : m_segmentFetchers) {
      if (fetcher.first == nullptr)
        continue;

      if (fetcher.second > m_lastSegmentNo) {
        // stop trying to fetch segments that are beyond m_lastSegmentNo
        fetcher.first->cancel();
      }
      else if (fetcher.first->hasError()) { // fetcher.second <= m_lastSegmentNo
        // there was an error while fetching a segment that is part of the content
        return onFailure("Failure retrieving segment #" + to_string(fetcher.second));
      }
    }
  }
  fetchNextSegment(pipeNo);   //保持在当前的通道号上，提取下一个文件分块
}

void PipelineInterestsFixedWindow::handleFail(const std::string& reason, std::size_t pipeNo)
{//cout<<"PipelineInterestsFixedWindow::handleFail"<<endl;
  if (Filedlg1::DownLoadStatus==2)
    return;

  // if the failed segment is definitely part of the content, raise a fatal error
  if (m_hasFinalBlockId && m_segmentFetchers[pipeNo].second <= m_lastSegmentNo)
    return onFailure(reason);

  if (!m_hasFinalBlockId) {
    bool areAllFetchersStopped = true;
    for (auto& fetcher : m_segmentFetchers) {
      if (fetcher.first == nullptr)
        continue;

      // cancel fetching all segments that follow
      if (fetcher.second > m_segmentFetchers[pipeNo].second) {
        fetcher.first->cancel();
      }
      else if (fetcher.first->isRunning()) { // fetcher.second <= m_segmentFetchers[pipeNo].second
        areAllFetchersStopped = false;
      }
    }

    if (areAllFetchersStopped) {
      onFailure("Fetching terminated but no final segment number has been found");
    }
    else {
      m_hasFailure = true;
    }
  }
}

} // namespace chunks
} // namespace ndn
