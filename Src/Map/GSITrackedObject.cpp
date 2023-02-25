#include "GSITrackedObject.h"
#include "MapGraphicsScene.h"
#include "Common/CommonUtils.h"
#include "ApplicationSettings.h"

GSITrackedObject::GSITrackedObject(QGraphicsScene *scene) : QGraphicsPixmapItem(nullptr)
{
    setFlag(QGraphicsItem::ItemIgnoresTransformations, true);
    setVisible(false);
    scene->addItem(this);
    setZValue(DEFAULT_TRACKED_OBJECT_Z_ORDER);
}

GSITrackedObject::~GSITrackedObject()
{

}

void GSITrackedObject::updateForTelemetry(const TelemetryDataFrame &telemetryFrame)
{
    auto targetCoords = getTrackedTargetCoordsFromTelemetry(telemetryFrame);
    if (targetCoords.isIncorrect())
    {
        setVisible(false);
    }
    else
    {
        setVisible(true);
        setPos(coordAsScenePoint(targetCoords));
    };
}

void GSITrackedObject::setImageName(const QString &name)
{
    if (_imageName == name)
        return;
    _imageName = name;

    auto image = changeImageColor(QPixmap(_imageName), Qt::yellow);
    setPixmap(image);
    setScale(TRACKED_OBJECT_MARKER_SIZE / image.width());
    setOffset(-(qreal)image.width() / 2, -(qreal)image.height() / 2);
}

const QString GSITrackedObject::imageName()
{
    return _imageName;
}
