#include "pipeline-interests.hpp"
#include <iostream>
#include "filedlg1.h"

using namespace std;

namespace ndn {
namespace chunks {

PipelineInterests::PipelineInterests(Face& face)
  : m_face(face)
  , m_lastSegmentNo(0)
  , m_excludedSegmentNo(0)
  , m_hasFinalBlockId(false)   //false
{
    string str_outfile;
    str_outfile=string((const char *)Filedlg1::Output_File_Path.toLocal8Bit())+"/"+string((const char *)Filedlg1::Output_File_Name.toLocal8Bit())+"_log.txt";
    outfile.open(str_outfile,ios::out|ios::binary|ios::app);

    //outfile.close();   //最后没有的话，会显示文件为空（无论写入了多少数据）
}

PipelineInterests::~PipelineInterests() = default;

void
PipelineInterests::run(const Data& data, DataCallback onData, FailureCallback onFailure)   //运行
{cout<<"PipelineInterests::run"<<endl;
  BOOST_ASSERT(onData != nullptr);
  m_onData = std::move(onData);
  m_onFailure = std::move(onFailure);
  m_prefix = data.getName().getPrefix(-1);
  m_excludedSegmentNo = getSegmentFromPacket(data);


  if (!data.getFinalBlockId().empty()) {
    m_lastSegmentNo = data.getFinalBlockId().toSegment();
    m_hasFinalBlockId = true;
  }

  // record the start time of the pipeline
  m_startTime = time::steady_clock::now();   //记录开启时间

  doRun();
}

void
PipelineInterests::cancel()
{
    if (Filedlg1::DownLoadStatus==2)   //下载停止状态
        return;

    Filedlg1::DownLoadStatus=2;
    doCancel();
}

void
PipelineInterests::onFailure(const std::string& reason)   //失败
{//cout<<"PipelineInterests::onFailure"<<endl;
  if (Filedlg1::DownLoadStatus==2)
    return;

  cancel();

  if (m_onFailure)
    m_face.getIoService().post([this, reason] { m_onFailure(reason); });
}

} // namespace chunks
} // namespace ndn
