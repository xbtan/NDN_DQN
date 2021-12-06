#ifndef NFD_START_H
#define NFD_START_H

#include <QDialog>
#include <QButtonGroup>

namespace Ui {
class Nfd_Start;
}

class Nfd_Start : public QDialog
{
    Q_OBJECT

public:
    explicit Nfd_Start(QWidget *parent = 0);
    ~Nfd_Start();

    QButtonGroup buttongroup;

private slots:
    void on_pushButton_clicked();

    void on_commandLinkButton_clicked();

private:
    Ui::Nfd_Start *ui;
};

#endif // NFD_START_H
