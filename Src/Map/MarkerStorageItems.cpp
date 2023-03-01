#include "MarkerStorageItems.h"
#include "Common/CommonUtils.h"

int TargetMapMarker::_lastTargetNumber = 0;

// ----------------------------------------------------------------------------------------

bool MapMarker::dirty() const
{
    return _dirty;
}

ArtillerySpotterState MapMarker::artillerySpotterState() const
{
    return _artillerySpotterState;
}

void MapMarker::setArtillerySpotterState(const ArtillerySpotterState &artillerySpotterState)
{
    if (_artillerySpotterState != artillerySpotterState)
    {
        _artillerySpotterState = artillerySpotterState;
        _dirty = true;
        _needImageUpdate = true;
        emit onDisplayedImageChanged();
    }
}

MapMarker::MapMarker(QObject *parent, const QString &guid, const WorldGPSCoord &gpsCoord, MarkerTemplate *mapMarkerTemplate,
                     int tag, const QString &description, ArtillerySpotterState artillerySpotterState) : QObject(parent)
{
    _needImageUpdate = true;
    _dirty = false;
    _GUID = guid;
    _gpsCoord = gpsCoord;
    _mapMarkerTemplate = mapMarkerTemplate;
    _tag = tag;
    _artillerySpotterState = artillerySpotterState;
    if (description.isEmpty())
        _description = _mapMarkerTemplate->description();
    else
        _description = description;
}

const WorldGPSCoord MapMarker::gpsCoord() const
{
    return _gpsCoord;
}

void MapMarker::setGPSCoord(const WorldGPSCoord &coord)
{
    Q_ASSERT(coord.CoordSystem == WGS84);

    if (_gpsCoord != coord)
    {
        _gpsCoord = coord;
        _dirty = true;
        emit onCoodChanged();
    }
}

const QString MapMarker::GUID() const
{
    return _GUID;
}

const QString MapMarker::templateGUID() const
{
    return _mapMarkerTemplate->GUID();
}

const QString MapMarker::description() const
{
    return _description;
}

const QString MapMarker::hint() const
{
    QPointF sk42coord = _gpsCoord.getSK42();

    QString spotterStateCaption = _artillerySpotterState == ArtillerySpotterState::Unspecified ?
                QString("") :
                QString("\n%1").arg(MapArtillerySpotterStateCaptions()[_artillerySpotterState]);

    QString hint = QString(tr("%1\n%2\n%3\nHeight: %4\nX:%5\nY:%6%7"))
            .arg(_description)
            .arg(_gpsCoord.EncodeLatitude(DegreeMinutesSeconds))
            .arg(_gpsCoord.EncodeLongitude(DegreeMinutesSeconds))
            .arg(_gpsCoord.hmsl)
            .arg(sk42coord.x(), 0, 'f', 2, '0').arg(sk42coord.y(), 0, 'f', 2, '0')
            .arg(spotterStateCaption);
    return hint;
}

void MapMarker::setDescription(const QString &description)
{
    if (_description != description)
    {
        _description = description;
        _dirty = true;
        emit onDescriptionChanged();
    }
}

int MapMarker::tag()
{
    return _tag;
}

void MapMarker::setDirty(bool value)
{
    _dirty = value;
}

const QPixmap MapMarker::displayedImage()
{
    return _mapMarkerTemplate->image();
}

// ----------------------------------------------------------------------------------------

TargetMapMarker::TargetMapMarker(QObject *parent, const QString &guid, const WorldGPSCoord &gpsCoord, MarkerTemplate *mapMarkerTemplate,
                                 int tag, const QString &description, ArtillerySpotterState artillerySpotterState) :
    MapMarker(parent, guid, gpsCoord, mapMarkerTemplate, tag, description, artillerySpotterState)
{
    if (tag == 0) //new marker will be created
        _tag = ++TargetMapMarker::_lastTargetNumber;
    else
        TargetMapMarker::_lastTargetNumber = qMax(tag, TargetMapMarker::_lastTargetNumber);

    _isHighlighted = false;
    const QString tamplateDescription = mapMarkerTemplate->description();

    if (_description == tamplateDescription)
        _description = QString("%1 # %2 %3").arg(tamplateDescription).arg(_tag).arg(QDateTime::currentDateTime().toString("yyyy.MM.dd hh:mm:ss"));
}

