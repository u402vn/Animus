#ifndef MAPGRAPHICSVIEW_H
#define MAPGRAPHICSVIEW_H

#include <QGraphicsView>
#include <QGraphicsPixmapItem>
#include "MapGraphicsScene.h"

class MapGraphicsView final : public QGraphicsView
{
    Q_OBJECT

    MapGraphicsScene *mapScene();
protected:
    void wheelEvent(QWheelEvent *event) override;
    void enterEvent(QEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;

    void dragEnterEvent(QDragEnterEvent *event) override;
    void dragMoveEvent(QDragMoveEvent *event) override;
    void dropEvent(QDropEvent *event) override;
public:
    explicit MapGraphicsView(QWidget *parent);
    void loadMapMarkers();
    void loadArealObjects();
};

#endif // MAPGRAPHICSVIEW_H
