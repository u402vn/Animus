#include "MarkerStorage.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QUuid>
#include <QDebug>
#include <QDateTime>
#include <QPainter>
#include <QFont>
#include <QPen>
#include <QColor>
#include "Common/CommonUtils.h"
#include "ApplicationSettings.h"
#include "EnterProc.h"


MarkerStorage::MarkerStorage(QObject *parent) : QObject(parent),
    _artillerySpotter(this)
{
    EnterProc("MarkerStorage::MarkerStorage");

    ApplicationSettings& applicationSettings = ApplicationSettings::Instance();
    QString databaseFile = applicationSettings.MarkerStorageDatabase;


    _mapMarkerDatabase = QSqlDatabase::addDatabase("QSQLITE", "MarkerDatabaseConnection");
    _mapMarkerDatabase.setDatabaseName(databaseFile);
    _mapMarkerDatabase.open();
    LOG_SQL_ERROR(_mapMarkerDatabase);

    EXEC_SQL(_mapMarkerDatabase, "CREATE TABLE IF NOT EXISTS MapMarker (GUID Varchar(20), TemplateGUID Varchar(20), MarkerTag INTEGER, "
                                 "Description Varchar(255), MarkerParty INTEGER, ArtillerySpotterState INTEGER,"
                                 "Latitude REAL, Longitude REAL, Hmsl REAL, DeletedDT REAL)");

    //EXEC_SQL(_mapMarkerDatabase, "CREATE TABLE IF NOT EXISTS MapMarker (GUID Varchar(20), TemplateGUID Varchar(20), Latitude REAL, Longitude REAL, DeletedDT REAL)");
    EXEC_SQL(_mapMarkerDatabase, "ALTER TABLE MapMarker ADD COLUMN MarkerTag INTEGER");
    EXEC_SQL(_mapMarkerDatabase, "ALTER TABLE MapMarker ADD COLUMN Description Varchar(255)");
    EXEC_SQL(_mapMarkerDatabase, "ALTER TABLE MapMarker ADD COLUMN MarkerParty INTEGER");
    EXEC_SQL(_mapMarkerDatabase, "ALTER TABLE MapMarker ADD COLUMN Hmsl REAL");
    EXEC_SQL(_mapMarkerDatabase, "ALTER TABLE MapMarker ADD COLUMN ArtillerySpotterState INTEGER");

    _mapMarkersLoaded = false;
    startTimer(1000); //save dirty markers

    if (applicationSettings.EnableArtilleryMountNotification)
        _artillerySpotter.openSocket(QHostAddress(applicationSettings.ArtilleryMountAddress), applicationSettings.ArtilleryMountTCPPort);
    //connect(this, &MarkerStorage::onMarkerListUpdated, &_artillerySpotter, &ArtillerySpotter::onMarkerListUpdated, Qt::DirectConnection);
}

MarkerStorage::~MarkerStorage()
{
    saveAllDirtyMarkers();
    _mapMarkerDatabase.close();
}

void MarkerStorage::timerEvent(QTimerEvent *event)
{
    //http://doc.qt.io/qt-5/qobject.html#startTimer
    Q_UNUSED(event)
    EnterProc("MarkerStorage::timerEvent");
    saveAllDirtyMarkers();
}

void MarkerStorage::OutDatabaseError(bool dbResult)
{
    if (!dbResult)
    {
        QSqlError error = _mapMarkerDatabase.lastError();
        qDebug() << "SqLite error:" << error.text() << ", SqLite error code:" << error.number();
    }
}

MarkerStorage &MarkerStorage::Instance()
{
    static MarkerStorage s(nullptr);
    return s;
}

void MarkerStorage::saveMarker(MapMarker *mapMarker)
{
    EnterProc("MarkerStorage::saveMarker");

    auto partyMapMarker = dynamic_cast<PartyMapMarker*>(mapMarker);
    MarkerParty markerParty = (partyMapMarker == nullptr ? MarkerParty::Neutral : partyMapMarker->getParty());


    QSqlQuery updateQuery(_mapMarkerDatabase);
    updateQuery.prepare("UPDATE MapMarker SET TemplateGUID = ?, Latitude = ?, Longitude = ?, Hmsl = ?,"
                        "Description = ?, MarkerTag = ?, MarkerParty = ?, ArtillerySpotterState = ? WHERE GUID = ?");
    LOG_SQL_ERROR(updateQuery);

    updateQuery.addBindValue(mapMarker->templateGUID());
    updateQuery.addBindValue(mapMarker->gpsCoord().lat);
    updateQuery.addBindValue(mapMarker->gpsCoord().lon);
    updateQuery.addBindValue(mapMarker->gpsCoord().hmsl);
    updateQuery.addBindValue(mapMarker->description());
    updateQuery.addBindValue(mapMarker->tag());
    updateQuery.addBindValue(markerParty);
    updateQuery.addBindValue(mapMarker->artillerySpotterState());
    updateQuery.addBindValue(mapMarker->GUID());

    updateQuery.exec();
    LOG_SQL_ERROR(updateQuery);

    mapMarker->setDirty(false);
}

