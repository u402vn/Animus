#ifndef SCREENCOORDHOLDER_H
#define SCREENCOORDHOLDER_H

#include <QObject>
#include <QPoint>
#include <QRect>
#include "Common/CommonData.h"
#include "TelemetryDataFrame.h"
#include "CamPreferences.h"
#include "Map/HeightMapContainer.h"

class ScreenCoordHolder final : public QObject
{
    Q_OBJECT

    QRect _trackedTargetRect;
    WorldGPSCoord _trackedTargetGPSCoord;

    WorldGPSCoord _bombingPlacePos;

    TelemetryDataFrame _telemetryFrame;

    WorldGPSCoord _rangefinderPosition;

    float _distanceToTarget, _azimuthToTarget;
    float _azimuthUAVToTarget; //доворот на цель для бомбардировки, directionAngle
    bool _vectorToTargetExists;

    void recalculateTargetVector();
public:
    explicit ScreenCoordHolder(QObject *parent);

    void setTelemetryDataFrame(const TelemetryDataFrame &telemetryFrame);
    const TelemetryDataFrame getTelemetryDataFrame() const;

    const WorldGPSCoord getBombingPlacePos() const;

    float distanceToTarget() const;
    float azimuthToTarget() const;
    float azimuthUAVToTarget() const;
    bool vectorToTargetExists() const;

    const QList<WorldGPSCoord> getVisibleArea();

    const WorldGPSCoord getRangefinderPointCoords() const;
    const WorldGPSCoord getTrackedTargetCoords() const;

    const WorldGPSCoord getUavCoords() const;
signals:
    void changed();
};

#endif // SCREENCOORDHOLDER_H
