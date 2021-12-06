#include <qthread.h>
#include <QObject>
#include <QThread>
#include <qdebug.h>
#include "filedlg1.h"

class NDNCatSegmentThread : public QThread
{
    Q_OBJECT
public slots:
    void test()
    {
        qDebug()<< "hello from test" ;
        //start(HighestPriority);         //启动线程
        if (Filedlg1::DownloadNo!=0)
        {
            terminate();
            sleep(1);
        }
        start();
    }
private:
    void run();
};
