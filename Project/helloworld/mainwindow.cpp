#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QApplication>
#include "nfd_start.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    return a.exec();
}

void MainWindow::on_commandLinkButton_3_clicked()
{
    Nfd_Start nfdstart;
    nfdstart.exec();
}
