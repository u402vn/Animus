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
protected:
    void timerEvent(QTimerEvent *event);
public:
    explicit CalibrationImageVideoReceiver(QObject *parent, const QString &selectedImage, const QString &defaultImage);
signals:
    void frameAvailable(const QImage &frame);
};

#endif // CALIBRATIONIMAGEVIDEORECEIVER_H
