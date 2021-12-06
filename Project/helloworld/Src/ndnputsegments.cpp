#include "ndnputsegments.hpp"
#include <qtimer.h>
#include <qmutex.h>
#include "version.hpp"
#include "Publisher.hpp"
#include "Add.h"
#include "filedlg.h"
#include "ui_filedlg.h"
#include "qmessagebox.h"

namespace ndn {
namespace segment {
int Put::PutSegment()
{
  uint64_t freshnessPeriod = 10000;
  size_t maxChunkSize = MAX_NDN_PACKET_SIZE >>1;
  bool isVerbose = false;
  string prefix;

  //QMessageBox::information(NULL, "P1","Prefix:" +Filedlg::Input_Prefix);
  //QMessageBox::information(NULL, "P2","File:" +Filedlg::Input_File_Path);
  string Input_Prefix_String,Input_FilePath_String;
  Input_Prefix_String="/"+string((const char *)Filedlg::Input_Prefix.toLocal8Bit());
  Input_FilePath_String=string((const char *)Filedlg::Input_File_Path.toLocal8Bit());
  prefix=Input_Prefix_String;

  security::SigningInfo signingInfo;
  try
  {
    signingInfo = security::SigningInfo("");
  }
  catch (const std::invalid_argument& e)
  {
    std::cerr << "ERROR: " << e.what() << std::endl;
    return 2;
  }

  try
  {
    Face face;
    KeyChain keyChain;

    ifstream infile(Input_FilePath_String,ios::in|ios::binary);

    Publisher publisher(prefix, face, keyChain, signingInfo, time::milliseconds(freshnessPeriod),
                      maxChunkSize, isVerbose,infile);

    publisher.run();

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
