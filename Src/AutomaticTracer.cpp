#include "AutomaticTracer.h"
#include <QRect>
#include "Common/CommonUtils.h"

// http://we.easyelectronics.ru/Theory/pid-regulyatory--dlya-chaynikov-praktikov.html
// http://robot-develop.org/archives/2833
// http://pidcontrol.narod.ru/

AutomaticTracer::AutomaticTracer(QObject *parent, CamAssemblyPreferences *camAssemblyPreferences) : QObject(parent)
{
    _camAssemblyPreferences = camAssemblyPreferences;

    _tracerMode = atmSleep;

    _lastVideoFrameNumber = 0;
    _lastTelemetryFrameNumber = 0;
}

AutomaticTracer::~AutomaticTracer()
{
}

double calculateAngleSpeed(double offsetDegree, double automaticTracerMultiplier)
{
    double result = offsetDegree * automaticTracerMultiplier;
    return result;
}

double calculateAngleSpeed2(double offsetDegree, double encoderAutomaticTracerMultiplier)
{
    offsetDegree = constrainAngle180(offsetDegree);
    double result;
    double sign = offsetDegree > 0 ? 1 : -1;
    double abs = qAbs(offsetDegree);

    if (abs > 50)
        result = 80 * encoderAutomaticTracerMultiplier;
    else if (abs > 30)
        result = 40 * encoderAutomaticTracerMultiplier;
    else if (abs > 10)
        result = abs * encoderAutomaticTracerMultiplier;
    else if (abs > 2)
        result = (abs * encoderAutomaticTracerMultiplier) / 2;
    else
        result  = (abs * encoderAutomaticTracerMultiplier) / 4;

    result = result * sign;
    return result;
}

void AutomaticTracer::moveToScreenPoint(const TelemetryDataFrame &telemetryFrame, const QPoint &screenPoint, bool screenPointValid)
{
    //    if (telemetryFrame.VideoFrameNumber == _lastVideoFrameNumber ||
    //            telemetryFrame.TelemetryFrameNumber <= _lastTelemetryFrameNumber)
    //        return;
    if (telemetryFrame.VideoFrameNumber == _lastVideoFrameNumber)
        return;

    if (!screenPointValid)
    {
        stopRotation();
        return;
    }

    _lastVideoFrameNumber = telemetryFrame.VideoFrameNumber;
    _lastTelemetryFrameNumber = telemetryFrame.TelemetryFrameNumber;

    double angleXDegree, angleYDegree;
    qint32 zoom = telemetryFrame.CamZoom;

    auto camPreferences = _camAssemblyPreferences->opticalDevice(telemetryFrame.OpticalSystemId);
    double automaticTracerMultiplier = camPreferences->automaticTracerSpeedMultiplier(zoom);
    camPreferences->getScreenPointAnglesDegree(zoom, screenPoint.x(), screenPoint.y(), angleXDegree, angleYDegree);

    double speedXDegree = calculateAngleSpeed(-angleXDegree, automaticTracerMultiplier);
    double speedYDegree = calculateAngleSpeed(-angleYDegree, automaticTracerMultiplier);
    double speedZDegree = 0;

    emit onAutomaticCamMovingSpeedChange(speedZDegree, speedYDegree, speedXDegree);
}

void AutomaticTracer::moveToEncoderValues(const TelemetryDataFrame &telemetryFrame)
{
    double currRollDegree = encoderAngleDegree(telemetryFrame.CamEncoderRoll);
    double currPitchDegree = encoderAngleDegree(telemetryFrame.CamEncoderPitch);
    double currYawDegree = encoderAngleDegree(telemetryFrame.CamEncoderYaw);

    double offsetRoll = _destEncoderRollDegree - currRollDegree;
    double offsetPitch = _destEncoderPitchDegree - currPitchDegree;
    double offsetYaw = _destEncoderYawDegree - currYawDegree;

    double encoderAutomaticTracerMultiplier = _camAssemblyPreferences->encoderAutomaticTracerMultiplier();

    double rollSpeed = calculateAngleSpeed2(offsetRoll, encoderAutomaticTracerMultiplier);
    double pitchSpeed = calculateAngleSpeed2(offsetPitch, encoderAutomaticTracerMultiplier);
    double yawSpeed = calculateAngleSpeed2(offsetYaw, encoderAutomaticTracerMultiplier);
    emit onAutomaticCamMovingSpeedChange(rollSpeed, pitchSpeed, yawSpeed);

    const double MaximalEncoderTracerErrorDegree = 0.1;

    if ( (qAbs(offsetPitch) < MaximalEncoderTracerErrorDegree) && (qAbs(offsetYaw) < MaximalEncoderTracerErrorDegree) ) // ignore (qAbs(rollSpeed < 1))
        sleep();
}

void AutomaticTracer::setTracerMode(AutomaticTracerMode tracerMode)
{
    if (_tracerMode == tracerMode)
        return;
    _tracerMode = tracerMode;
    emit onTracerModeChanged(tracerMode);
}

void AutomaticTracer::processTelemetry(const TelemetryDataFrame &telemetryFrame, const QPoint &screenPoint, bool screenPointValid)
{    
    if (_tracerMode == atmScreenPoint)
        moveToScreenPoint(telemetryFrame, screenPoint, screenPointValid);
    else if (_tracerMode == atmEncoderValues)
        moveToEncoderValues(telemetryFrame);
}

void AutomaticTracer::sleep()
{
    if (_tracerMode != AutomaticTracerMode::atmSleep)
    {
        setTracerMode(atmSleep);
        stopRotation();
        QTimer::singleShot(10, Qt::PreciseTimer, this, &AutomaticTracer::stopRotation);
        QTimer::singleShot(50, Qt::PreciseTimer, this, &AutomaticTracer::stopRotation);
    }
}

void AutomaticTracer::followScreenPoint()
{
    setTracerMode(atmScreenPoint);
    stopRotation();
}

void AutomaticTracer::followEncoderValues(float encoderRollDegree, float encoderPitchDegree, float encoderYawDegree)
{
    setTracerMode(atmEncoderValues);
    stopRotation();

    _destEncoderRollDegree = encoderRollDegree;
    _destEncoderPitchDegree = encoderPitchDegree;
    _destEncoderYawDegree = encoderYawDegree;
}

AutomaticTracerMode AutomaticTracer::tracerMode()
{
    return _tracerMode;
}

void AutomaticTracer::stopRotation()
{
    emit onAutomaticCamMovingSpeedChange(0, 0, 0);
}
