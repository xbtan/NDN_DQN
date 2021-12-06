#include "Src/PipelineCalCulateSpeed.hpp"
#include <qdebug.h>
#include "Src/consumer.hpp"

void
PipelineCalCulateSpeed::CalculateSpeed()
{
    QString StrDisplay;
    float Speed=0;
    int UnitFlag = 0;
    QString Unit="";

    Speed=(ndn::chunks::Consumer::SegmentNoForTimer)*4400*8*2;
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
    StrDisplay="Real-time speed:"+StrDisplay+Unit;
    emit BeginSpeedDisplay(StrDisplay);
    ndn::chunks::Consumer::SegmentNoForTimer=0;
}
