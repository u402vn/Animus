#include "MapTileContainer.h"
#include <QObject>
#include <QTranslator>
#include <QDebug>
#include <QSqlError>
#include "EnterProc.h"
#include "Common/CommonUtils.h"

quint32 TileDatabaseConnection::_databaseCounter = 0;

const int UPPER_TILE_HASH_SIZE = 120;
const int LOWER_TILE_HASH_SIZE = (UPPER_TILE_HASH_SIZE * 70) / 100;
const int USING_TILE_HISTORY_DEPTH = 6;
const int UNCOMMITED_TILES_MAX_COUNT = 50;

void ConvertGoogleXY2GPS_2D(int scale, double x, double y, WorldGPSCoord &coord);

void MapTileContainer::createTileDatabaseConnections(const QList<QString> &mapDatabaseFiles, const QString &downloadCasheDatabaseFile)
{
    EnterProc("MapTileContainer::createTileDatabaseConnections");


    qInfo() << "Begin Create Tile Database Connections";

    foreach (const QString mapFile, mapDatabaseFiles)
    {
        if (mapFile == downloadCasheDatabaseFile)
            continue;

        TileDatabaseConnection *databaseConnection = new TileDatabaseConnection(this, mapFile);
        auto supportedSources = databaseConnection->getSupportedSources();

        foreach (const int sourceId, supportedSources)
        {
            MapTileSourceInfo *sourceInfo = _sourceInfos.value(sourceId);
            sourceInfo->dbConnections()->append(databaseConnection);
        }
    }

    if (!downloadCasheDatabaseFile.isEmpty())
    {
        _downloadCasheDatabaseConnection = new TileDatabaseConnection(this, downloadCasheDatabaseFile);

        auto supportedSourcesForDownload = _mapTileDownloader.getSupportedSources();

        auto i = _sourceInfos.begin();
        while (i != _sourceInfos.end())
        {
            if (supportedSourcesForDownload.contains(i.key()))
                i.value()->dbConnections()->append(_downloadCasheDatabaseConnection);
            ++i;
        }
    }
    else
        _downloadCasheDatabaseConnection = nullptr;

    _lastSuccessfulConnection = nullptr;

    qInfo() << "End Create Tile Database Connections";
}

MapTileContainer::MapTileContainer(QObject *parent, const QList<QString> &mapDatabaseFiles, const QString &downloadCasheDatabaseFile, const QString &heightMapFile) : QObject(parent),
    _mapTileDownloader(this),
    _heightMapContainer(this, heightMapFile)
{
    EnterProc("MapTileContainer::MapTileContainer");

    qInfo() << "Begin Init Map Tile Container";

    _tileReceivingMode = TileReceivingMode::DatabaseOnly;

    _coordFormat = DegreeMinutesSeconds; //todo move to settings
    _coordSystem = WGS84;

    _usingCounter = 1;
    _mapBaseId = NoBaseTile;
    _mapHybridId = NoHybridTile;
    _scale = 12;

    connect(this, &MapTileContainer::needTile, &_mapTileDownloader, &MapTileDownloader::needTile, Qt::ConnectionType::QueuedConnection);
    connect(&_mapTileDownloader, &MapTileDownloader::tileReceived, this, &MapTileContainer::tileReceived, Qt::ConnectionType::QueuedConnection);

    fillSourceInfos();

    createTileDatabaseConnections(mapDatabaseFiles, downloadCasheDatabaseFile);

    _noTileImageBlack = new QPixmap(TILE_WIDTH, TILE_HEIGHT);
    _noTileImageBlack->fill(qRgb(0, 0, 0));
    //http://qtdevnotes.blogspot.com.by/2010/04/transparent-qimage-or-qpixmap-picture.html
    _noTileImageTransparent = new QPixmap(TILE_WIDTH, TILE_HEIGHT);
    _noTileImageTransparent->fill(Qt::transparent);

    _lastTileIndex = 0;

    //_geoCoder = new GeoCoder(applicationSettings.getDatabaseGeocoder().toLatin1().data());

    qInfo() << "End Init Map Tile Container";
}

MapTileContainer::~MapTileContainer()
{
    delete _noTileImageBlack;
    delete _noTileImageTransparent;
    //delete _geoCoder;

    auto i = _tileHash.begin();
    while (i != _tileHash.end())
    {
        MapTile *tile = i.value();
        i = _tileHash.erase(i);
        delete tile;
    }
}

void MapTileContainer::setImageCenterGPS(const WorldGPSCoord &screenCenter)
{
    Q_ASSERT(screenCenter.CoordSystem == WGS84);
    _screenCenter = screenCenter;

    double screenCenterHeight;
    if (_heightMapContainer.GetHeight(screenCenter.lat, screenCenter.lon, screenCenterHeight))
        _screenCenter.hmsl = screenCenterHeight;
    else
        _screenCenter.hmsl = 0;
}

// http://gis-lab.info/qa/wgs84-sk42-wgs84-formula.html
// http://gis-lab.info/qa/datum-transform-sets.html#.D0.A1.D0.9A-42_-.3E_WGS84
// http://belarus.news-city.info/docs/2011by/crfxfnm-tcgkfnyj10602.htm
void MapTileContainer::setImageCenter(const WorldGPSCoord &screenCenter)
{    
    switch(screenCenter.CoordSystem)
    {
    case WGS84:
        setImageCenterGPS(screenCenter);
        break;
    case SK42:
        setImageCenterGPS(screenCenter.convertSK42toWGS84());
        break;
    }
}

