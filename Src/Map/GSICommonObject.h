#ifndef GSICOMMONOBJECT_H
#define GSICOMMONOBJECT_H

#include <QObject>
#include <QGraphicsItem>
#include <QGraphicsSceneMouseEvent>
#include <QKeyEvent>
#include "MarkerStorage.h"
#include "TelemetryDataFrame.h"

class GSICommonObject : public QObject, public QGraphicsPixmapItem
{
    Q_OBJECT

    friend class MapGraphicsScene;

    void updateToolTip();
    void updateImage();
protected:
    MapMarker *_mapMarker;
    virtual void resizeToSceneScale(unsigned int sceneScale);
    virtual void updatePosOnScene();
    virtual void updateForTelemetry(const TelemetryDataFrame &telemetryFrame);

    explicit GSICommonObject(MapMarker *mapMarker, QGraphicsItem *parent);

    void mouseMoveEvent(QGraphicsSceneMouseEvent *event) Q_DECL_OVERRIDE;
    QVariant itemChange(GraphicsItemChange change, const QVariant &value) Q_DECL_OVERRIDE;
    void mousePressEvent(QGraphicsSceneMouseEvent *event) Q_DECL_OVERRIDE;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) Q_DECL_OVERRIDE;
    void keyPressEvent(QKeyEvent *event) Q_DECL_OVERRIDE;

    MapMarker *getMapMarker();
public:
    ~GSICommonObject();
private slots:
    void onCoodChanged();
    void onDisplayedImageChanged();
    void onDescriptionChanged();
};

#endif // GSICOMMONOBJECT_H
