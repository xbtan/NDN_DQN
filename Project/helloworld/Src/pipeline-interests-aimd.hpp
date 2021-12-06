#ifndef NDN_TOOLS_CHUNKS_CATCHUNKS_PIPELINE_INTERESTS_AIMD_HPP
#define NDN_TOOLS_CHUNKS_CATCHUNKS_PIPELINE_INTERESTS_AIMD_HPP

#include "options.hpp"
#include "aimd-rtt-estimator.hpp"
#include "pipeline-interests.hpp"

#include <queue>
#include <QTime>
//#include <Python.h>
#include<iomanip>

namespace ndn {
namespace chunks {
namespace aimd {

class PipelineInterestsAimdOptions : public Options
{
public:
  explicit
  PipelineInterestsAimdOptions(const Options& options = Options())
    : Options(options)
  {
  }

public:
  double initCwnd = 1.0;   //初始化拥塞窗口大小
  double initSsthresh = std::numeric_limits<double>::max();   //初始慢启动阈值
  double aiStep = 1.0;   //和式增加值 (1个分块数)
  double mdCoef = 0.5;   //积式减少系数
  time::milliseconds rtoCheckInterval{10};   //检查重传定时器的间隔10?
  bool disableCwa = false;   //禁用保守窗口配置
  bool resetCwndToInit = false;   //当发生丢失事件时，将cwnd还原为initCwnd
};

/**
 * @brief indicates the state of the segment
 */
enum class SegmentState {   //兴趣包状态类
  FirstTimeSent,   //兴趣包第一次就发送出去
  InRetxQueue,   //兴趣包处于重传队列
  Retransmitted,   //兴趣包已重传
  RetxReceived,   //兴趣包重传后已收到数据包
};

std::ostream&
operator<<(std::ostream& os, SegmentState state);

//封装分块传输所需的信息
struct SegmentInfo
{
  const PendingInterestId* interestId;   //ndn::Face::expressInterest返回的待处理兴趣ID
  time::steady_clock::TimePoint timeSent;   //兴趣包发送时间
  Milliseconds rto;   //恢复时间目标
  SegmentState state;   //兴趣包状态
  uint64_t SendCount;
  uint64_t DeliveredDataSent;
  uint64_t LossDataSent;
  uint64_t SentDataSent;
  uint64_t SentCwnd;
  int SentCwndAction;
};

/**
 * 通过兴趣管道检索数据的服务
 *
 * 通过AIMD与保守丢失适配算法相结合维护动态拥塞窗口，检索指定前缀下的所有分段数据。
 *
 * 在抵达时提供检索数据，无需订购保证。 抵达后立即通过回拨将数据传送给PipelineInterests用户
 *
 *
 * Retrieves all segmented Data under the specified prefix by maintaining a dynamic AIMD
 * congestion window combined with a Conservative Loss Adaptation algorithm. For details,
 * please refer to the description in section "Interest pipeline types in ndncatchunks" of
 * tools/chunks/README.md
 *
 * Provides retrieved Data on arrival with no ordering guarantees. Data is delivered to the
 * PipelineInterests' user via callback immediately upon arrival.
 */
class PipelineInterestsAimd : public PipelineInterests
{
public:
  typedef PipelineInterestsAimdOptions Options;

public:
  /**
   * 创建一个PipelineInterestsAimd服务
   *
   *配置兴趣通道服务，不指定检索命名空间。在此配置之后，必须调用运行函数来开启管道
   */
  PipelineInterestsAimd(Face& face, RttEstimator& rttEstimator,
                        const Options& options = Options());

  ~PipelineInterestsAimd() final;

  /**
   * cwnd更改时的信号
   *
   * The callback function should be: void(Milliseconds age, double cwnd) where age is the
   * duration since pipeline starts, and cwnd is the new congestion window size (in segments).
   */
  signal::Signal<PipelineInterestsAimd, Milliseconds, double> afterCwndChange;

private:
  /**
   * 提取相应前缀文件下的所有分块
   *
   * 管道打开之后，用AIMD算法控制兴趣管道的窗口尺寸，兴趣管道会提取所需文件的所有分块，直到收到最后
   * 一个分块或者有错误发生。
   *
   * Starts the pipeline with an AIMD algorithm to control the window size. The pipeline will fetch
   * every segment until the last segment is successfully received or an error occurs.
   * The segment with segment number equal to m_excludedSegmentNo will not be fetched.
   */
  void
  doRun() final;

  /**
   * 停止所有的提取操作
   */
  void
  doCancel() final;

  void
  Check_DQN();

  void
  Check_BBR();
  /**
   * @brief check RTO for all sent-but-not-acked segments.
   */
  void
  checkRto();

  /**
   * @param segNo the segment # of the to-be-sent Interest
   * @param isRetransmission true if this is a retransmission
   */
  void
  sendInterest(uint64_t segNo, bool isRetransmission);

  void
  schedulePackets();

  void
  handleData(const Interest& interest, const Data& data);

  void
  RttMeasurement(Milliseconds rtt);

  void
  handleNack(const Interest& interest, const lp::Nack& nack);

  void
  handleLifetimeExpiration(const Interest& interest);

  void
  recordTimeout();

  void
  enqueueForRetransmission(uint64_t segNo);

  void
  handleFail(uint64_t segNo, const std::string& reason);

  /**
   * @brief increase congestion window size based on AIMD scheme
   */
  void
  increaseWindow();