void MapTileContainer::setScale(int scale)
{
    _scale = scale;
}

int MapTileContainer::getScale() const
{
    return _scale;
}

void MapTileContainer::setCoordSystem(GlobalCoordSystem coordSystem)
{
    _coordSystem = coordSystem;
}

GlobalCoordSystem MapTileContainer::getCoordSystem() const
{
    return _coordSystem;
}

void MapTileContainer::setTileReceivingMode(TileReceivingMode mode)
{
    _tileReceivingMode = mode;
}

void MapTileContainer::setMapBaseSourceId(MapBaseTileSource sourceId)
{
    if (_mapBaseId == sourceId)
        return;
    _mapBaseId = sourceId;
}

MapBaseTileSource MapTileContainer::getMapBaseSourceId() const
{
    return _mapBaseId;
}

void MapTileContainer::setMapHybridSourceId(MapHybridTileSource hybridId)
{
    if (_mapHybridId == hybridId)
        return;
    _mapHybridId = hybridId;
}

MapHybridTileSource MapTileContainer::getMapHybridSourceId() const
{
    return _mapHybridId;
}

void MapTileContainer::clenupTileHash()
{
    int tileHashSize = _tileHash.size();
    if (tileHashSize > UPPER_TILE_HASH_SIZE)
    {
        auto i = _tileHash.begin();
        while ((i != _tileHash.end()) && (tileHashSize > LOWER_TILE_HASH_SIZE))
        {
            MapTile *tile = i.value();
            if (tile->lastUsing < _usingCounter - USING_TILE_HISTORY_DEPTH)
            {
                i = _tileHash.erase(i);
                delete tile;
                tileHashSize--;
            }
            else
                ++i;
        }
    }
}

