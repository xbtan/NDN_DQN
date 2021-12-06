#include "filedlg.h"
#include "ui_filedlg.h"
#include <QMessageBox>
#include <QTextEdit>
#include <QDebug>
#include <qobject.h>
#include <iostream>
#include <string>
#include <sstream>
#include <cstring>
#include <string.h>
#include <qbuttongroup.h>
#include "QFileDialog"
#include "nfd_start.h"
#include "Src/Publisher.hpp"
#include "Src/ndn-thread.hpp"
#include "Src/md5.h"
#include "Src/ndnputsegments.hpp"
using namespace std;

//静态变量初始化
QString Filedlg::Input_Prefix="";   //用户输入前缀
QString Filedlg::Input_File_Path="";   //用户输入文件路径
QString Filedlg::File_Version="";   //文件版本编码
int Filedlg::PublishMode=0;   //上传模式
int Filedlg::PublishedNo=0;   //上传数据包数
int Filedlg::SegmentAmount=0;   //文件分块总数
int Filedlg::UploadBeginSegmentNo=-1;   //需要上传文件段的起始分块号
int Filedlg::UploadEndSegmentNo=-1;   //需要上传文件段的截止分块号

Filedlg::Filedlg(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Filedlg)
{
    ui->setupUi(this);   //定义ui的this指针
    connect(timer3,SIGNAL(timeout()),this,SLOT(SystemTimeDisplay()));   //连接定时器3的超时信号和显示系统时间的槽函数
    timer3->start(1000);   //开启定时器3（设定时间为1s）
    buttongroup.addButton(ui->radioButton);   //将模式1的单选按钮加入按钮组
    buttongroup.setId(ui->radioButton,0);   //设定模式1单选按钮在按钮组中的ID为0
    buttongroup.addButton(ui->radioButton_2);   //将模式2的单选按钮加入按钮组
    buttongroup.setId(ui->radioButton_2,1);  //设定模式2单选按钮在按钮组中的ID为1
    NDNThread *NdnThread = new NDNThread;   //定义上传文件次线程的对象（start之后不会立即中断,不需要用wait函数）
    QObject::connect(this, SIGNAL(PutSegmentSignal()),NdnThread, SLOT(test()));   //连接该对象的上传数据块信号和上传文件次线程的测试槽函数
    QObject::connect(CalCulateSpeed2, SIGNAL(BeginSpeedDisplay(QString)),this,SLOT(PipelineSpeedDisplay(QString)));   //连接计算上传速度对象的开始速度显示信号和该对象的上传速度显示槽函数
}

Filedlg::~Filedlg()
{
    delete ui;
}

void Filedlg::on_pushButton_clicked()   //上传文件选择按钮
{
    Filedlg::Input_File_Path= QFileDialog::getOpenFileName(this, tr("Open File"), ".");   //获取上传文件路径
    if(Filedlg::Input_File_Path.length() == 0)
    {
         QMessageBox::information(NULL, tr("Path"), tr("You didn't select any files."));   //未选择上传文件
    }
    else
    {
         ui->textEdit->setText(Filedlg::Input_File_Path);   //显示选择的上传文件路径
    }
}

void Filedlg::on_pushButton_3_clicked()   //前缀确认按钮
{
    Filedlg::Input_Prefix=ui->textEdit_2->toPlainText();   //将输入前缀赋给相应变量
}

void Filedlg::on_pushButton_2_clicked()   //模式2的上传按钮
{
    ndn::segment::Publisher::SegmentNoForTimer=0;   //计时器的数据包计数清零
    ndn::segment::Publisher::PublishedSegment=0;   //已上传的数据包计数清零
    int a=buttongroup.checkedId();   //获取安按键组ID
    if (a==0)   //上传模式1
    {
        PublishMode=1;
        QMessageBox::information(this, "Tips", "Please choose the correct publish mode!", QMessageBox::Ok);   //模式选择不正确
    }
    else if (a==1)   //上传模式2
    {
        PublishMode=2;
        if (PublishedNo==0)   //第一个上传的数据包
        {
            emit this->PutSegmentSignal();   //发出开始上传数据包信号
            connect(timer2,SIGNAL(timeout()),CalCulateSpeed2,SLOT(CalculateSpeed()));   //连接计时器2的超时信号和计算上传速度对象的计算速度槽函数
            timer2->start(500);   //开启定时器2（设定时间为500ms）
        }
        else   //后续上传的数据包
        {
            emit this->PutSegmentSignal();   //发出开始上传数据包信号
        }
        PublishedNo++;   //上传数据包数加一
    }
    else if (a==-1)   //未选择上传模式
    {
        QMessageBox::information(this, "Tips", "Please choose the publish mode!", QMessageBox::Ok);
    }
}

void Filedlg::on_commandLinkButton_3_clicked()   //上一步的按钮
{
    Nfd_Start nfd_start;
    nfd_start.exec();   //打开上一步的对话框
}

