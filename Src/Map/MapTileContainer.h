#ifndef MAPTILECONTAINER_H
#define MAPTILECONTAINER_H

#include <QObject>
#include <QPainter>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSet>
#include <QPixmap>
#include <QMultiMap>
#include <QPointF>
#include "Common/CommonData.h"
#include "Map/HeightMapContainer.h"
#include "Map/MapTileDownloader.h"

// http://wiki.openstreetmap.org/wiki/Slippy_map_tilenames
// http://habrahabr.ru/post/146107/
// http://habrahabr.ru/post/233809/
// http://www.foxbase.ru/Java/google-maps-preobrazovanie-koordinat.htm

//http://habrahabr.ru/post/151103/ - преобразования координат для тайлов Яндекса. Брал формулы отсюда
//http://gis-lab.info/qa/tile-matrix.html - флрмуля для расчета разрешения и масштаба карты от широты и долготы (внизу)
//http://www.qtcentre.org/threads/23518-How-to-change-completion-rule-of-QCompleter - пример использования QCompleter
//http://habrahabr.ru/post/139428/ - быстрое изменение яркости изображения (в комментариях Imp5 - функция contrastFilter)
//http://habrahabr.ru/post/223449/ - формат файла SQLite
// http://do.gendocs.ru/docs/index-80251.html - Номенклатура, бланковка, разграфка топографических карт

#define MapTileHashValue quint32

constexpr int MIN_ZOOM_VALUE =  6;
constexpr int MAX_ZOOM_VALUE = 20;
constexpr int TILE_WIDTH = 256;
constexpr int TILE_HEIGHT = 256;

enum MapBaseTileSource
{
    NoBaseTile      = -1,
    YandexSatellite = 1,
    JointStaffMap   = 2,
    GoogleSatellite = 3,
    WikiMap         = 4,
    BingSatellite   = 6,
    MapsByMap       = 7,
    YandexMap       = 9,
    ForestMap       = 10
};

enum MapHybridTileSource
{
    NoHybridTile    = -2,
    YandexHybrid    = 5,
    GoogleHybrid    = 8
};

enum TileReceivingMode
{
    DatabaseOnly = 0,
    NetworkOnly,
    DatabaseAndNetwork
};

class TileDatabaseConnection final : public QObject
{
private:
    enum MapTileImageFormat
    {
        ImageJPEG = 1,
        ImagePNG = 2
    };
private:
    Q_OBJECT

    static quint32 _databaseCounter;

    QSet<int> _supportedSources;
    QString _fileName;
    QSqlDatabase _tileDatabase;
    QSqlQuery *_selectTileQuery;
    QSqlQuery *_insertTileQuery;
    quint32 _uncommitedTilesCount;

    void connectToDatabase();
    void processMapTileSources();
public:
    TileDatabaseConnection(QObject *parent, const QString &fileName);
    ~TileDatabaseConnection();
    QPixmap * getTileData(int x, int y, int scale, int sourceId);
    QSet<int> &getSupportedSources();
    QString getFileName();
    void saveTile(int sourceId, int scale, int x, int y, const QByteArray &tileImageRawData);
};


class MapTileSourceInfo final : public QObject
{
private:
    Q_OBJECT

    int _sourceId;
    QVector<TileDatabaseConnection*> _dbConnections;
    QString _name;
    bool _isHybrid;
public:
    MapTileSourceInfo(QObject *parent, int sourceId, bool isHybrid, const QString &name);
    QVector<TileDatabaseConnection *> *dbConnections();
    const QString &name() const;
    bool isHybrid() const;
    int sourceId() const;
};


struct MapTile final
{
private:
    int x;
    int y;
    int scale;
    int sourceId;
    bool deleteImageOnDestroy;
public:
    unsigned long lastUsing;
    QPixmap *image;
    MapTile(int tileSourceId, int tileScale, int tileX, int tileY, QPixmap *tileImage, bool deleteTileImageOnDestroy);
    ~MapTile();
    bool isEqual(int tileSourceId, int tileScale, int tileX, int tileY);
    static MapTileHashValue calculateMapTileHash(int sourceId, int scale, int x, int y);
};

struct LegendPresentationParam final
{
    bool drawLegend;
    bool drawBaseTileNumber;
    bool drawParallelsMeridians;
};

class MapTileContainer final : public QObject
{
    Q_OBJECT
private:
    QMap<int, MapTileSourceInfo *> _sourceInfos;