  /**
   * @brief decrease congestion window size based on AIMD scheme
   */
  void
  decreaseWindow();

  /** \return next segment number to retrieve
   *  \post m_nextSegmentNo == return-value + 1
   */
  uint64_t
  getNextSegmentNo();

  void
  cancelInFlightSegmentsGreaterThan(uint64_t segNo);

  void
  printSummary() const;

PUBLIC_WITH_TESTS_ELSE_PRIVATE:
  const Options m_options;
  RttEstimator& m_rttEstimator;
  Scheduler m_scheduler;
  Scheduler DQN_Scheduler;
  Scheduler BBR_Scheduler;

  scheduler::ScopedEventId m_checkRtoEvent;
  scheduler::ScopedEventId DQN_Event;
  scheduler::ScopedEventId BBR_Event;

  uint64_t m_nextSegmentNo;
  size_t m_receivedSize;

  uint64_t m_highData; //需求者目前接收到的最大分块号
  uint64_t m_highInterest; //需求者目前所发送兴趣包的最大分块号
  uint64_t m_recPoint; //当发生丢包时m_highInterest的值，一直保持不变，直到下一次发生丢包事件

  uint64_t SentInterestHighNo;

  time::steady_clock::TimePoint timebeginlast;   //兴趣包发送时间  ////////////

  uint64_t DeliveredData;
  uint64_t LossData;
  uint64_t SentData;

  int StepLossData;
  uint64_t LossDataLast;
  int Action1;

  int AIMDLossData;
  int LossDataSum;

  int FileEnd;

  int64_t m_nInFlight; //还在请求之中的分块数
  int64_t m_nReceived; //已经接收到的分块数
  int64_t m_nLossEvents; //哪个分块发生了丢包现象
  int64_t m_nRetransmitted; //发出的分块请求

  double m_cwnd; //目前的拥塞窗口尺寸
  double cwnd_Last;
  double m_cwndSampleFirst;
  double m_ssthresh; //当前缓慢启动阈值
  double RtoInFlightTimeoutSegment;

  double SegmentDeliveryRate;

  float RttTemp1Last;

  double SegmentDeliveryRatio;

  time::milliseconds BBR_CheckInterval{10};
  time::milliseconds DQN_CheckInterval{30};   //10?
  time::milliseconds DQN_CheckIntervalInitial{4000};
  //time::milliseconds BbrCheckIntervalLast{10};   //200?   time::duration<double, time::milliseconds::period>
  //time::milliseconds BbrCheckIntervalexpense{10};   //200?
  double DeliveryRateQueue[3]={0};   //投递率队列
  int DeliveryRateQueueCount;

  double BBRBwSampleGain[8]={1.25,0.75,1,1,1,1,1,1};
  int BBRBwSampleGainCount;
  double Cwnd_Temp;
  double DeliveredRateTemp;
  double DeliveryRateSample;

  time::milliseconds BbrCheckIntervalexpense_msTemp{0};

  Milliseconds BBRCheckIntervalexpense{10};
  time::milliseconds BBRCheckIntervalexpense_ms{10};
  Milliseconds StepExpense;
  time::milliseconds StepExpense_ms;
  uint64_t DeliveredDataBBRInterval;
  uint64_t SentDataBBRInterval;
  time::steady_clock::TimePoint BBRCheckTimeLast;

  time::milliseconds RttValue{0};
  //time::milliseconds RttValueQueue[3]={0,0,0};
  Milliseconds minRtt{0};
  Milliseconds maxRtt{20000};   //20000

  std::unordered_map<uint64_t, SegmentInfo> m_segmentInfo; ///< keeps all the internal information
                                                           ///< on sent but not acked segments
  std::unordered_map<uint64_t, int> m_retxCount; ///< maps segment number to its retransmission count;
                                                 ///< if the count reaches to the maximum number of
                                                 ///< timeout/nack retries, the pipeline will be aborted
  std::queue<uint64_t> m_retxQueue;   //先进先出队列

  bool m_hasFailure;
  uint64_t m_failedSegNo;
  std::string m_failureReason;

  int DQN_State;    //3:在线训练（在线测试只需要改变python部分）,2:NDN_BBR,1:NDN_AIMD
                    //4:调用DQN_Graph,5:在线测试（调用及更新），6：NDN_DDPG
  int EpisodeDone;
  int EpisodeDoneUsed;
  int StepDone;
  int StepDoneUsed;
  int EpisodeDoneCount;
  int EpisodeStepCount;
  int EpisodeCount;
  int StepDoneCount;
  int TrainsCount;
  int StepStateUsed;
  uint64_t SegmentDeliveredData;
  uint64_t SegmentLossData;
  float StepRttMean;
  float StepRttSum;
  float StepRttCount;
  float StepDataCount;
  Milliseconds StepTime;
  Milliseconds EpisodeTime;
  time::steady_clock::TimePoint EpisodeStartTime;
  time::steady_clock::TimePoint StepStartTime;

  time::steady_clock::TimePoint GraphBegTime;

  int BBRState;

  QTime time;
  std::ostream& m_log;
};

std::ostream&
operator<<(std::ostream& os, const PipelineInterestsAimdOptions& options);

} // namespace aimd

using aimd::PipelineInterestsAimd;

} // namespace chunks
} // namespace ndn

#endif // NDN_TOOLS_CHUNKS_CATCHUNKS_PIPELINE_INTERESTS_AIMD_HPP
