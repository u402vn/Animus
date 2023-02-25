#ifndef AUTOMATICPATROL_H
#define AUTOMATICPATROL_H

#include "AutomaticTracer.h"
#include <QObject>


class AutomaticPatrol : public QObject
{
    Q_OBJECT

public:
    explicit AutomaticPatrol(QObject *parent);
    ~AutomaticPatrol();
signals:

public slots:
};

#endif // AUTOMATICPATROL_H
