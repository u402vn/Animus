#include "BallisticCalculator.h"
#include <QScriptContext>
#include "Common/CommonUtils.h"

BallisticCalculator::BallisticCalculator(QObject *parent, const QString &ballisticMacro) : QObject(parent)
{
    _bombTypes << tr("F-1") << tr("RGD-5");
    _ballisticMacro = ballisticMacro;
    // _scriptEngine.checkSyntax
}

const QStringList &BallisticCalculator::getBombTypes() const
{
    return _bombTypes;
}

void outPropertyToDebug(const QScriptValue &activationObject, const QString name)
{
    QScriptValue property = activationObject.property(name);
    if (property.isNumber())
        qDebug() << name << " = " << property.toNumber();
    else
        qDebug() << name << " = ?";
}

float BallisticCalculator::remainingTimeToDropBomb(const TelemetryDataFrame &telemetryDataFrame)
{
    auto targetPos = getBombingPlaceCoordsFromTelemetry(telemetryDataFrame);

    if (targetPos.isIncorrect() || telemetryDataFrame.TelemetryFrameNumber <= 0)
        return -1;

    WorldGPSCoord UAVPos(telemetryDataFrame.UavLatitude_GPS, telemetryDataFrame.UavLongitude_GPS, telemetryDataFrame.UavAltitude_GPS);
    double distanceToTarget, azimuthToToTarget;
    UAVPos.getDistanceAzimuthTo(targetPos, distanceToTarget, azimuthToToTarget);

    QScriptContext* context = _scriptEngine.currentContext();

    QScriptValue activationObject = context->activationObject();

    activationObject.setProperty("wind_direction", telemetryDataFrame.WindDirection);
    activationObject.setProperty("wind_speed", telemetryDataFrame.WindSpeed);

    activationObject.setProperty("uav_lat", telemetryDataFrame.UavLatitude_GPS);
    activationObject.setProperty("uav_lon", telemetryDataFrame.UavLongitude_GPS);
    activationObject.setProperty("uav_hmsl", telemetryDataFrame.UavAltitude_GPS);
    activationObject.setProperty("uav_groundspeed", telemetryDataFrame.GroundSpeed_GPS);
    activationObject.setProperty("uav_airspeed", telemetryDataFrame.AirSpeed);
    activationObject.setProperty("uav_course", telemetryDataFrame.Course_GPS);
    activationObject.setProperty("uav_verticalspeed", telemetryDataFrame.VerticalSpeed);

    activationObject.setProperty("target_lat", targetPos.lat);
    activationObject.setProperty("target_lon", targetPos.lon);
    activationObject.setProperty("target_hmsl", targetPos.hmsl);

    activationObject.setProperty("target_distance", distanceToTarget);
    activationObject.setProperty("target_azimuth", azimuthToToTarget);

    activationObject.setProperty("droppoint_time", 0);
    activationObject.setProperty("droppoint_distance", 0);

    activationObject.setProperty("debug_info", QString());

    _scriptEngine.evaluate(_ballisticMacro);

    QScriptValue scriptDropPointTimeProperty = activationObject.property("droppoint_time");
    qsreal timeToDrop = -1;
    if (scriptDropPointTimeProperty.isNumber())
        timeToDrop = scriptDropPointTimeProperty.toNumber();

    QScriptValue scriptDebugInfoProperty = activationObject.property("debug_info");
    if (!scriptDebugInfoProperty.isNull())
    {
        QString info = scriptDebugInfoProperty.toString();
        if (!info.isEmpty())
            qDebug() << "TFN: " << telemetryDataFrame.TelemetryFrameNumber << " DebugInfo: " << info;
    }

    /*
    outPropertyToDebug(activationObject, "uav_hmsl");
    outPropertyToDebug(activationObject, "uav_groundspeed");
    outPropertyToDebug(activationObject, "target_hmsl");
    outPropertyToDebug(activationObject, "target_distance");
    outPropertyToDebug(activationObject, "droppoint_time");
    outPropertyToDebug(activationObject, "droppoint_distance");
*/
    return timeToDrop;
}
