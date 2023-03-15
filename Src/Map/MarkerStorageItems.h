#ifndef MARKERSTORAGEITEMS_H
#define MARKERSTORAGEITEMS_H

#include <QObject>
#include "Common/CommonData.h"
#include "MarkerThesaurus.h"
#include "Constants.h"

const QMap <int, QColor> MapArtillerySpotterStateColors {
    { ArtillerySpotterState::Unspecified,       QColor("#31363b") },
    { ArtillerySpotterState::DefeatRequired,    QColor("#991111") },
    { ArtillerySpotterState::TrialShot,         QColor("#111199") },
    { ArtillerySpotterState::RealShot,          QColor("#119911") } };


const QMap<int, QString> MapArtillerySpotterStateCaptions();

class MapMarker : public QObject
{
    Q_OBJECT

    QString _GUID;
    WorldGPSCoord _gpsCoord;
protected:
    QPixmap _image;
    bool _needImageUpdate;

    bool _dirty; //marker will be saved by timer
    int _tag;
    QString _description;
    ArtillerySpotterState _artillerySpotterState;
    MarkerTemplate *_mapMarkerTemplate;
public:
    explicit MapMarker(QObject *parent, const QString &guid, const WorldGPSCoord &gpsCoord, MarkerTemplate *mapMarkerTemplate,
                       int tag, const QString &description, ArtillerySpotterState artillerySpotterState);

    const QString GUID() const;
    const QString templateGUID() const;
    const QString hint() const;
    int tag();

    const WorldGPSCoord gpsCoord() const;
    void setGPSCoord(const WorldGPSCoord &coord);
    const QString description() const;
    void setDescription(const QString &description);
    ArtillerySpotterState artillerySpotterState() const;
    void setArtillerySpotterState(const ArtillerySpotterState &artillerySpotterState);
    bool dirty() const;
    void setDirty(bool value);

    virtual const QPixmap displayedImage();

signals:
    void onCoodChanged();
    void onDisplayedImageChanged();
    void onDescriptionChanged();
};


enum MarkerParty {Neutral = 0, Allies = 1, Enemies = 2};

class PartyMapMarker: public MapMarker
{
private:
    Q_OBJECT

    MarkerParty _party;
public:
    explicit PartyMapMarker(QObject *parent, const QString &guid, const WorldGPSCoord &gpsCoord, MarkerTemplate *mapMarkerTemplate,
                            const QString &description, MarkerParty party, ArtillerySpotterState artillerySpotterState);
    const QPixmap displayedImage();
    MarkerParty getParty();
    void setParty(MarkerParty value);
    void setNextParty();
};

class SAMMapMarker final: public PartyMapMarker
{
private:
    Q_OBJECT
protected:
    const QList<SAMInfo*> samInfoList();
public:
    explicit SAMMapMarker(QObject *parent, const QString &guid, const WorldGPSCoord &gpsCoord, MarkerTemplate *mapMarkerTemplate,
                          const QString &description, MarkerParty party, ArtillerySpotterState artillerySpotterState);
    SAMInfo getSAMinfo(double height);
};

class TargetMapMarker final: public MapMarker
{
    Q_OBJECT

    static int _lastTargetNumber;

    bool _isHighlighted;
public:
    explicit TargetMapMarker(QObject *parent, const QString &guid, const WorldGPSCoord &gpsCoord, MarkerTemplate *mapMarkerTemplate,
                             int tag, const QString &description, ArtillerySpotterState artillerySpotterState);
    const QPixmap displayedImage();
    bool isHighlighted();
    void setHighlighted(bool value);
};


class ArtillerySalvoCenterMarker: public MapMarker
{
private:
    Q_OBJECT
public:
    explicit ArtillerySalvoCenterMarker(QObject *parent, const QString &guid, const WorldGPSCoord &gpsCoord, MarkerTemplate *mapMarkerTemplate );
};

#endif // MARKERSTORAGEITEMS_H
