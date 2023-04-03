#include "MapGraphicsScene.h"
#include <QPainter>
#include <QGraphicsItem>
#include <QGraphicsView>
#include <QList>
#include <QDebug>
#include "ArealObjectContainer.h"
#include "MapTileContainer.h"
#include "MapTileExportDialog.h"
#include "MarkerStorage.h"
#include "GSISAMObject.h"
#include "GSIArealObject.h"
#include "Common/CommonWidgets.h"
#include "Common/CommonUtils.h"
#include "EnterProc.h"
#include "ApplicationSettings.h"

const quint8 MINIMAL_MAP_ZOOM = 2;
const quint8 MAXIMAL_MAP_ZOOM = 19;

const QPointF coordAsScenePoint(const WorldGPSCoord &coords)
{
    return ConvertGPS2GoogleXY(coords, DEFAULT_GOOGLE_SCALE_FOR_SCENE);
}

//---------------------------------------------------------------------------------------------------------

void checkActionByData(QAction *action, int data)
{
    int actionData = action->data().toInt();
    action->setChecked(actionData == data);
}

void MapGraphicsScene::contextMenuEvent(QGraphicsSceneContextMenuEvent *event)
{
    _acBaseLayers->setText(tr("Map %1").arg(_mapTileContainer->getTileSourceName(_mapTileContainer->getMapBaseSourceId())));
    _acHybridLayers->setText(tr("Labels %1").arg(_mapTileContainer->getTileSourceName(_mapTileContainer->getMapHybridSourceId())));

    double markerSize = _targetSizesForScales.at(_scale);
    foreach (auto action, _groupMapTargetSize->actions())
        action->setChecked(qFuzzyCompare(action->data().toDouble(), markerSize));

    _mainMenu->exec(event->screenPos());

    showArealObjects(_acShowArealObjects->isChecked());

    if (_acFollowThePlane->isChecked())
        centerOnUAV();
    else
        forceRepaintViews();
}

void MapGraphicsScene::clearUAVPath()
{
    bool needClear = CommonWidgetUtils::showConfirmDialog(tr("Do you want to clear the UAV path?"), false);
    if (needClear)
        clearTrajectory();
}

void MapGraphicsScene::exportVisibleAreaMapTriggered()
{
    auto allViews = this->views();
    if (allViews.count() == 0)
        return;
    auto viewport = allViews[0]->viewport();
    WorldGPSCoord coordLeftTop, coordRightBottom;
    _mapTileContainer->calculateVisibleAreaCornersCoords(viewport->width(), viewport->height(), coordLeftTop, coordRightBottom);
    auto exportDialog = new MapTileExportDialog(nullptr, _mapTileContainer, coordLeftTop, coordRightBottom);
    exportDialog->showNormal();
}

GSICommonObject *MapGraphicsScene::findMarkerItemByGUID(const QString &markerGUID)
{
    auto alItems = this->items();
    foreach (auto item, alItems)
    {
        auto markerItem = dynamic_cast<GSICommonObject*>(item);
        if (markerItem != nullptr)
        {
            if (markerItem->getMapMarker()->GUID() == markerGUID)
                return markerItem;
        }
    }
    return nullptr;
}

void MapGraphicsScene::resizeMrkersForScale()
{
    foreach (auto item, _allMarkerItems)
        item->resizeToSceneScale(_scale, _targetSizesForScales);
}

bool MapGraphicsScene::changeScaneScale(int delta)
{
    int newScale = _scale + delta;
    if (newScale > MAXIMAL_MAP_ZOOM)
        newScale = MAXIMAL_MAP_ZOOM;
    else if (newScale < MINIMAL_MAP_ZOOM)
        newScale = MINIMAL_MAP_ZOOM;

    if (newScale == _scale)
        return false;
    _scale = newScale;

    resizeMrkersForScale();

    return true;
}

void MapGraphicsScene::onMapMarkerDeleted(const QString &markerGUID)
{
    auto markerItem = findMarkerItemByGUID(markerGUID);
    this->removeItem(markerItem);
    _allMarkerItems.removeOne(markerItem);
    delete markerItem;
}