const QPixmap TargetMapMarker::displayedImage()
{
    if (_needImageUpdate)
    {
        _image = _isHighlighted ? _mapMarkerTemplate->highlightedImage().copy() :
                                  _mapMarkerTemplate->image().copy();
        auto color = _isHighlighted ?  qRgb(255, 0, 0) : qRgb(0, 255, 255);

        QPainter painter(&_image);

        //Draw Artillery Spotter State Indicator
        if (_artillerySpotterState != ArtillerySpotterState::Unspecified)
        {
            QPen pen = painter.pen();
            pen.setColor(MapArtillerySpotterStateColors[_artillerySpotterState]);
            pen.setWidth(4);
            painter.setPen(pen);
            painter.drawEllipse(_image.rect());
        }

        //Draw Target Number
        QFont font = painter.font();
        font.setPointSize(12);
        font.setBold(true);
        painter.setFont(font);
        QPen pen = painter.pen();
        pen.setColor(color);
        painter.setPen(pen);
        painter.drawText(_image.rect(), QString::number(_tag), Qt::AlignLeft | Qt::AlignTop);

        _needImageUpdate = false;
    }
    return _image;
}

bool TargetMapMarker::isHighlighted()
{
    return _isHighlighted;
}

void TargetMapMarker::setHighlighted(bool value)
{
    if (_isHighlighted != value)
    {
        _isHighlighted = value;
        _needImageUpdate = true;
        emit onDisplayedImageChanged();
    }
}

// ----------------------------------------------------------------------------------------

PartyMapMarker::PartyMapMarker(QObject *parent, const QString &guid, const WorldGPSCoord &gpsCoord, MarkerTemplate *mapMarkerTemplate,
                               const QString &description, MarkerParty party, ArtillerySpotterState artillerySpotterState):
    MapMarker(parent, guid, gpsCoord, mapMarkerTemplate, 0, description, artillerySpotterState)
{
    _party = party;
}

const QPixmap PartyMapMarker::displayedImage()
{
    if (_needImageUpdate)
    {
        if (_party == MarkerParty::Neutral)
            _image = MapMarker::displayedImage();

        QColor color = Qt::black;
        if (_party == MarkerParty::Allies)
            color = Qt::blue;
        else if (_party == MarkerParty::Enemies)
            color = Qt::red;

        _image =  changeImageColor(_mapMarkerTemplate->image(), color);
        _needImageUpdate = false;
    }

    return _image;
}

MarkerParty PartyMapMarker::getParty()
{
    return _party;
}

void PartyMapMarker::setParty(MarkerParty value)
{
    if (_party != value)
    {
        _party = value;
        _dirty = true;
        _needImageUpdate = true;
        emit onDisplayedImageChanged();
    }
}

void PartyMapMarker::setNextParty()
{
    switch (_party)
    {
    case MarkerParty::Neutral:
        setParty(MarkerParty::Allies);
        break;
    case MarkerParty::Allies:
        setParty(MarkerParty::Enemies);
        break;
    case MarkerParty::Enemies:
        setParty(MarkerParty::Neutral);
        break;
    default:
        setParty(MarkerParty::Neutral);
        break;
    }
}

const QList<SAMInfo *> SAMMapMarker::samInfoList()
{
    return _mapMarkerTemplate->samInfoList();
}

SAMMapMarker::SAMMapMarker(QObject *parent, const QString &guid, const WorldGPSCoord &gpsCoord, MarkerTemplate *mapMarkerTemplate,
                           const QString &description, MarkerParty party, ArtillerySpotterState artillerySpotterState) :
    PartyMapMarker(parent, guid, gpsCoord, mapMarkerTemplate, description, party, artillerySpotterState)
{

}

SAMInfo SAMMapMarker::getSAMinfo(double height)
{
    return _mapMarkerTemplate->getSAMinfo(height);
}



const QMap<int, QString> MapArtillerySpotterStateCaptions()
{
    static const QMap <int, QString> MapArtillerySpotterStateCaptions {
        { ArtillerySpotterState::Unspecified,       QObject::tr("Unspecified") },
        { ArtillerySpotterState::DefeatRequired,    QObject::tr("Defeat Required") },
        { ArtillerySpotterState::TrialShot,         QObject::tr("Trial Shot") },
        { ArtillerySpotterState::RealShot,          QObject::tr("Real Shot") } };

    return MapArtillerySpotterStateCaptions;
}
