#include "TelemetryDataFrame.h"

bool TelemetryDataFrame::UseGimbalTelemetryOnlyForCalculation = false;

double TelemetryDataFrame::recalculatedCamYaw() const
{
    double recalculatedCamYaw = TelemetryDataFrame::UseGimbalTelemetryOnlyForCalculation ? CamYaw + 180 : UavYaw + encoderAngleDegree(CamEncoderYaw);
    return recalculatedCamYaw;
}

const QQuaternion TelemetryDataFrame::getCamQuaternion() const
{
    QQuaternion camQ =
            QQuaternion::fromAxisAndAngle(QVector3D(0, 0, 1), recalculatedCamYaw() ) *
            QQuaternion::fromAxisAndAngle(QVector3D(0, -1, 0), CamPitch - 90) *
            QQuaternion::fromAxisAndAngle(QVector3D(1, 0, 0), CamRoll);
    return camQ;
}

const QRect TelemetryDataFrame::targetRect() const
{
    QRect result(TrackedTargetCenterX - TrackedTargetRectWidth / 2,
                 TrackedTargetCenterY - TrackedTargetRectHeight / 2,
                 TrackedTargetRectWidth, TrackedTargetRectHeight);
    return result;
}

bool TelemetryDataFrame::targetIsVisible() const
{
    bool isVisible = (TrackedTargetRectWidth > 0) && (TrackedTargetRectHeight > 0) &&
            (TrackedTargetCenterX > 0) && (TrackedTargetCenterY > 0);
    return isVisible;
}

const QPoint TelemetryDataFrame::stabilizedCenter() const
{
    QPoint result(StabilizedCenterX, StabilizedCenterY);
    return result;
}

double encoderAngleDegree(qint32 camEncoderValue)
{
    double angle = (double)camEncoderValue / 16384.0 * 360.0;
    return angle;
}



void EmulatorTelemetryDataFrame::applyToTelemetryDataFrame(TelemetryDataFrame &telemetryDataFrame)
{
    telemetryDataFrame.UavRoll = UavRoll;
    telemetryDataFrame.UavPitch = UavPitch;
    telemetryDataFrame.UavYaw = UavYaw;
    telemetryDataFrame.UavLatitude_GPS = UavLatitude_GPS;
    telemetryDataFrame.UavLongitude_GPS = UavLongitude_GPS;
    telemetryDataFrame.UavAltitude_GPS = UavAltitude_GPS;
    telemetryDataFrame.UavAltitude_Barometric = UavAltitude_Barometric;
    telemetryDataFrame.Course_GPS = Course_GPS;
}

void CameraTelemetryDataFrame::applyToTelemetryDataFrame(TelemetryDataFrame &telemetryDataFrame)
{
    telemetryDataFrame.CamRoll = CamRoll;
    telemetryDataFrame.CamPitch = CamPitch;
    telemetryDataFrame.CamYaw = CamYaw;
    telemetryDataFrame.CamZoom = CamZoom;
    telemetryDataFrame.CamEncoderRoll = CamEncoderRoll;
    telemetryDataFrame.CamEncoderPitch = CamEncoderPitch;
    telemetryDataFrame.CamEncoderYaw = CamEncoderYaw;
    telemetryDataFrame.RangefinderDistance = RangefinderDistance;
    telemetryDataFrame.RangefinderTemperature = RangefinderTemperature;
}

void ExtendedTelemetryDataFrame::applyToTelemetryDataFrame(TelemetryDataFrame &telemetryDataFrame)
{
        telemetryDataFrame.AtmosphereTemperature = AtmosphereTemperature;
}
