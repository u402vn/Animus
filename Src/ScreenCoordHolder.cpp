#include "ScreenCoordHolder.h"
#include <QDebug>
#include "Map/HeightMapContainer.h"
#include "Common/CommonUtils.h"

ScreenCoordHolder::ScreenCoordHolder(QObject *parent) : QObject(parent)
{
    _trackedTargetGPSCoord.setIncorrect();
    _rangefinderPosition.setIncorrect();
    _bombingPlacePos.setIncorrect();
    recalculateTargetVector();
}

void ScreenCoordHolder::setTelemetryDataFrame(const TelemetryDataFrame &telemetryFrame)
{
    _telemetryFrame = telemetryFrame;

    _rangefinderPosition = getRangefinderCoordsFromTelemetry(_telemetryFrame);
    _trackedTargetGPSCoord = getTrackedTargetCoordsFromTelemetry(_telemetryFrame);
    _bombingPlacePos = getBimbingPlaceCoordsFromTelemetry(_telemetryFrame);

    recalculateTargetVector();

    emit changed();
}

const TelemetryDataFrame ScreenCoordHolder::getTelemetryDataFrame() const
{
    return _telemetryFrame;
}

const WorldGPSCoord ScreenCoordHolder::getBombingPlacePos() const
{
    return _bombingPlacePos;
}

void ScreenCoordHolder::recalculateTargetVector()
{
    const WorldGPSCoord uavCoords = getUavCoords();
    const WorldGPSCoord bombingPlacePos = getBombingPlacePos();

    _vectorToTargetExists = !uavCoords.isIncorrect() && !bombingPlacePos.isIncorrect();

    if (!_vectorToTargetExists)
    {
        _distanceToTarget = 0.0f;
        _azimuthToTarget = 360.0f;
        _azimuthUAVToTarget = 0.0f;
    }
    else
    {
        double distanceToTarget, azimuthToTarget;
        uavCoords.getDistanceAzimuthTo(bombingPlacePos, distanceToTarget, azimuthToTarget);
        _distanceToTarget = distanceToTarget;
        _azimuthToTarget = azimuthToTarget;

        _azimuthUAVToTarget = _azimuthToTarget;
        if (_azimuthUAVToTarget != _azimuthUAVToTarget) //NaN
            _azimuthUAVToTarget = 0;
        _azimuthUAVToTarget = constrainAngle180(_azimuthUAVToTarget - _telemetryFrame.Course_GPS);
    }
}


float ScreenCoordHolder::distanceToTarget() const
{
    return _distanceToTarget;
}

float ScreenCoordHolder::azimuthToTarget() const
{
    return _azimuthToTarget;
}

float ScreenCoordHolder::azimuthUAVToTarget() const
{
    return _azimuthUAVToTarget;
}

bool ScreenCoordHolder::vectorToTargetExists() const
{
    return _vectorToTargetExists;
}

const WorldGPSCoord ScreenCoordHolder::getUavCoords() const
{
    return getUavCoordsFromTelemetry(_telemetryFrame);
}

const WorldGPSCoord ScreenCoordHolder::getRangefinderPointCoords() const
{
    return _rangefinderPosition;
}

const WorldGPSCoord ScreenCoordHolder::getTrackedTargetCoords() const
{
    return _trackedTargetGPSCoord;
}
