#include "MapView.h"
#include <QTime>
#include <QGraphicsPathItem>
#include <QAction>
#include <QVBoxLayout>
#include "EnterProc.h"

void MapView::timerEvent(QTimerEvent *event)
{
    if (_appenedPoints > 0)
    {
        _appenedPoints = 0;
        _scene->refreshTrajectoryOnMap();
    }
    Q_UNUSED(event)
}

void MapView::closeEvent(QCloseEvent *event)
{
    event->ignore();
}

MapView::MapView(QWidget *parent) :
    QWidget(parent)
{    
    EnterProc("MapView::MapView");

    qInfo() << "Begin Init Map View";

    _scene = new MapGraphicsScene(this);
    _view = new MapGraphicsView(this);
    _view->setScene(_scene);

    //_view->setupViewport(new QGLWidget());
    //_view->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    auto verticalLayout = new QVBoxLayout(this);
    verticalLayout->setMargin(0);
    verticalLayout->setSpacing(0);
    verticalLayout->addWidget(_view);

    _appenedPoints = 0;
    startTimer(500);

    qInfo() << "End Init Map View";
}

MapView::~MapView()
{

}

void MapView::showMapMarkers()
{
    _view->loadMapMarkers();
}

void MapView::showArealObjects()
{
    _view->loadArealObjects();
}

void MapView::loadTrajectory(const QVector<TelemetryDataFrame> &telemetryFrames)
{
    EnterProc("MapView::loadTrajectory");

    int frameCount = telemetryFrames.count();
    if (frameCount == 0)
        return;
    TelemetryDataFrame telemetryFrame;
    for (int i = 0; i < frameCount; i++)
    {
        telemetryFrame = telemetryFrames.at(i);
        WorldGPSCoord pointCoords(telemetryFrame.UavLatitude_GPS, telemetryFrame.UavLongitude_GPS, 0);
        _scene->addTrajectoryPoint(pointCoords, i == frameCount - 1);
    }

    if (telemetryFrame.TelemetryFrameNumber > 0)
    {
        WorldGPSCoord uavCoord(telemetryFrame.UavLatitude_GPS, telemetryFrame.UavLongitude_GPS, telemetryFrame.UavAltitude_GPS);
        setViewCenter(uavCoord);
    }
}

void MapView::appendTrajectoryPoint(const TelemetryDataFrame &telemetryFrame)
{
    WorldGPSCoord pointCoord(telemetryFrame.UavLatitude_GPS, telemetryFrame.UavLongitude_GPS, 0);
    _scene->addTrajectoryPoint(pointCoord, false);
    _appenedPoints++;
    // will be showed in MapView::timerEvent
}

void MapView::processTelemetry(const TelemetryDataFrame &telemetryFrame)
{
    _scene->processTelemetry(telemetryFrame);
}

void MapView::clearTrajectory()
{
    _scene->clearTrajectory();
    _appenedPoints = 0;
}

void MapView::setViewCenter(const WorldGPSCoord &coord)
{
    _scene->setViewCenter(coord);
    //qDebug() << "setViewCenter. lat:" << coord.lat << " lon: " << coord.lon;
}
