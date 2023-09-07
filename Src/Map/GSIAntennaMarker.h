#ifndef GSIANTENNAMARKER_H
#define GSIANTENNAMARKER_H

#include <QGraphicsItem>
#include <QGraphicsLineItem>
#include <QGraphicsPathItem>
#include <QGraphicsScene>
#include <QString>
#include "MarkerStorage.h"
#include "TelemetryDataFrame.h"

class GSIAntennaMarker : public QGraphicsPixmapItem
{
public:
    explicit GSIAntennaMarker(QGraphicsScene *scene);
public:
    ~GSIAntennaMarker();
    void updateForTelemetry(const TelemetryDataFrame &telemetryFrame);
};

#endif // GSIANTENNAMARKER_H
