#include "consumer.hpp"
#include "pipeline-interests-aimd.hpp"
#include "filedlg1.h"

#include <cmath>

namespace ndn {
namespace chunks {
namespace aimd {

PipelineInterestsAimd::PipelineInterestsAimd(Face& face, RttEstimator& rttEstimator,
                                             const Options& options)
  : PipelineInterests(face)
  , m_options(options)
  , m_rttEstimator(rttEstimator)
  , m_scheduler(m_face.getIoService())
  , DQN_Scheduler(m_face.getIoService())
  , BBR_Scheduler(m_face.getIoService())
  , m_checkRtoEvent(m_scheduler)
  , DQN_Event(DQN_Scheduler)
  , BBR_Event(BBR_Scheduler)
  , m_nextSegmentNo(0)
  , m_receivedSize(0)
  , m_highData(0)
  , m_highInterest(0)
  , m_recPoint(0)
  , m_nInFlight(0)
  , m_nReceived(0)
  , m_nLossEvents(0)
  , m_nRetransmitted(0)
  , m_cwnd(m_options.initCwnd)
  , m_ssthresh(m_options.initSsthresh)
  , m_hasFailure(false)
  , m_failedSegNo(0)
  , m_log(outfile)
  , SentInterestHighNo(0)
  , DeliveredData(0)
  , LossData(0)
  , SentData(0)
  , SegmentDeliveryRate(0)
  , DQN_State(1)    //
  , EpisodeDone(0)
  , EpisodeDoneUsed(0)
  , EpisodeDoneCount(0)
  , TrainsCount(0)
  , StepDoneCount(0)
  , StepDone(0)
  , StepDoneUsed(0)
  , StepStateUsed(0)
  , StepLossData(0)
  , LossDataLast(0)
  , Action1(0)
  , BBRState(0)
  , cwnd_Last(0)
  , FileEnd(0)
  , EpisodeCount(0)
  , EpisodeStepCount(0)
{
    if (m_options.isVerbose)
    {
        std::cerr << m_options;
    }

    time.start();

    QDateTime DateTime;
    QString Timestr=DateTime.currentDateTime().toString("yyyy-MM-dd hh:mm:ss");   //设置显示格式
    m_log<<"Time:"<<string((const char *)Timestr.toLocal8Bit())<<"\n";
    m_log<<"Test Name:DQN Test"<<"\n";
    m_log<<"Topology:local"<<"\n\n";
    m_log<<"Data Log"<<"\n";

    m_log<<std::left<<setw(20)<<"cwnd";
    m_log<<std::left<<setw(20)<<"m_nInFlight";
    m_log<<std::left<<setw(20)<<"DeliveredData";
    m_log<<std::left<<setw(20)<<"LossData";   //punish
    m_log<<std::left<<setw(20)<<"RTT";
    m_log<<std::left<<setw(20)<<"DeliveryRate"<<"\n";   //rwd
}

PipelineInterestsAimd::~PipelineInterestsAimd()
{
    cancel();
}

void
PipelineInterestsAimd::doRun()
{cout<<"doRun"<<endl;
    // count the excluded segment
    m_nReceived++;

    // schedule the event to check retransmission timer
    m_checkRtoEvent = m_scheduler.scheduleEvent(m_options.rtoCheckInterval, [this] { checkRto(); });

    if (DQN_State==2)
    {
        BBR_Event=BBR_Scheduler.scheduleEvent(BBR_CheckInterval,[this]{Check_BBR();});
        BBRCheckTimeLast=time::steady_clock::now();
    }
    /*
    else if (DQN_State==3||DQN_State==4)
    {
        DQN_Event=DQN_Scheduler.scheduleEvent(DQN_CheckIntervalInitial,[this]{Check_DQN();});
        StepStartTime=time::steady_clock::now();
    }
    */
    StepStartTime=time::steady_clock::now();

    system("rm -f ../DownLoad/*");
    system("rm -f ../UpLoad/*");

    schedulePackets();
}

void
PipelineInterestsAimd::doCancel()
{
    for (const auto& entry : m_segmentInfo)
    {
        m_face.removePendingInterest(entry.second.interestId);
    }

    m_face.removeAllPendingInterests();

    m_checkRtoEvent.cancel();
    DQN_Event.cancel();
    BBR_Event.cancel();

    m_segmentInfo.clear();
    m_retxCount.clear();
    while (!m_retxQueue.empty())
    {
        m_retxQueue.pop();
    }

    //m_face.shutdown();
}

void
PipelineInterestsAimd::Check_DQN()
{/*
    if (DQN_State==1)
    {
        if (StepDone==0)
        {
            StepDone=1;
        }
    }
    else if (DQN_State==3)   //Train on line
    {
        if (StepDone==0&&StepDoneUsed==0)
        {
            StepDone=1;

            /*
           //记步
           if (EpisodeDone==0&&EpisodeDoneUsed==0)
           {
               if (EpisodeDoneCount==0)
               {
                   EpisodeStartTime=time::steady_clock::now();//记录开始时间
               }
               else if (EpisodeDoneCount>=25)   //episode count
               {
                   EpisodeDone=1;
                   EpisodeTime=time::steady_clock::now()-EpisodeStartTime;
               }
               EpisodeDoneCount++;
           }*/
/*        }
    }
    else if (DQN_State==4)
    {
        if (StepDone==0&&StepDoneUsed==0)
        {
            StepDone=1;
        }
    }

    DQN_Event=DQN_Scheduler.scheduleEvent(DQN_CheckInterval,[this]{Check_DQN();});

    schedulePackets();
*/

    //sleep(10);
    doCancel();
    //m_face.processEvents();
/*
    int i=1;
    while (i<15000)
    {
        i++;
    }*/

    StepDoneUsed=0;
    StepDone=0;
    StepDoneCount=0;
    StepStateUsed=0;
    StepLossData=0;
    StepRttSum=0;
    StepRttCount=0;
    StepDataCount=0;
    cwnd_Last=1;
    m_cwnd=1;
    m_nInFlight=0;
    EpisodeDone=0;
    DQN_CheckInterval=DQN_CheckIntervalInitial;
    EpisodeDoneCount=0;
    EpisodeDoneUsed=0;

     TrainsCount++;
     m_nextSegmentNo=0;
     m_receivedSize=0;
     m_highData=0;
     m_highInterest=0;
     m_recPoint=0;
     m_nInFlight=0;
     m_nReceived=1;
     m_nLossEvents=0;
     m_nRetransmitted=0;
     m_cwnd=m_options.initCwnd;
     m_ssthresh=m_options.initSsthresh;
     m_hasFailure=false;
     m_failedSegNo=0;
     SentInterestHighNo=0;
     DeliveredData=0;
     LossData=0;
     SentData=0;
     SegmentDeliveryRate=0;
     EpisodeCount=0;
     //DQN_State=3;
     //EpisodeDone=0;
     //EpisodeDoneUsed=0;
     //EpisodeDoneCount=0;
     ndn::chunks::Consumer::ReceivedSegment=0;
     ndn::chunks::Consumer::SegmentNoForTimer=0;

     m_rttEstimator.RTOInit();

     doRun();
     FileEnd=0;
cout<<"FileEnd::::"<<FileEnd<<endl;
}

void
PipelineInterestsAimd::Check_BBR()
{
    BBRCheckIntervalexpense=time::steady_clock::now()-BBRCheckTimeLast;
    BBRCheckIntervalexpense_ms=boost::chrono::duration_cast<boost::chrono::milliseconds>(BBRCheckIntervalexpense);

    double DeliveryRate=double(std::min(SentDataBBRInterval,DeliveredDataBBRInterval))/double(BBRCheckIntervalexpense_ms.count());
    //double DeliveryRate=double(DeliveredDataBBRInterval)/double(std::max(m_cwnd,double(SentDataBBRInterval)));

    float CheckIntervalexpense=BBRCheckIntervalexpense_ms.count();

    ofstream outfile;
    string str_outfile;
    str_outfile="../DownLoad/BBR_test.txt";
    outfile.open(str_outfile,ios::out|ios::binary|ios::app);
    std::ostream& log(outfile);
    log<<std::left<<setw(10)<<m_cwnd<<std::left<<setw(15)<<DeliveryRate<<std::left<<setw(10)<<SentDataBBRInterval<<std::left<<setw(10)<<DeliveredDataBBRInterval<<std::left<<setw(10)<<CheckIntervalexpense<<endl;
    outfile.close();

    if (BBRState==0)
    {
        if (DeliveryRateQueueCount<=1)
        {
            DeliveryRateQueue[DeliveryRateQueueCount]=DeliveryRate;
            DeliveryRateQueueCount++;
        }
        else
        {
            DeliveryRateQueue[0]=DeliveryRateQueue[1];
            DeliveryRateQueue[1]=DeliveryRateQueue[2];
            DeliveryRateQueue[2]=DeliveryRate;

            if ((DeliveryRateQueue[1]<DeliveryRateQueue[0]&&DeliveryRateQueue[2]<DeliveryRateQueue[1])||m_cwnd>=200)
            {
                m_cwnd=m_cwnd/3;
                BBRState=1;
            }
        }
    }
    if (BBRState==1&&m_nInFlight<=m_cwnd)
    {
        BBRState=2;
        BBRBwSampleGainCount=0;
    }
    if (BBRState==2)
    {
        if (BBRBwSampleGainCount==0)
        {
            Cwnd_Temp=m_cwnd;
            DeliveredRateTemp=DeliveryRate;

            m_cwnd=m_cwnd*BBRBwSampleGain[BBRBwSampleGainCount];
            BBRBwSampleGainCount++;
        }
        else if (BBRBwSampleGainCount==1)
        {
            DeliveryRateSample=DeliveryRate;

            m_cwnd=Cwnd_Temp*BBRBwSampleGain[BBRBwSampleGainCount];
            BBRBwSampleGainCount++;
        }
        else
        {
            float ChangeRatio=(double(DeliveryRateSample)/double(DeliveredRateTemp));
            if (ChangeRatio>1.25)
            {
                ChangeRatio=1.25;
            }
            else if (ChangeRatio<0.75)
            {
                ChangeRatio=0.75;
            }

            m_cwnd=Cwnd_Temp*ChangeRatio;

            if (m_cwnd<30)    //4
            {
                m_cwnd=30;
            }
            else if (m_cwnd>60)    //200
            {
                m_cwnd=60;
            }

            BBRBwSampleGainCount++;
            if (BBRBwSampleGainCount==8)
            {
                BBRBwSampleGainCount=0;
            }
        }
    }

    if (m_cwnd<4)    //4
    {
        m_cwnd=4;
    }
    else if (m_cwnd>150)    //200
    {
        m_cwnd=150;
    }

    DeliveredDataBBRInterval=0;
    SentDataBBRInterval=0;
    BBRCheckTimeLast=time::steady_clock::now();

    BBR_Event=BBR_Scheduler.scheduleEvent(BBR_CheckInterval,[this]{Check_BBR();});

    schedulePackets();
}

void
PipelineInterestsAimd::checkRto()
{
  if (Filedlg1::DownLoadStatus==2)
    return;

  RtoInFlightTimeoutSegment=0;   //reset the timeout segment number

  bool hasTimeout = false;

  for (auto& entry : m_segmentInfo) {
    SegmentInfo& segInfo = entry.second;
    if (segInfo.state != SegmentState::InRetxQueue && // do not check segments currently in the retx queue
        segInfo.state != SegmentState::RetxReceived) { // or already-received retransmitted segments
      Milliseconds timeElapsed = time::steady_clock::now() - segInfo.timeSent;
      if (timeElapsed.count() > segInfo.rto.count()) {
      //if (timeElapsed.count() > 300000) {
        hasTimeout = true;
        enqueueForRetransmission(entry.first);
        RtoInFlightTimeoutSegment++;   //count the timeout segment number in the RTT
        StepLossData++;
      }
    }
  }

  if (hasTimeout) {
    recordTimeout();
    schedulePackets();
  }
  // schedule the next check after predefined interval
  m_checkRtoEvent = m_scheduler.scheduleEvent(m_options.rtoCheckInterval, [this] { checkRto(); });   //这个间隔与rto无关，主要是定时处理需要重传的,实际间隔和处理速度有关，在设定的周围，跟测试点也有关                                                                                                  //时间很短，基本保证超时的数据包就被立即处理
}

void
PipelineInterestsAimd::sendInterest(uint64_t segNo, bool isRetransmission)
{
  if (Filedlg1::DownLoadStatus==2)
    return;

  while (Filedlg1::DownLoadStatus==1)
  {
     outfile.close();
  }

  if (segNo>SentInterestHighNo)   //Update the highest interest number has been delivered
      SentInterestHighNo=segNo;

  if (m_hasFinalBlockId && segNo > m_lastSegmentNo && !isRetransmission)
    return;

  if (!isRetransmission && m_hasFailure)
    return;

  if (m_options.isVerbose) {
    std::cerr << (isRetransmission ? "Retransmitting" : "Requesting")
              << " segment #" << segNo << std::endl;
  }

  if (isRetransmission) {
    // keep track of retx count for this segment
    auto ret = m_retxCount.emplace(segNo, 1);
    if (ret.second == false) { // not the first retransmission
      m_retxCount[segNo] += 1;
      if (m_retxCount[segNo] > m_options.maxRetriesOnTimeoutOrNack) {
        return handleFail(segNo, "Reached the maximum number of retries (" +
                          to_string(m_options.maxRetriesOnTimeoutOrNack) +
                          ") while retrieving segment #" + to_string(segNo));
      }

      if (m_options.isVerbose) {
        std::cerr << "# of retries for segment #" << segNo
                  << " is " << m_retxCount[segNo] << std::endl;
      }
    }

    m_face.removePendingInterest(m_segmentInfo[segNo].interestId);
  }

  Interest interest(Name(m_prefix).appendSegment(segNo));
  interest.setInterestLifetime(m_options.interestLifetime);
  //interest.setInterestLifetime(time::milliseconds(300000));

  interest.setMustBeFresh(m_options.mustBeFresh);
  interest.setMaxSuffixComponents(1);

  auto interestId = m_face.expressInterest(interest,
                                           bind(&PipelineInterestsAimd::handleData, this, _1, _2),
                                           bind(&PipelineInterestsAimd::handleNack, this, _1, _2),
                                           bind(&PipelineInterestsAimd::handleLifetimeExpiration,
                                                this, _1));

  ofstream outfile1;
  string str_outfile1;
  str_outfile1="../DownLoad/SendInterest_Log.txt";
  outfile1.open(str_outfile1,ios::out|ios::binary|ios::app);
  std::ostream& log1(outfile1);

  struct timeval tv;
  gettimeofday(&tv,NULL);
  struct tm* pTime;
  pTime = localtime(&tv.tv_sec);
  char sTemp[30] = {0};
  snprintf(sTemp, sizeof(sTemp), "%04d-%02d-%02d %02d:%02d:%02d %03d:%03d", pTime->tm_year+1900, \
         pTime->tm_mon+1, pTime->tm_mday, pTime->tm_hour, pTime->tm_min, pTime->tm_sec, \
         tv.tv_usec/1000,tv.tv_usec%1000);
  log1<<std::left<<setw(10)<<segNo<<std::left<<setw(30)<<(string)sTemp<<endl;

  outfile1.close();

  SentDataBBRInterval++;

  m_nInFlight++;

  if (isRetransmission) {
    SegmentInfo& segInfo = m_segmentInfo[segNo];
    segInfo.timeSent = time::steady_clock::now();
    segInfo.rto = m_rttEstimator.getEstimatedRto();
    segInfo.state = SegmentState::Retransmitted;
    segInfo.SendCount++;
    segInfo.SentCwnd=m_cwnd;
    segInfo.SentCwndAction=Action1;
    m_nRetransmitted++;
  }
  else {
    m_highInterest = segNo;
    m_segmentInfo[segNo] = {interestId,
                            time::steady_clock::now(),
                            m_rttEstimator.getEstimatedRto(),
                            SegmentState::FirstTimeSent,
                            0,
                            DeliveredData,
                            LossData,
                            SentData,
                            m_cwnd,
                            Action1};
  }

  SentData++;
}

void
PipelineInterestsAimd::schedulePackets()
{
  BOOST_ASSERT(m_nInFlight >= 0);

  //if (EpisodeDone!=2&&StepDone!=2)
  //{

  auto availableWindowSize = static_cast<int64_t>(m_cwnd) - m_nInFlight;
  while (availableWindowSize > 0) {
    if (!m_retxQueue.empty()) {
      uint64_t retxSegNo = m_retxQueue.front();
      m_retxQueue.pop();

      auto it = m_segmentInfo.find(retxSegNo);
      if (it == m_segmentInfo.end()) {
        continue;
      }
      // the segment is still in the map, it means that it needs to be retransmitted
      sendInterest(retxSegNo, true);
    }
    else {
      sendInterest(getNextSegmentNo(), false);
    }
    availableWindowSize--;
  }

  //}
}

void
PipelineInterestsAimd::handleData(const Interest& interest, const Data& data)
{
  if (Filedlg1::DownLoadStatus==2)
    return;

  DeliveredData++;

  // Data name will not have extra components because MaxSuffixComponents is set to 1
  BOOST_ASSERT(data.getName().equals(interest.getName()));

  if (!m_hasFinalBlockId && !data.getFinalBlockId().empty()) {
    m_lastSegmentNo = data.getFinalBlockId().toSegment();
    m_hasFinalBlockId = true;
    cancelInFlightSegmentsGreaterThan(m_lastSegmentNo);
    if (m_hasFailure && m_lastSegmentNo >= m_failedSegNo) {
      // previously failed segment is part of the content
      return onFailure(m_failureReason);
    } else {
      m_hasFailure = false;
    }
  }

  uint64_t recvSegNo = getSegmentFromPacket(data);
  SegmentInfo& segInfo = m_segmentInfo[recvSegNo];

  if (segInfo.state == SegmentState::RetxReceived) {
    m_segmentInfo.erase(recvSegNo);

    return; // ignore already-received segment
  }

  Milliseconds rtt = time::steady_clock::now() - segInfo.timeSent;
  if (m_options.isVerbose) {
    std::cerr << "Received segment #" << recvSegNo
              << ", rtt=" << rtt.count() << "ms"
              << ", rto=" << segInfo.rto.count() << "ms" << std::endl;
  }

  //cout<<"rtt:"<<rtt.count()<<"   rto:"<<segInfo.rto.count()<<endl;    //

  if (m_highData < recvSegNo) {
    m_highData = recvSegNo;
  }

  // for segments in retx queue, we must not decrement m_nInFlight
  // because it was already decremented when the segment timed out
  if (segInfo.state != SegmentState::InRetxQueue) {
    //m_nInFlight--;
      if (m_nInFlight>=1)
          m_nInFlight--;
  }

  m_nReceived++;

  DeliveredDataBBRInterval++;

  m_receivedSize += data.getContent().value_size();

  increaseWindow();

  onData(interest, data);

      //DQN数据记录
      SegmentDeliveredData=DeliveredData-segInfo.DeliveredDataSent;
      SegmentLossData=LossData-segInfo.LossDataSent;

      uint64_t SegmentSentData=SentData-segInfo.SentDataSent;
      //SegmentDeliveryRate=(double(std::min(SegmentDeliveredData,SegmentSentData))/double(SegmentRTT.count()));
      //m_log<<std::left<<setw(20)<<m_cwnd<<std::left<<setw(20)<<m_nInFlight<<std::left<<setw(20)<<SegmentDeliveredData<<std::left<<setw(20)<<SegmentLossData<<std::left<<setw(20)<<SegmentRTT.count()<<std::left<<setw(20)<<SegmentDeliveryRate<<"\n";

      struct timeval tv;
      gettimeofday(&tv,NULL);
      struct tm* pTime;
      pTime = localtime(&tv.tv_sec);
      char sTemp[30] = {0};
      snprintf(sTemp, sizeof(sTemp), "%04d-%02d-%02d %02d:%02d:%02d %03d:%03d", pTime->tm_year+1900, \
             pTime->tm_mon+1, pTime->tm_mday, pTime->tm_hour, pTime->tm_min, pTime->tm_sec, \
             tv.tv_usec/1000,tv.tv_usec%1000);
      //log3<<std::left<<setw(10)<<recvSegNo<<std::left<<setw(30)<<(string)sTemp<<std::left<<setw(10)<<m_cwnd<<std::left<<setw(10)<<m_nInFlight<<std::left<<setw(10)<<SegmentDeliveredData<<std::left<<setw(10)<<SegmentLossData<<std::left<<setw(15)<<SegmentRTT.count()<<std::left<<setw(15)<<SegmentDeliveryRate<<"\n";

      if (DQN_State==1)   //NDN_AIMD
      {
          //if (segInfo.state != SegmentState::FirstTimeSentsegInfo.state != SegmentState::FirstTimeSent)
          Milliseconds RttTemp=time::steady_clock::now()-segInfo.timeSent;
          float RttTemp1=RttTemp.count();
          if (segInfo.SendCount>=1)
          {
              RttTemp1=(segInfo.rto.count()/2)+RttTemp.count();
          }
          //float RttTemp1=(segInfo.SendCount*segInfo.rto.count())+RttTemp.count();

          //if (segInfo.state != SegmentState::FirstTimeSent &&
          //    segInfo.state != SegmentState::InRetxQueue)
          //{
          //    RttTemp1=RttTemp1Last;
          //}

          if (RttTemp1<=10 || RttTemp1>=200)    //排除重传的
          {
              RttTemp1=RttTemp1Last;
          }

          RttTemp1Last=RttTemp1;
/*
          if (RttTemp1<10)    //排除重传的
          {
              RttTemp1=10;
          }
          else if (RttTemp1>100)    //排除失效的
          {
              RttTemp1=100;
          }
*/
          ofstream outfile3;
          string str_outfile3;
          str_outfile3="../DownLoad/OnData_AIMD.txt";
          outfile3.open(str_outfile3,ios::out|ios::binary|ios::app);
          std::ostream& log3(outfile3);
          log3<<m_cwnd<<"   "<<RttTemp1<<"   "<<AIMDLossData<<endl;
          LossDataSum+=AIMDLossData;
          cout<<ceil(m_cwnd)<<endl;
          AIMDLossData=0;

          outfile3.close();

          if (StepDone==1)
          {
              StepDone=0;
          }

          EpisodeDoneCount++;
          if (EpisodeDoneCount>=400000)    //270000
          {
              while (1)
              {
              }
              EpisodeDoneCount=0;
              m_cwnd=1;
              m_ssthresh=m_options.initSsthresh;
          }
      }
      else if (DQN_State==2)   //NDN_BBR
      {
          Milliseconds RttTemp=time::steady_clock::now()-segInfo.timeSent;
          float RttTemp1=RttTemp.count();

          if (segInfo.state != SegmentState::FirstTimeSent &&
              segInfo.state != SegmentState::InRetxQueue)
          {
              RttTemp1=RttTemp1Last;
          }
          if (RttTemp1<=10 || RttTemp1>=100)    //排除重传的
          {
              RttTemp1=RttTemp1Last;
          }

          RttTemp1Last=RttTemp1;

          //if (RttTemp1<10)    //排除重传的
          //{
          //    RttTemp1=10;
          //}
          //else if (RttTemp1>100)    //排除失效的
          //{
          //    RttTemp1=100;
          //}

          ofstream outfile3;
          string str_outfile3;
          str_outfile3="../DownLoad/OnData_BBR.txt";
          outfile3.open(str_outfile3,ios::out|ios::binary|ios::app);
          std::ostream& log3(outfile3);
          log3<<m_cwnd<<"   "<<RttTemp1<<"   "<<AIMDLossData<<endl;
          LossDataSum+=AIMDLossData;
          cout<<ceil(m_cwnd)<<endl;
          AIMDLossData=0;

          outfile3.close();

          EpisodeDoneCount++;
          if (EpisodeDoneCount>=400000)    //270000
          {
              while (1)
              {
              }
          }
      }
    else  if (DQN_State==3)   //Train on line
    {
if (EpisodeCount<=7)
{
          Milliseconds RttTemp=time::steady_clock::now()-segInfo.timeSent;
          float RttTemp1=RttTemp.count();

          if (segInfo.state != SegmentState::FirstTimeSent &&
              segInfo.state != SegmentState::InRetxQueue)
          {
              RttTemp1=RttTemp1Last;
          }
          if (RttTemp1<=10 || RttTemp1>=100)    //排除重传的
          {
              RttTemp1=RttTemp1Last;
          }

          RttTemp1Last=RttTemp1;

        if (StepDone==0)
        {
            if (segInfo.SentCwndAction==Action1&&segInfo.SentCwnd==m_cwnd)
            {
                RttTemp1=RttTemp1-10;
                //if (RttTemp1<=10)    //针对特殊错误，因为延时设定最低就是10
                //    RttTemp1=10.7;

                if ((segInfo.state == SegmentState::FirstTimeSent ||
                    segInfo.state == SegmentState::InRetxQueue)
                        && (StepDataCount>=cwnd_Last))
                {
                    ofstream outfile3;
                    string str_outfile3;
                    str_outfile3="../DownLoad/NQD_Log.txt";
                    outfile3.open(str_outfile3,ios::out|ios::binary|ios::app);
                    std::ostream& log3(outfile3);
                    log3<<m_cwnd<<"   "<<RttTemp1<<endl;
                    outfile3.close();

                    StepRttSum += RttTemp1;
                    StepRttCount++;
                }

                StepDataCount++;

                if (StepDataCount>=cwnd_Last+m_cwnd && StepRttCount>=5)
                {
                    StepDone=1;
                }
            }
        }
        if (StepDone==1) //if (StepStateUsed==0)
        {
            if (StepRttCount!=0)
            {
                StepRttMean=StepRttSum/StepRttCount;
            }

            //cout<<"StepRttSum:"<<StepRttSum<<"   StepRttCount:"<<StepRttCount<<"   StepRttMean:"<<StepRttMean<<endl;
        }

        if (EpisodeDone==0&&EpisodeDoneUsed==0)
        {
            if (EpisodeDoneCount==0)
            {
                EpisodeStartTime=time::steady_clock::now();//记录开始时间
            }
            else if (EpisodeDoneCount>=10000)   //episode count
            {
                EpisodeDone=1;
            }
            EpisodeDoneCount++;
        }

        if (EpisodeDone==2)    //drain    ||StepDone==2
        {
            if (m_nInFlight<=2)    //2
            {
                cwnd_Last=1;
                EpisodeDone=0;
                StepDone=0;
                StepStartTime=time::steady_clock::now();
                DQN_CheckInterval=DQN_CheckIntervalInitial;
                StepLossData=0;
                //StepTimeLast=time::steady_clock::now();
                //if (LossData>65000)
                //    LossData=0;
                //LossDataLast=LossData;

                //RTO初始化
                m_rttEstimator.RTOInit();
            }
        }

        if (StepDone==1)
        {

            StepExpense=time::steady_clock::now()-StepStartTime;
            StepExpense_ms=boost::chrono::duration_cast<boost::chrono::milliseconds>(StepExpense);

            if (StepStateUsed==0)
            {
                //StepTime=time::steady_clock::now()-StepTimeLast;
                //StepLossData=LossData-LossDataLast;

                StepStateUsed=1;
            }
            if (StepStateUsed==1)
            {
                if (StepDoneUsed==0)
                {
                    fstream file1;
                    string str_file1;
                    str_file1="../../Data/Train/Observation.txt";
                    file1.open(str_file1,ios::out|ios::binary|ios::app);
                    file1.seekg(0,std::ios::end);
                    if(file1.tellg()==0)
                    {
                        file1.close();
                        ofstream file2;
                        string str_file2;
                        str_file2="../../Data/Train/Observation.txt";
                        file2.open(str_file2,ios::out|ios::binary|ios::app);
                        std::ostream& log2(file2);
                        //log2<<m_cwnd<<"   "<<SegmentRTT.count()<<"   "<<StepLossData<<"   "<<m_nInFlight<<"   "<<Action1<<"   "<<StepTime.count()<<"   "<<EpisodeDone<<"   "<<StepTime.count();
                        EpisodeTime=time::steady_clock::now()-EpisodeStartTime;
                        log2<<m_cwnd<<"   "<<StepDataCount<<"   "<<StepRttMean<<"   "<<StepLossData<<"   "<<StepExpense_ms.count()<<"   "<<EpisodeDone<<"   "<<EpisodeDoneCount<<"   "<<EpisodeTime.count();

                        file2.close();
                        StepDoneUsed=1;

                        if (EpisodeDone==1&&EpisodeDoneUsed==0)
                        {
                            EpisodeDone=0;
                            EpisodeDoneUsed=1;
                        }
                    }
                }
                if (StepDoneUsed==1)
                {
                    fstream file3;
                    string str_file3;
                    str_file3="../../Data/Train/Action.txt";
                    file3.open(str_file3,ios::in|ios::out|ios::binary|ios::app);
                    file3.seekg(0,std::ios::end);
                    if(file3.tellg()!=0)
                    {
                        file3.close();

                        ifstream file5;
                        string str_file5;
                        str_file5="../../Data/Train/Action.txt";
                        file5.open(str_file5,ios::in|ios::binary|ios::app);
                        std::istream& log5(file5);
                        float num1=0,num2=0;
                        log5>>num1;
                        log5>>num2;
                        file5.close();

                        ofstream file4;   //Clear
                        string str_file4;
                        str_file4="../../Data/Train/Action.txt";
                        file4.open(str_file4);
                        std::ostream& log4(file4);
                        file4.close();

                        //int m_cwndTemp=m_cwnd;
                        int resultt=0;
                        resultt=num1;

                        cwnd_Last=m_cwnd;

/*                     m_cwnd=m_cwnd+1;    //测试时一直用这个策略
*/
                        if (resultt==1)
                            m_cwnd=m_cwnd*1.5;
                        else if (resultt==2)
                            m_cwnd=m_cwnd*1.25;
                        else if (resultt==3)
                            m_cwnd=m_cwnd+1;
                        else if (resultt==4)
                            m_cwnd=m_cwnd;
                        else if (resultt==5)
                            m_cwnd=m_cwnd*0.5;
                        else if (resultt==6)
                            m_cwnd=m_cwnd*0.75;
                        else if (resultt==7)
                            m_cwnd=m_cwnd-1;
                        m_cwnd=ceil(m_cwnd);    //向上取整
                        Action1=resultt;

                        if (m_cwnd>300)   //Limit
                        {
                            m_cwnd=300;
                        }
                        else if (m_cwnd<1)
                        {
                            m_cwnd=1;
                        }
/*
                        if (m_cwndTemp==1 && (resultt==1||resultt==3||resultt==5))
                        {
                            m_cwnd=2;
                        }
                        else if (m_cwndTemp==500 && (resultt==1||resultt==2||resultt==4))
                        {
                            m_cwnd=499;
                        }
*/
                        StepDoneUsed=0;
                        StepDone=0;
                        StepStartTime=time::steady_clock::now();
                        StepDoneCount=0;
                        StepStateUsed=0;
                        StepLossData=0;
                        StepRttSum=0;
                        StepRttCount=0;
                        StepDataCount=0;

                        if (EpisodeDone==0&&EpisodeDoneUsed==1&&num2==1)
                        {cout<<"****************"<<endl;
                            m_cwnd=1;
                            cwnd_Last=1;
                            EpisodeDoneCount=0;
                            EpisodeDoneUsed=0;
                            StepDone=2;
                            EpisodeDone=2;

                            EpisodeCount++;
                        }

                        cout<<"m_cwnd:"<<m_cwnd<<endl;
                    }
                }
            }
        }
    }
}
      else  if (DQN_State==4)   //Use on line
      {
          Milliseconds RttTemp=time::steady_clock::now()-segInfo.timeSent;
          float RttTemp1=RttTemp.count();

          if (segInfo.state != SegmentState::FirstTimeSent &&
              segInfo.state != SegmentState::InRetxQueue)
          {
              RttTemp1=RttTemp1Last;
          }
          if (RttTemp1<=10 || RttTemp1>=100)    //排除重传的
          {
              RttTemp1=RttTemp1Last;
          }

          ofstream outfile3;
          string str_outfile3;
          str_outfile3="../DownLoad/OnData_DQN.txt";
          outfile3.open(str_outfile3,ios::out|ios::binary|ios::app);
          std::ostream& log3(outfile3);
          log3<<m_cwnd<<"   "<<RttTemp1<<endl;
          outfile3.close();

          RttTemp1Last=RttTemp1;

          if (StepDone==0)
          {
              if (segInfo.SentCwndAction==Action1&&segInfo.SentCwnd==m_cwnd)
              {
                  RttTemp1=RttTemp1-10;
                  //if (RttTemp1<=10)    //针对特殊错误，因为延时设定最低就是10
                  //    RttTemp1=10.7;

                  if ((segInfo.state == SegmentState::FirstTimeSent ||
                      segInfo.state == SegmentState::InRetxQueue)
                          && (StepDataCount>=cwnd_Last))
                  {
                      StepRttSum += RttTemp1;
                      StepRttCount++;
                  }

                  StepDataCount++;

                  if (StepDataCount>=cwnd_Last+m_cwnd && StepRttCount>=5)
                  {
                      StepDone=1;
                  }
              }
          }
          if (StepDone==1) //if (StepStateUsed==0)
          {
              if (StepRttCount!=0)
              {
                  StepRttMean=StepRttSum/StepRttCount;
              }

              //cout<<"StepRttSum:"<<StepRttSum<<"   StepRttCount:"<<StepRttCount<<"   StepRttMean:"<<StepRttMean<<endl;
          }

          if (EpisodeDone==0&&EpisodeDoneUsed==0)
          {
              if (EpisodeDoneCount==0)
              {
                  EpisodeStartTime=time::steady_clock::now();//记录开始时间
              }
              else if (EpisodeDoneCount>=400000)   //270000
              {
                  while (1)
                  {
                  }


                  EpisodeDone=1;
              }
              EpisodeDoneCount++;
          }

          if (EpisodeDone==2)    //drain    ||StepDone==2
          {
              if (m_nInFlight<=2)    //2
              {
                  cwnd_Last=1;
                  EpisodeDone=0;
                  StepDone=0;
                  StepStartTime=time::steady_clock::now();
                  DQN_CheckInterval=DQN_CheckIntervalInitial;
                  StepLossData=0;
                  //StepTimeLast=time::steady_clock::now();
                  //if (LossData>65000)
                  //    LossData=0;
                  //LossDataLast=LossData;
              }
          }

          if (StepDone==1)
          {

              StepExpense=time::steady_clock::now()-StepStartTime;
              StepExpense_ms=boost::chrono::duration_cast<boost::chrono::milliseconds>(StepExpense);

              if (StepStateUsed==0)
              {
                  //StepTime=time::steady_clock::now()-StepTimeLast;
                  //StepLossData=LossData-LossDataLast;

                  StepStateUsed=1;
              }
              if (StepStateUsed==1)
              {
                  if (StepDoneUsed==0)
                  {
                      fstream file1;
                      string str_file1;
                      str_file1="../../Data/Train/Observation.txt";
                      file1.open(str_file1,ios::out|ios::binary|ios::app);
                      file1.seekg(0,std::ios::end);
                      if(file1.tellg()==0)
                      {
                          file1.close();
                          ofstream file2;
                          string str_file2;
                          str_file2="../../Data/Train/Observation.txt";
                          file2.open(str_file2,ios::out|ios::binary|ios::app);
                          std::ostream& log2(file2);
                          //log2<<m_cwnd<<"   "<<SegmentRTT.count()<<"   "<<StepLossData<<"   "<<m_nInFlight<<"   "<<Action1<<"   "<<StepTime.count()<<"   "<<EpisodeDone<<"   "<<StepTime.count();
                          EpisodeTime=time::steady_clock::now()-EpisodeStartTime;
                          log2<<m_cwnd<<"   "<<StepDataCount<<"   "<<StepRttMean<<"   "<<StepLossData<<"   "<<StepExpense_ms.count()<<"   "<<EpisodeDone<<"   "<<EpisodeDoneCount<<"   "<<EpisodeTime.count();

                          file2.close();
                          StepDoneUsed=1;

                          if (EpisodeDone==1&&EpisodeDoneUsed==0)
                          {
                              EpisodeDone=0;
                              EpisodeDoneUsed=1;
                          }
                      }
                  }
                  if (StepDoneUsed==1)
                  {
                      fstream file3;
                      string str_file3;
                      str_file3="../../Data/Train/Action.txt";
                      file3.open(str_file3,ios::in|ios::out|ios::binary|ios::app);
                      file3.seekg(0,std::ios::end);
                      if(file3.tellg()!=0)
                      {
                          file3.close();

                          ifstream file5;
                          string str_file5;
                          str_file5="../../Data/Train/Action.txt";
                          file5.open(str_file5,ios::in|ios::binary|ios::app);
                          std::istream& log5(file5);
                          float num1=0,num2=0;
                          log5>>num1;
                          log5>>num2;
                          file5.close();

                          ofstream file4;   //Clear
                          string str_file4;
                          str_file4="../../Data/Train/Action.txt";
                          file4.open(str_file4);
                          std::ostream& log4(file4);
                          file4.close();

                          //int m_cwndTemp=m_cwnd;
                          int resultt=0;
                          resultt=num1;

                          cwnd_Last=m_cwnd;

                          if (resultt==1)
                              m_cwnd=m_cwnd*1.5;
                          else if (resultt==2)
                              m_cwnd=m_cwnd*1.25;
                          else if (resultt==3)
                              m_cwnd=m_cwnd+1;
                          else if (resultt==4)
                              m_cwnd=m_cwnd;
                          else if (resultt==5)
                              m_cwnd=m_cwnd*0.5;
                          else if (resultt==6)
                              m_cwnd=m_cwnd*0.75;
                          else if (resultt==7)
                              m_cwnd=m_cwnd-1;
                          m_cwnd=ceil(m_cwnd);    //向上取整
                          Action1=resultt;

                          if (m_cwnd>300)   //Limit
                          {
                              m_cwnd=300;
                          }
                          else if (m_cwnd<1)
                          {
                              m_cwnd=1;
                          }
  /*
                          if (m_cwndTemp==1 && (resultt==1||resultt==3||resultt==5))
                          {
                              m_cwnd=2;
                          }
                          else if (m_cwndTemp==500 && (resultt==1||resultt==2||resultt==4))
                          {
                              m_cwnd=499;
                          }
  */
                          StepDoneUsed=0;
                          StepDone=0;
                          StepStartTime=time::steady_clock::now();
                          StepDoneCount=0;
                          StepStateUsed=0;
                          StepLossData=0;
                          StepRttSum=0;
                          StepRttCount=0;
                          StepDataCount=0;

                          if (EpisodeDone==0&&EpisodeDoneUsed==1&&num2==1)
                          {cout<<"****************"<<endl;
                              m_cwnd=1;
                              cwnd_Last=1;
                              EpisodeDoneCount=0;
                              EpisodeDoneUsed=0;
                              StepDone=2;
                              EpisodeDone=2;
                          }

                          cout<<"m_cwnd:"<<m_cwnd<<endl;
                      }
                  }
              }
          }
      }

      else  if (DQN_State==5)   //Use and train on line
      {
  if (EpisodeCount<=7)
  {
            Milliseconds RttTemp=time::steady_clock::now()-segInfo.timeSent;
            float RttTemp1=RttTemp.count();

            if (segInfo.state != SegmentState::FirstTimeSent &&
                segInfo.state != SegmentState::InRetxQueue)
            {
                RttTemp1=RttTemp1Last;
            }
            RttTemp1Last=RttTemp1;

          if (StepDone==0)
          {
              if (segInfo.SentCwndAction==Action1&&segInfo.SentCwnd==m_cwnd)
              {
                  //if (RttTemp1<10)    //排除重传的
                  //{
                  //    RttTemp1=10;
                  //}
                  //else if (RttTemp1>100)    //排除失效的
                  //{
                  //    RttTemp1=100;
                  //}

                  RttTemp1=RttTemp1-10;
                  //if (RttTemp1<=10)    //针对特殊错误，因为延时设定最低就是10
                  //    RttTemp1=10.7;

                  if ((segInfo.state == SegmentState::FirstTimeSent ||
                      segInfo.state == SegmentState::InRetxQueue)
                          && (StepDataCount>=cwnd_Last))
                  {
                      ofstream outfile3;
                      string str_outfile3;
                      str_outfile3="../DownLoad/NQD_Log.txt";
                      outfile3.open(str_outfile3,ios::out|ios::binary|ios::app);
                      std::ostream& log3(outfile3);
                      log3<<m_cwnd<<"   "<<RttTemp1<<endl;
                      outfile3.close();

                      StepRttSum += RttTemp1;
                      StepRttCount++;
                  }

                  StepDataCount++;

                  if (StepDataCount>=cwnd_Last+m_cwnd && StepRttCount>=10)
                  {
                      StepDone=1;
                      EpisodeStepCount++;
                  }
              }
          }
          if (StepDone==1) //if (StepStateUsed==0)
          {
              if (StepRttCount!=0)
              {
                  StepRttMean=StepRttSum/StepRttCount;
              }

              //cout<<"StepRttSum:"<<StepRttSum<<"   StepRttCount:"<<StepRttCount<<"   StepRttMean:"<<StepRttMean<<endl;
          }

          if (EpisodeDone==0&&EpisodeDoneUsed==0)
          {
              if (EpisodeDoneCount==0)
              {
                  EpisodeStartTime=time::steady_clock::now();//记录开始时间
              }
              else if (EpisodeDoneCount>=30000)   //episode count
              {
                  EpisodeDone=1;
              }

              if (EpisodeStepCount>200)
              {
                  //EpisodeDone=1;    //过200跳出
              }
              EpisodeDoneCount++;
          }

          if (EpisodeDone==2)    //drain    ||StepDone==2
          {
              if (m_nInFlight<=2)    //2
              {
                  cwnd_Last=1;
                  EpisodeDone=0;
                  StepDone=0;
                  StepStartTime=time::steady_clock::now();
                  DQN_CheckInterval=DQN_CheckIntervalInitial;
                  StepLossData=0;
                  //StepTimeLast=time::steady_clock::now();
                  //if (LossData>65000)
                  //    LossData=0;
                  //LossDataLast=LossData;

                  //RTO初始化
                  m_rttEstimator.RTOInit();
              }
          }

          if (StepDone==1)
          {

              StepExpense=time::steady_clock::now()-StepStartTime;
              StepExpense_ms=boost::chrono::duration_cast<boost::chrono::milliseconds>(StepExpense);

              if (StepStateUsed==0)
              {
                  //StepTime=time::steady_clock::now()-StepTimeLast;
                  //StepLossData=LossData-LossDataLast;

                  StepStateUsed=1;
              }
              if (StepStateUsed==1)
              {
                  if (StepDoneUsed==0)
                  {
                      fstream file1;
                      string str_file1;
                      str_file1="../../Data/Train/Observation.txt";
                      file1.open(str_file1,ios::out|ios::binary|ios::app);
                      file1.seekg(0,std::ios::end);
                      if(file1.tellg()==0)
                      {
                          file1.close();
                          ofstream file2;
                          string str_file2;
                          str_file2="../../Data/Train/Observation.txt";
                          file2.open(str_file2,ios::out|ios::binary|ios::app);
                          std::ostream& log2(file2);
                          //log2<<m_cwnd<<"   "<<SegmentRTT.count()<<"   "<<StepLossData<<"   "<<m_nInFlight<<"   "<<Action1<<"   "<<StepTime.count()<<"   "<<EpisodeDone<<"   "<<StepTime.count();
                          EpisodeTime=time::steady_clock::now()-EpisodeStartTime;
                          log2<<m_cwnd<<"   "<<StepDataCount<<"   "<<StepRttMean<<"   "<<StepLossData<<"   "<<StepExpense_ms.count()<<"   "<<EpisodeDone<<"   "<<EpisodeDoneCount<<"   "<<EpisodeTime.count();

                          file2.close();
                          StepDoneUsed=1;

                          if (EpisodeDone==1&&EpisodeDoneUsed==0)
                          {
                              EpisodeDone=0;
                              EpisodeDoneUsed=1;
                          }
                      }
                  }
                  if (StepDoneUsed==1)
                  {
                      fstream file3;
                      string str_file3;
                      str_file3="../../Data/Train/Action.txt";
                      file3.open(str_file3,ios::in|ios::out|ios::binary|ios::app);
                      file3.seekg(0,std::ios::end);
                      if(file3.tellg()!=0)
                      {
                          file3.close();

                          ifstream file5;
                          string str_file5;
                          str_file5="../../Data/Train/Action.txt";
                          file5.open(str_file5,ios::in|ios::binary|ios::app);
                          std::istream& log5(file5);
                          float num1=0,num2=0;
                          log5>>num1;
                          log5>>num2;
                          file5.close();

                          ofstream file4;   //Clear
                          string str_file4;
                          str_file4="../../Data/Train/Action.txt";
                          file4.open(str_file4);
                          std::ostream& log4(file4);
                          file4.close();

                          //int m_cwndTemp=m_cwnd;
                          int resultt=0;
                          resultt=num1;

                          cwnd_Last=m_cwnd;

  /*                     m_cwnd=m_cwnd+1;    //测试时一直用这个策略
  */
                          if (resultt==1)
                              m_cwnd=m_cwnd*1.5;
                          else if (resultt==2)
                              m_cwnd=m_cwnd*1.25;
                          else if (resultt==3)
                              m_cwnd=m_cwnd+1;
                          else if (resultt==4)
                              m_cwnd=m_cwnd;
                          else if (resultt==5)
                              m_cwnd=m_cwnd*0.5;
                          else if (resultt==6)
                              m_cwnd=m_cwnd*0.75;
                          else if (resultt==7)
                              m_cwnd=m_cwnd-1;
                          m_cwnd=ceil(m_cwnd);    //向上取整
                          Action1=resultt;

                          if (m_cwnd>300)   //Limit
                          {
                              m_cwnd=300;
                          }
                          else if (m_cwnd<1)
                          {
                              m_cwnd=1;
                          }
  /*
                          if (m_cwndTemp==1 && (resultt==1||resultt==3||resultt==5))
                          {
                              m_cwnd=2;
                          }
                          else if (m_cwndTemp==500 && (resultt==1||resultt==2||resultt==4))
                          {
                              m_cwnd=499;
                          }
  */
                          StepDoneUsed=0;
                          StepDone=0;
                          StepStartTime=time::steady_clock::now();
                          StepDoneCount=0;
                          StepStateUsed=0;
                          StepLossData=0;
                          StepRttSum=0;
                          StepRttCount=0;
                          StepDataCount=0;

                          if (EpisodeDone==0&&EpisodeDoneUsed==1&&num2==1)
                          {cout<<"****************"<<endl;
                              m_cwnd=1;
                              cwnd_Last=1;
                              EpisodeDoneCount=0;
                              EpisodeDoneUsed=0;
                              StepDone=2;
                              EpisodeDone=2;
                              EpisodeStepCount=0;
                              EpisodeCount++;
                          }

                          cout<<"m_cwnd:"<<m_cwnd<<endl;
                      }
                  }
              }
          }
      }
  }

else if (DQN_State==6)    //Train_DDPG
{
if (EpisodeCount<=7)
{
          Milliseconds RttTemp=time::steady_clock::now()-segInfo.timeSent;
          float RttTemp1=RttTemp.count();

          if (segInfo.state != SegmentState::FirstTimeSent &&
              segInfo.state != SegmentState::InRetxQueue)
          {
              RttTemp1=RttTemp1Last;
          }
          if (RttTemp1<=10 || RttTemp1>=100)    //排除重传的
          {
              RttTemp1=RttTemp1Last;
          }

          RttTemp1Last=RttTemp1;

        if (StepDone==0)
        {
            if (segInfo.SentCwndAction==Action1&&segInfo.SentCwnd==m_cwnd)
            {
                RttTemp1=RttTemp1-10;
                //if (RttTemp1<=10)    //针对特殊错误，因为延时设定最低就是10
                //    RttTemp1=10.7;

                if ((segInfo.state == SegmentState::FirstTimeSent ||
                    segInfo.state == SegmentState::InRetxQueue)
                        && (StepDataCount>=cwnd_Last))
                {
                    ofstream outfile3;
                    string str_outfile3;
                    str_outfile3="../DownLoad/NQD_Log.txt";
                    outfile3.open(str_outfile3,ios::out|ios::binary|ios::app);
                    std::ostream& log3(outfile3);
                    log3<<m_cwnd<<"   "<<RttTemp1<<endl;
                    outfile3.close();

                    StepRttSum += RttTemp1;
                    StepRttCount++;
                }

                StepDataCount++;

                if (StepDataCount>=cwnd_Last+m_cwnd && StepRttCount>=5)
                {
                    StepDone=1;
                }
            }
        }
        if (StepDone==1) //if (StepStateUsed==0)
        {
            if (StepRttCount!=0)
            {
                StepRttMean=StepRttSum/StepRttCount;
            }

            //cout<<"StepRttSum:"<<StepRttSum<<"   StepRttCount:"<<StepRttCount<<"   StepRttMean:"<<StepRttMean<<endl;
        }

        if (EpisodeDone==0&&EpisodeDoneUsed==0)
        {
            if (EpisodeDoneCount==0)
            {
                EpisodeStartTime=time::steady_clock::now();//记录开始时间
            }
            else if (EpisodeDoneCount>=8000)   //episode count
            {
                EpisodeDone=1;
            }
            EpisodeDoneCount++;
        }

        if (EpisodeDone==2)    //drain    ||StepDone==2
        {
            if (m_nInFlight<=2)    //2
            {
                cwnd_Last=1;
                EpisodeDone=0;
                StepDone=0;
                StepStartTime=time::steady_clock::now();
                DQN_CheckInterval=DQN_CheckIntervalInitial;
                StepLossData=0;
                //StepTimeLast=time::steady_clock::now();
                //if (LossData>65000)
                //    LossData=0;
                //LossDataLast=LossData;

                //RTO初始化
                m_rttEstimator.RTOInit();
            }
        }

        if (StepDone==1)
        {

            StepExpense=time::steady_clock::now()-StepStartTime;
            StepExpense_ms=boost::chrono::duration_cast<boost::chrono::milliseconds>(StepExpense);

            if (StepStateUsed==0)
            {
                //StepTime=time::steady_clock::now()-StepTimeLast;
                //StepLossData=LossData-LossDataLast;

                StepStateUsed=1;
            }
            if (StepStateUsed==1)
            {
                if (StepDoneUsed==0)
                {
                    fstream file1;
                    string str_file1;
                    str_file1="../../Data/Train/Observation.txt";
                    file1.open(str_file1,ios::out|ios::binary|ios::app);
                    file1.seekg(0,std::ios::end);
                    if(file1.tellg()==0)
                    {
                        file1.close();
                        ofstream file2;
                        string str_file2;
                        str_file2="../../Data/Train/Observation.txt";
                        file2.open(str_file2,ios::out|ios::binary|ios::app);
                        std::ostream& log2(file2);
                        //log2<<m_cwnd<<"   "<<SegmentRTT.count()<<"   "<<StepLossData<<"   "<<m_nInFlight<<"   "<<Action1<<"   "<<StepTime.count()<<"   "<<EpisodeDone<<"   "<<StepTime.count();
                        EpisodeTime=time::steady_clock::now()-EpisodeStartTime;
                        log2<<cwnd_Last<<"   "<<m_cwnd<<"   "<<StepDataCount<<"   "<<StepRttMean<<"   "<<StepLossData<<"   "<<StepExpense_ms.count()+1<<"   "<<EpisodeDone<<"   "<<EpisodeDoneCount<<"   "<<EpisodeTime.count()+1;

                        file2.close();
                        StepDoneUsed=1;

                        if (EpisodeDone==1&&EpisodeDoneUsed==0)
                        {
                            EpisodeDone=0;
                            EpisodeDoneUsed=1;
                        }
                    }
                }
                if (StepDoneUsed==1)
                {
                    fstream file3;
                    string str_file3;
                    str_file3="../../Data/Train/Action.txt";
                    file3.open(str_file3,ios::in|ios::out|ios::binary|ios::app);
                    file3.seekg(0,std::ios::end);
                    if(file3.tellg()!=0)
                    {
                        file3.close();

                        ifstream file5;
                        string str_file5;
                        str_file5="../../Data/Train/Action.txt";
                        file5.open(str_file5,ios::in|ios::binary|ios::app);
                        std::istream& log5(file5);
                        float num1=0,num2=0;
                        log5>>num1;
                        log5>>num2;
                        file5.close();

                        ofstream file4;   //Clear
                        string str_file4;
                        str_file4="../../Data/Train/Action.txt";
                        file4.open(str_file4);
                        std::ostream& log4(file4);
                        file4.close();

                        //int m_cwndTemp=m_cwnd;
                        int resultt=0;
                        resultt=num1;

                        cwnd_Last=m_cwnd;

//                       m_cwnd=m_cwnd+1;    //测试时一直用这个策略

                       //m_cwnd=(resultt+2)*50+1;
                          m_cwnd=resultt;
                          m_cwnd=ceil(m_cwnd);    //向上取整
                          Action1=resultt;

                        if (m_cwnd>300)   //Limit
                        {
                            m_cwnd=300;
                        }
                        else if (m_cwnd<1)
                        {
                            m_cwnd=1;
                        }
/*
                        if (m_cwndTemp==1 && (resultt==1||resultt==3||resultt==5))
                        {
                            m_cwnd=2;
                        }
                        else if (m_cwndTemp==500 && (resultt==1||resultt==2||resultt==4))
                        {
                            m_cwnd=499;
                        }
*/
                        StepDoneUsed=0;
                        StepDone=0;
                        StepStartTime=time::steady_clock::now();
                        StepDoneCount=0;
                        StepStateUsed=0;
                        StepLossData=0;
                        StepRttSum=0;
                        StepRttCount=0;
                        StepDataCount=0;

                        if (EpisodeDone==0&&EpisodeDoneUsed==1&&num2==1)
                        {cout<<"****************"<<endl;
                            m_cwnd=1;
                            cwnd_Last=1;
                            EpisodeDoneCount=0;
                            EpisodeDoneUsed=0;
                            StepDone=2;
                            EpisodeDone=2;

                            EpisodeCount++;
                        }

                        cout<<"m_cwnd:"<<m_cwnd<<endl;
                    }
                }
            }
        }
}
}
else if (DQN_State==7)    //Use_DDPG
{
      if (EpisodeCount<=7)
      {
                Milliseconds RttTemp=time::steady_clock::now()-segInfo.timeSent;
                float RttTemp1=RttTemp.count();

                if (segInfo.state != SegmentState::FirstTimeSent &&
                    segInfo.state != SegmentState::InRetxQueue)
                {
                    RttTemp1=RttTemp1Last;
                }
                if (RttTemp1<=10 || RttTemp1>=100)    //排除重传的
                {
                    RttTemp1=RttTemp1Last;
                }

                ofstream outfile3;
                string str_outfile3;
                str_outfile3="../DownLoad/OnData_DDPG.txt";
                outfile3.open(str_outfile3,ios::out|ios::binary|ios::app);
                std::ostream& log3(outfile3);
                log3<<m_cwnd<<"   "<<RttTemp1<<endl;
                outfile3.close();

                RttTemp1Last=RttTemp1;

              if (StepDone==0)
              {
                  if (segInfo.SentCwndAction==Action1&&segInfo.SentCwnd==m_cwnd)
                  {
                      RttTemp1=RttTemp1-10;
                      //if (RttTemp1<=10)    //针对特殊错误，因为延时设定最低就是10
                      //    RttTemp1=10.7;

                      if ((segInfo.state == SegmentState::FirstTimeSent ||
                          segInfo.state == SegmentState::InRetxQueue)
                              && (StepDataCount>=cwnd_Last))
                      {
                          ofstream outfile3;
                          string str_outfile3;
                          str_outfile3="../DownLoad/NQD_Log.txt";
                          outfile3.open(str_outfile3,ios::out|ios::binary|ios::app);
                          std::ostream& log3(outfile3);
                          log3<<m_cwnd<<"   "<<RttTemp1<<endl;
                          outfile3.close();

                          StepRttSum += RttTemp1;
                          StepRttCount++;
                      }

                      StepDataCount++;

                      if (StepDataCount>=cwnd_Last+m_cwnd && StepRttCount>=5)
                      {
                          StepDone=1;
                      }
                  }
              }
              if (StepDone==1) //if (StepStateUsed==0)
              {
                  if (StepRttCount!=0)
                  {
                      StepRttMean=StepRttSum/StepRttCount;
                  }

                  //cout<<"StepRttSum:"<<StepRttSum<<"   StepRttCount:"<<StepRttCount<<"   StepRttMean:"<<StepRttMean<<endl;
              }

              if (EpisodeDone==0&&EpisodeDoneUsed==0)
              {
                  if (EpisodeDoneCount==0)
                  {
                      EpisodeStartTime=time::steady_clock::now();//记录开始时间
                  }
                  else if (EpisodeDoneCount>=100000)   //episode count
                  {
                      while (1)
                      {
                      }

                      //EpisodeDone=1;
                  }
                  EpisodeDoneCount++;
              }

              if (EpisodeDone==2)    //drain    ||StepDone==2
              {
                  if (m_nInFlight<=2)    //2
                  {
                      cwnd_Last=1;
                      EpisodeDone=0;
                      StepDone=0;
                      StepStartTime=time::steady_clock::now();
                      DQN_CheckInterval=DQN_CheckIntervalInitial;
                      StepLossData=0;
                      //StepTimeLast=time::steady_clock::now();
                      //if (LossData>65000)
                      //    LossData=0;
                      //LossDataLast=LossData;

                      //RTO初始化
                      m_rttEstimator.RTOInit();
                  }
              }

              if (StepDone==1)
              {

                  StepExpense=time::steady_clock::now()-StepStartTime;
                  StepExpense_ms=boost::chrono::duration_cast<boost::chrono::milliseconds>(StepExpense);

                  if (StepStateUsed==0)
                  {
                      //StepTime=time::steady_clock::now()-StepTimeLast;
                      //StepLossData=LossData-LossDataLast;

                      StepStateUsed=1;
                  }
                  if (StepStateUsed==1)
                  {
                      if (StepDoneUsed==0)
                      {
                          fstream file1;
                          string str_file1;
                          str_file1="../../Data/Train/Observation.txt";
                          file1.open(str_file1,ios::out|ios::binary|ios::app);
                          file1.seekg(0,std::ios::end);
                          if(file1.tellg()==0)
                          {
                              file1.close();
                              ofstream file2;
                              string str_file2;
                              str_file2="../../Data/Train/Observation.txt";
                              file2.open(str_file2,ios::out|ios::binary|ios::app);
                              std::ostream& log2(file2);
                              //log2<<m_cwnd<<"   "<<SegmentRTT.count()<<"   "<<StepLossData<<"   "<<m_nInFlight<<"   "<<Action1<<"   "<<StepTime.count()<<"   "<<EpisodeDone<<"   "<<StepTime.count();
                              EpisodeTime=time::steady_clock::now()-EpisodeStartTime;
                              log2<<cwnd_Last<<"   "<<m_cwnd<<"   "<<StepDataCount<<"   "<<StepRttMean<<"   "<<StepLossData<<"   "<<StepExpense_ms.count()+1<<"   "<<EpisodeDone<<"   "<<EpisodeDoneCount<<"   "<<EpisodeTime.count()+1;

                              file2.close();
                              StepDoneUsed=1;

                              if (EpisodeDone==1&&EpisodeDoneUsed==0)
                              {
                                  EpisodeDone=0;
                                  EpisodeDoneUsed=1;
                              }
                          }
                      }
                      if (StepDoneUsed==1)
                      {
                          fstream file3;
                          string str_file3;
                          str_file3="../../Data/Train/Action.txt";
                          file3.open(str_file3,ios::in|ios::out|ios::binary|ios::app);
                          file3.seekg(0,std::ios::end);
                          if(file3.tellg()!=0)
                          {
                              file3.close();

                              ifstream file5;
                              string str_file5;
                              str_file5="../../Data/Train/Action.txt";
                              file5.open(str_file5,ios::in|ios::binary|ios::app);
                              std::istream& log5(file5);
                              float num1=0,num2=0;
                              log5>>num1;
                              log5>>num2;
                              file5.close();

                              ofstream file4;   //Clear
                              string str_file4;
                              str_file4="../../Data/Train/Action.txt";
                              file4.open(str_file4);
                              std::ostream& log4(file4);
                              file4.close();

                              //int m_cwndTemp=m_cwnd;
                              int resultt=0;
                              resultt=num1;

                              cwnd_Last=m_cwnd;

      //                       m_cwnd=m_cwnd+1;    //测试时一直用这个策略

                             //m_cwnd=(resultt+2)*50+1;
                                m_cwnd=resultt;
                                m_cwnd=ceil(m_cwnd);    //向上取整
                                Action1=resultt;

                              if (m_cwnd>300)   //Limit
                              {
                                  m_cwnd=300;
                              }
                              else if (m_cwnd<1)
                              {
                                  m_cwnd=1;
                              }
      /*
                              if (m_cwndTemp==1 && (resultt==1||resultt==3||resultt==5))
                              {
                                  m_cwnd=2;
                              }
                              else if (m_cwndTemp==500 && (resultt==1||resultt==2||resultt==4))
                              {
                                  m_cwnd=499;
                              }
      */
                              StepDoneUsed=0;
                              StepDone=0;
                              StepStartTime=time::steady_clock::now();
                              StepDoneCount=0;
                              StepStateUsed=0;
                              StepLossData=0;
                              StepRttSum=0;
                              StepRttCount=0;
                              StepDataCount=0;

                              if (EpisodeDone==0&&EpisodeDoneUsed==1&&num2==1)
                              {cout<<"****************"<<endl;
                                  m_cwnd=1;
                                  cwnd_Last=1;
                                  EpisodeDoneCount=0;
                                  EpisodeDoneUsed=0;
                                  StepDone=2;
                                  EpisodeDone=2;

                                  EpisodeCount++;
                              }

                              cout<<"m_cwnd:"<<m_cwnd<<endl;
                          }
                      }
                  }
              }
      }
}
else  if (DQN_State==8)   //DDPG_Before
{
  if (EpisodeCount<=7)
  {
            Milliseconds RttTemp=time::steady_clock::now()-segInfo.timeSent;
            float RttTemp1=RttTemp.count();

            if (segInfo.state != SegmentState::FirstTimeSent &&
                segInfo.state != SegmentState::InRetxQueue)
            {
                RttTemp1=RttTemp1Last;
            }
            RttTemp1Last=RttTemp1;

          if (StepDone==0)
          {
              if (segInfo.SentCwndAction==Action1&&segInfo.SentCwnd==m_cwnd)
              {
                  //if (RttTemp1<10)    //排除重传的
                  //{
                  //    RttTemp1=10;
                  //}
                  //else if (RttTemp1>100)    //排除失效的
                  //{
                  //    RttTemp1=100;
                  //}

                  RttTemp1=RttTemp1-10;
                  //if (RttTemp1<=10)    //针对特殊错误，因为延时设定最低就是10
                  //    RttTemp1=10.7;

                  if ((segInfo.state == SegmentState::FirstTimeSent ||
                      segInfo.state == SegmentState::InRetxQueue)
                          && (StepDataCount>=cwnd_Last))
                  {
                      ofstream outfile3;
                      string str_outfile3;
                      str_outfile3="../DownLoad/NQD_Log.txt";
                      outfile3.open(str_outfile3,ios::out|ios::binary|ios::app);
                      std::ostream& log3(outfile3);
                      log3<<m_cwnd<<"   "<<RttTemp1<<endl;
                      outfile3.close();

                      StepRttSum += RttTemp1;
                      StepRttCount++;
                  }

                  StepDataCount++;

                  if (StepDataCount>=cwnd_Last+m_cwnd && StepRttCount>=10)
                  {
                      StepDone=1;
                      EpisodeStepCount++;
                  }
              }
          }
          if (StepDone==1) //if (StepStateUsed==0)
          {
              if (StepRttCount!=0)
              {
                  StepRttMean=StepRttSum/StepRttCount;
              }

              //cout<<"StepRttSum:"<<StepRttSum<<"   StepRttCount:"<<StepRttCount<<"   StepRttMean:"<<StepRttMean<<endl;
          }

          if (EpisodeDone==0&&EpisodeDoneUsed==0)
          {
              if (EpisodeDoneCount==0)
              {
                  EpisodeStartTime=time::steady_clock::now();//记录开始时间
              }
              else if (EpisodeDoneCount>=20000)   //episode count
              {
                  EpisodeDone=1;
              }

              if (EpisodeStepCount>200)
              {
                  //EpisodeDone=1;    //过200跳出
              }
              EpisodeDoneCount++;
          }

          if (EpisodeDone==2)    //drain    ||StepDone==2
          {
              if (m_nInFlight<=2)    //2
              {
                  cwnd_Last=1;
                  EpisodeDone=0;
                  StepDone=0;
                  StepStartTime=time::steady_clock::now();
                  DQN_CheckInterval=DQN_CheckIntervalInitial;
                  StepLossData=0;
                  //StepTimeLast=time::steady_clock::now();
                  //if (LossData>65000)
                  //    LossData=0;
                  //LossDataLast=LossData;

                  //RTO初始化
                  m_rttEstimator.RTOInit();
              }
          }

          if (StepDone==1)
          {
              StepExpense=time::steady_clock::now()-StepStartTime;
              StepExpense_ms=boost::chrono::duration_cast<boost::chrono::milliseconds>(StepExpense);

              if (StepStateUsed==0)
              {
                  //StepTime=time::steady_clock::now()-StepTimeLast;
                  //StepLossData=LossData-LossDataLast;

                  StepStateUsed=1;
              }
              if (StepStateUsed==1)
              {
                  if (StepDoneUsed==0)
                  {
                      fstream file1;
                      string str_file1;
                      str_file1="../../Data/Train/Observation.txt";
                      file1.open(str_file1,ios::out|ios::binary|ios::app);
                      file1.seekg(0,std::ios::end);
                      if(file1.tellg()==0)
                      {
                          file1.close();
                          ofstream file2;
                          string str_file2;
                          str_file2="../../Data/Train/Observation.txt";
                          file2.open(str_file2,ios::out|ios::binary|ios::app);
                          std::ostream& log2(file2);
                          //log2<<m_cwnd<<"   "<<SegmentRTT.count()<<"   "<<StepLossData<<"   "<<m_nInFlight<<"   "<<Action1<<"   "<<StepTime.count()<<"   "<<EpisodeDone<<"   "<<StepTime.count();
                          EpisodeTime=time::steady_clock::now()-EpisodeStartTime;
                          log2<<m_cwnd<<"   "<<StepDataCount<<"   "<<StepRttMean<<"   "<<StepLossData<<"   "<<StepExpense_ms.count()<<"   "<<EpisodeDone<<"   "<<EpisodeDoneCount<<"   "<<EpisodeTime.count();

                          file2.close();
                          StepDoneUsed=1;

                          if (EpisodeDone==1&&EpisodeDoneUsed==0)
                          {
                              EpisodeDone=0;
                              EpisodeDoneUsed=1;
                          }
                      }
                  }
                  if (StepDoneUsed==1)
                  {
                      fstream file3;
                      string str_file3;
                      str_file3="../../Data/Train/Action.txt";
                      file3.open(str_file3,ios::in|ios::out|ios::binary|ios::app);
                      file3.seekg(0,std::ios::end);
                      if(file3.tellg()!=0)
                      {
                          file3.close();

                          ifstream file5;
                          string str_file5;
                          str_file5="../../Data/Train/Action.txt";
                          file5.open(str_file5,ios::in|ios::binary|ios::app);
                          std::istream& log5(file5);
                          double num1=0,num2=0;
                          log5>>num1;
                          log5>>num2;
                          file5.close();

                          ofstream file4;   //Clear
                          string str_file4;
                          str_file4="../../Data/Train/Action.txt";
                          file4.open(str_file4);
                          std::ostream& log4(file4);
                          file4.close();

                          //int m_cwndTemp=m_cwnd;
                          double resultt=0;
                          resultt=num1;

                          cwnd_Last=m_cwnd;

  /*                     m_cwnd=m_cwnd+1;    //测试时一直用这个策略
  */                     //cout<<"!!!!"<<resultt<<endl;
                          m_cwnd=(resultt+2)*50+1;

                          m_cwnd=ceil(m_cwnd);    //向上取整
                          Action1=resultt;

                          if (m_cwnd>300)   //Limit
                          {
                              m_cwnd=300;
                          }
                          else if (m_cwnd<1)
                          {
                              m_cwnd=1;
                          }
  /*
                          if (m_cwndTemp==1 && (resultt==1||resultt==3||resultt==5))
                          {
                              m_cwnd=2;
                          }
                          else if (m_cwndTemp==500 && (resultt==1||resultt==2||resultt==4))
                          {
                              m_cwnd=499;
                          }
  */
                          StepDoneUsed=0;
                          StepDone=0;
                          StepStartTime=time::steady_clock::now();
                          StepDoneCount=0;
                          StepStateUsed=0;
                          StepLossData=0;
                          StepRttSum=0;
                          StepRttCount=0;
                          StepDataCount=0;

                          if (EpisodeDone==0&&EpisodeDoneUsed==1&&num2==1)
                          {cout<<"****************"<<endl;
                              m_cwnd=1;
                              cwnd_Last=1;
                              EpisodeDoneCount=0;
                              EpisodeDoneUsed=0;
                              StepDone=2;
                              EpisodeDone=2;
                              EpisodeStepCount=0;
                              EpisodeCount++;
                          }

                          cout<<"m_cwnd:"<<m_cwnd<<endl;
                      }
                  }
              }
          }
      }
}
  //if (m_nReceived>=15000)
  if ((EpisodeCount>7)&&(TrainsCount<=400))
  {
      if (FileEnd==0)
      {
          if (m_nInFlight>3)
          {cout<<"m_nInFlight:"<<m_nInFlight<<"!!!!!"<<endl;
              m_cwnd=1;    //0
          }
          else
          {cout<<"FileEnd:"<<FileEnd<<endl;
              FileEnd=1;
          }
      }
      else if (FileEnd==1)
      {
          m_cwnd=0;
          DQN_Event=DQN_Scheduler.scheduleEvent(DQN_CheckIntervalInitial,[this]{Check_DQN();});
          FileEnd=2;
      }
  }

  if (segInfo.state == SegmentState::FirstTimeSent ||
      segInfo.state == SegmentState::InRetxQueue) { // do not sample RTT for retransmitted segments
    auto nExpectedSamples = std::max<int64_t>((m_nInFlight + 1) >> 1, 1);
    BOOST_ASSERT(nExpectedSamples > 0);
    m_rttEstimator.addMeasurement(recvSegNo, rtt, static_cast<size_t>(nExpectedSamples));
    m_segmentInfo.erase(recvSegNo); // remove the entry associated with the received segment
    RttMeasurement(rtt);

    BBR_CheckInterval=boost::chrono::duration_cast<boost::chrono::milliseconds>(m_rttEstimator.getEstimatedBBR_RTT());
    DQN_CheckInterval=boost::chrono::duration_cast<boost::chrono::milliseconds>(m_rttEstimator.getEstimatedDQN_RTT());
  }
  else { // retransmission
    BOOST_ASSERT(segInfo.state == SegmentState::Retransmitted);
    segInfo.state = SegmentState::RetxReceived;
  }
  BOOST_ASSERT(m_nReceived > 0);
  if (m_hasFinalBlockId &&
      static_cast<uint64_t>(m_nReceived - 1) >= m_lastSegmentNo) { // all segments have been received
    cancel();
    if (m_options.isVerbose) {
      printSummary();
    }
  }
  else {
    schedulePackets();
  }
}

void
PipelineInterestsAimd::RttMeasurement(Milliseconds rtt)
{
    RttValue=boost::chrono::duration_cast<boost::chrono::milliseconds>(rtt);
}

void
PipelineInterestsAimd::handleNack(const Interest& interest, const lp::Nack& nack)
{
  if (Filedlg1::DownLoadStatus==2)
    return;

  DeliveredData++;

  if (m_options.isVerbose)
    std::cerr << "Received Nack with reason " << nack.getReason()
              << " for Interest " << interest << std::endl;

  uint64_t segNo = getSegmentFromPacket(interest);

  switch (nack.getReason()) {
    case lp::NackReason::DUPLICATE:
      // ignore duplicates
      break;
    case lp::NackReason::CONGESTION:
      // treated the same as timeout for now
      enqueueForRetransmission(segNo);
      recordTimeout();

      schedulePackets();
      break;
    default:
      handleFail(segNo, "Could not retrieve data for " + interest.getName().toUri() +
                 ", reason: " + boost::lexical_cast<std::string>(nack.getReason()));
      break;
  }
}

void
PipelineInterestsAimd::handleLifetimeExpiration(const Interest& interest)
{
  if (Filedlg1::DownLoadStatus==2)
    return;

  enqueueForRetransmission(getSegmentFromPacket(interest));
  recordTimeout();

  schedulePackets();
}

void
PipelineInterestsAimd::recordTimeout()
{
  if (m_options.disableCwa || m_highData > m_recPoint) {

    // react to only one timeout per RTT (conservative window adaptation)
    m_recPoint = m_highInterest;

    decreaseWindow();
    m_rttEstimator.backoffRto();
    m_nLossEvents++;
LossData++;
AIMDLossData++;

    if (m_options.isVerbose) {
      std::cerr << "Packet loss event, cwnd = " << m_cwnd
                << ", ssthresh = " << m_ssthresh << std::endl;
    }
  }
}

void
PipelineInterestsAimd::enqueueForRetransmission(uint64_t segNo)
{
  //BOOST_ASSERT(m_nInFlight > 0);
BOOST_ASSERT(m_nInFlight >=0);
  //m_nInFlight--;
//  if (m_nInFlight>1)
    if (m_nInFlight>0)
      m_nInFlight--;

  m_retxQueue.push(segNo);
  m_segmentInfo.at(segNo).state = SegmentState::InRetxQueue;
}

void
PipelineInterestsAimd::handleFail(uint64_t segNo, const std::string& reason)
{cout<<"********6*******"<<endl;
  if (Filedlg1::DownLoadStatus==2)
    return;

  // if the failed segment is definitely part of the content, raise a fatal error
  if (m_hasFinalBlockId && segNo <= m_lastSegmentNo)
    return onFailure(reason);

  if (!m_hasFinalBlockId) {
    m_segmentInfo.erase(segNo);
    //m_nInFlight--;
    if (m_nInFlight>=1)
        m_nInFlight--;

    if (m_segmentInfo.empty()) {
      onFailure("Fetching terminated but no final segment number has been found");
    }
    else {
      cancelInFlightSegmentsGreaterThan(segNo);
      m_hasFailure = true;
      m_failedSegNo = segNo;
      m_failureReason = reason;
    }
  }
}

void
PipelineInterestsAimd::increaseWindow()
{
  //m_cwnd +=1;
    //m_cwnd += m_options.aiStep; // additive increase

    //m_cwnd += 4.0 / std::floor(m_cwnd); // congestion avoidance

  /*
  if (m_cwnd < m_ssthresh) {
    m_cwnd += m_options.aiStep; // additive increase
  } else {
    m_cwnd += m_options.aiStep / std::floor(m_cwnd); // congestion avoidance
  }

  //if (m_cwnd>30)
  //    m_cwnd=30;   //?
  */

    if (DQN_State==1)
    {
        if (m_cwnd < m_ssthresh) {
            m_cwnd += m_options.aiStep; // additive increase
          }
          else {
            m_cwnd += m_options.aiStep / std::floor(m_cwnd); // congestion avoidance
          }

        afterCwndChange(time::steady_clock::now() - getStartTime(), m_cwnd);
    }
    else if (DQN_State==2&&BBRState==0)
    {
        if (m_cwnd < m_ssthresh) {
            m_cwnd += m_options.aiStep; // additive increase
          }
          else {
            m_cwnd += m_options.aiStep / std::floor(m_cwnd); // congestion avoidance
          }

        afterCwndChange(time::steady_clock::now() - getStartTime(), m_cwnd);
    }
}

void
PipelineInterestsAimd::decreaseWindow()
{
  // please refer to RFC 5681, Section 3.1 for the rationale behind it

  if (DQN_State==1)
  {
      m_ssthresh = std::max(2.0, m_cwnd * m_options.mdCoef); // multiplicative decrease
        m_cwnd = m_options.resetCwndToInit ? m_options.initCwnd : m_ssthresh;   //m=a<b?a:b

    //  if (m_cwnd>30)
    //      m_cwnd=30;   //?

      afterCwndChange(time::steady_clock::now() - getStartTime(), m_cwnd);
  }
}

uint64_t
PipelineInterestsAimd::getNextSegmentNo()
{
  Filedlg1::TotalTime=time.elapsed();
  // get around the excluded segment
  if (m_nextSegmentNo == m_excludedSegmentNo)
    m_nextSegmentNo++;
  return m_nextSegmentNo++;
}

void
PipelineInterestsAimd::cancelInFlightSegmentsGreaterThan(uint64_t segNo)
{
  for (auto it = m_segmentInfo.begin(); it != m_segmentInfo.end();) {
    // cancel fetching all segments that follow
    if (it->first > segNo) {
      m_face.removePendingInterest(it->second.interestId);
      it = m_segmentInfo.erase(it);
      //m_nInFlight--;
      if (m_nInFlight>=1)
         m_nInFlight--;

    }
    else {
      ++it;
    }
  }
}

void
PipelineInterestsAimd::printSummary() const
{
  Milliseconds timeElapsed = time::steady_clock::now() - getStartTime();
  double throughput = (8 * m_receivedSize * 1000) / timeElapsed.count();

  int pow = 0;
  std::string throughputUnit;
  while (throughput >= 1000.0 && pow < 4) {
    throughput /= 1000.0;
    pow++;
  }
  switch (pow) {
    case 0:
      throughputUnit = "bit/s";
      break;
    case 1:
      throughputUnit = "kbit/s";
      break;
    case 2:
      throughputUnit = "Mbit/s";
      break;
    case 3:
      throughputUnit = "Gbit/s";
      break;
    case 4:
      throughputUnit = "Tbit/s";
      break;
  }

  std::cerr << "\nAll segments have been received.\n"
            << "Time elapsed: " << timeElapsed << "\n"
            << "Total # of segments received: " << m_nReceived << "\n"
            << "Total # of packet loss events: " << m_nLossEvents << "\n"
            << "Packet loss rate: "
            << static_cast<double>(m_nLossEvents) / static_cast<double>(m_nReceived) << "\n"
            << "Total # of retransmitted segments: " << m_nRetransmitted << "\n"
            << "Goodput: " << throughput << " " << throughputUnit << "\n";
}

std::ostream&
operator<<(std::ostream& os, SegmentState state)
{
  switch (state) {
  case SegmentState::FirstTimeSent:
    os << "FirstTimeSent";
    break;
  case SegmentState::InRetxQueue:
    os << "InRetxQueue";
    break;
  case SegmentState::Retransmitted:
    os << "Retransmitted";
    break;
  case SegmentState::RetxReceived:
    os << "RetxReceived";
    break;
  }
  return os;
}

std::ostream&
operator<<(std::ostream& os, const PipelineInterestsAimdOptions& options)
{
  os << "PipelineInterestsAimd initial parameters:\n"
     << "\tInitial congestion window size = " << options.initCwnd << "\n"
     << "\tInitial slow start threshold = " << options.initSsthresh << "\n"
     << "\tAdditive increase step = " << options.aiStep << "\n"
     << "\tMultiplicative decrease factor = " << options.mdCoef << "\n"
     << "\tRTO check interval = " << options.rtoCheckInterval << "\n"
     << "\tMax retries on timeout or Nack = " << options.maxRetriesOnTimeoutOrNack << "\n"
     << "\tConservative Window Adaptation " << (options.disableCwa ? "disabled" : "enabled") << "\n"
     << "\tResetting cwnd to " << (options.resetCwndToInit ? "initCwnd" : "ssthresh") << " upon loss event\n";
  return os;
}

} // namespace aimd
} // namespace chunks
} // namespace ndn
