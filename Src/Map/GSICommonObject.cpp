#include "GSICommonObject.h"
#include <QPixmap>
#include "MapGraphicsScene.h"
#include "EnterProc.h"

void GSICommonObject::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    if (event->modifiers() == Qt::ControlModifier)
    {
        if (cursor().shape() != Qt::ClosedHandCursor)
            setCursor(QCursor(Qt::ClosedHandCursor));
        if (_mapMarker->templateGUID() != ArtillerySalvoCenterMarkerTemplateGUID)
            QGraphicsPixmapItem::mouseMoveEvent(event);
        else
            event->ignore();
    }
    else
        event->ignore();
}

QVariant GSICommonObject::itemChange(QGraphicsItem::GraphicsItemChange change, const QVariant &value)
{
    EnterProc("QMarkerItem::itemChange");

    if (change == ItemPositionHasChanged)
    {
        auto mapScene = (MapGraphicsScene*)scene();
        auto coord = mapScene->getSceneCoord(pos());
        _mapMarker->setGPSCoord(coord);
    }
    return QGraphicsPixmapItem::itemChange(change, value);
    //todo place onposchange and onnewitem processing here using MarkerStorage
}

void GSICommonObject::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    /*
    qDebug() << event->type() ;
    if (event->type() == QGraphicsSceneMouseEvent::Type::GraphicsSceneMouseDoubleClick )
    {
        if (cursor().shape() != Qt::ClosedHandCursor)
            setCursor(QCursor(Qt::ClosedHandCursor));
    }
*/
    if (event->modifiers() == Qt::ControlModifier)
    {
        if (cursor().shape() != Qt::ClosedHandCursor)
            setCursor(QCursor(Qt::ClosedHandCursor));
    }

    QGraphicsPixmapItem::mousePressEvent(event);
}

void GSICommonObject::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    if (cursor().shape() != Qt::ArrowCursor)
        setCursor(QCursor(Qt::ArrowCursor));
    QGraphicsPixmapItem::mouseReleaseEvent(event);
}

void GSICommonObject::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Space)
    {
        auto targetMarker = dynamic_cast<TargetMapMarker*>(_mapMarker);
        if (targetMarker != nullptr)
            targetMarker->setHighlighted(!targetMarker->isHighlighted());

        auto partyMarker = dynamic_cast<PartyMapMarker*>(_mapMarker);
        if (partyMarker != nullptr)
            partyMarker->setNextParty();
    }
    QGraphicsPixmapItem::keyPressEvent(event);
}

MapMarker *GSICommonObject::getMapMarker()
{
    return _mapMarker;
}

GSICommonObject::GSICommonObject(MapMarker *mapMarker, QGraphicsItem *parent) : QGraphicsPixmapItem(parent)
{
    EnterProc("QMarkerItem::QMarkerItem");

    _mapMarker = mapMarker;

    connect(_mapMarker, &MapMarker::onCoodChanged, this, &GSICommonObject::onCoodChanged);
    connect(_mapMarker, &MapMarker::onDisplayedImageChanged, this, &GSICommonObject::onDisplayedImageChanged);
    connect(_mapMarker, &MapMarker::onDescriptionChanged, this, &GSICommonObject::onDescriptionChanged);

    updateToolTip();
    updateImage();

    setVisible(true);

    setFlag(ItemIgnoresTransformations);
    setFlag(ItemIsMovable);
    setFlag(ItemIsSelectable);
    setFlag(ItemIsFocusable);
    setCacheMode(DeviceCoordinateCache);
    setCursor(QCursor(Qt::ArrowCursor));

    updatePosOnScene();

    setFlag(ItemSendsGeometryChanges); //this flag should be defined after setPos to exclude itemChange call
}

void GSICommonObject::updatePosOnScene()
{
    auto posOnScene = ConvertGPS2GoogleXY(_mapMarker->gpsCoord(), DEFAULT_GOOGLE_SCALE_FOR_SCENE);
    setPos(posOnScene);
}

void GSICommonObject::updateForTelemetry(const TelemetryDataFrame &telemetryFrame)
{
    Q_UNUSED(telemetryFrame)
    //todo implement this method when it will be requested by customer
}

void GSICommonObject::resizeToSceneScale(quint8 sceneScale, const QList<double> &targetSizesForScales)
{
    qreal itemScale = targetSizesForScales[sceneScale];
    if (itemScale == 0)
    {
        itemScale = 1.0;
        if (DEFAULT_GOOGLE_SCALE_FOR_SCENE > sceneScale)
            itemScale = 1.0 / (DEFAULT_GOOGLE_SCALE_FOR_SCENE - sceneScale);
    }

    const QPixmap pixmap = this->pixmap();
    const qreal maxSide = qMax(pixmap.width(), pixmap.height());
    qreal minScale = MINIMAL_SCALED_MARKER_SIZE / maxSide;

    if (itemScale < minScale)
        itemScale = minScale;

    this->setScale(itemScale);
}

void GSICommonObject::updateToolTip()
{
    setToolTip(_mapMarker->hint());
}

void GSICommonObject::updateImage()
{
    const QPixmap pixmap = _mapMarker->displayedImage();

    setPixmap(pixmap);
    qreal markerOffsetX = (qreal)pixmap.width() / 2;
    qreal markerOffsetY = (qreal)pixmap.height() / 2;
    setOffset(-markerOffsetX, -markerOffsetY);
}

GSICommonObject::~GSICommonObject()
{

}

void GSICommonObject::onCoodChanged()
{
    updatePosOnScene();
    updateToolTip();
}

void GSICommonObject::onDisplayedImageChanged()
{
    updateToolTip();
    updateImage();
}

void GSICommonObject::onDescriptionChanged()
{
    updateToolTip();
}
