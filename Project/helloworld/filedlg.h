#ifndef FILEDLG_H
#define FILEDLG_H

#include <QDialog>
#include <string.h>
#include <string>
#include <iostream>
#include <sstream>
#include <cstring>
#include <qreadwritelock.h>
#include <qmutex.h>

#include "Src/version.hpp"
#include "Src/Publisher.hpp"
#include <qtimer.h>
#include "Src/PipelineCalCulateSpeed-Upload.hpp"
#include <QButtonGroup>
#include <qdatetime.h>

namespace Ui {
class Filedlg;
}

class Filedlg : public QDialog
{
    Q_OBJECT

public:
    explicit Filedlg(QWidget *parent = 0);
    ~Filedlg();

    //定义静态变量用于类之间的参数传递，需在只调用一次的文件开头初始化
    static QString Input_Prefix;
    static QString Input_File_Path;
    static QString File_Version;
    static int SegmentAmount;
    static int UploadBeginSegmentNo;
    static int UploadEndSegmentNo;
    static int PublishMode;
    static int PublishedNo;
    QButtonGroup buttongroup;

signals:
     void PutSegmentSignal();

private slots:
    void on_pushButton_clicked();

    void on_pushButton_3_clicked();

    void on_pushButton_2_clicked();

    void on_commandLinkButton_3_clicked();

    void on_pushButton_5_clicked();

    void on_pushButton_7_clicked();

    void PipelineSpeedDisplay(QString DisplayString);

    void SystemTimeDisplay();

    void on_pushButton_8_clicked();

    void PublishedSegmentDisplay();

    void on_pushButton_9_clicked();

    void on_pushButton_10_clicked();

private:
    Ui::Filedlg *ui;
    QDateTime time;
    QTimer *timer2=new QTimer;
    QTimer *timer3=new QTimer;
    PipelineCalCulateUploadSpeed *CalCulateSpeed2=new PipelineCalCulateUploadSpeed;
};

#endif // FILEDLG_H
