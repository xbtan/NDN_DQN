#ifndef PIPELINECALCULATEUPLOADSPEED_H
#define PIPELINECALCULATEUPLOADSPEED_H

#include <qthread.h>
#include <QObject>
#include <QThread>
#include <qstring.h>


class PipelineCalCulateUploadSpeed : public QThread
{
    Q_OBJECT

signals:
    void BeginSpeedDisplay(QString Str);

public slots:
  void CalculateSpeed();
};


#endif
