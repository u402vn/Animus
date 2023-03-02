#include "GSISAMObject.h"
#include "MapGraphicsScene.h"
#include "EnterProc.h"
#include "Common/CommonUtils.h"

void SAMMarkerSceneItem::resizeToSceneScale(unsigned int sceneScale)
{
    GSICommonObject::resizeToSceneScale(sceneScale);
}

void SAMMarkerSceneItem::updatePosOnScene()
{
    GSICommonObject::updatePosOnScene();
    updateSAMRings();
}

QGraphicsEllipseItem *createSAMRing(MapGraphicsScene *mapScene, Qt::PenStyle style)
{
    auto circle = new QGraphicsEllipseItem();
    circle->setFlag(QGraphicsItem::ItemIgnoresTransformations, false);

    QPen pen = circle->pen();
    qreal penWidth = 2.0 / DEFAULT_GOOGLE_SCALE_FOR_SCENE; //pow(2, mapScene->scale() - 18);
    pen.setWidthF(penWidth);
    pen.setStyle(style);
    circle->setPen(pen);

    QBrush brush = circle->brush();
    brush.setStyle(Qt::SolidPattern);
    brush.setColor(QColor(0, 0, 0, 100));
    //QRadialGradient https://books.google.by/books?id=XGV7xc2uZXkC&pg=PA257&lpg=PA257&dq=qradial+qgradient&source=bl&ots=aj6ITQZYKI&sig=ACfU3U1dCGy_qHh7C9zigNCgcqTz3ZDYIQ&hl=ru&sa=X&ved=2ahUKEwivoezStMHhAhXKlYsKHY0PDJw4ChDoATAHegQIBxAB#v=onepage&q=qradial%20qgradient&f=false
    circle->setBrush(brush);

    mapScene->addItem(circle);
    return circle;
}

void SAMMarkerSceneItem::createSAMRings()
{
    if (this->scene() == nullptr)
        return;
    auto mapScene = dynamic_cast<MapGraphicsScene*>(this->scene());

    if (_circleMinKillingRange == nullptr)
        _circleMinKillingRange = createSAMRing(mapScene, Qt::DashDotLine);
    if (_circleMaxKillingRange == nullptr)
        _circleMaxKillingRange = createSAMRing(mapScene, Qt::DashLine);
    if (_circleVisibleRange == nullptr)
        _circleVisibleRange = createSAMRing(mapScene, Qt::DotLine);

    updateSAMRings();
}

void updateSAMRing(QGraphicsEllipseItem *circle, const QPointF &centerPos, qreal sizePix)
{
    if (sizePix > 0)
    {
        QRectF rect = QRectF(centerPos, QSizeF(sizePix, sizePix));
        rect.moveCenter(centerPos);
        circle->setRect(rect);
        circle->setVisible(true);
    }
    else
        circle->setVisible(false);
}

void SAMMarkerSceneItem::updateSAMRings()
{
    auto markerCoords = _mapMarker->gpsCoord();

    double deltaHeight = _uavCoords.hmsl - markerCoords.hmsl;

    auto samMapMarker = dynamic_cast<SAMMapMarker*>(_mapMarker);
    qreal resolutionMperPix = getImageResolutionMperPix(samMapMarker->gpsCoord(), DEFAULT_GOOGLE_SCALE_FOR_SCENE);

    auto samInfo = samMapMarker->getSAMinfo(deltaHeight);

    qreal sizeM2PixRatio = 1.0 / resolutionMperPix / TILE_WIDTH;

    auto centerPos = this->pos();

    //todo update only if prev size differs with new size more than 5pix, f.e.
    //store max value of (range * sizeM2PixRatio) and compare with current value
    updateSAMRing(_circleMinKillingRange, centerPos, samInfo.minKillingRange() * sizeM2PixRatio);
    updateSAMRing(_circleMaxKillingRange, centerPos, samInfo.maxKillingRange() * sizeM2PixRatio);
    updateSAMRing(_circleVisibleRange, centerPos, samInfo.visibleRange() * sizeM2PixRatio);
}

void SAMMarkerSceneItem::updateForTelemetry(const TelemetryDataFrame &telemetryFrame)
{
    _uavCoords = getUavCoordsFromTelemetry(telemetryFrame);
    updateSAMRings();
}

SAMMarkerSceneItem::SAMMarkerSceneItem(SAMMapMarker *mapMarker, QGraphicsItem *parent) :
    GSICommonObject(mapMarker, parent)
{
    _circleMinKillingRange = nullptr;
    _circleMaxKillingRange = nullptr;
    _circleVisibleRange = nullptr;
}

SAMMarkerSceneItem::~SAMMarkerSceneItem()
{
    delete _circleMinKillingRange;
    delete _circleMaxKillingRange;
    delete _circleVisibleRange;
}

QVariant SAMMarkerSceneItem::itemChange(QGraphicsItem::GraphicsItemChange change, const QVariant &value)
{
    if (change == ItemSceneHasChanged)
    {
        //Q_ASSERT(_circleMinKillingRange == nullptr && _circleMaxKillingRange == nullptr && _circleVisibleRange == nullptr);
        createSAMRings();
    }
    return GSICommonObject::itemChange(change, value);
}
