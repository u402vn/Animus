#include "CoordinateCalculator.h"
#include <QScriptContext>
#include "Common/CommonUtils.h"

//https://coderoad.ru/17578881/%D0%9A%D0%B0%D0%BA-%D0%BF%D0%BE%D0%B2%D0%B5%D1%80%D0%BD%D1%83%D1%82%D1%8C-QQuaternion-%D0%BD%D0%B0-%D0%B4%D0%B2%D1%83%D1%85-%D0%BE%D1%81%D1%8F%D1%85
//https://forum.qt.io/topic/106798/convention-for-qquaternion-conversion-to-from-euler-angles/6
WorldGPSCoord CalculatePointPosition(const TelemetryDataFrame &telemetryDataFrame,
                                     OpticalDevicePreferences *opticalDevice,
                                     const int screenX, const int screenY,
                                     const double groundLevel
                                     )
{
    double angleXRad, angleYRad;
    opticalDevice->getScreenPointAnglesRad(telemetryDataFrame.CamZoom, screenX, screenY, angleXRad, angleYRad);
    QQuaternion scrQ = QQuaternion::rotationTo(QVector3D(0, 0, 10), QVector3D(tan(angleYRad), -tan(angleXRad), 1));
    //    QQuaternion scrQ = QQuaternion::rotationTo(QVector3D(0, 0, 10), QVector3D(tan(angleXRad), tan(angleYRad), 1));

    const QQuaternion camQ = telemetryDataFrame.getCamQuaternion();

    QVector3D v(0, 0, 1);
    v = camQ * scrQ * v;

    WorldGPSCoord point_coord;

    double k = -1;
    if (v.z() > 0) //exclude devision on 0
        k = (telemetryDataFrame.UavAltitude_GPS - groundLevel) / v.z();
    double dx = v.x() * k;
    double dy = v.y() * k;

    const int MAX_VISIBLE_RANGE = 5000;

    if (k < 0 || dx > MAX_VISIBLE_RANGE || dy > MAX_VISIBLE_RANGE)
        point_coord.setIncorrect();
    else
    {
        point_coord.lat = telemetryDataFrame.UavLatitude_GPS + dx * 180 / (EARTH_RADIUS_M * PI);
        point_coord.lon = telemetryDataFrame.UavLongitude_GPS + dy * 180 / (cos(telemetryDataFrame.UavLatitude_GPS * PI / 180) * EARTH_RADIUS_M * PI);
        point_coord.hmsl = groundLevel;
    }

    if (telemetryDataFrame.TelemetryFrameNumber <= 0)
        point_coord.setIncorrect();

    return point_coord;
}


//---------------------------------------------------------------------------------------

const WorldGPSCoord CoordinateCalculator::getScreenPointCoord(TelemetryDataFrame *telemetryFrame, int x, int y) const
{
    double groundLevel = telemetryFrame->CalculatedGroundLevel;
    auto camPreferences = _camAssemblyPreferences->opticalDevice(telemetryFrame->OpticalSystemId);
    WorldGPSCoord result = CalculatePointPosition(*telemetryFrame, camPreferences, x, y, groundLevel);
    return result;
}

CoordinateCalculator::CoordinateCalculator(QObject *parent, HeightMapContainer *heightMapContainer): QObject(parent),
    _heightMapContainer(heightMapContainer)
{
    ApplicationSettings& applicationSettings = ApplicationSettings::Instance();
    _ballisticMacro = applicationSettings.BallisticMacro;
    _useLaserRangefinderForGroundLevelCalculation = applicationSettings.UseLaserRangefinderForGroundLevelCalculation && applicationSettings.isLaserRangefinderLicensed();
    _useBombCaclulation = applicationSettings.isBombingTabLicensed();

    _trackedTargetSpeed = 0;
    _trackedTargetDirection = 0;
    _targetSpeedFrames = new TelemetryDelayLine(this, 1000); //???
    startTimer(1000);
}

CoordinateCalculator::~CoordinateCalculator()
{

}

