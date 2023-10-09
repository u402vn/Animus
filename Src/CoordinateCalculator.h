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
    TelemetryDataFrame *_telemetryFrame;
    CamAssemblyPreferences *_camAssemblyPreferences;
    QString _ballisticMacro;
    bool _useLaserRangefinderForGroundLevelCalculation;
    bool _useBombCaclulation;

    TelemetryDelayLine *_targetSpeedFrames;

    const WorldGPSCoord getScreenPointCoord(int x, int y) const;

    bool needUpdateBombingData();
    void updateLaserRangefinderPosition();
    void updateGroundLevel();
    void updateTrackedTargetPosition();
    void updateViewFieldBorderPoints();
    void updateBombingData();
    void updateRemainingTimeToDropBomb();
    void updateTargetSpeedFrames();
public:
    explicit CoordinateCalculator(QObject *parent, HeightMapContainer *heightMapContainer);
    ~CoordinateCalculator();

    void processTelemetryDataFrame(TelemetryDataFrame *telemetryFrame);
};

#endif // COORDINATECALCULATOR_H