void MapGraphicsScene::onMapMarkerCreated(const QString &markerGUID)
{
    MarkerStorage& markerStorage = MarkerStorage::Instance();
    auto mapMarker = markerStorage.getMapMarkerByGUID(markerGUID);
    addMapMarkerToScene(mapMarker);
}

void MapGraphicsScene::onMapContentUpdated()
{
    update();
}

QAction *MapGraphicsScene::createUavImageMenuAction(const QString &caption, const QString &imageName, QActionGroup *groupUavImages)
{
    QString currentImageName = _uavMarker->imageName();
    bool isChecked = currentImageName == imageName;
    auto menu = _acLegend->menu();

    auto action = CommonWidgetUtils::createCheckableMenuGroupAction(caption, isChecked, groupUavImages, menu, imageName);
    action->setIcon(QIcon(imageName));
    connect(action, &QAction::triggered, this, [action, this](bool checked)
    {
        if (checked)
            _uavMarker->setImageName(action->data().toString());
    });
    return action;
}

QAction *MapGraphicsScene::createCoordSystemActions(GlobalCoordSystem coordSystem, QActionGroup *groupCoordSystems)
{
    bool isChecked = _mapTileContainer->getCoordSystem() == coordSystem;
    QString caption = _mapTileContainer->getGlobalCoordSystemName(coordSystem);
    auto menu = _acLegend->menu();
    auto action = CommonWidgetUtils::createCheckableMenuGroupAction(caption, isChecked, groupCoordSystems, menu, coordSystem);
    connect(action, &QAction::triggered, this, [action, this](bool checked)
    {
        if (!checked)
            return;
        ApplicationSettings& applicationSettings = ApplicationSettings::Instance();
        GlobalCoordSystem coordSystem = GlobalCoordSystem(action->data().toInt());
        applicationSettings.UIPresentationCoordSystem = coordSystem;
        _mapTileContainer->setCoordSystem(coordSystem);
    });

    return action;
}

