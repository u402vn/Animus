#ifndef GSISAMOBJECT_H
#define GSISAMOBJECT_H

#include <QObject>
#include <QGraphicsEllipseItem>
#include "GSICommonObject.h"

class SAMMarkerSceneItem final: public GSICommonObject
{
    Q_OBJECT

    friend class MapGraphicsScene;

    QGraphicsEllipseItem *_circleMinKillingRange;
    QGraphicsEllipseItem *_circleMaxKillingRange;
    QGraphicsEllipseItem *_circleVisibleRange;
    WorldGPSCoord _uavCoords;

    void resizeToSceneScale(unsigned int sceneScale);
    void updatePosOnScene();

    void createSAMRings();
    void updateSAMRings();

    virtual void updateForTelemetry(const TelemetryDataFrame &telemetryFrame);

    explicit SAMMarkerSceneItem(SAMMapMarker *mapMarker, QGraphicsItem *parent);

    QVariant itemChange(GraphicsItemChange change, const QVariant &value);
public:
    ~SAMMarkerSceneItem();
};

#endif // GSISAMOBJECT_H