void CoordinateCalculator::updateLaserRangefinderPosition(TelemetryDataFrame *telemetryFrame)
{
    telemetryFrame->CalculatedRangefinderGPSLat = telemetryFrame->UavLatitude_GPS;
    telemetryFrame->CalculatedRangefinderGPSLon = telemetryFrame->UavLongitude_GPS;
    telemetryFrame->CalculatedRangefinderGPSHmsl = telemetryFrame->UavAltitude_GPS;

    bool validRangefinderDistance = (telemetryFrame->RangefinderDistance > 0);

    if (validRangefinderDistance)
    {
        const QQuaternion camQ = telemetryFrame->getCamQuaternion();
        QVector3D v(0, 0, telemetryFrame->RangefinderDistance);
        v = camQ * v;
        telemetryFrame->CalculatedRangefinderGPSLat += v.x() * 180 / (EARTH_RADIUS_M * PI);
        telemetryFrame->CalculatedRangefinderGPSLon += v.y() * 180 / (cos(telemetryFrame->UavLatitude_GPS * PI / 180) * EARTH_RADIUS_M * PI);
        telemetryFrame->CalculatedRangefinderGPSHmsl -= v.z();
    }
    else
        telemetryFrame->CalculatedRangefinderGPSHmsl = INCORRECT_COORDINATE; //ground level by default

    if (telemetryFrame->TelemetryFrameNumber <= 0)
    {
        telemetryFrame->CalculatedRangefinderGPSLat = INCORRECT_COORDINATE;
        telemetryFrame->CalculatedRangefinderGPSLon = INCORRECT_COORDINATE;
        telemetryFrame->CalculatedRangefinderGPSHmsl = INCORRECT_COORDINATE;
    }
}

void CoordinateCalculator::updateGroundLevel(TelemetryDataFrame *telemetryFrame)
{
    bool canCalculateFromRangefinder = _useLaserRangefinderForGroundLevelCalculation &&
            (telemetryFrame->CalculatedRangefinderGPSHmsl != INCORRECT_COORDINATE) &&
            ( telemetryFrame->CalculatedRangefinderGPSLat != INCORRECT_COORDINATE) &&
            ( telemetryFrame->CalculatedRangefinderGPSLon != INCORRECT_COORDINATE);

    telemetryFrame->CalculatedGroundLevel = 0;

    if (canCalculateFromRangefinder)
        telemetryFrame->CalculatedGroundLevel = telemetryFrame->CalculatedRangefinderGPSHmsl;
    else
    {
        double heightFromMap;
        bool canCalulateFromMap = _heightMapContainer->GetHeight(telemetryFrame->UavLatitude_GPS, telemetryFrame->UavLongitude_GPS, heightFromMap);
        if (canCalulateFromMap)
            telemetryFrame->CalculatedGroundLevel = heightFromMap;
    };
}

void CoordinateCalculator::updateTrackedTargetPosition(TelemetryDataFrame *telemetryFrame)
{
    WorldGPSCoord coord;
    if (telemetryFrame->targetIsVisible())
        coord = getScreenPointCoord(telemetryFrame, telemetryFrame->TrackedTargetCenterX, telemetryFrame->TrackedTargetCenterY);
    else
        coord.setIncorrect();

    telemetryFrame->CalculatedTrackedTargetGPSLat = coord.lat;
    telemetryFrame->CalculatedTrackedTargetGPSLon = coord.lon;
    telemetryFrame->CalculatedTrackedTargetGPSHmsl = coord.hmsl;

    telemetryFrame->CalculatedTrackedTargetSpeed = _trackedTargetSpeed;
    telemetryFrame->CalculatedTrackedTargetDirection = _trackedTargetDirection;
}

void CoordinateCalculator::updateTrackedTargetSpeed()
{
    auto prevSpeed = _trackedTargetSpeed;

    _trackedTargetSpeed = 0;
    _trackedTargetDirection = 0;

    if (_targetSpeedFrames->isEmpty())
        return;

    auto headFrame = _targetSpeedFrames->head();
    auto tailFrame = _targetSpeedFrames->tail();

    if (!headFrame.targetIsVisible() || !tailFrame.targetIsVisible())
        return;

    auto headCoord = getTrackedTargetCoordsFromTelemetry(headFrame);
    auto tailCoord = getTrackedTargetCoordsFromTelemetry(tailFrame);

    double distance, azimut, speed;
    if (headCoord.getDistanceAzimuthTo(tailCoord, distance, azimut))
    {
        auto time = (tailFrame.SessionTimeMs - headFrame.SessionTimeMs) * 0.001;
        speed = distance / time;

        if ( (prevSpeed == 0) || (qAbs(speed - prevSpeed) < 10) )
        {
            _trackedTargetSpeed = distance / time;
            _trackedTargetDirection = azimut;
        }

    }
}