void MapGraphicsScene::initMainMenu()
{
    EnterProc("MapGraphicsScene::initMainMenu");

    auto parentWidget = (QWidget*)this->parent();
    _mainMenu = new QMenu(parentWidget);

    auto submenuBaseLayers = new QMenu(parentWidget);
    _acBaseLayers = CommonWidgetUtils::createMenuAction(tr("Map"), _mainMenu);
    _acBaseLayers->setMenu(submenuBaseLayers);

    auto submenuHybridLayers = new QMenu(parentWidget);
    _acHybridLayers = CommonWidgetUtils::createMenuAction(tr("Labels"), _mainMenu);
    _acHybridLayers->setMenu(submenuHybridLayers);

    auto submenuLegend = new QMenu(parentWidget);
    _acLegend = CommonWidgetUtils::createMenuAction(tr("Legend"), _mainMenu);
    _acLegend->setMenu(submenuLegend);

    auto groupBaseTileSource = new QActionGroup(this);
    auto groupHybridTileSource = new QActionGroup(this);

    ApplicationSettings& applicationSettings = ApplicationSettings::Instance();
    auto sourceInfos = _mapTileContainer->getTileSourceInfos();
    foreach (auto sourceInfo, sourceInfos)
    {
        int sourceId = sourceInfo->sourceId();
        int connectionsCount = sourceInfo->dbConnections()->count();

        if ((sourceId > 0) && (connectionsCount == 0))
            continue;

        QAction *action;
        QString caption = _mapTileContainer->getTileSourceName(sourceId);
        if (sourceInfo->isHybrid())
        {
            action = CommonWidgetUtils::createCheckableMenuGroupAction(caption, sourceId == applicationSettings.LastUsedMapHybridSourceId,
                                                                       groupHybridTileSource, _acHybridLayers->menu(), sourceId);
            connect(action, &QAction::triggered, this, [action, this](bool checked)
            {
                if (!checked)
                    return;
                ApplicationSettings& applicationSettings = ApplicationSettings::Instance();
                auto sourceId = MapHybridTileSource(action->data().toInt());
                _mapTileContainer->setMapHybridSourceId(sourceId);
                applicationSettings.LastUsedMapHybridSourceId = sourceId;
            });
        }
        else
        {
            action = CommonWidgetUtils::createCheckableMenuGroupAction(caption, sourceId == applicationSettings.LastUsedMapBaseSourceId,
                                                                       groupBaseTileSource, _acBaseLayers->menu(), sourceId);
            connect(action, &QAction::triggered, this, [action, this](bool checked)
            {
                if (!checked)
                    return;
                ApplicationSettings& applicationSettings = ApplicationSettings::Instance();
                auto sourceId = MapBaseTileSource(action->data().toInt());
                _mapTileContainer->setMapBaseSourceId(sourceId);
                applicationSettings.LastUsedMapBaseSourceId = sourceId;
            });
        }
        _mapActions.append(action);
    }

    auto groupCoordSystems = new QActionGroup(this);
    createCoordSystemActions(WGS84, groupCoordSystems);
    createCoordSystemActions(SK42,  groupCoordSystems);

    submenuLegend->addSeparator();

    _acShowTileNumber = CommonWidgetUtils::createCheckableMenuSingleAction(tr("Tile Numbers"), false, submenuLegend);
    _acShowParallelsMeridians = CommonWidgetUtils::createCheckableMenuSingleAction(tr("Parallels and meridians"), true, submenuLegend);

    submenuLegend->addSeparator();

    auto groupUavImages = new QActionGroup(this);
    createUavImageMenuAction(tr("Black Marker"),    ":/plane_black.png", groupUavImages);
    createUavImageMenuAction(tr("White Marker"),    ":/plane_white.png", groupUavImages);
    createUavImageMenuAction(tr("Blue Marker"),     ":/plane_blue.png",  groupUavImages);
    createUavImageMenuAction(tr("Red Marker"),      ":/plane_red.png",   groupUavImages);
    createUavImageMenuAction(tr("Green Marker"),    ":/plane_green.png", groupUavImages);

    submenuLegend->addSeparator();

    auto submenuMapTargetSizes = new QMenu(parentWidget);
    auto acMapTargetSizes = CommonWidgetUtils::createMenuAction(tr("Target Sizes"), submenuLegend);
    acMapTargetSizes->setMenu(submenuMapTargetSizes);

    _groupMapTargetSize = new QActionGroup(this);
    for (int i = 100; i > 20; i -= 5)
    {
        QString caption = QString("%1 %").arg(i);
        double value = 0.01 * i;
        auto action = CommonWidgetUtils::createCheckableMenuGroupAction(caption, false,
                                                                        _groupMapTargetSize, acMapTargetSizes->menu(), value);

        connect(action, &QAction::triggered, this, [action, this](bool checked)
        {
            if (checked)
            {
                ApplicationSettings& applicationSettings = ApplicationSettings::Instance();
                _targetSizesForScales[_scale] = action->data().toDouble();
                applicationSettings.TargetMarkerSizes.setValue(_targetSizesForScales);
                resizeMrkersForScale();
            }
        });
    }

    submenuLegend->addSeparator();

    _acShowUAVPath = CommonWidgetUtils::createCheckableMenuSingleAction(tr("Show UAV Path"), true, submenuLegend);
    connect(_acShowUAVPath, &QAction::triggered, this, [=](bool checked)
    {
        foreach(auto item, _allTrajectoryPathItems)
            item->setVisible(checked);
    });

    auto acClearUAVPath = CommonWidgetUtils::createMenuAction(tr("Clear UAV Path"), submenuLegend);
    connect(acClearUAVPath, &QAction::triggered, this, &MapGraphicsScene::clearUAVPath);

    _acFollowThePlane = CommonWidgetUtils::createCheckableMenuSingleAction(tr("Follow aircraft"), true, _mainMenu);
    _acShowArealObjects = CommonWidgetUtils::createCheckableMenuSingleAction(tr("Show areas"), true, _mainMenu);

    _mainMenu->addSeparator();

    auto submenuImportExport = new QMenu(parentWidget);
    auto acImportExportMap = CommonWidgetUtils::createMenuAction(tr("Import/Export Map"), _mainMenu);
    acImportExportMap->setMenu(submenuImportExport);

    auto acExportVisibleAreaMap = CommonWidgetUtils::createMenuAction(tr("Export Visible Area Map"), submenuImportExport);
    connect(acExportVisibleAreaMap, &QAction::triggered, this, &MapGraphicsScene::exportVisibleAreaMapTriggered);
}

