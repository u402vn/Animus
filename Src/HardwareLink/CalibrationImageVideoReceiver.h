#ifndef CALIBRATIONIMAGEVIDEORECEIVER_H
#define CALIBRATIONIMAGEVIDEORECEIVER_H

#include <QObject>
#include <QThread>
#include <QLibrary>
#include <QImage>

class CalibrationImageVideoReceiver final : public QObject
{
    Q_OBJECT

    QImage _staticImage;
    quint32 _videoConnectionId;
protected:
    void timerEvent(QTimerEvent *event);
public:
    explicit CalibrationImageVideoReceiver(QObject *parent, quint32 videoConnectionId, const QString &selectedImage, const QString &defaultImage);
signals:
    void frameAvailable(const QImage &frame, quint32 videoConnectionId);
};

#endif // CALIBRATIONIMAGEVIDEORECEIVER_H
