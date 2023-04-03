#include "MapView.h"
#include <QTime>
#include <QGraphicsPathItem>
#include <QAction>
#include <QVBoxLayout>
#include <QKeyEvent>
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
    EnterProc("MapView::showMapMarkers");
    _view->loadMapMarkers();
}

void MapView::showArealObjects()
{
    EnterProc("MapView::showArealObjects");
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

void MapView::onMapZoomInClicked()
{
    EnterProc("MapView::onMapZoomInClicked");
    if (_scene->ScaleUp())
        _view->scale(2, 2);
}

void MapView::onMapZoomOutClicked()
{
    EnterProc("MapView::onMapZoomOutClicked");
    if (_scene->ScaleDown())
        _view->scale(.5, .5);
}

void MapView::onFollowThePlaneClicked()
{
    _scene->setFollowThePlane(!_scene->followThePlane());
}

void MapView::onMapMoveClicked(int directionAngle)
{
    if (_scene->followThePlane())
        _scene->setFollowThePlane(false);

    int keyV = 0;
    int keyH = 0;

    switch (directionAngle)
    {
    case 0:
        keyV = Qt::Key_Up;
        break;
    case 45:
        keyV = Qt::Key_Up;
        keyH = Qt::Key_Right;
        break;
    case 90:
        keyH = Qt::Key_Right;
        break;
    case 135:
        keyH = Qt::Key_Right;
        keyV = Qt::Key_Down;
        break;
    case 180:
        keyV = Qt::Key_Down;
        break;
    case 225:
        keyH = Qt::Key_Left;
        keyV = Qt::Key_Down;
        break;
    case 270:
        keyH = Qt::Key_Left;
        break;
    case 315:
        keyH = Qt::Key_Left;
        keyV = Qt::Key_Up;
        break;
    }

    if (keyH != 0)
    {
        QKeyEvent key(QEvent::KeyPress, keyH, Qt::NoModifier);
        QApplication::sendEvent(_view, &key);
    }
    if (keyV != 0)
    {
        QKeyEvent key(QEvent::KeyPress, keyV, Qt::NoModifier);
        QApplication::sendEvent(_view, &key);
    }


    qDebug() << _view->verticalScrollBar()->singleStep();
}