void CoordinateCalculator::updateViewFieldBorderPoints(TelemetryDataFrame *telemetryFrame)
{
    constexpr double ViewFieldBorderPoints[ViewFieldBorderPointsCount][2] = {
        {0.0, 0.0}, {0.1, 0.0}, {0.2, 0.0}, {0.3, 0.0}, {0.4, 0.0}, {0.5, 0.0}, {0.6, 0.0}, {0.7, 0.0}, {0.8, 0.0}, {0.9, 0.0},
        {1.1, 0.0}, {1.1, 0.1}, {1.1, 0.2}, {1.1, 0.3}, {1.1, 0.4}, {1.1, 0.5}, {1.1, 0.6}, {1.1, 0.7}, {1.1, 0.8}, {1.1, 0.9},
        {1.1, 1.1}, {0.9, 1.1}, {0.8, 1.1}, {0.7, 1.1}, {0.6, 1.1}, {0.5, 1.1}, {0.4, 1.1}, {0.3, 1.1}, {0.2, 1.1}, {0.1, 1.1},
        {0.0, 1.1}, {0.0, 0.9}, {0.0, 0.8}, {0.0, 0.7}, {0.0, 0.6}, {0.0, 0.5}, {0.0, 0.4}, {0.0, 0.3}, {0.0, 0.2}, {0.0, 0.1}};

    auto camPreferences = _camAssemblyPreferences->opticalDevice(telemetryFrame->OpticalSystemId);
    int imageWidth  = camPreferences->frameWidth();
    int imageHeight = camPreferences->frameHeight();
    for (int n = 0; n < ViewFieldBorderPointsCount; n++)
    {
        auto point = getScreenPointCoord(telemetryFrame, ViewFieldBorderPoints[n][0] * imageWidth, ViewFieldBorderPoints[n][1] * imageHeight);
        telemetryFrame->ViewFieldBorderPointsLat[n] = point.lat;
        telemetryFrame->ViewFieldBorderPointsLon[n] = point.lon;
        telemetryFrame->ViewFieldBorderPointsHmsl[n] = point.hmsl;
    }
}

bool CoordinateCalculator::needUpdateBombingData(TelemetryDataFrame *telemetryFrame)
{
    bool needUpdate = _useBombCaclulation &&
            !(  telemetryFrame->TelemetryFrameNumber <= 0 ||
                telemetryFrame->UavLongitude_GPS == INCORRECT_COORDINATE ||
                telemetryFrame->UavLongitude_GPS == INCORRECT_COORDINATE ||
                telemetryFrame->UavAltitude_GPS == INCORRECT_COORDINATE ||
                telemetryFrame->BombingPlacePosLat == INCORRECT_COORDINATE ||
                telemetryFrame->BombingPlacePosLon == INCORRECT_COORDINATE ||
                telemetryFrame->BombingPlacePosHmsl == INCORRECT_COORDINATE);
    return needUpdate;
}

void CoordinateCalculator::updateBombingData(TelemetryDataFrame *telemetryFrame)
{
    if (needUpdateBombingData(telemetryFrame))
    {
        double distanceToTarget, azimuthToTarget;
        WorldGPSCoord uavCoords(telemetryFrame->UavLatitude_GPS, telemetryFrame->UavLongitude_GPS, telemetryFrame->UavAltitude_GPS);
        WorldGPSCoord bombingPlaceCoords(telemetryFrame->BombingPlacePosLat, telemetryFrame->BombingPlacePosLon, telemetryFrame->BombingPlacePosHmsl);
        uavCoords.getDistanceAzimuthTo(bombingPlaceCoords, distanceToTarget, azimuthToTarget);
        telemetryFrame->DistanceToBombingPlace = distanceToTarget;
        telemetryFrame->AzimuthToBombingPlace = azimuthToTarget;
        telemetryFrame->AzimuthUAVToBombingPlace = constrainAngle180(azimuthToTarget - telemetryFrame->Course_GPS);
    }
    else
    {
        telemetryFrame->DistanceToBombingPlace = INCORRECT_DISTANCE;
        telemetryFrame->AzimuthToBombingPlace = 0;
        telemetryFrame->AzimuthUAVToBombingPlace = 0;
    }
}

