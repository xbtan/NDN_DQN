#include "Src/PipelineCalCulateSpeed-Upload.hpp"
#include <qdebug.h>
#include "Src/Publisher.hpp"

void
PipelineCalCulateUploadSpeed::CalculateSpeed()
{
    QString StrDisplay;
    float Speed=0;
    int UnitFlag = 0;
    QString Unit="";

    Speed=(ndn::segment::Publisher::SegmentNoForTimer)*4400*8*2;   //*2? 500ms
    while (Speed >= 1000.0 && UnitFlag < 4) {
      Speed /= 1000.0;
      UnitFlag++;
    }
    switch (UnitFlag) {
      case 0:
        Unit = "bit/s";
        break;
      case 1:
        Unit = "kbit/s";
        break;
      case 2:
        Unit = "Mbit/s";
        break;
      case 3:
        Unit = "Gbit/s";
        break;
      case 4:
        Unit = "Tbit/s";
        break;
    }
    StrDisplay=QString("%1").arg(Speed);
    StrDisplay=StrDisplay+Unit;
    if (Filedlg::PublishMode==2)
    {
        StrDisplay="Real-time speed:"+StrDisplay;
    }
    emit BeginSpeedDisplay(StrDisplay);
    ndn::segment::Publisher::SegmentNoForTimer=0;
}