MapMarker *MarkerStorage::addMapMarkerToList(const QString &templateGUID, const QString &markerGUID, const WorldGPSCoord &gpsCoord,
                                             int markerTag, const QString &description, MarkerParty party, ArtillerySpotterState artillerySpotterState)
{
    EnterProc("MarkerStorage::addMapMarkerToList");

    MarkerThesaurus& markerThesaurus = MarkerThesaurus::Instance();
    auto mapMarkerTemplate = markerThesaurus.getMarkerTemplateByGUID(templateGUID);

    MapMarker *mapMarker = nullptr;
    if (mapMarkerTemplate->GUID() == TargetMarkerTemplateGUID)
    {
        auto targetMapMarker = new TargetMapMarker(this, markerGUID, gpsCoord, mapMarkerTemplate, markerTag, description, artillerySpotterState);
        connect(targetMapMarker, &MapMarker::onDisplayedImageChanged, this, &MarkerStorage::onTargetMapMarkerHighlightedChanged_Internal);
        connect(targetMapMarker, &MapMarker::onCoodChanged, this, &MarkerStorage::onMapMarkerCoodChanged_Internal);
        _targetMapMarkers.append(targetMapMarker);
        mapMarker = targetMapMarker;
    }
    else if (mapMarkerTemplate->samInfoList().count() > 0)
    {
        mapMarker = new SAMMapMarker(this, markerGUID, gpsCoord, mapMarkerTemplate, description, party, artillerySpotterState);
    }
    else if (mapMarkerTemplate->useParty())
    {
        mapMarker = new PartyMapMarker(this, markerGUID, gpsCoord, mapMarkerTemplate, description, party, artillerySpotterState);
    }
    else
    {
        mapMarker = new MapMarker(this, markerGUID, gpsCoord, mapMarkerTemplate, markerTag, description, artillerySpotterState);
    }

    _mapMarkers.append(mapMarker);

    return mapMarker;
}

void MarkerStorage::saveAllDirtyMarkers()
{
    EnterProc("MarkerStorage::saveAllDirtyMarkers");
    foreach (auto mapMarker, _mapMarkers)
        if (mapMarker->dirty())
            saveMarker(mapMarker);
}

MapMarker *MarkerStorage::createNewMarker(const QString &markerTemplateGUID, const WorldGPSCoord &gpsCoord)
{
    EnterProc("MarkerStorage::createNewMarker");

    auto mapMarker = addMapMarkerToList(markerTemplateGUID, QUuid::createUuid().toString(), gpsCoord, 0, "", MarkerParty::Neutral, ArtillerySpotterState::Unspecified);

    auto partyMapMarker = dynamic_cast<PartyMapMarker*>(mapMarker);
    MarkerParty markerParty = (partyMapMarker == nullptr ? MarkerParty::Neutral : partyMapMarker->getParty());

    QSqlQuery insertQuery(_mapMarkerDatabase);
    insertQuery.prepare("INSERT INTO MapMarker (GUID, TemplateGUID, Latitude, Longitude, hmsl, "
                        "Description, MarkerTag, MarkerParty, ArtillerySpotterState) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?)");
    LOG_SQL_ERROR(insertQuery);

    insertQuery.addBindValue(mapMarker->GUID());
    insertQuery.addBindValue(mapMarker->templateGUID());
    insertQuery.addBindValue(mapMarker->gpsCoord().lat);
    insertQuery.addBindValue(mapMarker->gpsCoord().lon);
    insertQuery.addBindValue(mapMarker->gpsCoord().hmsl);
    insertQuery.addBindValue(mapMarker->description());
    insertQuery.addBindValue(mapMarker->tag());
    insertQuery.addBindValue(markerParty);
    insertQuery.addBindValue(mapMarker->artillerySpotterState());
    insertQuery.exec();
    LOG_SQL_ERROR(insertQuery);

    emit onMapMarkerCreated(mapMarker->GUID());
    emit onMarkerListUpdated(&_mapMarkers);
    return mapMarker;
}

