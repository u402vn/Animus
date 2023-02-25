#ifndef MARKERSTORAGEITEMS_H
#define MARKERSTORAGEITEMS_H

#include <QObject>
#include "Common/CommonData.h"
#include "MarkerStorageItems.h"
#include "MarkerThesaurus.h"

enum ArtillerySpotterState {Unspecified = 0, DefeatRequired = 1, TrialShot = 2, RealShot = 3, EllipseCenter = 10};

class MapMarker : public QObject
{
    Q_OBJECT

    QString _GUID;
    WorldGPSCoord _gpsCoord;
protected:
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

    QPixmap _image;
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
    QPixmap _image;
    QPixmap _highlightedImage;
    void initImageWithTag();
public:
    explicit TargetMapMarker(QObject *parent, const QString &guid, const WorldGPSCoord &gpsCoord, MarkerTemplate *mapMarkerTemplate,
                             int tag, const QString &description, ArtillerySpotterState artillerySpotterState);
    const QPixmap displayedImage();
    bool isHighlighted();
    void setHighlighted(bool value);
};

#endif // MARKERSTORAGEITEMS_H
