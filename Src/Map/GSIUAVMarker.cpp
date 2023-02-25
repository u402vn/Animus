#include "GSIUAVMarker.h"
#include <QLineF>
#include "MapGraphicsScene.h"
#include "Common/CommonUtils.h"
#include "ApplicationSettings.h"

GSIUAVMarker::GSIUAVMarker(QGraphicsScene *scene) : QGraphicsPixmapItem(nullptr)
{
    ApplicationSettings& applicationSettings = ApplicationSettings::Instance();

    // 1. Init UAV Marker
    setFlag(QGraphicsItem::ItemIgnoresTransformations, true);
    setVisible(false);
    scene->addItem(this);
    setZValue(DEFAULT_UAV_MARKER_Z_ORDER);

    // 2. Init Laser Line
    QPen laserLinePen;
    laserLinePen.setWidth(applicationSettings.ViewFieldLineWidth);
    laserLinePen.setColor(applicationSettings.ViewFieldLineColor);
    laserLinePen.setStyle(Qt::DashLine);
    laserLinePen.setCosmetic(true);

    _laserLineItem = new QGraphicsLineItem();
    _laserLineItem->setPen(laserLinePen);
    scene->addItem(_laserLineItem);

    // 3. Init View Field
    QPen viewFieldPen;
    viewFieldPen.setWidth(applicationSettings.ViewFieldLineWidth);
    viewFieldPen.setColor(applicationSettings.ViewFieldLineColor);
    viewFieldPen.setCosmetic(true);

    _viewFieldPathItem = new QGraphicsPathItem();
    _viewFieldPathItem->setZValue(DEFAULT_VIEW_FIELD_Z_ORDER);
    _viewFieldPathItem->setPen(viewFieldPen);
    scene->addItem(_viewFieldPathItem);

    // 4. Init Wind Marker
    _windItem = new QGraphicsPixmapItem();
    _windItem->setFlag(QGraphicsItem::ItemIgnoresTransformations, true);
    _windItem->setZValue(DEFAULT_UAV_MARKER_Z_ORDER);
    auto windImage = QPixmap(":/wind_arrow.png");
    _windItem->setPixmap(windImage);
    auto scale = (qreal)WIND_MARKER_SIZE / windImage.width();
    _windItem->setScale(scale);
    _windItem->setOffset(-(qreal)windImage.width() / 2, -(qreal)windImage.height() / 2 - (qreal)WIND_MARKER_SHIFT / scale);
    scene->addItem(_windItem);
}

GSIUAVMarker::~GSIUAVMarker()
{

}

void GSIUAVMarker::updateForTelemetry(const TelemetryDataFrame &telemetryFrame)
{
    auto uavCoords = getUavCoordsFromTelemetry(telemetryFrame);

    if (uavCoords.isIncorrect())
    {
        setVisible(false);
        return;
    }

    // 1. Process Plane
    auto uavPosPoint = coordAsScenePoint(uavCoords);
    setVisible(true);
    setPos(uavPosPoint);
    setRotation(telemetryFrame.UavYaw);

    // 2. Process Laser Line
    auto laserPosPoint = coordAsScenePoint(getRangefinderCoordsFromTelemetry(telemetryFrame));
    QLineF laserLine(uavPosPoint, laserPosPoint);
    bool laserVisible = (laserLine.dx() != 0) || (laserLine.dy() != 0);
    _laserLineItem->setVisible(laserVisible);
    _laserLineItem->setLine(laserLine);

    // 3. Process View Field
    QList<WorldGPSCoord> visibleAreaBorderPoints;
    WorldGPSCoord firstBorderPoint;
    for (int n = 0; n < ViewFieldBorderPointsCount; n++)
    {
        WorldGPSCoord borderPoint(telemetryFrame.ViewFieldBorderPointsLat[n],
                                  telemetryFrame.ViewFieldBorderPointsLon[n],
                                  telemetryFrame.ViewFieldBorderPointsHmsl[n]);
        if (!borderPoint.isIncorrect())
            visibleAreaBorderPoints.append(borderPoint);
        if (n == 0)
            firstBorderPoint = borderPoint;
    }
    if (!firstBorderPoint.isIncorrect())
        visibleAreaBorderPoints.append(firstBorderPoint); //close border if it is possible

    QPainterPath viewFieldPath;
    bool firstPoint = true;
    foreach (auto areaPoint, visibleAreaBorderPoints)
    {
        if (firstPoint)
        {
            viewFieldPath.moveTo(coordAsScenePoint(areaPoint));
            firstPoint = false;
        }
        else
            viewFieldPath.lineTo(coordAsScenePoint(areaPoint));
    }
    //viewFieldPath.closeSubpath();
    _viewFieldPathItem->setPath(viewFieldPath);

    // 4. Process Wind Marker
    //_windItem->setRotation(telemetryFrame.WindDirection + 180);
    _windItem->setRotation(telemetryFrame.WindDirection);
    _windItem->setPos(uavPosPoint);
}

void GSIUAVMarker::setImageName(const QString &name)
{
    if (_imageName == name)
        return;
    _imageName = name;

    auto image = QPixmap(_imageName);
    setPixmap(image);
    setScale((qreal)UAV_MARKER_SIZE / image.width());
    setOffset(-(qreal)image.width() / 2, -(qreal)image.height() / 2);
}

const QString GSIUAVMarker::imageName()
{
    return _imageName;
}
