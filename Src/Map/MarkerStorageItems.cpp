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
        emit onDisplayedImageChanged();
    }
}

MapMarker::MapMarker(QObject *parent, const QString &guid, const WorldGPSCoord &gpsCoord, MarkerTemplate *mapMarkerTemplate,
                     int tag, const QString &description, ArtillerySpotterState artillerySpotterState) : QObject(parent)
{
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

    QString hint = QString(tr("%1\n%2\n%3\nHeight: %4\nX:%5\nY:%6"))
            .arg(_description)
            .arg(_gpsCoord.EncodeLatitude(DegreeMinutesSeconds))
            .arg(_gpsCoord.EncodeLongitude(DegreeMinutesSeconds))
            .arg(_gpsCoord.hmsl)
            .arg(sk42coord.x(), 0, 'f', 2, '0').arg(sk42coord.y(), 0, 'f', 2, '0');
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

void TargetMapMarker::initImageWithTag()
{
    _image = _mapMarkerTemplate->image().copy();
    {
        QPainter painter(&_image);
        QFont font = painter.font();
        font.setPointSize(12);
        font.setBold(true);
        painter.setFont(font);
        QPen pen = painter.pen();
        pen.setColor(qRgb(0, 255, 255));
        painter.setPen(pen);
        painter.drawText(_image.rect(), QString::number(_tag), Qt::AlignLeft | Qt::AlignTop);
    }
    _highlightedImage = _mapMarkerTemplate->highlightedImage().copy();
    {
        QPainter painter(&_highlightedImage);
        QFont font = painter.font();
        font.setPointSize(12);
        font.setBold(true);
        painter.setFont(font);
        QPen pen = painter.pen();
        pen.setColor(qRgb(255, 0, 0));
        painter.setPen(pen);
        painter.drawText(_highlightedImage.rect(), QString::number(_tag), Qt::AlignLeft | Qt::AlignTop);
    }
}

TargetMapMarker::TargetMapMarker(QObject *parent, const QString &guid, const WorldGPSCoord &gpsCoord, MarkerTemplate *mapMarkerTemplate,
                                 int tag, const QString &description, ArtillerySpotterState artillerySpotterState) :
    MapMarker(parent, guid, gpsCoord, mapMarkerTemplate, tag, description, artillerySpotterState)
{
    if (tag == 0) //new marker will be created
        _tag = ++TargetMapMarker::_lastTargetNumber;
    else
        TargetMapMarker::_lastTargetNumber = qMax(tag, TargetMapMarker::_lastTargetNumber);

    initImageWithTag();

    _isHighlighted = false;
    const QString tamplateDescription = mapMarkerTemplate->description();

    if (_description == tamplateDescription)
        _description = QString("%1 # %2 %3").arg(tamplateDescription).arg(_tag).arg(QDateTime::currentDateTime().toString("yyyy.MM.dd hh:mm:ss"));
}

const QPixmap TargetMapMarker::displayedImage()
{
    if (_isHighlighted)
        return _highlightedImage;
    else
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
    if (_party == MarkerParty::Neutral)
        return MapMarker::displayedImage();

    QColor color = Qt::black;
    if (_party == MarkerParty::Allies)
        color = Qt::blue;
    else if (_party == MarkerParty::Enemies)
        color = Qt::red;

    return changeImageColor(_mapMarkerTemplate->image(), color);
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
    case Enemies:
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