QPixmap * MapTileContainer::getTileImage(int tileX, int tileY, int scale, int sourceId)
{
    EnterProc("MapTileContainer::getTileImage");

    MapTileHashValue tileHashValue = MapTile::calculateMapTileHash(sourceId, scale, tileX, tileY);

    MapTile *tile = _tileHash.value(tileHashValue, nullptr);
    if (tile != nullptr)
    {
        if (tile->isEqual(sourceId, scale, tileX, tileY))
        {
            tile->lastUsing = _usingCounter;
            return tile->image;
        }
        else
        {
            _tileHash.remove(tileHashValue);
            delete tile;
        }
    }

    //Need Loading. Try to load tile from databse
    QPixmap * resultTileImage = nullptr;

    //try to find cuccessful connection from presented in list
    if (_tileReceivingMode == DatabaseOnly || _tileReceivingMode == DatabaseAndNetwork)
    {

        if (_lastSuccessfulConnection != nullptr)
            resultTileImage = _lastSuccessfulConnection->getTileData(tileX, tileY, scale, sourceId);
        if (resultTileImage == nullptr)
        {
            auto sourceInfo = _sourceInfos.value(sourceId);
            auto sourceConnections = *(sourceInfo->dbConnections());
            foreach (auto connection, sourceConnections)
            {
                if (_lastSuccessfulConnection == connection)
                    continue;

                resultTileImage = connection->getTileData(tileX, tileY, scale, sourceId);
                if (resultTileImage != nullptr)
                {
                    _lastSuccessfulConnection = connection;
                    break; //foreach
                }
            }
        }
    }
    else if (_downloadCasheDatabaseConnection != nullptr)
        resultTileImage = _downloadCasheDatabaseConnection->getTileData(tileX, tileY, scale, sourceId);


    //try to download tile
    if ( (_tileReceivingMode == NetworkOnly || _tileReceivingMode == DatabaseAndNetwork) && (resultTileImage == nullptr) )
        emit needTile(sourceId, scale, tileX, tileY);

    //try to make tile from other scales
    bool isHybridTile = isHybridTileSource(sourceId);

    if ((resultTileImage == nullptr) && (!isHybridTile))
    {
        if (scale > MIN_ZOOM_VALUE)
        {
            QPixmap * upperImage = getTileImage(tileX / 2, tileY / 2, scale - 1, sourceId);
            if (upperImage != nullptr)
            {
                QPixmap scaledImage = upperImage->scaled(TILE_WIDTH * 2, TILE_HEIGHT * 2, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
                int tilePosOnUpperX = (tileX % 2) * TILE_WIDTH;
                int tilePosOnUpperY = (tileY % 2) * TILE_HEIGHT;
                resultTileImage = new QPixmap(TILE_WIDTH, TILE_HEIGHT);
                QPainter painter;
                painter.begin(resultTileImage);
                painter.drawPixmap(0, 0, scaledImage, tilePosOnUpperX, tilePosOnUpperY, TILE_WIDTH, TILE_HEIGHT);
                painter.end();
            }
        }
    }

    bool deleteImageOnDestroy = true;
    if (resultTileImage == nullptr)
    {
        if (isHybridTile)
            resultTileImage = _noTileImageTransparent;
        else
            resultTileImage = _noTileImageBlack;
        deleteImageOnDestroy = false;
    }

    MapTile *resultTile = new MapTile(sourceId, scale, tileX, tileY, resultTileImage, deleteImageOnDestroy);
    resultTile->lastUsing = _usingCounter;
    _tileHash.insert(tileHashValue, resultTile);

    return resultTile->image;
}

void MapTileContainer::tileReceived(int sourceId, int scale, int x, int y, const QByteArray &tileImageRawData)
{
    QPixmap *tileImage = new QPixmap();
    if (!tileImage->loadFromData(tileImageRawData))
        return;

    MapTileHashValue tileHashValue = MapTile::calculateMapTileHash(sourceId, scale, x, y);

    MapTile *tile = _tileHash.value(tileHashValue, nullptr);
    if (tile != nullptr)
        if (tile->isEqual(sourceId, scale, x, y))
        {
            _tileHash.remove(tileHashValue);
            delete tile;
        }

    tile = new MapTile(sourceId, scale, x, y, tileImage, true);
    tile->lastUsing = _usingCounter;
    _tileHash.insert(tileHashValue, tile);

    if (_downloadCasheDatabaseConnection != nullptr)
        _downloadCasheDatabaseConnection->saveTile(sourceId, scale, x, y, tileImageRawData);

    emit contentUpdated();
}

// http://habrahabr.ru/post/233809/
// http://wiki.openstreetmap.org/wiki/Slippy_map_tilenames
void ConvertGPS2GoogleXY(WorldGPSCoord coord, int scale, double &x, double &y)
{
    Q_ASSERT(coord.CoordSystem == WGS84);
    double scaleK = pow((double)2.0, scale);
    x = ((coord.lon + 180) / 360) * scaleK;
    double rLat = deg2rad(coord.lat);
    y = (1.0 - log(tan(rLat) + 1.0 / cos(rLat)) / PI) / 2.0 * scaleK;
}

const QPointF ConvertGPS2GoogleXY(WorldGPSCoord coord, int scale)
{
    double x, y;
    ConvertGPS2GoogleXY(coord, scale, x, y);
    QPointF point(x, y);
    return point;
}

// http://habrahabr.ru/post/233809/
// http://wiki.openstreetmap.org/wiki/Slippy_map_tilenames
void ConvertGoogleXY2GPS_2D(int scale, double x, double y, WorldGPSCoord &coord)
{
    double scaleK = pow((double)2.0, scale);
    double lon = x / scaleK * 360 - 180;
    double lat = atan(sinh(PI - y / scaleK * 2 * PI)) * 180 / PI;
    coord.lat = lat;
    coord.lon = lon;
    coord.hmsl = 0;
    coord.CoordSystem = WGS84;
}

// http://habrahabr.ru/post/151103/  - getMapTileFromCoordinates
void ConvertGPS2YandexXY(WorldGPSCoord coord, int scale, double &x, double &y)
{
    Q_ASSERT(coord.CoordSystem == WGS84);
    double E2 = (double)coord.lat * PI / 180;
    double sradiusa = 6378137.0;
    double sradiusb = 6356752.0;
    double J2 = (double)sqrt(sradiusa * sradiusa - sradiusb	* sradiusb)	/ sradiusa;
    double M2 = (double)log((1 + sin(E2)) / (1 - sin(E2))) / 2 - J2	* log((1 + J2 * sin(E2)) / (1 - J2 * sin(E2))) / 2;
    double B2 = (double)(1 << scale);

    x = (coord.lon + 180) / 360 * (1 << scale);
    y = B2 / 2 - M2 * B2 / 2 / PI;
}

void ConvertGPS2XY(int sourceId, int scale, const WorldGPSCoord coord, double &x, double &y)
{
    if ((sourceId == YandexSatellite) || (sourceId == YandexHybrid) ||  (sourceId == YandexMap))
        ConvertGPS2YandexXY(coord, scale, x, y);
    else
        ConvertGPS2GoogleXY(coord, scale, x, y);
}


double getImageResolutionMperPix(const WorldGPSCoord &coord, int scale)
{
    Q_ASSERT(coord.CoordSystem == WGS84);
    double rLat = deg2rad(coord.lat);
    double rz = 4 * PI * EARTH_RADIUS_M * cos(rLat) / ((double)TILE_WIDTH * pow((double)2, scale + 1));
    return rz;
}

WorldGPSCoord MapTileContainer::convertScreenXY2GPS_2D(int screenCenterDx, int screenCenterDy) const
{
    double x, y;
    ConvertGPS2GoogleXY(_screenCenter, _scale, x, y);
    x += ((double)screenCenterDx / TILE_WIDTH);
    y += ((double)screenCenterDy / TILE_HEIGHT);
    WorldGPSCoord coord;
    ConvertGoogleXY2GPS_2D(_scale, x, y, coord);
    return coord;
}

void MapTileContainer::drawTileLayerInternal(QPainter *imagePainter, int sourceId, int scale, double centerX, double centerY, bool drawTileNumber)
{
    int imageHeight = imagePainter->device()->height();
    int imageWidth = imagePainter->device()->width();
    int tileX = (int)floor(centerX);
    int tileY = (int)floor(centerY);
    int offsetX = (int)floor((centerX - tileX) * TILE_WIDTH);
    int offsetY = (int)floor((centerY - tileY) * TILE_HEIGHT);
    _usingCounter++;

    int fromTileX = (int)ceil((double)tileX - (imageWidth / 2 - offsetX) / TILE_WIDTH - 1);
    int toTileX = (int)ceil((double)tileX + (imageWidth / 2 + offsetX) / TILE_WIDTH + 0);
    int fromTileY = (int)ceil((double)tileY - (imageHeight / 2 - offsetY) / TILE_HEIGHT - 1);
    int toTileY = (int)ceil((double)tileY + (imageHeight / 2 + offsetY) / TILE_HEIGHT + 0);

    //Draw tiles
    for (int i = fromTileX; i <= toTileX; i++)
    {
        int posX = (i - tileX) * TILE_WIDTH + imageWidth / 2 - offsetX;

        if ((posX < -TILE_WIDTH) || (posX >= imageWidth))
            continue;

        for (int j = fromTileY; j <= toTileY; j++)
        {
            int posY = (j - tileY) * TILE_HEIGHT + imageHeight / 2 - offsetY;

            if ((posY < -TILE_HEIGHT) || (posY >= imageHeight))
                continue;

            QPixmap * tileImage = getTileImage(i, j, scale, sourceId);
            if (tileImage == nullptr)
                tileImage = _noTileImageBlack;

            imagePainter->drawPixmap(posX, posY, *tileImage);

            if (drawTileNumber)
            {
                imagePainter->setPen(QColor(Qt::white));
                imagePainter->drawRect(posX, posY, TILE_WIDTH, TILE_HEIGHT);
                const int tileIndexesTextOffset = 4;
                QString tileInexesText = QString("x=%1\ny=%2").arg(i).arg(j);
                imagePainter->drawText(QRectF(posX + tileIndexesTextOffset, posY + tileIndexesTextOffset, TILE_WIDTH, TILE_HEIGHT), Qt::TextWordWrap, tileInexesText);
            }
        }  // for j
    } // for i
}


void MapTileContainer::drawTileLayer(QPainter * imagePainter, int sourceId, bool drawTileNumber)
{
    double centerX, centerY;
    ConvertGPS2XY(sourceId, _scale, _screenCenter, centerX, centerY);
    drawTileLayerInternal(imagePainter, sourceId, _scale, centerX, centerY, drawTileNumber);
}

inline bool DecodeSingleCoord(QString coordStr, double &coord, QString &site)
{
    QString gradStr, minStr, secStr;
    int gradPos = coordStr.indexOf("°");
    int minPos = coordStr.indexOf("'");
    int secPos = coordStr.indexOf("\"");
    gradStr = coordStr.mid(0, gradPos);
    minStr = coordStr.mid(gradPos + 1, minPos - gradPos - 1);
    secStr = coordStr.mid(minPos + 1, secPos - minPos - 1);
    site = coordStr.mid(secPos + 1, 2);

    bool ok;
    double grad = gradStr.toDouble(&ok);
    if (!ok)
        return false;
    double min = minStr.toDouble(&ok);
    if (!ok)
        return false;
    double sec = secStr.toDouble(&ok);
    if (!ok)
        return false;

    coord = grad + min / 60.0 + sec / 3600.0;
    return true;
}

bool DecodeLatLon(const QString &encodedLatLon, GeographicalCoordinatesFormat format, double &lat, double &lon)
{
    QStringList coordList = encodedLatLon.split("\t");
    bool ok;
    double coord = 0;
    QString site;
    ok = DecodeSingleCoord(coordList[0], coord, site);
    if (site == WorldGPSCoord::postfixN()) // "СШ"
        lat =  coord;
    else if (site == WorldGPSCoord::postfixS()) // "ЮШ"
        lat = -coord;
    else
        ok = false;

    coord = 0;
    ok = ok && DecodeSingleCoord(coordList[1], coord, site);
    if (site == WorldGPSCoord::postfixE()) // "ВД"
        lon =  coord;
    else if (site == WorldGPSCoord::postfixW()) // "ЗД"
        lon = -coord;
    else
        ok = false;

    return ok;
}

int MapTileContainer::calculateScaleMaxWidth(double resolution, QString &middleLabelText, QString &maxLabelText)
{
    const double maxWidthPix = 400;
    const double minWidthPix = 200;

    double minMeter = minWidthPix * resolution;
    double maxMeter = maxWidthPix * resolution;

    int k = (int)log10(maxMeter);
    double startScaleMeter = pow(10, k);

    double n [8] = {1, 1.5, 2, 3, 4, 5, 10, 15};
    for (int i = 0; i < 8; i++)
    {
        double transformedScaleMeter = startScaleMeter * n[i];
        if ((transformedScaleMeter >= minMeter) && (transformedScaleMeter <= maxMeter))
        {
            if (transformedScaleMeter <= 2000)
            {
                middleLabelText = tr("%1 m").arg(transformedScaleMeter / 2, 0, 'f', 1);
                maxLabelText = tr("%1 m").arg(transformedScaleMeter, 0, 'f', 1);
            }
            else
            {
                middleLabelText = tr("%1 km").arg(transformedScaleMeter / 2000, 0, 'f', 1);
                maxLabelText = tr("%1 km").arg(transformedScaleMeter / 1000, 0, 'f', 1);
            }
            return (int)(transformedScaleMeter / resolution);
        }
    }

    middleLabelText = "?";
    maxLabelText = "?";
    return minWidthPix; //error behaviour
}

void MapTileContainer::drawLegend(QPainter * imagePainter)
{
    const int legendX = 0;
    int legendWidth = imagePainter->device()->width();
    const int legendHeight = 20;
    int legendY = imagePainter->device()->height() - legendHeight;

    double mapResolution = getImageResolutionMperPix(_screenCenter, _scale);

    QBrush transparentBrush(QColor(0, 0, 0, 200));
    imagePainter->fillRect(legendX, legendY, legendWidth, legendHeight, transparentBrush);

    imagePainter->setPen(QColor(Qt::white));
    imagePainter->setFont(QFont("Arial", 10, QFont::Normal));

    //todo trim long info for short width

    QString fieldScale = QString::number(_scale + 1);
    //todo support SK42 _coordSystem
    QString fieldLatLon;
    if (_coordSystem == SK42)
        fieldLatLon = _screenCenter.convertWGS84toSK42().EncodeLatLon(_coordFormat, false);
    else
        fieldLatLon = _screenCenter.EncodeLatLon(_coordFormat, false);

    QString fieldCoordSystem = getGlobalCoordSystemName(_coordSystem);
    QString fieldHeight = QString("%1").arg(_screenCenter.hmsl, 5, 'f', 0);
    QString fieldMapResolution = QString("%1").arg(mapResolution, 5, 'f', 2);
    QString fieldBaseLayer = getTileSourceName(_mapBaseId);
    QString fieldHybridLayer = getTileSourceName(_mapHybridId);
    if (!fieldHybridLayer.isEmpty())
        fieldHybridLayer = " + " + fieldHybridLayer;

    QString legendText = tr("\t  %COORDSYSTEM% %LATLON%   %HEIGHT%m       %MAPRESOLUTION%m/pix              Source: %BASELAYER%%HYBRIDLAYER% (%SCALE%)");
    legendText.replace("%SCALE%",         fieldScale);
    legendText.replace("%COORDSYSTEM%",   fieldCoordSystem);
    legendText.replace("%LATLON%",        fieldLatLon);
    legendText.replace("%HEIGHT%",        fieldHeight);
    legendText.replace("%MAPRESOLUTION%", fieldMapResolution);
    legendText.replace("%BASELAYER%",     fieldBaseLayer);
    legendText.replace("%HYBRIDLAYER%",   fieldHybridLayer);

    imagePainter->drawText(legendX, legendY, legendWidth, legendHeight, Qt::AlignLeft | Qt::AlignVCenter, legendText);

    //Out Scale
    QBrush whiteBrush(QColor(255, 255, 255, 255));
    QString middleLabelText, maxLabelText;
    int scaleWidth = calculateScaleMaxWidth(mapResolution, middleLabelText, maxLabelText);

    const int scaleHeight = 5;
    int scaleLeft = legendWidth - legendX - 40 - scaleWidth;
    int scaleTop = legendY + 12;
    imagePainter->drawRect(scaleLeft, scaleTop, scaleWidth, scaleHeight);
    imagePainter->fillRect(scaleLeft + scaleWidth / 4, scaleTop, scaleWidth / 4, scaleHeight, whiteBrush);
    imagePainter->fillRect(scaleLeft + scaleWidth * 3 / 4, scaleTop, scaleWidth - scaleWidth * 3 / 4, scaleHeight, whiteBrush);

    imagePainter->setFont(QFont("Arial", 8, QFont::Normal));

    QRect minLabelRect = QRect(scaleLeft - 40, scaleTop - 14, 80, 20);
    imagePainter->drawText(minLabelRect, Qt::AlignHCenter | Qt::AlignTop, "0");

    QRect middleLabelRect = QRect(scaleLeft + scaleWidth / 2 - 40, scaleTop - 14, 80, 20);
    imagePainter->drawText(middleLabelRect, Qt::AlignHCenter | Qt::AlignTop, middleLabelText);
    QRect maxLabelRect = QRect(scaleLeft + scaleWidth - 40, scaleTop - 14, 80, 20);
    imagePainter->drawText(maxLabelRect, Qt::AlignHCenter | Qt::AlignTop, maxLabelText);
    //    imagePainter.drawText(scaleLeft + scaleWidth / 2, scaleTop, middleLabel);
    //    imagePainter.drawText(scaleLeft + scaleWidth, scaleTop, maxLabel);
}

void MapTileContainer::drawParallelsMeridians(QPainter *imagePainter)
{
    //todo SUPPORT SK42 _coordSystem
    double imageHeight = imagePainter->device()->height();
    double imageWidth = imagePainter->device()->width();
    double centerX, centerY;
    // copy from ConvertScreenXY2GPS_2D
    ConvertGPS2GoogleXY(_screenCenter, _scale, centerX, centerY);
    double minX = centerX - imageWidth / TILE_WIDTH / 2;
    double maxX = centerX + imageWidth / TILE_WIDTH / 2;
    double minY = centerY + imageHeight / TILE_HEIGHT / 2;
    double maxY = centerY - imageHeight / TILE_HEIGHT / 2;

    WorldGPSCoord minCoord, maxCoord;
    ConvertGoogleXY2GPS_2D(_scale, minX, minY, minCoord);
    ConvertGoogleXY2GPS_2D(_scale, maxX, maxY, maxCoord);

    //Prepare text out
    imagePainter->setPen(QColor(Qt::white));
    imagePainter->setFont(QFont("Arial", 8, QFont::Normal));
    QBrush transparentBrush(QColor(0, 0, 0, 200));
    QFontMetrics currentFontMetrics(imagePainter->font());
    WorldGPSCoord metricsCoord(-180, -90);
    int textRectangleWidth = currentFontMetrics.width(metricsCoord.EncodeLatitude(_coordFormat)) + 5;
    int textRectangleHeight = currentFontMetrics.height();

    //Calculate lines step and range
    double latSecPerImage = fabs(maxCoord.lat - minCoord.lat) * 3600;
    double lonSecPerImage = fabs(maxCoord.lon - minCoord.lon) * 3600;

    const int gridStepsCount = 17;
    const double gridStepsSec [gridStepsCount] = {1, 2, 3, 5, 10, 20, 30, 60, 120, 300, 600, 1200, 1800, 3600, 7200, 18000, 36000};
    double gridStepSec = gridStepsSec[gridStepsCount - 1];

    bool needShowGrid = false;
    for (int i = 0; i < gridStepsCount; i++)
    {
        double gridStepCandidateSec = gridStepsSec[i];
        needShowGrid = needShowGrid ||
                ((latSecPerImage <= gridStepsSec[0])|| (lonSecPerImage <= gridStepsSec[0])) ||
                ((gridStepCandidateSec < latSecPerImage) && (gridStepCandidateSec < lonSecPerImage));
        bool canShowGrid = (lonSecPerImage / gridStepCandidateSec * textRectangleWidth < imageWidth);

        if (needShowGrid && canShowGrid)
        {
            gridStepSec = gridStepCandidateSec;
            needShowGrid = false;
        }
    }

    double minGridLat = floor(minCoord.lat * 3600 / gridStepSec) * gridStepSec / 3600;
    double maxGridLat = ceil(maxCoord.lat * 3600 / gridStepSec) * gridStepSec / 3600;

    double minGridLon = floor(minCoord.lon * 3600 / gridStepSec) * gridStepSec / 3600;
    if (minGridLon < -180)
        minGridLon = -180;
    double maxGridLon = ceil(maxCoord.lon * 3600 / gridStepSec) * gridStepSec / 3600;
    if (maxGridLon > 180)
        maxGridLon = 180;

    //Draw Parallels
    for (double gridLat = minGridLat; gridLat <= maxGridLat; gridLat += ((double)gridStepSec / 3600))
    {
        WorldGPSCoord lineCoord(gridLat, minGridLon, 0);
        double gridX, gridY;
        ConvertGPS2GoogleXY(lineCoord, _scale, gridX, gridY);
        int gridLinePosY = imageHeight / 2 - (centerY - gridY) * TILE_HEIGHT;
        imagePainter->drawLine(0, gridLinePosY, imageWidth, gridLinePosY);

        QRectF textRectangle = QRectF(0, gridLinePosY + 1, textRectangleWidth, textRectangleHeight);
        imagePainter->fillRect(textRectangle, transparentBrush);
        QString latText = lineCoord.EncodeLatitude(_coordFormat);
        imagePainter->drawText(textRectangle, latText);
    }

    //Draw Meridians
    for (double gridLon = minGridLon; gridLon <= maxGridLon; gridLon += ((double)gridStepSec / 3600))
    {
        WorldGPSCoord lineCoord(minGridLat, gridLon, 0);
        double gridX, gridY;
        ConvertGPS2GoogleXY(lineCoord, _scale, gridX, gridY);
        int gridLinePosX = imageWidth / 2 - (centerX - gridX) * TILE_WIDTH;
        imagePainter->drawLine(gridLinePosX, 0, gridLinePosX, imageHeight);

        QRectF textRectangle = QRectF(gridLinePosX + 1, 0, textRectangleWidth, textRectangleHeight);
        imagePainter->fillRect(textRectangle, transparentBrush);
        QString lonText = lineCoord.EncodeLongitude(_coordFormat);
        imagePainter->drawText(textRectangle, lonText);
    }
}

void MapTileContainer::addSourceInfo(int sourceId, bool isHybrid, const QString &name)
{
    _sourceInfos.insert(sourceId, new MapTileSourceInfo(this, sourceId, isHybrid, name));
}

void MapTileContainer::fillSourceInfos()
{
    addSourceInfo(NoBaseTile,      false, tr("None"));
    addSourceInfo(YandexSatellite, false, tr("Yandex Satellite"));
    addSourceInfo(JointStaffMap,   false, tr("General Staff"));
    addSourceInfo(GoogleSatellite, false, tr("Google Satellite"));
    addSourceInfo(WikiMap,         false, tr("Wiki Map"));
    addSourceInfo(BingSatellite,   false, tr("Bing Satellite"));
    addSourceInfo(MapsByMap,       false, tr("Maps.By Map"));
    addSourceInfo(YandexMap,       false, tr("Yandex Map"));
    addSourceInfo(ForestMap,       false, tr("Forest Map"));

    addSourceInfo(NoHybridTile,    true, tr("None"));
    addSourceInfo(YandexHybrid,    true, tr("Yandex hybrid"));
    addSourceInfo(GoogleHybrid,    true, tr("Google hybrid"));
}

void MapTileContainer::getMapImageInternal(QPainter * imagePainter, const LegendPresentationParam &legendPresentationParam)
{
    EnterProc("MapTileContainer::GetMapImageInternal");

    bool worldMatrixEnabled = imagePainter->worldMatrixEnabled();

    imagePainter->setWorldMatrixEnabled(false);

    drawTileLayer(imagePainter, _mapBaseId, legendPresentationParam.drawBaseTileNumber);

    if (_mapHybridId != NoHybridTile)
        drawTileLayer(imagePainter, _mapHybridId, false);

    clenupTileHash();

    if (legendPresentationParam.drawLegend)
        drawLegend(imagePainter);
    if (legendPresentationParam.drawParallelsMeridians)
        drawParallelsMeridians(imagePainter);

    imagePainter->setWorldMatrixEnabled(worldMatrixEnabled);
}

void MapTileContainer::getMapImage(QPainter * painter, const LegendPresentationParam &legendPresentationParam)
{
    getMapImageInternal(painter, legendPresentationParam);
}

QImage *MapTileContainer::getLayerImage(const QSize &size, int sourceId, int scale, double centerX, double centerY)
{
    QImage *image = new QImage(size, QImage::Format::Format_RGB32); //???
    QPainter imagePainter;
    imagePainter.begin(image);
    drawTileLayerInternal(&imagePainter, sourceId, scale, centerX, centerY, false);
    imagePainter.end();
    return image;
}

bool MapTileContainer::goToPosByAddress(const QString &address)
{
    /*
    double gps_lat, gps_lon;
    bool posResult = _geoCoder->GetCoordByAddress(address, gps_lat, gps_lon);
    if (posResult)
    {
        WorldGPSCoord screenCenter(gps_lat, gps_lon);
        setImageCenterGPS(screenCenter);
    }
    return posResult;
    */
    return false; //stub only
}

const QString MapTileContainer::getGlobalCoordSystemName(GlobalCoordSystem system) const
{
    switch (system)
    {
    case WGS84:
        return tr("WGS-84");
    case SK42:
        return tr("СК-42");
    default:
        return QString("");
    };
}

const QString MapTileContainer::getTileSourceName(int sourceId) const
{
    auto sourceInfo = _sourceInfos.value(sourceId);
    if (sourceInfo == nullptr)
        return QString();
    return sourceInfo->name();
}

bool MapTileContainer::isHybridTileSource(int sourceId) const
{    
    if (sourceId == _mapBaseId)
        return false;
    else if (sourceId == _mapHybridId)
        return true;

    auto sourceInfo = _sourceInfos.value(sourceId);
    if (sourceInfo == nullptr)
        return false;
    return sourceInfo->isHybrid();
}

const QList<MapTileSourceInfo *> MapTileContainer::getTileSourceInfos() const
{
    return _sourceInfos.values();
}

QMultiMap<int, QString> MapTileContainer::getTileSource2DBFileMap()
{
    QMultiMap<int, QString> result;

    foreach (auto sourceInfo, _sourceInfos)
    {
        auto conections = *(sourceInfo->dbConnections());
        foreach (auto connection, conections)
            result.insert(sourceInfo->sourceId(), connection->getFileName());
    }

    return result;
}

void MapTileContainer::calculateVisibleAreaCornersCoords(int areaWidth, int areaHeight, WorldGPSCoord &coordLeftTop, WorldGPSCoord &coordRightBottom)
{
    coordLeftTop = convertScreenXY2GPS_2D(-areaWidth / 2, -areaHeight / 2);
    coordRightBottom = convertScreenXY2GPS_2D(areaWidth / 2, areaHeight / 2);
}

void MapTileContainer::convertGoogleXY2GPS(int scale, double x, double y, WorldGPSCoord &coord)
{
    ConvertGoogleXY2GPS_2D(scale, x, y, coord);

    double screenCenterHeight;
    if (_heightMapContainer.GetHeight(coord.lat, coord.lon, screenCenterHeight))
        coord.hmsl = screenCenterHeight;
    else
        coord.hmsl = 0;
}

//-----------------------------------------------------------

void TileDatabaseConnection::connectToDatabase()
{
    EnterProc("TileDatabaseConnection::connectToDatabase");

    qInfo() << "Connect to Database: " << _fileName;

    _databaseCounter++;

    _tileDatabase =  QSqlDatabase::addDatabase("QSQLITE", QString("TileDatabase%1").arg(_databaseCounter));
    _tileDatabase.setDatabaseName(_fileName);
    _tileDatabase.setConnectOptions("OCI_ATTR_PREFETCH_ROWS=1;OCI_ATTR_PREFETCH_MEMORY=1"); //???
    _tileDatabase.open();
    LOG_SQL_ERROR(_tileDatabase);

    EXEC_SQL(_tileDatabase, "PRAGMA journal_mode = MEMORY");

    EXEC_SQL(_tileDatabase, "CREATE TABLE IF NOT EXISTS MapTile (x INTEGER, y INTEGER, scale INTEGER, sourceId INTEGER, format INTEGER, autogenerated INTEGER, datetime REAL, signature INTEGER, tile BLOB)");

    EXEC_SQL(_tileDatabase, "CREATE TABLE IF NOT EXISTS MapTileSources (sourceId Integer)");


    EXEC_SQL(_tileDatabase, "CREATE INDEX IF NOT EXISTS TILE_SCALE_X_Y_SourceID ON MapTile (x, y, scale, sourceId)");
    //EXEC_SQL(_tileDatabase, "CREATE UNIQUE INDEX IF NOT EXISTS TILE_SCALE_X_Y_SourceID ON MapTile (x, y, scale, sourceId)");

    _selectTileQuery = new QSqlQuery(_tileDatabase);
    _selectTileQuery->prepare("SELECT tile, format FROM MapTile WHERE x=? AND y=? AND scale=? AND sourceId=?");
    LOG_SQL_ERROR(_selectTileQuery);


    _insertTileQuery = new QSqlQuery(_tileDatabase);
    _insertTileQuery->prepare("INSERT INTO MapTile (x, y, scale, sourceId, format, autogenerated, datetime, signature, tile) " \
                              "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?)");
    LOG_SQL_ERROR(_insertTileQuery);

    processMapTileSources();

    _tileDatabase.transaction();
    LOG_SQL_ERROR(_tileDatabase);
}

void TileDatabaseConnection::processMapTileSources()
{
    EnterProc("TileDatabaseConnection::processMapTileSources");

    QSqlQuery sqlQuery = EXEC_SQL(_tileDatabase, "SELECT COUNT (*) FROM MapTileSources");
    if (sqlQuery.isSelect() && sqlQuery.next())
    {
        int sourceCount = sqlQuery.value(0).toInt();
        if (sourceCount == 0)
            EXEC_SQL(_tileDatabase, "INSERT INTO MapTileSources SELECT DISTINCT sourceId FROM MapTile WHERE scale > 0 and x > 0 and y > 0");
    }

    _supportedSources.clear();
    sqlQuery = EXEC_SQL(_tileDatabase, "SELECT sourceId FROM MapTileSources");;
    while (sqlQuery.isSelect() && sqlQuery.next())
    {
        int sourceId = sqlQuery.value(0).toInt();
        _supportedSources.insert(sourceId);
    }
}

TileDatabaseConnection::TileDatabaseConnection(QObject *parent, const QString &fileName) : QObject(parent)
{
    EnterProc("TileDatabaseConnection::TileDatabaseConnection");
    _fileName = fileName;
    _uncommitedTilesCount = 0;
    connectToDatabase();
}

TileDatabaseConnection::~TileDatabaseConnection()
{
    _tileDatabase.commit();
    LOG_SQL_ERROR(_tileDatabase);

    delete _selectTileQuery;
}

QPixmap * TileDatabaseConnection::getTileData(int x, int y, int scale, int sourceId)
{
    EnterProc("TileDatabaseConnection::getTileData");
    QPixmap * resultTileImage = nullptr;

    _selectTileQuery->addBindValue(x);
    _selectTileQuery->addBindValue(y);
    _selectTileQuery->addBindValue(scale);
    _selectTileQuery->addBindValue(sourceId);

    _selectTileQuery->exec();
    LOG_SQL_ERROR(_selectTileQuery);

    while (_selectTileQuery->next())
    {
        int pos = 0;

        QByteArray imageBytes = _selectTileQuery->value(pos++).toByteArray();
        int tileFormat = _selectTileQuery->value(pos++).toInt();

        if (tileFormat == MapTileImageFormat::ImageJPEG)
        {
            resultTileImage = new QPixmap();
            resultTileImage->loadFromData(imageBytes, "JPEG");
        }
        else if (tileFormat == MapTileImageFormat::ImagePNG)
        {
            resultTileImage = new QPixmap();
            resultTileImage->loadFromData(imageBytes, "PNG");
        }
    }

    return resultTileImage;
}

QSet<int> &TileDatabaseConnection::getSupportedSources()
{
    return _supportedSources;
}

QString TileDatabaseConnection::getFileName()
{
    return _fileName;
}

void TileDatabaseConnection::saveTile(int sourceId, int scale, int x, int y, const QByteArray &tileImageRawData)
{
    EnterProc("TileDatabaseConnection::saveTile");

    int format;

    if (tileImageRawData.startsWith("\x89\x50\x4E\x47\x0D\x0A\x1A\x0A"))
        format = MapTileImageFormat::ImagePNG;
    else if (tileImageRawData.startsWith("\xFF\xD8\xFF"))
        format = MapTileImageFormat::ImageJPEG;
    else
    {
        qDebug() << "Incorrect tile format: " << sourceId << scale << x << y;
        return;
    }

    bool autogenerated = false;
    double datetime = GetCurrentDateTimeForDB();
    int signature = 0;

    _insertTileQuery->addBindValue(x);
    _insertTileQuery->addBindValue(y);
    _insertTileQuery->addBindValue(scale);
    _insertTileQuery->addBindValue(sourceId);
    _insertTileQuery->addBindValue(format);
    _insertTileQuery->addBindValue(autogenerated);
    _insertTileQuery->addBindValue(datetime);
    _insertTileQuery->addBindValue(signature);
    _insertTileQuery->addBindValue(tileImageRawData);

    _insertTileQuery->exec();
    LOG_SQL_ERROR(_insertTileQuery);

    _uncommitedTilesCount++;

    if (_uncommitedTilesCount >= UNCOMMITED_TILES_MAX_COUNT)
    {
        _tileDatabase.commit();
        LOG_SQL_ERROR(_tileDatabase);
        _tileDatabase.transaction();
        LOG_SQL_ERROR(_tileDatabase);
        _uncommitedTilesCount = 0;
    }
}

//-----------------------------------------------------------

MapTile::MapTile(int tileSourceId, int tileScale, int tileX, int tileY, QPixmap *tileImage, bool deleteTileImageOnDestroy)
{
    x = tileX;
    y = tileY;
    scale = tileScale;
    lastUsing = 0;
    image = tileImage;
    sourceId = tileSourceId;
    deleteImageOnDestroy = deleteTileImageOnDestroy;
}

MapTile::~MapTile()
{
    if (deleteImageOnDestroy)
        delete image;
}

bool MapTile::isEqual(int tileSourceId, int tileScale, int tileX, int tileY)
{
    return (scale == tileScale) && (x == tileX) && (y == tileY) && (sourceId == tileSourceId);
}

MapTileHashValue MapTile::calculateMapTileHash(int sourceId, int scale, int x, int y)
{
    MapTileHashValue value =
            (sourceId & 0x000000FF) |       //0..7
            ((scale & 0x0000001F) << 8) |   //8..12
            ((x & 0x000003FF) << 13)|       //13..22
            ((y & 0x000001FF) << 23);       //23..31
    return value;
}

//-----------------------------------------------------------

MapTileSourceInfo::MapTileSourceInfo(QObject *parent, int sourceId, bool isHybrid, const QString &name) : QObject(parent)
{
    _sourceId = sourceId;
    _isHybrid = isHybrid;
    _name = name;
}

QVector<TileDatabaseConnection *> *MapTileSourceInfo::dbConnections()
{
    return &_dbConnections;
}

const QString &MapTileSourceInfo::name() const
{
    return _name;
}

bool MapTileSourceInfo::isHybrid() const
{
    return _isHybrid;
}

int MapTileSourceInfo::sourceId() const
{
    return _sourceId;
}
