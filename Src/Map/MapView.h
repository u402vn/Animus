#ifndef MAPVIEW_H
#define MAPVIEW_H

#include <QWidget>
#include <QCloseEvent>
#include "MapGraphicsScene.h"
#include "MapGraphicsView.h"

class MapView final : public QWidget
{
    Q_OBJECT
private:
    MapGraphicsScene *_scene;
    MapGraphicsView *_view;

    int _appenedPoints;
protected:
    void timerEvent(QTimerEvent *event);
    void virtual closeEvent(QCloseEvent * event);
public:
    explicit MapView(QWidget *parent);
    ~MapView();
    void showMapMarkers();
    void showArealObjects();
    void loadTrajectory(const QVector<TelemetryDataFrame> &telemetryFrames);
    void appendTrajectoryPoint(const TelemetryDataFrame &telemetryFrame);
    void processTelemetry(const TelemetryDataFrame &telemetryFrame);
    void clearTrajectory();
    void setViewCenter(const WorldGPSCoord &coord);
};

#endif // MAPVIEW_H
