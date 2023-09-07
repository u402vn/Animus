#include "GSIAntennaMarker.h"
#include <QLineF>
#include "MapGraphicsScene.h"
#include "Common/CommonUtils.h"
#include "ApplicationSettings.h"

GSIAntennaMarker::GSIAntennaMarker(QGraphicsScene *scene) : QGraphicsPixmapItem(nullptr)
{
    // 1. Init Antenna Marker
    setFlag(QGraphicsItem::ItemIgnoresTransformations, true);
    setVisible(false);
    scene->addItem(this);
    setZValue(DEFAULT_ANTENNA_MARKER_Z_ORDER);


    auto image = QPixmap(":/plane_blue.png");
    setPixmap(image);
    setScale((qreal)ANTENNA_MARKER_SIZE / image.width());
    setOffset(-(qreal)image.width() / 2, -(qreal)image.height() / 2);
}

GSIAntennaMarker::~GSIAntennaMarker()
{

}

void GSIAntennaMarker::updateForTelemetry(const TelemetryDataFrame &telemetryFrame)
{
    auto antennaCoords = getAntennaCoordsFromTelemetry(telemetryFrame);

    if (antennaCoords.isIncorrect())
    {
        setVisible(false);
        return;
    }

    // 1. Process Antenna
    auto antennaPosPoint = coordAsScenePoint(antennaCoords);
    setVisible(true);
    setPos(antennaPosPoint);
    setRotation(telemetryFrame.AntennaAzimuth);
}
