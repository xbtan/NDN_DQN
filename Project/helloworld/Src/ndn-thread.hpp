#include <qthread.h>
#include <QObject>
#include <QThread>
#include "filedlg.h"

class NDNThread : public QThread
{
    Q_OBJECT
public slots:
    void test()   //上传文件次线程测试槽函数
    {
        if (Filedlg::PublishedNo!=0)   //多次上传文件时，每次重新开启线程
        {
            terminate();   //终止次线程
            sleep(1);   //滞留1秒再进行下面操作
        }
        start();   //开启次线程
    }
private:
    void run();   //重载的run函数
};
