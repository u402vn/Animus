#ifndef COORDINATECALCULATOR_H
#define COORDINATECALCULATOR_H

#include <QObject>
#include <QPoint>
#include <QScriptEngine>
#include <QString>
#include "Common/CommonData.h"
#include "TelemetryDataFrame.h"
#include "CamPreferences.h"
#include "Map/HeightMapContainer.h"
#include "HardwareLink/DelayLine.h"
#include "ApplicationSettings.h"

class CoordinateCalculator final: public QObject
{
    Q_OBJECT
    QScriptEngine _scriptEngine;
    HeightMapContainer *_heightMapContainer;
    CamAssemblyPreferences *_camAssemblyPreferences;
    QString _ballisticMacro;
    bool _useLaserRangefinderForGroundLevelCalculation;
    bool _useBombCaclulation;

    double _trackedTargetSpeed, _trackedTargetDirection;
    TelemetryDelayLine *_targetSpeedFrames;

    const WorldGPSCoord getScreenPointCoord(TelemetryDataFrame *telemetryFrame, int x, int y) const;

    bool needUpdateBombingData(TelemetryDataFrame *telemetryFrame);
    void updateLaserRangefinderPosition(TelemetryDataFrame *telemetryFrame);
    void updateGroundLevel(TelemetryDataFrame *telemetryFrame);
    void updateTrackedTargetPosition(TelemetryDataFrame *telemetryFrame);
    void updateViewFieldBorderPoints(TelemetryDataFrame *telemetryFrame);
    void updateBombingData(TelemetryDataFrame *telemetryFrame);
    void updateRemainingTimeToDropBomb(TelemetryDataFrame *telemetryFrame);
    void updateTargetSpeedFrames(TelemetryDataFrame *telemetryFrame);

    void updateTrackedTargetSpeed();
protected:
    void timerEvent(QTimerEvent *event);
public:
    explicit CoordinateCalculator(QObject *parent, HeightMapContainer *heightMapContainer);
    ~CoordinateCalculator();

    void processTelemetryDataFrame(TelemetryDataFrame *_telemetryFrame);
};

#endif // COORDINATECALCULATOR_H
