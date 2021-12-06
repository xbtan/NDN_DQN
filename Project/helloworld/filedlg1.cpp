#include "filedlg1.h"
#include "ui_filedlg1.h"
#include <QMessageBox>
#include "QFileDialog"
#include "nfd_start.h"
#include "Src/consumer.hpp"
#include "Src/ndncatchunks.hpp"
#include "Src/ndn-catsegment-thread.hpp"
#include "Src/md5.h"
#include "Src/pipeline-interests-fixed-window.hpp"

QString Filedlg1::Output_Prefix="";
QString Filedlg1::Output_File_Path="";
QString Filedlg1::Output_File_Version="";
QString Filedlg1::Output_File_Name="";
QString Filedlg1::DiscoverType="";
QString Filedlg1::PipelineType="";
int Filedlg1::BeginSegmentNo=0;
int Filedlg1::FinishSegmentNo=-1;
char Filedlg1::DownLoadStatus=0;
int Filedlg1::DownloadNo=0;
uint64_t Filedlg1::TotalTime=0;

Filedlg1::Filedlg1(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Filedlg1)
{
    ui->setupUi(this);

    NDNCatSegmentThread *NdnThread = new NDNCatSegmentThread;   //如此定义，start之后不会立即中断,不需要用wait函数
    NdnThread->setParent(this);   //SetParent
    CalCulateSpeed->setParent(this);
    timer1->setParent(this);
    QObject::connect(this, SIGNAL(CatSegmentSignal()),NdnThread,SLOT(test()));
    QObject::connect(CalCulateSpeed, SIGNAL(BeginSpeedDisplay(QString)),this,SLOT(PipelineSpeedDisplay(QString)));
}

Filedlg1::~Filedlg1()
{
    delete ui;
}

void Filedlg1::on_pushButton_clicked()
{
    Filedlg1::Output_File_Path= QFileDialog::getExistingDirectory(this, tr("Open Directory"),"./");
    if(Filedlg1::Output_File_Path.length() == 0)
    {
         QMessageBox::information(NULL, tr("Path"), tr("You didn't select any files."));
    }
    else
    {
         ui->textEdit->setText(Filedlg1::Output_File_Path);
    }
}

void Filedlg1::on_pushButton_2_clicked()
{
     TotalTime=0;
     DownLoadStatus=0;
     ndn::chunks::Consumer::SegmentNoForTimer=0;
     ndn::chunks::Consumer::ReceivedSegment=0;

     Filedlg1::Output_File_Path=ui->textEdit->toPlainText();
     //Filedlg1::Output_File_Path="../DownLoad";

     Filedlg1::Output_File_Name=ui->textEdit_2->toPlainText();
     Filedlg1::Output_Prefix=ui->textEdit_3->toPlainText();
     Filedlg1::Output_File_Version=ui->textEdit_4->toPlainText();
     Filedlg1::DiscoverType=ui->comboBox->currentText();
     Filedlg1::PipelineType=ui->comboBox_2->currentText();

     if (DownloadNo==0)
     {
     emit this->CatSegmentSignal();

     connect(timer1,SIGNAL(timeout()),CalCulateSpeed,SLOT(CalculateSpeed()));
     timer1->start(500);
     }
     else
     {
         emit this->CatSegmentSignal();
     }
     DownloadNo++;
}

void Filedlg1::on_commandLinkButton_3_clicked()
{
    Nfd_Start nfd_start;
    nfd_start.exec();
}

void Filedlg1::on_pushButton_3_clicked()
{
    string str_outfile;
    str_outfile=string((const char *)Filedlg1::Output_File_Path.toLocal8Bit())+"/"+string((const char *)Filedlg1::Output_File_Name.toLocal8Bit());
    ifstream in(str_outfile, ios::binary);
    if (!in)
        return;

    MD5 md5;
    std::streamsize length;
    char buffer[1024];
    while (!in.eof()) {
        in.read(buffer, 1024);
        length = in.gcount();
        if (length > 0)
            md5.update(buffer, length);
    }
    in.close();
    ui->textEdit_7->setText(QString(QString::fromLocal8Bit(md5.toString().c_str())));
}

void Filedlg1::on_pushButton_4_clicked()   //PAUSE
{
    DownLoadStatus=1;
}

void Filedlg1::on_pushButton_5_clicked()   //CONTINUE
{
    DownLoadStatus=0;
}

void Filedlg1::on_pushButton_6_clicked()   //STOP
{
    DownLoadStatus=2;
}

void Filedlg1::PipelineSpeedDisplay(QString DisplayString)
{
    ui->label_5->setText(DisplayString);
    ReceivedSegmentDisplay();
}

void Filedlg1::ReceivedSegmentDisplay()
{
    uint64_t DisplayReceivedSegmentNo;
    DisplayReceivedSegmentNo=ndn::chunks::Consumer::ReceivedSegment;
    QString DisplayString;
    DisplayString=QString::number(DisplayReceivedSegmentNo,10);
    ui->label_8->setText(DisplayString);

    QString DisplayTotalTimeString;   //Display total time
    DisplayTotalTimeString="Total time:"+QString("%1").arg(TotalTime/1000.0)+"s";
    ui->label_11->setText(DisplayTotalTimeString);

    QString StrDisplay;
    float Speed=0;
    int UnitFlag = 0;
    QString Unit="";
    Speed=((ndn::chunks::Consumer::ReceivedSegment)*4400*8)/(TotalTime/1000.0);   //2?
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
    StrDisplay="Average speed:"+StrDisplay+Unit;
    ui->label_12->setText(StrDisplay);
}

void Filedlg1::on_commandLinkButton_clicked()
{
    this->deleteLater();
    this->close();
    Filedlg1 filedlg1;
    filedlg1.exec();
    return;
}
