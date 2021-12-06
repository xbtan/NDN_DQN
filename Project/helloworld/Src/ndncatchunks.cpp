#include "ndncatchunks.hpp"
#include <qtimer.h>
#include <qobject.h>
#include <ndn-cxx/security/validator-null.hpp>

#include <fstream>
#include "version.hpp"
#include "aimd-statistics-collector.hpp"   //
#include "aimd-rtt-estimator.hpp"   //
#include "consumer.hpp"
#include "discover-version-fixed.hpp"
#include "discover-version-iterative.hpp"
//#include "pipeline-interests-fixed-window.hpp"
#include "pipeline-interests-aimd.hpp"
#include "pipeline-interests-fixed-window.hpp"
#include "options.hpp"
#include "Add.h"
#include "filedlg1.h"
#include "qmessagebox.h"

namespace ndn {
namespace chunks {
int Cat::CatChunks()
{
  Options options;
  QString discoverType=Filedlg1::DiscoverType;
  QString pipelineType=Filedlg1::PipelineType;
  size_t maxPipelineSize(1);
  int maxRetriesAfterVersionFound(1);
  string uri;

  // congestion control parameters, CWA refers to conservative window adaptation,
  // i.e. only reduce window size at most once per RTT
  bool disableCwa(false), resetCwndToInit(false);
  double aiStep(1.0), mdCoef(0.5), alpha(0.125), beta(0.25),
         minRto(200.0), maxRto(4000.0);
  int initCwnd(1), initSsthresh(std::numeric_limits<int>::max()), k(4);
  std::string cwndPath, rttPath;

  string Input_Prefix_String,Input_FileVersion_String;
  Input_Prefix_String=string((const char *)Filedlg1::Output_Prefix.toLocal8Bit());
  Input_FileVersion_String=string((const char *)Filedlg1::Output_File_Version.toLocal8Bit());

  uri=Input_Prefix_String+"/"+Input_FileVersion_String;
  
  Name prefix(uri);

  //options.interestLifetime = time::milliseconds(vm["lifetime"].as<uint64_t>());

  try
  {
    Face face;

    unique_ptr<DiscoverVersion> discover;

    //auto data = make_shared<Data>(Name(m_versionedPrefix).appendSegment(m_store.size()));   //?

    if (discoverType == "FIXED") {
      discover = make_unique<DiscoverVersionFixed>(prefix, face, options);
    }
    else if (discoverType == "ITERATIVE") {
      DiscoverVersionIterative::Options optionsIterative(options);
      optionsIterative.maxRetriesAfterVersionFound = maxRetriesAfterVersionFound;   //检索到一个版本的数据包之后，重新检索的次数
      discover = make_unique<DiscoverVersionIterative>(prefix, face, optionsIterative);
    }
    else {
      std::cerr << "ERROR: discover version type not valid" << std::endl;
      return 2;
    }

    unique_ptr<PipelineInterests> pipeline;
    unique_ptr<aimd::StatisticsCollector> statsCollector;
    unique_ptr<aimd::RttEstimator> rttEstimator;
    std::ofstream statsFileCwnd;
    std::ofstream statsFileRtt;

    if (pipelineType == "FIXED") {
      PipelineInterestsFixedWindow::Options optionsPipeline(options);
      optionsPipeline.maxPipelineSize = maxPipelineSize;   //设置最大的兴趣通道尺寸
      pipeline = make_unique<PipelineInterestsFixedWindow>(face, optionsPipeline);
    }
    else if (pipelineType == "AIMD") {
      aimd::RttEstimator::Options optionsRttEst;
      optionsRttEst.isVerbose = options.isVerbose;
      optionsRttEst.alpha = alpha;
      optionsRttEst.beta = beta;
      optionsRttEst.k = k;
      //optionsRttEst.minRto = aimd::Milliseconds(minRto);
      //optionsRttEst.maxRto = aimd::Milliseconds(maxRto);

      rttEstimator = make_unique<aimd::RttEstimator>(optionsRttEst);

      PipelineInterestsAimd::Options optionsPipeline(options);
      optionsPipeline.disableCwa = disableCwa;
      optionsPipeline.resetCwndToInit = resetCwndToInit;
      optionsPipeline.initCwnd = static_cast<double>(initCwnd);
      optionsPipeline.initSsthresh = static_cast<double>(initSsthresh);
      optionsPipeline.aiStep = aiStep;
      optionsPipeline.mdCoef = mdCoef;

      auto aimdPipeline = make_unique<PipelineInterestsAimd>(face, *rttEstimator, optionsPipeline);

      if (!cwndPath.empty() || !rttPath.empty()) {
        if (!cwndPath.empty()) {
          statsFileCwnd.open(cwndPath);
          if (statsFileCwnd.fail()) {
            std::cerr << "ERROR: failed to open " << cwndPath << std::endl;
            return 4;
          }
        }
        if (!rttPath.empty()) {
          statsFileRtt.open(rttPath);
          if (statsFileRtt.fail()) {
            std::cerr << "ERROR: failed to open " << rttPath << std::endl;
            return 4;
          }
        }
        statsCollector = make_unique<aimd::StatisticsCollector>(*aimdPipeline, *rttEstimator,
                                                                statsFileCwnd, statsFileRtt);
      }

      pipeline = std::move(aimdPipeline);
    }
    else {
      std::cerr << "ERROR: Interest pipeline type not valid" << std::endl;
      return 2;
    }

    //ValidatorNull validator;   //之前版本
//    ndn::security::v2::ValidatorNull validator;

    //QMessageBox::information(NULL, "P1","Prefix:" +Filedlg1::Output_File_Path);
    //QMessageBox::information(NULL, "P2","Prefix:" +Filedlg1::Output_File_Name);
    string str_outfile;
    str_outfile=string((const char *)Filedlg1::Output_File_Path.toLocal8Bit())+"/"+string((const char *)Filedlg1::Output_File_Name.toLocal8Bit());
    ofstream outfile;   //
    outfile.open(str_outfile,ios::out|ios::binary);   //
//    Consumer consumer(validator, options.isVerbose,outfile);//

Consumer consumer(security::v2::getAcceptAllValidator(), options.isVerbose,outfile);


    BOOST_ASSERT(discover != nullptr);
    BOOST_ASSERT(pipeline != nullptr);
    consumer.run(std::move(discover), std::move(pipeline));   //->run
    face.processEvents();   //->store
  }
  catch (const Consumer::ApplicationNackError& e)
  {
    std::cerr << "ERROR: " << e.what() << std::endl;
    return 3;
  }
  catch (const std::exception& e)
  {
    std::cerr << "ERROR: " << e.what() << std::endl;
    return 1;
  }
  return 0;
}

}
}