void MapGraphicsScene::centerOnUAV()
{
    if (_uavMarker->isVisible())
        setViewCenterXY(_uavMarker->x(), _uavMarker->y());
}

void MapGraphicsScene::forceRepaintViews()
{
    foreach (auto view, this->views())
        view->viewport()->update();
}

void MapGraphicsScene::setViewCenterXY(double x, double y)
{
    EnterProcStart("MapGraphicsScene::setViewCenterXY");

    foreach (auto view, this->views())
        view->centerOn(x, y);

    forceRepaintViews();
}

MapGraphicsScene::MapGraphicsScene(QObject *parent) :
    QGraphicsScene(parent)
{    
    EnterProc("MapGraphicsScene::MapGraphicsScene");

    MarkerStorage& markerStorage = MarkerStorage::Instance();
    connect(&markerStorage, &MarkerStorage::onMapMarkerDeleted, this, &MapGraphicsScene::onMapMarkerDeleted);
    connect(&markerStorage, &MarkerStorage::onMapMarkerCreated, this, &MapGraphicsScene::onMapMarkerCreated);

    ApplicationSettings& applicationSettings = ApplicationSettings::Instance();

    //init map tile container
    _mapTileContainer = new MapTileContainer(this, applicationSettings.getMapDatabaseFiles(), applicationSettings.DatabaseMapDownloadCashe, applicationSettings.DatabaseHeightMap);
    _mapTileContainer->setCoordSystem(applicationSettings.UIPresentationCoordSystem);
    _mapTileContainer->setTileReceivingMode(TileReceivingMode(applicationSettings.TileReceivingMode.value()));
    _mapTileContainer->setMapBaseSourceId(MapBaseTileSource(applicationSettings.LastUsedMapBaseSourceId.value()));
    _mapTileContainer->setMapHybridSourceId(MapHybridTileSource(applicationSettings.LastUsedMapHybridSourceId.value()));
    connect(_mapTileContainer, &MapTileContainer::contentUpdated, this, &MapGraphicsScene::onMapContentUpdated, Qt::ConnectionType::QueuedConnection);

    _scale = DEFAULT_GOOGLE_SCALE_FOR_SCENE;

    _visiblePathPointsDistance2 = 1.0 / 256.0 * applicationSettings.VisiblePathPointsPixelDistance.value();
    _visiblePathPointsDistance2 = _visiblePathPointsDistance2 * _visiblePathPointsDistance2;

    _trajectoryPointsCount = 0;
    _trajectoryPen.setWidth(applicationSettings.TrajectoryPathLineWidth);
    _trajectoryPen.setColor(applicationSettings.TrajectoryPathLineColor);
    _trajectoryPen.setCosmetic(true);

    // init UAV marker
    _uavMarker = new GSIUAVMarker(this);
    _uavMarker->setImageName(":/plane_blue.png");

    // init tracked object marker
    _trackedObjectMarker = new GSITrackedObject(this);
    _trackedObjectMarker->setImageName(":/tracked_object.png");

    // init map area on graphics scene
    this->addRect(0, 0, 262143, 262143); //magic numbers of tile count, next (524287, 524287)

    // init Target Marker Sizes
    _targetSizesForScales = applicationSettings.TargetMarkerSizes;
    if (_targetSizesForScales.size() < MAXIMAL_MAP_ZOOM)
        for (quint8 i = _targetSizesForScales.size(); i <= MAXIMAL_MAP_ZOOM; i++)
            _targetSizesForScales.append(0);

    // init popup menu
    initMainMenu();
}

MapGraphicsScene::~MapGraphicsScene()
{

}

