#include "GSIArealObject.h"
#include <QPen>
#include <QBrush>
#include <QColor>
#include "MapGraphicsScene.h"

const QPolygonF GSIArealObject::getPolygonPoints(ArealObject *arealObject) const
{
    auto points = arealObject->points();
    int count = points->count();

    QPolygonF polygonPoints;
    for (int i = 0; i < count; i++)
        polygonPoints.append(coordAsScenePoint(points->at(i)));
    return polygonPoints;
}

GSIArealObject::GSIArealObject(ArealObject *arealObject) :
    QGraphicsPolygonItem(getPolygonPoints(arealObject)),
    _arealObject(arealObject)
{
    QPen pen;
    pen.setWidth(1);
    pen.setColor(arealObject->color());
    pen.setCosmetic(true);

    QBrush brush;
    QColor brushColor = arealObject->color();
    brushColor.setAlpha(30);
    brush.setColor(brushColor);
    brush.setStyle(Qt::SolidPattern);

    this->setPen(pen);
    this->setBrush(brush);
    this->setZValue(DEFAULT_VIEW_FIELD_Z_ORDER);
}

void GSIArealObject::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    QGraphicsPolygonItem::paint(painter, option, widget);
    QRectF rect = this->boundingRect();
    painter->drawText(rect, _arealObject->description(), Qt::AlignCenter | Qt::AlignVCenter);
}