void Filedlg::on_pushButton_5_clicked()   //MD5编码生成按钮
{
    ifstream in(string((const char *)Filedlg::Input_File_Path.toLocal8Bit()), ios::binary);   //定义文件输入流
    if (!in)
        return;
    MD5 md5;   //定义MD5的对象
    std::streamsize length;
    char buffer[1024];   //每次读取1024字节
    while (!in.eof()) {   //循环读取文件，直至读取完
        in.read(buffer, 1024);
        length = in.gcount();
        if (length > 0)
            md5.update(buffer, length);   //调用MD5类的更新MD5编码函数
    }
    in.close();   //关闭文件输入流
    ui->textEdit_5->setText(QString(QString::fromLocal8Bit(md5.toString().c_str())));   //显示生成的MD5编码
}

void Filedlg::on_pushButton_7_clicked()   //模式2下显示文件版本编码按钮
{
    if (File_Version.isEmpty())
    {
       ui->textEdit_3->setText("empty!");   //还未生成文件版本编码
    }
    else
    {
       ui->textEdit_3->setText(File_Version);   //显示文件版本编码
    }
}

void Filedlg::PipelineSpeedDisplay(QString DisplayString)   //上传速度显示函数
{
    if (Filedlg::PublishMode==1)
    {
        ui->label_8->setText(DisplayString);   //模式1的速度显示
    }
    else if (Filedlg::PublishMode==2)
    {
        ui->label_4->setText(DisplayString);   //模式2的速度显示
    }
    PublishedSegmentDisplay();   //调用已上传数据包显示函数
}

void Filedlg::SystemTimeDisplay()   //系统时间显示函数
{
    QString Timestr=time.currentDateTime().toString("yyyy-MM-dd hh:mm:ss ddd");   //设置显示格式
    ui->label_11->setText(Timestr);   //显示系统时间
}

void Filedlg::PublishedSegmentDisplay()
{
    uint64_t DisplayNo;   //已上传数据包数
    DisplayNo=ndn::segment::Publisher::PublishedSegment;
    QString DisplayString;
    DisplayString=QString::number(DisplayNo,10);
    if (Filedlg::PublishMode==1)   //模式1显示已上传数据包数
    {
        uint64_t DisplayNo2;
        DisplayNo2=ndn::segment::Publisher::WholeFileSegmentNo;
        DisplayString=DisplayString+"/"+QString::number(DisplayNo2,10);
        ui->label_7->setText(DisplayString);
        ui->progressBar->setValue(DisplayNo);
    }
    else if (Filedlg::PublishMode==2)   //模式2显示已上传数据包数
    {
        ui->label_10->setText(DisplayString);
    }
}

void Filedlg::on_pushButton_8_clicked()   //文件分块按钮
{
    ndn::segment::Publisher::SegmentFile();   //调用文件分块函数

    uint64_t DisplayNo;
    DisplayNo=ndn::segment::Publisher::WholeFileSegmentNo;
    ui->progressBar->setMaximum(DisplayNo);   //进度条设置
    ui->progressBar->setValue(0);
    QString DisplayString;
    DisplayString=QString::number(DisplayNo,10);

    ui->textEdit_4->setText(DisplayString);   //显示文件分块数
    ui->textEdit_7->setText(DisplayString);
    DisplayString="0/"+DisplayString;
    ui->label_7->setText(DisplayString);
}

void Filedlg::on_pushButton_9_clicked()   //模式1上传按钮
{
    ndn::segment::Publisher::SegmentNoForTimer=0;   //计时器的数据包计数清零
    ndn::segment::Publisher::PublishedSegment=0;   //已上传的数据包计数清零
    int a=buttongroup.checkedId();   //获取安按键组ID
    if (a==0)   //上传模式1
    {
        PublishMode=1;

        if (PublishedNo==0)   //第一个上传的数据包
        {
            emit this->PutSegmentSignal();   //发出开始上传数据包信号
            connect(timer2,SIGNAL(timeout()),CalCulateSpeed2,SLOT(CalculateSpeed()));   //连接计时器2的超时信号和计算上传速度对象的计算速度槽函数
            timer2->start(500);   //开启定时器2（设定时间为500ms）
        }
        else
        {
            emit this->PutSegmentSignal();   //发出开始上传数据包信号
        }
        PublishedNo++;   //上传数据包数加一
    }
    else if (a==1)   //上传模式2
    {
        PublishMode=2;
        QMessageBox::information(this, "Tips", "Please choose the publish mode!", QMessageBox::Ok);
    }
    else if (a==-1)   //未选择上传模式
    {
        QMessageBox::information(this, "Tips", "Please choose the publish mode!", QMessageBox::Ok);
    }
}

void Filedlg::on_pushButton_10_clicked()   //模式1下显示文件版本编码按钮
{
    if (File_Version.isEmpty())
    {
       ui->textEdit_8->setText("empty!");    //还未生成文件版本编码
    }
    else
    {
       ui->textEdit_8->setText("/"+File_Version);   //显示文件版本编码
    }
}
