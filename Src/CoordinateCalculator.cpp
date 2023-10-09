#include "CoordinateCalculator.h"
#include <QScriptContext>
#include "Common/CommonUtils.h"

//https://coderoad.ru/17578881/%D0%9A%D0%B0%D0%BA-%D0%BF%D0%BE%D0%B2%D0%B5%D1%80%D0%BD%D1%83%D1%82%D1%8C-QQuaternion-%D0%BD%D0%B0-%D0%B4%D0%B2%D1%83%D1%85-%D0%BE%D1%81%D1%8F%D1%85
//https://forum.qt.io/topic/106798/convention-for-qquaternion-conversion-to-from-euler-angles/6
WorldGPSCoord CalculatePointPosition(const TelemetryDataFrame &telemetryDataFrame,
                                     CamPreferences *camPreferences,
                                     const int screenX, const int screenY,
                                     const double groundLevel
                                     )
{
    double angleXRad, angleYRad;
    camPreferences->getScreenPointAnglesRad(telemetryDataFrame.CamZoom, screenX, screenY, angleXRad, angleYRad);
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

const WorldGPSCoord CoordinateCalculator::getScreenPointCoord(int x, int y) const
{
    double groundLevel = _telemetryFrame->CalculatedGroundLevel;
    auto camPreferences = _camAssemblyPreferences->device(_telemetryFrame->OpticalSystemId);
    WorldGPSCoord result = CalculatePointPosition(*_telemetryFrame, camPreferences, x, y, groundLevel);
    return result;
}

CoordinateCalculator::CoordinateCalculator(QObject *parent, HeightMapContainer *heightMapContainer): QObject(parent),
    _heightMapContainer(heightMapContainer)
{
    ApplicationSettings& applicationSettings = ApplicationSettings::Instance();
    _ballisticMacro = applicationSettings.BallisticMacro;
    _useLaserRangefinderForGroundLevelCalculation =
            applicationSettings.UseLaserRangefinderForGroundLevelCalculation && applicationSettings.isLaserRangefinderLicensed();
    _useBombCaclulation = applicationSettings.isBombingTabLicensed();

    _targetSpeedFrames = new TelemetryDelayLine(this, 1000); //???
    //connect(_targetSpeedFrames, &TelemetryDelayLine::dequeue, this, &CoordinateCalculator::onTelemetryDelayLineDequeue, Qt::DirectConnection);
}

CoordinateCalculator::~CoordinateCalculator()
{

}

void CoordinateCalculator::updateLaserRangefinderPosition()
{
    _telemetryFrame->CalculatedRangefinderGPSLat = _telemetryFrame->UavLatitude_GPS;
    _telemetryFrame->CalculatedRangefinderGPSLon = _telemetryFrame->UavLongitude_GPS;
    _telemetryFrame->CalculatedRangefinderGPSHmsl = _telemetryFrame->UavAltitude_GPS;

    bool validRangefinderDistance = (_telemetryFrame->RangefinderDistance > 0);

    if (validRangefinderDistance)
    {
        const QQuaternion camQ = _telemetryFrame->getCamQuaternion();
        QVector3D v(0, 0, _telemetryFrame->RangefinderDistance);
        v = camQ * v;
        _telemetryFrame->CalculatedRangefinderGPSLat += v.x() * 180 / (EARTH_RADIUS_M * PI);
        _telemetryFrame->CalculatedRangefinderGPSLon += v.y() * 180 / (cos(_telemetryFrame->UavLatitude_GPS * PI / 180) * EARTH_RADIUS_M * PI);
        _telemetryFrame->CalculatedRangefinderGPSHmsl -= v.z();
    }
    else
        _telemetryFrame->CalculatedRangefinderGPSHmsl = INCORRECT_COORDINATE; //ground level by default

    if (_telemetryFrame->TelemetryFrameNumber <= 0)
    {
        _telemetryFrame->CalculatedRangefinderGPSLat = INCORRECT_COORDINATE;
        _telemetryFrame->CalculatedRangefinderGPSLon = INCORRECT_COORDINATE;
        _telemetryFrame->CalculatedRangefinderGPSHmsl = INCORRECT_COORDINATE;
    }
}

void CoordinateCalculator::updateGroundLevel()
{
    bool canCalculateFromRangefinder = _useLaserRangefinderForGroundLevelCalculation &&
            (_telemetryFrame->CalculatedRangefinderGPSHmsl != INCORRECT_COORDINATE) &&
            ( _telemetryFrame->CalculatedRangefinderGPSLat != INCORRECT_COORDINATE) &&
            ( _telemetryFrame->CalculatedRangefinderGPSLon != INCORRECT_COORDINATE);

    _telemetryFrame->CalculatedGroundLevel = 0;

    if (canCalculateFromRangefinder)
        _telemetryFrame->CalculatedGroundLevel = _telemetryFrame->CalculatedRangefinderGPSHmsl;
    else
    {
        double heightFromMap;
        bool canCalulateFromMap = _heightMapContainer->GetHeight(_telemetryFrame->UavLatitude_GPS, _telemetryFrame->UavLongitude_GPS, heightFromMap);
        if (canCalulateFromMap)
            _telemetryFrame->CalculatedGroundLevel = heightFromMap;
    };
}

void CoordinateCalculator::updateTrackedTargetPosition()
{
    WorldGPSCoord coord;

    if (_telemetryFrame->targetIsVisible())
        coord = getScreenPointCoord(_telemetryFrame->TrackedTargetCenterX, _telemetryFrame->TrackedTargetCenterY);
    else
        coord.setIncorrect();

    _telemetryFrame->CalculatedTrackedTargetGPSLat = coord.lat;
    _telemetryFrame->CalculatedTrackedTargetGPSLon = coord.lon;
    _telemetryFrame->CalculatedTrackedTargetGPSHmsl = coord.hmsl;

    _telemetryFrame->CalculatedTrackedTargetSpeed = 0;
    _telemetryFrame->CalculatedTrackedTargetDirection = 0;
    if (!_targetSpeedFrames->isEmpty())
    {
        auto prevTelemetryFrame = _targetSpeedFrames->head();
        auto prevCoord = getTrackedTargetCoordsFromTelemetry(prevTelemetryFrame);
        double distance, azimut;
        if (prevCoord.getDistanceAzimuthTo(coord, distance, azimut))
        {
            auto time = _telemetryFrame->SessionTimeMs - prevTelemetryFrame.SessionTimeMs;
            _telemetryFrame->CalculatedTrackedTargetSpeed = distance / (0.001 * time);
            _telemetryFrame->CalculatedTrackedTargetDirection = azimut;
        }
    }
}

void CoordinateCalculator::updateViewFieldBorderPoints()
{
    constexpr double ViewFieldBorderPoints[ViewFieldBorderPointsCount][2] = {
        {0.0, 0.0}, {0.1, 0.0}, {0.2, 0.0}, {0.3, 0.0}, {0.4, 0.0}, {0.5, 0.0}, {0.6, 0.0}, {0.7, 0.0}, {0.8, 0.0}, {0.9, 0.0},
        {1.1, 0.0}, {1.1, 0.1}, {1.1, 0.2}, {1.1, 0.3}, {1.1, 0.4}, {1.1, 0.5}, {1.1, 0.6}, {1.1, 0.7}, {1.1, 0.8}, {1.1, 0.9},
        {1.1, 1.1}, {0.9, 1.1}, {0.8, 1.1}, {0.7, 1.1}, {0.6, 1.1}, {0.5, 1.1}, {0.4, 1.1}, {0.3, 1.1}, {0.2, 1.1}, {0.1, 1.1},
        {0.0, 1.1}, {0.0, 0.9}, {0.0, 0.8}, {0.0, 0.7}, {0.0, 0.6}, {0.0, 0.5}, {0.0, 0.4}, {0.0, 0.3}, {0.0, 0.2}, {0.0, 0.1}};

    auto camPreferences = _camAssemblyPreferences->device(_telemetryFrame->OpticalSystemId);
    int imageWidth  = camPreferences->frameWidth();
    int imageHeight = camPreferences->frameHeight();
    for (int n = 0; n < ViewFieldBorderPointsCount; n++)
    {
        auto point = getScreenPointCoord(ViewFieldBorderPoints[n][0] * imageWidth, ViewFieldBorderPoints[n][1] * imageHeight);
        _telemetryFrame->ViewFieldBorderPointsLat[n] = point.lat;
        _telemetryFrame->ViewFieldBorderPointsLon[n] = point.lon;
        _telemetryFrame->ViewFieldBorderPointsHmsl[n] = point.hmsl;
    }
}

bool CoordinateCalculator::needUpdateBombingData()
{
    bool needUpdate = _useBombCaclulation &&
            !(  _telemetryFrame->TelemetryFrameNumber <= 0 ||
                _telemetryFrame->UavLongitude_GPS == INCORRECT_COORDINATE ||
                _telemetryFrame->UavLongitude_GPS == INCORRECT_COORDINATE ||
                _telemetryFrame->UavAltitude_GPS == INCORRECT_COORDINATE ||
                _telemetryFrame->BombingPlacePosLat == INCORRECT_COORDINATE ||
                _telemetryFrame->BombingPlacePosLon == INCORRECT_COORDINATE ||
                _telemetryFrame->BombingPlacePosHmsl == INCORRECT_COORDINATE);
    return needUpdate;
}

void CoordinateCalculator::updateBombingData()
{
    if (needUpdateBombingData())
    {
        double distanceToTarget, azimuthToTarget;
        WorldGPSCoord uavCoords(_telemetryFrame->UavLatitude_GPS, _telemetryFrame->UavLongitude_GPS, _telemetryFrame->UavAltitude_GPS);
        WorldGPSCoord bombingPlaceCoords(_telemetryFrame->BombingPlacePosLat, _telemetryFrame->BombingPlacePosLon, _telemetryFrame->BombingPlacePosHmsl);
        uavCoords.getDistanceAzimuthTo(bombingPlaceCoords, distanceToTarget, azimuthToTarget);
        _telemetryFrame->DistanceToBombingPlace = distanceToTarget;
        _telemetryFrame->AzimuthToBombingPlace = azimuthToTarget;
        _telemetryFrame->AzimuthUAVToBombingPlace = constrainAngle180(azimuthToTarget - _telemetryFrame->Course_GPS);
    }
    else
    {
        _telemetryFrame->DistanceToBombingPlace = INCORRECT_DISTANCE;
        _telemetryFrame->AzimuthToBombingPlace = 0;
        _telemetryFrame->AzimuthUAVToBombingPlace = 0;
    }
}

void CoordinateCalculator::updateRemainingTimeToDropBomb()
{
    if (!needUpdateBombingData())
    {
        _telemetryFrame->RemainingTimeToDropBomb = INCORRECT_TIME;
        return;
    }

    auto context = _scriptEngine.currentContext();
    auto activationObject = context->activationObject();

    activationObject.setProperty("wind_direction", _telemetryFrame->WindDirection);
    activationObject.setProperty("wind_speed",_telemetryFrame->WindSpeed);

    activationObject.setProperty("uav_lat", _telemetryFrame->UavLatitude_GPS);
    activationObject.setProperty("uav_lon", _telemetryFrame->UavLongitude_GPS);
    activationObject.setProperty("uav_hmsl", _telemetryFrame->UavAltitude_GPS);
    activationObject.setProperty("uav_groundspeed", _telemetryFrame->GroundSpeed_GPS);
    activationObject.setProperty("uav_airspeed", _telemetryFrame->AirSpeed);
    activationObject.setProperty("uav_course", _telemetryFrame->Course_GPS);
    activationObject.setProperty("uav_verticalspeed", _telemetryFrame->VerticalSpeed);

    activationObject.setProperty("target_lat", _telemetryFrame->BombingPlacePosLat);
    activationObject.setProperty("target_lon", _telemetryFrame->BombingPlacePosLon);
    activationObject.setProperty("target_hmsl", _telemetryFrame->BombingPlacePosHmsl);

    activationObject.setProperty("target_distance", _telemetryFrame->DistanceToBombingPlace);
    activationObject.setProperty("target_azimuth", _telemetryFrame->AzimuthToBombingPlace);

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
            qDebug() << "TFN: " << _telemetryFrame->TelemetryFrameNumber << " DebugInfo: " << info;
    }

    /*
    outPropertyToDebug(activationObject, "uav_hmsl");
    outPropertyToDebug(activationObject, "uav_groundspeed");
    outPropertyToDebug(activationObject, "target_hmsl");
    outPropertyToDebug(activationObject, "target_distance");
    outPropertyToDebug(activationObject, "droppoint_time");
    outPropertyToDebug(activationObject, "droppoint_distance");
    */

    _telemetryFrame->RemainingTimeToDropBomb = timeToDrop;
}

void CoordinateCalculator::updateTargetSpeedFrames()
{
    if (_telemetryFrame->TelemetryFrameNumber <= 1)
        _targetSpeedFrames->clear(); //clean collected data from previous session

    if (!getTrackedTargetCoordsFromTelemetry(*_telemetryFrame).isIncorrect())
        _targetSpeedFrames->enqueue(*_telemetryFrame);
}

void CoordinateCalculator::processTelemetryDataFrame(TelemetryDataFrame *telemetryFrame)
{
    ApplicationSettings& applicationSettings = ApplicationSettings::Instance();

    _telemetryFrame = telemetryFrame;
    _camAssemblyPreferences = applicationSettings.getCurrentCamAssemblyPreferences();

    updateLaserRangefinderPosition();
    updateGroundLevel();
    updateTrackedTargetPosition();
    updateViewFieldBorderPoints();
    updateBombingData();
    updateRemainingTimeToDropBomb();
    updateTargetSpeedFrames();
}