void MapGraphicsScene::drawBackground(QPainter *painter, const QRectF &rect)
{
    EnterProc("MapGraphicsScene::drawBackground");

    auto coord = getSceneCoord(rect.center());

    _mapTileContainer->setScale(_scale);
    _mapTileContainer->setImageCenter(coord);

    double imageHeight = painter->device()->height();
    double imageWidth = painter->device()->width();

    LegendPresentationParam legendPresentationParam =
    {
        .drawLegend = (imageWidth > 400),
        .drawBaseTileNumber = _acShowTileNumber->isChecked(),
        .drawParallelsMeridians = (imageHeight > 200) && (imageWidth > 200) && _acShowParallelsMeridians->isChecked()
    };

    _mapTileContainer->getMapImage(painter, legendPresentationParam);
}

bool MapGraphicsScene::ScaleUp()
{
    return changeScaneScale(+1);
}

bool MapGraphicsScene::ScaleDown()
{
    return changeScaneScale(-1);
}

int MapGraphicsScene::scale()
{
    return _scale;
}

bool MapGraphicsScene::followThePlane()
{
    return _acFollowThePlane->isChecked();
}

bool MapGraphicsScene::setFollowThePlane(bool value)
{
    _acFollowThePlane->setChecked(value);

    if (_acFollowThePlane->isChecked())
        centerOnUAV();
    else
        forceRepaintViews();
}

void MapGraphicsScene::addTrajectoryPoint(const WorldGPSCoord &pointCoords, bool immediateShow)
{
    auto point = ConvertGPS2GoogleXY(pointCoords, DEFAULT_GOOGLE_SCALE_FOR_SCENE);

    if (_trajectoryPointsCount == 0)
    {
        _trajectoryPoints = new QPainterPath();
        _trajectoryPathItem = this->addPath(*_trajectoryPoints, _trajectoryPen);
        _allTrajectoryPathItems.append(_trajectoryPathItem);
        _trajectoryPathItem->setZValue(DEFAULT_TRAJECTORY_Z_ORDER);
        _trajectoryPoints->moveTo(point);
        _prevPoint = point;
        _trajectoryPointsCount++;
    }
    else
    {
        double dx = _prevPoint.x() - point.x();
        double dy = _prevPoint.y() - point.y();

        if (dx * dx + dy * dy >= _visiblePathPointsDistance2)
        {
            _trajectoryPoints->lineTo(point);
            _prevPoint = point;
            _trajectoryPointsCount++;
        }
        if (_trajectoryPointsCount == 10000)
        {
            refreshTrajectoryOnMap();
            _trajectoryPointsCount = 0;
            addTrajectoryPoint(pointCoords, false);
        }
    }

    if (immediateShow)
        refreshTrajectoryOnMap();
}

void MapGraphicsScene::refreshTrajectoryOnMap()
{
    if (_trajectoryPointsCount > 0)
        _trajectoryPathItem->setPath(*_trajectoryPoints);
}

void MapGraphicsScene::clearTrajectory()
{
    foreach(auto item, _allTrajectoryPathItems)
        this->removeItem(item);

    _allTrajectoryPathItems.clear();
    _trajectoryPointsCount = 0;
    _trajectoryPathItem = nullptr;
}

void MapGraphicsScene::processTelemetry(const TelemetryDataFrame &telemetryFrame)
{
    EnterProc("MapGraphicsScene::processTelemetry");

    _telemetryFrame = telemetryFrame;

    if (telemetryFrame.TelemetryFrameNumber <= 0)
        return;

    //1. draw UAV marker, laser line, view field on the map
    _uavMarker->updateForTelemetry(telemetryFrame);

    //2. draw tracked object on the map
    _trackedObjectMarker->updateForTelemetry(telemetryFrame);

    //3. update Markers
    foreach (auto item, _allMarkerItems)
        item->updateForTelemetry(telemetryFrame);

    //4. try center on UAV
    if (_acFollowThePlane->isChecked())
        centerOnUAV();
}

const WorldGPSCoord MapGraphicsScene::getUavCoords()
{
    return getUavCoordsFromTelemetry(_telemetryFrame);
}

void MapGraphicsScene::showArealObjects(bool visible)
{
    foreach (auto item, _allArealObjectItems)
        item->setVisible(visible);
}

