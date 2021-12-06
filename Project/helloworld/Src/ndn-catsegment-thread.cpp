#include "Src/ndn-catsegment-thread.hpp"
#include <QDebug>
#include <qtimer.h>
#include "filedlg1.h"
#include "Src/ndncatchunks.hpp"

void NDNCatSegmentThread::run()
{
    ndn::chunks::Cat catchunk;
    catchunk.CatChunks();
}
