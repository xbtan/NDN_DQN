#ifndef FILEDLG1_H
#define FILEDLG1_H

#include <QDialog>
#include <QButtonGroup>
#include <qtimer.h>
#include "Src/PipelineCalCulateSpeed.hpp"

namespace Ui {
class Filedlg1;
}

class Filedlg1 : public QDialog
{
    Q_OBJECT

public:
    explicit Filedlg1(QWidget *parent = 0);
    ~Filedlg1();

    static QString Output_Prefix;
    static QString Output_File_Path;
    static QString Output_File_Version;
    static QString Output_File_Name;
    static QString DiscoverType;
    static QString PipelineType;
    static int BeginSegmentNo;
    static int FinishSegmentNo;
    static char DownLoadStatus;
    static int DownloadNo;
    static uint64_t TotalTime;

signals:
     void CatSegmentSignal();

private slots:
    void on_pushButton_clicked();

    void on_pushButton_2_clicked();

    void on_commandLinkButton_3_clicked();

    void on_pushButton_3_clicked();

    void on_pushButton_4_clicked();
    
    void on_pushButton_5_clicked();
    
    void on_pushButton_6_clicked();
    
    void PipelineSpeedDisplay(QString DisplayString);

    void ReceivedSegmentDisplay();

    void on_commandLinkButton_clicked();

private:
    Ui::Filedlg1 *ui;
    QTimer *timer1=new QTimer;
    PipelineCalCulateSpeed *CalCulateSpeed=new PipelineCalCulateSpeed;

};

#endif // FILEDLG1_H
