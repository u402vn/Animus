#include "MapGraphicsView.h"
#include <QWheelEvent>
#include <QCursor>
#include <QMimeData>
#include <QDebug>
#include "EnterProc.h"

MapGraphicsScene *MapGraphicsView::mapScene()
{
    return (MapGraphicsScene*)(this->scene());
}

void MapGraphicsView::wheelEvent(QWheelEvent * event)
{
    EnterProc("MapGraphicsView::wheelEvent");

    MapGraphicsScene * scene = mapScene();
    if ((event->delta() > 0) && scene->ScaleUp())
        scale(2, 2);
    else if ((event->delta() < 0) && scene->ScaleDown())
        scale(.5, .5);
    event->accept();
}

void MapGraphicsView::enterEvent(QEvent *event)
{
    QGraphicsView::enterEvent(event);
    if (viewport()->cursor().shape() != Qt::CrossCursor)
        viewport()->setCursor(Qt::CrossCursor);
}

void MapGraphicsView::mousePressEvent(QMouseEvent *event)
{
    QGraphicsView::mousePressEvent(event);
    if (viewport()->cursor().shape() != Qt::CrossCursor)
        viewport()->setCursor(Qt::CrossCursor);
}

void MapGraphicsView::mouseReleaseEvent(QMouseEvent *event)
{
    QGraphicsView::mouseReleaseEvent(event);
    if (viewport()->cursor().shape() != Qt::CrossCursor)
        viewport()->setCursor(Qt::CrossCursor);
}

void MapGraphicsView::keyPressEvent(QKeyEvent *event)
{
    EnterProc("MapGraphicsView::keyPressEvent");

    auto scene = mapScene();

    switch (event->key())
    {
    case Qt::Key_Plus:
    case Qt::Key_Equal:
        if (scene->ScaleUp())
            scale(2, 2);
        break;
    case Qt::Key_Minus:
        if (scene->ScaleDown())
            scale(.5, .5);
        break;
    case Qt::Key_Delete:
        scene->deleteSelectedMarkers();
        break;
    default:
        QGraphicsView::keyPressEvent(event);
    };

    event->accept();
}

void MapGraphicsView::dragEnterEvent(QDragEnterEvent *event)
{    
    if (event->mimeData()->hasFormat(MarkerTemplateMIMEFormat))
    {
        event->acceptProposedAction();
    }
    else
    {
        event->ignore();
    }
}

void MapGraphicsView::dragMoveEvent(QDragMoveEvent *event)
{
    if (event->mimeData()->hasFormat(MarkerTemplateMIMEFormat))
    {
        event->acceptProposedAction();
    }
    else
    {
        event->ignore();
    }
}

void MapGraphicsView::dropEvent(QDropEvent *event)
{
    EnterProc("MapGraphicsView::dropEvent");

    if (event->mimeData()->hasFormat(MarkerTemplateMIMEFormat))
    {
        QPointF posOnScene = this->mapToScene(event->pos());

        QByteArray itemData = event->mimeData()->data(MarkerTemplateMIMEFormat);
        QDataStream dataStream(&itemData, QIODevice::ReadOnly);
        QString markerTemplateGUID;
        dataStream >> markerTemplateGUID;

        mapScene()->addNewMarker(posOnScene, markerTemplateGUID);
    }
    QGraphicsView::dropEvent(event);
}

void MapGraphicsView::loadMapMarkers()
{
    EnterProc("MapGraphicsView::loadMapMarkers");

    mapScene()->loadMapMarkers();
}

void MapGraphicsView::loadArealObjects()
{
    mapScene()->loadArealObjects();
}

MapGraphicsView::MapGraphicsView(QWidget * parent) : QGraphicsView(parent)
{
    EnterProc("MapGraphicsView::MapGraphicsView");

    this->setRenderHint(QPainter::Antialiasing, false);
    this->setDragMode(QGraphicsView::ScrollHandDrag);
    this->setOptimizationFlags(QGraphicsView::DontSavePainterState | QGraphicsView::DontAdjustForAntialiasing);

    //???this->setCacheMode(QGraphicsView::CacheNone);

    this->setViewportUpdateMode(QGraphicsView::FullViewportUpdate);
    this->setTransformationAnchor(QGraphicsView::AnchorUnderMouse);

    this->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    this->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    scale(TILE_WIDTH, TILE_HEIGHT);
}