    TileDatabaseConnection *_downloadCasheDatabaseConnection;
    TileDatabaseConnection *_lastSuccessfulConnection;

    MapTileDownloader _mapTileDownloader;

    QPixmap * _noTileImageBlack;
    QPixmap * _noTileImageTransparent;

    unsigned long _usingCounter;

    QHash<MapTileHashValue, MapTile*> _tileHash;
    TileReceivingMode _tileReceivingMode;

    int _lastTileIndex;
    WorldGPSCoord _screenCenter;

    int _scale;
    MapBaseTileSource _mapBaseId;
    MapHybridTileSource _mapHybridId;
    GlobalCoordSystem _coordSystem;
    GeographicalCoordinatesFormat _coordFormat;

    void setImageCenterGPS(const WorldGPSCoord &screenCenter);

    QPixmap *getTileImage(int tileX, int tileY, int scale, int sourceId);
    void clenupTileHash();

    int calculateScaleMaxWidth(double resolution, QString &middleLabelText, QString &maxLabelText);

    void drawTileLayerInternal(QPainter *imagePainter, int sourceId, int scale, double centerX, double centerY, bool drawTileNumber);
    void drawTileLayer(QPainter *imagePainter, int sourceId, bool drawTileNumber);
    void drawLegend(QPainter *imagePainter);
    void drawParallelsMeridians(QPainter *imagePainter);

    //GeoCoder * _geoCoder;

    void addSourceInfo(int sourceId, bool isHybrid, const QString &name);
    void fillSourceInfos();
    void createTileDatabaseConnections(const QList<QString> &mapDatabaseFiles, const QString &downloadCasheDatabaseFile);

    WorldGPSCoord convertScreenXY2GPS_2D(int screenCenterDx, int screenCenterDy) const;
signals:
    void needTile(int sourceId, int scale, int x, int y);
    void contentUpdated();
private slots:
    void tileReceived(int sourceId, int scale, int x, int y, const QByteArray &tileImageRawData);
protected:
    HeightMapContainer  _heightMapContainer;
    virtual void getMapImageInternal(QPainter *imagePainter, const LegendPresentationParam &legendPresentationParam);
public:
    explicit MapTileContainer(QObject *parent, const QList<QString> &mapDatabaseFiles, const QString &downloadCasheDatabaseFile, const QString &heightMapFile);
    virtual ~MapTileContainer();

    void setImageCenter(const WorldGPSCoord &screenCenter);

    void setScale(int scale);
    int getScale() const;
    void setCoordSystem(GlobalCoordSystem coordSystem);
    GlobalCoordSystem getCoordSystem() const;
    void setTileReceivingMode(TileReceivingMode mode);


    void getMapImage(QPainter *painter, const LegendPresentationParam &legendPresentationParam);
    QImage *getLayerImage(const QSize &size, int sourceId, int scale, double centerX, double centerY);

    void calculateVisibleAreaCornersCoords(int areaWidth, int areaHeight, WorldGPSCoord &coordLeftTop, WorldGPSCoord &coordRightBottom);

    void convertGoogleXY2GPS(int scale, double x, double y, WorldGPSCoord &coord);

    void setMapBaseSourceId(MapBaseTileSource sourceId);
    MapBaseTileSource getMapBaseSourceId() const;

    void setMapHybridSourceId(MapHybridTileSource hybridId);
    MapHybridTileSource getMapHybridSourceId() const;

    bool goToPosByAddress(const QString &address);

    QMultiMap<int, QString> getTileSource2DBFileMap();
    const QString getGlobalCoordSystemName(GlobalCoordSystem system) const;
    const QString getTileSourceName(int sourceId) const;
    bool isHybridTileSource(int sourceId) const;
    const QList<MapTileSourceInfo *> getTileSourceInfos() const;
};

void ConvertGPS2YandexXY(WorldGPSCoord coord, int scale, double &x, double &y);
void ConvertGPS2GoogleXY(WorldGPSCoord coord, int scale, double &x, double &y);
const QPointF ConvertGPS2GoogleXY(WorldGPSCoord coord, int scale);
void ConvertGPS2XY(int sourceId, int scale, const WorldGPSCoord coord, double &x, double &y);

double getImageResolutionMperPix(const WorldGPSCoord &coord, int scale);

bool DecodeLatLon(const QString &encodedLatLon, GeographicalCoordinatesFormat format, double &lat, double &lon);



#endif // MAPTILECONTAINER_H
