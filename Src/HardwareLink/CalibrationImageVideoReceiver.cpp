#include "CalibrationImageVideoReceiver.h"
#include <QDebug>
#include "EnterProc.h"

void CalibrationImageVideoReceiver::timerEvent(QTimerEvent *event)
{
    emit frameAvailable(_staticImage, _videoConnectionId);
}

CalibrationImageVideoReceiver::CalibrationImageVideoReceiver(QObject *parent, quint32 videoConnectionId, const QString &selectedImage, const QString &defaultImage) : QObject(parent)
{
    _videoConnectionId = videoConnectionId;
    QImage staticImage(selectedImage);
    if (staticImage.isNull())
    {
        qDebug() << "Incorrect calibration image: " << selectedImage;
        staticImage.load(defaultImage);
        if (staticImage.isNull())
            qDebug() << "Incorrect calibration image: " << defaultImage;
    }
    _staticImage = staticImage.convertToFormat(QImage::Format_RGB32);

    this->startTimer(40); //ms
}
