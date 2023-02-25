#ifndef MAPGRAPHICSSCENE_H
#define MAPGRAPHICSSCENE_H

#include <QGraphicsScene>
#include <QGraphicsPathItem>
#include <QMenu>
#include <QPen>
#include <QVector>
#include <QPointF>
#include <QGraphicsSceneContextMenuEvent>
#include "TelemetryDataFrame.h"
#include "GSICommonObject.h"
#include "GSIUAVMarker.h"
#include "GSITrackedObject.h"
#include "MapTileContainer.h"
#include "MarkerStorage.h"

constexpr unsigned int DEFAULT_GOOGLE_SCALE_FOR_SCENE = 18;
constexpr int MINIMAL_SCALED_MARKER_SIZE = 16;

constexpr int UAV_MARKER_SIZE = 60;
constexpr int WIND_MARKER_SIZE = 30;
constexpr int WIND_MARKER_SHIFT = 100;
constexpr int TRACKED_OBJECT_MARKER_SIZE = 64;
constexpr unsigned int DEFAULT_UAV_MARKER_Z_ORDER = 1000002;
constexpr unsigned int DEFAULT_VIEW_FIELD_Z_ORDER = 1000001;
constexpr unsigned int DEFAULT_TRAJECTORY_Z_ORDER = 1000001;
constexpr unsigned int DEFAULT_TRACKED_OBJECT_Z_ORDER = 1000000;

class MapGraphicsScene final: public QGraphicsScene
{
    Q_OBJECT
private:
    MapTileContainer *_mapTileContainer;
    TelemetryDataFrame _telemetryFrame;
    int _scale;
    QPen _trajectoryPen;
    QPainterPath *_trajectoryPoints;
    int _trajectoryPointsCount;
    QPointF _prevPoint;
    qreal _visiblePathPointsDistance2;
    QGraphicsPathItem  *_trajectoryPathItem;
    QVector<QGraphicsPathItem  *> _allTrajectoryPathItems;
    QVector<QGraphicsPolygonItem *> _allArealObjectItems;
    QVector<GSICommonObject*> _allMarkerItems;

    GSIUAVMarker *_uavMarker;
    GSITrackedObject *_trackedObjectMarker;

    QMenu *_mainMenu;

    QList<QAction *> _mapActions;
    QAction *_acBaseLayers;
    QAction *_acHybridLayers;

    QAction *_acLegend;
    QAction *_acShowTileNumber;
    QAction *_acShowParallelsMeridians;
    QAction *_acShowUAVPath;

    QAction *_acFollowThePlane;
    QAction *_acShowArealObjects;

    QAction *createUavImageMenuAction(const QString &caption, const QString &imageName, QActionGroup *groupUavImages);
    QAction *createCoordSystemActions(GlobalCoordSystem coordSystem, QActionGroup *groupCoordSystems);

    void initMainMenu();

    void centerOnUAV();
    void forceRepaintViews();
    void setViewCenterXY(double x, double y);
    void updateUAVPosition();

    void showArealObjects(bool visible);
    GSICommonObject *findMarkerItemByGUID(const QString &markerGUID);
    bool changeScaneScale(int delta);
    void addMapMarkerToScene(MapMarker *mapMarker);

    const WorldGPSCoord getUavCoords();
protected:
    void contextMenuEvent(QGraphicsSceneContextMenuEvent *event) Q_DECL_OVERRIDE;
private slots:
    void clearUAVPath();

    void exportVisibleAreaMapTriggered();

    void onMapMarkerDeleted(const QString &markerGUID);
    void onMapMarkerCreated(const QString &markerGUID);

    void onMapContentUpdated();
public:    
    explicit MapGraphicsScene(QObject *parent);
    virtual ~MapGraphicsScene();
    void drawBackground(QPainter *painter, const QRectF &rect) override;

    void processTelemetry(const TelemetryDataFrame &telemetryFrame);

    bool ScaleUp();
    bool ScaleDown();
    int scale();

    void addTrajectoryPoint(const WorldGPSCoord &pointCoords, bool immediateShow);
    void refreshTrajectoryOnMap();
    void clearTrajectory();
    void setViewCenter(const WorldGPSCoord &coord);

    void loadMapMarkers();
    void loadArealObjects();
    void deleteSelectedMarkers();
    void addNewMarker(const QPointF &posOnScene, const QString &markerTemplateGUID);
    void addNewMarker(const WorldGPSCoord &coord, const QString &markerTemplateGUID);

    WorldGPSCoord getSceneCoord(const QPointF &pointOnScene);
};

const QPointF coordAsScenePoint(const WorldGPSCoord &coords);

#endif // MAPGRAPHICSSCENE_H
