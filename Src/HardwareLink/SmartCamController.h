#ifndef SMARTCAMCONTROLLER_H
#define SMARTCAMCONTROLLER_H

#include <QObject>

class SmartCamController : public QObject
{
    Q_OBJECT
public:
    explicit SmartCamController(QObject *parent);

    void lockTarget(const QPoint &targetCenter);
    void unlockTarget();
    void setTargetSize(int targetSize);
};

#endif // SMARTCAMCONTROLLER_H
