#ifndef GSITRACKEDOBJECT_H
#define GSITRACKEDOBJECT_H

#include <QGraphicsItem>
#include <QGraphicsLineItem>
#include <QGraphicsPathItem>
#include <QGraphicsScene>
#include <QString>
#include "MarkerStorage.h"
#include "TelemetryDataFrame.h"

class GSITrackedObject : public QGraphicsPixmapItem
{
    QString _imageName;
public:
    explicit GSITrackedObject(QGraphicsScene *scene);
public:
    ~GSITrackedObject();
    void updateForTelemetry(const TelemetryDataFrame &telemetryFrame);
    void setImageName(const QString &name);
    const QString imageName();
};

#endif // GSITRACKEDOBJECT_H
