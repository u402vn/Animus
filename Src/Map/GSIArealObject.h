#ifndef GSIAREALOBJECT_H
#define GSIAREALOBJECT_H

#include <QObject>
#include <QPolygonF>
#include <QGraphicsPolygonItem>
#include "ArealObjectContainer.h"

class GSIArealObject final: public QGraphicsPolygonItem
{
    ArealObject *_arealObject;

    const QPolygonF getPolygonPoints(ArealObject *arealObject) const;
public:
    explicit GSIArealObject(ArealObject *arealObject);
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
};

#endif // GSIAREALOBJECT_H
