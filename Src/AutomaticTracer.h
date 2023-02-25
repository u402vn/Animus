#ifndef AUTOMATICTRACER_H
#define AUTOMATICTRACER_H

#include <QObject>
#include "TelemetryDataFrame.h"
#include "CamPreferences.h"
#include "Constants.h"

class AutomaticTracer final: public QObject
{
    Q_OBJECT
    AutomaticTracerMode _tracerMode;

    CamAssemblyPreferences *_camAssemblyPreferences;

    float _destEncoderRollDegree, _destEncoderPitchDegree, _destEncoderYawDegree;

    quint32 _lastVideoFrameNumber, _lastTelemetryFrameNumber;

    void moveToScreenPoint(const TelemetryDataFrame &telemetryFrame, const QPoint &screenPoint, bool screenPointValid);
    void moveToEncoderValues(const TelemetryDataFrame &telemetryFrame);
    void setTracerMode(AutomaticTracerMode tracerMode);
public:
    explicit AutomaticTracer(QObject *parent, CamAssemblyPreferences *camAssemblyPreferences);
    ~AutomaticTracer();

    void processTelemetry(const TelemetryDataFrame &telemetryFrame, const QPoint &screenPoint, bool screenPointValid);

    void sleep();
    void followScreenPoint();
    void followEncoderValues(float encoderRollDegree, float encoderPitchDegree, float encoderYawDegree);
    AutomaticTracerMode tracerMode();
private slots:
    void stopRotation();
signals:
    void onAutomaticCamMovingSpeedChange(float roll, float pitch, float yaw);
    void onTracerModeChanged(const AutomaticTracerMode tracerMode);
};

#endif // AUTOMATICTRACER_H
