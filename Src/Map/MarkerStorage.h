#ifndef MARKERSTORAGE_H
#define MARKERSTORAGE_H

#include <QObject>
#include <QSqlDatabase>
#include <QList>
#include "MarkerThesaurus.h"
#include "ArtillerySpotter.h"
#include "Common/CommonData.h"
#include "MarkerStorageItems.h"

class MarkerStorage final : public QObject
{
    Q_OBJECT

    QSqlDatabase _mapMarkerDatabase;
    QList<MapMarker *> _mapMarkers;
    QList<TargetMapMarker *> _targetMapMarkers;
    ArtillerySalvoCenterMarker *_salvoCenterMarker;
    bool _mapMarkersLoaded;
    ArtillerySpotter _artillerySpotter;

    void updateArtillerySalvoCenterMarker();
    void loadMarkerList();
    MapMarker *addMapMarkerToList(const QString &templateGUID, const QString &markerGUID, const WorldGPSCoord &gpsCoord,
                                  int markerTag, const QString &description, MarkerParty party, ArtillerySpotterState artillerySpotterState);
    void saveMarker(MapMarker *mapMarker);
    void saveAllDirtyMarkers();

    void OutDatabaseError(bool dbResult);

    void cleanupObsoleteMapMarkers();

    explicit MarkerStorage(QObject *parent);
    ~MarkerStorage();

    MarkerStorage(MarkerStorage const&) = delete;
    MarkerStorage& operator= (MarkerStorage const&) = delete;
protected:
    void timerEvent(QTimerEvent *event); //save dirty markers by timer
public:
    static MarkerStorage& Instance();

    MapMarker *createNewMarker(const QString &markerTemplateGUID, const WorldGPSCoord &gpsCoord);
    void deleteMarker(MapMarker *mapMarker);
    const QList<MapMarker*> *getMapMarkers();
    const QList<TargetMapMarker*> *getTargetMapMarkers();
    ArtillerySalvoCenterMarker *getSalvoCenterMarker();
    MapMarker *getMapMarkerByGUID(const QString &markerGUID);

    ArtillerySpotter *artillerySpotter();
signals:
    void onMapMarkerDeleted(const QString &markerGUID);
    void onMapMarkerCreated(const QString &markerGUID);
    void onTargetMapMarkerHighlightedChanged(const QString &markerGUID, bool isHighlighted);
    void onMapMarkerCoordChanged(const QString &markerGUID, const WorldGPSCoord &coord);
    void onMarkerListUpdated(const QList<MapMarker *> *markers);
private slots:
    void onTargetMapMarkerHighlightedChanged_Internal();
    void onTargetMapMarkerCoodChanged_Internal();
};

#endif // MARKERSTORAGE_H
