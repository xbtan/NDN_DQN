#include <qthread.h>
#include <QObject>
#include <QThread>
#include <qstring.h>

class PipelineCalCulateSpeed : public QThread
{
    Q_OBJECT

signals:
    void BeginSpeedDisplay(QString Str);

public slots:
  void CalculateSpeed();
};