void CoordinateCalculator::updateRemainingTimeToDropBomb(TelemetryDataFrame *telemetryFrame)
{
    if (!needUpdateBombingData(telemetryFrame))
    {
        telemetryFrame->RemainingTimeToDropBomb = INCORRECT_TIME;
        return;
    }

    auto context = _scriptEngine.currentContext();
    auto activationObject = context->activationObject();

    activationObject.setProperty("wind_direction", telemetryFrame->WindDirection);
    activationObject.setProperty("wind_speed",telemetryFrame->WindSpeed);

    activationObject.setProperty("uav_lat", telemetryFrame->UavLatitude_GPS);
    activationObject.setProperty("uav_lon", telemetryFrame->UavLongitude_GPS);
    activationObject.setProperty("uav_hmsl", telemetryFrame->UavAltitude_GPS);
    activationObject.setProperty("uav_groundspeed", telemetryFrame->GroundSpeed_GPS);
    activationObject.setProperty("uav_airspeed", telemetryFrame->AirSpeed);
    activationObject.setProperty("uav_course", telemetryFrame->Course_GPS);
    activationObject.setProperty("uav_verticalspeed", telemetryFrame->VerticalSpeed);

    activationObject.setProperty("target_lat", telemetryFrame->BombingPlacePosLat);
    activationObject.setProperty("target_lon", telemetryFrame->BombingPlacePosLon);
    activationObject.setProperty("target_hmsl", telemetryFrame->BombingPlacePosHmsl);

    activationObject.setProperty("target_distance", telemetryFrame->DistanceToBombingPlace);
    activationObject.setProperty("target_azimuth", telemetryFrame->AzimuthToBombingPlace);

    activationObject.setProperty("droppoint_time", 0);
    activationObject.setProperty("droppoint_distance", 0);

    activationObject.setProperty("debug_info", QString());

    _scriptEngine.evaluate(_ballisticMacro);

    QScriptValue scriptDropPointTimeProperty = activationObject.property("droppoint_time");
    double timeToDrop = -1;
    if (scriptDropPointTimeProperty.isNumber())
        timeToDrop = scriptDropPointTimeProperty.toNumber();

    QScriptValue scriptDebugInfoProperty = activationObject.property("debug_info");
    if (!scriptDebugInfoProperty.isNull())
    {
        QString info = scriptDebugInfoProperty.toString();
        if (!info.isEmpty())
            qDebug() << "TFN: " << telemetryFrame->TelemetryFrameNumber << " DebugInfo: " << info;
    }

    /*
    outPropertyToDebug(activationObject, "uav_hmsl");
    outPropertyToDebug(activationObject, "uav_groundspeed");
    outPropertyToDebug(activationObject, "target_hmsl");
    outPropertyToDebug(activationObject, "target_distance");
    outPropertyToDebug(activationObject, "droppoint_time");
    outPropertyToDebug(activationObject, "droppoint_distance");
    */

    telemetryFrame->RemainingTimeToDropBomb = timeToDrop;
}

void CoordinateCalculator::updateTargetSpeedFrames(TelemetryDataFrame *telemetryFrame)
{
    if (telemetryFrame->TelemetryFrameNumber <= 1)
        _targetSpeedFrames->clear(); //clean collected data from previous session

    if (!getTrackedTargetCoordsFromTelemetry(*telemetryFrame).isIncorrect())
        _targetSpeedFrames->enqueue(*telemetryFrame);
}

void CoordinateCalculator::timerEvent(QTimerEvent *event)
{
    updateTrackedTargetSpeed();
}

void CoordinateCalculator::processTelemetryDataFrame(TelemetryDataFrame *_telemetryFrame)
{
    ApplicationSettings& applicationSettings = ApplicationSettings::Instance();

    //???_telemetryFrame = telemetryFrame;
    _camAssemblyPreferences = applicationSettings.getCurrentCamAssemblyPreferences();

    updateLaserRangefinderPosition(_telemetryFrame);
    updateGroundLevel(_telemetryFrame);
    updateTrackedTargetPosition(_telemetryFrame);
    updateViewFieldBorderPoints(_telemetryFrame);
    updateBombingData(_telemetryFrame);
    updateRemainingTimeToDropBomb(_telemetryFrame);
    updateTargetSpeedFrames(_telemetryFrame);
}