void MarkerStorage::deleteMarker(MapMarker *mapMarker)
{
    EnterProc("MarkerStorage::deleteMarker");

    double deletedDT = GetCurrentDateTimeForDB();

    QSqlQuery updateQuery(_mapMarkerDatabase);
    updateQuery.prepare("UPDATE MapMarker SET DeletedDT = ? WHERE GUID = ?");
    LOG_SQL_ERROR(updateQuery);
    updateQuery.addBindValue(deletedDT);
    updateQuery.addBindValue(mapMarker->GUID());
    updateQuery.exec();
    LOG_SQL_ERROR(updateQuery);
    _mapMarkers.removeOne(mapMarker);

    if (mapMarker->templateGUID() == TargetMarkerTemplateGUID)
        _targetMapMarkers.removeOne(dynamic_cast<TargetMapMarker*>(mapMarker));

    //???    TargetMapMarker::_lastTargetNumber = 0;
    //???    foreach (auto marker, _targetMapMarkers)
    //???        TargetMapMarker::_lastTargetNumber = qMax(TargetMapMarker::_lastTargetNumber, marker->tag());

    emit onMapMarkerDeleted(mapMarker->GUID());

    emit onMarkerListUpdated(&_mapMarkers);

    //todo remove mapMarker from memory
}

void MarkerStorage::loadMarkerList()
{
    if (_mapMarkersLoaded)
        return;

    EnterProc("MarkerStorage::loadMarkerList");

    _mapMarkersLoaded = true;

    QSqlQuery selectQuery = EXEC_SQL(_mapMarkerDatabase,
                                     "SELECT GUID, TemplateGUID, Latitude, Longitude, Hmsl, "
                                     "Description, MarkerTag, MarkerParty, ArtillerySpotterState FROM MapMarker WHERE DeletedDT IS NULL");
    while (selectQuery.next())
    {
        int pos = 0;
        QString markerGUID = selectQuery.value(pos++).toString();
        QString templateGUID = selectQuery.value(pos++).toString();
        WorldGPSCoord gpsCoord;
        gpsCoord.lat = selectQuery.value(pos++).toDouble();
        gpsCoord.lon = selectQuery.value(pos++).toDouble();
        gpsCoord.hmsl = selectQuery.value(pos++).toDouble();
        QString description = selectQuery.value(pos++).toString();
        int markerTag = selectQuery.value(pos++).toInt();
        MarkerParty markerParty = MarkerParty(selectQuery.value(pos++).toInt());
        ArtillerySpotterState artillerySpotterState = ArtillerySpotterState(selectQuery.value(pos++).toInt());

        addMapMarkerToList(templateGUID, markerGUID, gpsCoord, markerTag, description, markerParty, artillerySpotterState);
    }
    emit onMarkerListUpdated(&_mapMarkers);
}

const QList<MapMarker *> *MarkerStorage::getMapMarkers()
{
    EnterProc("MarkerStorage::getMapMarkers");

    loadMarkerList();
    return &_mapMarkers;
}

const QList<TargetMapMarker *> *MarkerStorage::getTargetMapMarkers()
{
    EnterProc("MarkerStorage::getTargetMapMarkers");

    loadMarkerList();
    return &_targetMapMarkers;
}

MapMarker *MarkerStorage::getMapMarkerByGUID(const QString &markerGUID)
{
    EnterProc("MarkerStorage::getMapMarkerByGUID");
    loadMarkerList();

    foreach (auto mapMarker, _mapMarkers)
        if (mapMarker->GUID() == markerGUID)
            return mapMarker;
    return nullptr;
}

void MarkerStorage::sendMarkersToArtillerySpotter()
{
    _artillerySpotter.sendMarkers(&_mapMarkers);
}

void MarkerStorage::sendWeatherToArtillerySpotter(const QVector<TelemetryDataFrame> &telemetryDataFrames)
{
    _artillerySpotter.sendWeather(telemetryDataFrames);
}

void MarkerStorage::cleanupObsoleteMapMarkers()
{
    EnterProc("MarkerStorage::CleanupObsoleteMapMarkers");
    EXEC_SQL(_mapMarkerDatabase, "DELETE FROM MapMarker WHERE DeletedDT IS NOT NULL");
}

void MarkerStorage::onTargetMapMarkerHighlightedChanged_Internal()
{
    //stay only one marker highlighted
    auto targetMarker = qobject_cast<TargetMapMarker *>(sender());
    if (targetMarker->isHighlighted())
    {
        foreach(auto marker, _targetMapMarkers)
            if (marker != targetMarker)
                marker->setHighlighted(false);
    }
    emit onTargetMapMarkerHighlightedChanged(targetMarker->GUID(), targetMarker->isHighlighted());
    emit onMarkerListUpdated(&_mapMarkers);
}

void MarkerStorage::onMapMarkerCoodChanged_Internal()
{
    auto targetMarker = qobject_cast<TargetMapMarker *>(sender());
    emit onMapMarkerCoordChanged(targetMarker->GUID(), targetMarker->gpsCoord());
    emit onMarkerListUpdated(&_mapMarkers);
}
