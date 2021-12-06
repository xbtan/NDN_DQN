#include "nfd_start.h"
#include "ui_nfd_start.h"
#include <QMessageBox>
#include "qprocess.h"
#include "filedlg.h"
#include "filedlg1.h"
#include "mainwindow.h"
#include "ui_mainwindow.h"

Nfd_Start::Nfd_Start(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Nfd_Start)
{
    ui->setupUi(this);

    buttongroup.addButton(ui->radioButton_2);
    buttongroup.setId(ui->radioButton_2,0);
    buttongroup.addButton(ui->radioButton);
    buttongroup.setId(ui->radioButton,1);
}

Nfd_Start::~Nfd_Start()
{
    delete ui;
}

void Nfd_Start::on_pushButton_clicked()
{
    QProcess process(this);

    process.setProcessChannelMode( QProcess::MergedChannels );
    process.start("gnome-terminal");
    process.waitForFinished();
}

void Nfd_Start::on_commandLinkButton_clicked()
{
    Filedlg filedlg;
    Filedlg1 filedlg1;
    filedlg.setAttribute(Qt::WA_DeleteOnClose);
    filedlg1.setAttribute(Qt::WA_DeleteOnClose);

    int a=buttongroup.checkedId();
    if (a==0)
    {
        filedlg.exec();
    }
    else if (a==1)
    {
        filedlg1.exec();
    }
    else if (a==-1)
    {
        QMessageBox::information(this, "Tips", "Please choose the function!", QMessageBox::Ok);
    }
}
