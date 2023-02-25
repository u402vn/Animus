#ifndef GSIUAVMARKER_H
#define GSIUAVMARKER_H

#include <QGraphicsItem>
#include <QGraphicsLineItem>
#include <QGraphicsPathItem>
#include <QGraphicsScene>
#include <QString>
#include "MarkerStorage.h"
#include "TelemetryDataFrame.h"

class GSIUAVMarker : public QGraphicsPixmapItem
{
    QString _imageName;
    QGraphicsLineItem  *_laserLineItem;
    QGraphicsPathItem  *_viewFieldPathItem;
    QGraphicsPixmapItem *_windItem;
public:
    explicit GSIUAVMarker(QGraphicsScene *scene);
public:
    ~GSIUAVMarker();
    void updateForTelemetry(const TelemetryDataFrame &telemetryFrame);
    void setImageName(const QString &name);
    const QString imageName();
};
#endif // GSIUAVMARKER_H