void MapGraphicsScene::setViewCenter(const WorldGPSCoord &coord)
{
    EnterProc("MapGraphicsScene::setViewCenter");
    double x, y;
    ConvertGPS2GoogleXY(coord, DEFAULT_GOOGLE_SCALE_FOR_SCENE, x, y);
    setViewCenterXY(x, y);
}

void MapGraphicsScene::addMapMarkerToScene(MapMarker *mapMarker)
{
    auto samMapMarker = dynamic_cast<SAMMapMarker*>(mapMarker);

    GSICommonObject *newMarkerItem = samMapMarker == nullptr ?
                new GSICommonObject(mapMarker, nullptr) :
                new SAMMarkerSceneItem(samMapMarker, nullptr);
    this->addItem(newMarkerItem);
    newMarkerItem->resizeToSceneScale(_scale, _targetSizesForScales);
    _allMarkerItems.append(newMarkerItem);

    //newMarkerItem->setZValue(i);

    /*
    //fix to 2D points (for broken data in database)
    WorldGPSCoord currentMarkerCoord = mapMarker->gpsCoord();
    if (currentMarkerCoord.hmsl == 0) //is 2D point ? todo correct checking
    {
        WorldGPSCoord coord = getSceneCoord(newMarkerItem->pos());
        currentMarkerCoord.hmsl = coord.hmsl;
        mapMarker->setGPSCoord(currentMarkerCoord);
    }
    */
}

void MapGraphicsScene::loadMapMarkers()
{
    EnterProc("MapGraphicsScene::loadMapMarkers");
    MarkerStorage& markerStorage = MarkerStorage::Instance();
    QList<MapMarker *> mapMarkers =  *(markerStorage.getMapMarkers());
    foreach (auto mapMarker, mapMarkers)
        addMapMarkerToScene(mapMarker);
}

void MapGraphicsScene::loadArealObjects()
{
    EnterProc("MapGraphicsScene::loadArealObjects");
    ArealObjectContainer& objectContainer = ArealObjectContainer::Instance();
    auto arealObjects = *(objectContainer.getArealObjects());

    foreach (auto arealObject, arealObjects)
    {
        if (!arealObject->isVisible() || arealObject->points()->isEmpty())
            continue;

        auto polygonItem = new GSIArealObject(arealObject);

        this->addItem(polygonItem);
        polygonItem->setVisible(false);

        _allArealObjectItems.append(polygonItem);
    }
    showArealObjects(_acShowArealObjects->isChecked());
}

void MapGraphicsScene::deleteSelectedMarkers()
{
    EnterProc("MapGraphicsScene::deleteSelectedMarkers");
    auto selectedItems = this->selectedItems(); // get list of selected items

    if (selectedItems.count() == 0)
        return;

    MarkerStorage& markerStorage = MarkerStorage::Instance();
    foreach (auto item, selectedItems)
    {
        auto markerItem = dynamic_cast<GSICommonObject*>(item);
        if (markerItem != nullptr)
        {
            auto mapMarker = markerItem->getMapMarker();
            if (mapMarker->templateGUID() != ArtillerySalvoCenterMarkerTemplateGUID)
                markerStorage.deleteMarker(markerItem->getMapMarker());
        }
    }
}

void MapGraphicsScene::addNewMarker(const QPointF &posOnScene, const QString &markerTemplateGUID)
{
    EnterProc("MapGraphicsScene::addNewMarker");

    WorldGPSCoord coord = getSceneCoord(posOnScene);
    addNewMarker(coord, markerTemplateGUID);
}

void MapGraphicsScene::addNewMarker(const WorldGPSCoord &coord, const QString &markerTemplateGUID)
{
    EnterProc("MapGraphicsScene::addNewMarker");
    MarkerStorage& markerStorage = MarkerStorage::Instance();
    markerStorage.createNewMarker(markerTemplateGUID, coord);
}

WorldGPSCoord MapGraphicsScene::getSceneCoord(const QPointF &pointOnScene)
{
    WorldGPSCoord coord;
    _mapTileContainer->convertGoogleXY2GPS(DEFAULT_GOOGLE_SCALE_FOR_SCENE, pointOnScene.x(), pointOnScene.y(), coord);
    return coord;
}

