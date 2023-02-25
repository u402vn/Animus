#include "HeightMapContainer.h"
#include <QQuaternion>
#include <QVector3D>
#include "Common/CommonUtils.h"

quint32 HeightMapContainer::_heightDatabaseCounter = 0;

HeightMapTile::HeightMapTile()
{
    x = -1;
    y = -1;
    lastUsing = 0;
}

HeightMapTile::~HeightMapTile()
{
}

HeightMapContainer::HeightMapContainer(QObject *parent, const QString &dbHeightMapFile) : QObject(parent)
{    
    _heightTileDatabase =  QSqlDatabase::addDatabase("QSQLITE", QString("HeightDatabase%1").arg(_heightDatabaseCounter++));
    _heightTileDatabase.setDatabaseName(dbHeightMapFile);
    _heightTileDatabase.setConnectOptions("OCI_ATTR_PREFETCH_ROWS=1;OCI_ATTR_PREFETCH_MEMORY=1"); //???
    _heightTileDatabase.open();
    LOG_SQL_ERROR(_heightTileDatabase);

    _selectQuery = new QSqlQuery(_heightTileDatabase);
    _selectQuery->prepare("SELECT tile FROM HeightMap WHERE x=? AND y=? AND sourceId=?");
    LOG_SQL_ERROR(_selectQuery);

    _lastTileIndex = 0;
}

HeightMapContainer::~HeightMapContainer()
{
    delete _selectQuery;
}

bool HeightMapContainer::GetHeight(double gps_lat, double gps_lon, double &height)
{
    return GetHeightSRTM1(gps_lat, gps_lon, height);
}

bool HeightMapContainer::GetHeightSRTM1(double gps_lat, double gps_lon, double &height)
{
    height = 0;

    HeightMapTile * resultTile = nullptr;
    HeightMapTile * obsoleteTile = nullptr;

    int tileX = (int)gps_lat;
    int tileY = (int)gps_lon;

    //Try to search tile in cache
    for (int i = 0; i < TILE_BUFFER_SIZE; i++)
    {
        HeightMapTile * tile = &_tiles[i];
        if (obsoleteTile == nullptr)
            obsoleteTile = tile;
        else if (tile->lastUsing < obsoleteTile->lastUsing)
            obsoleteTile = tile;
        if ((tile->x == tileX) && (tile->y == tileY) && (tile->sourceId == SRTM1))
        {
            resultTile = tile;
            break;
        }
    }

    if (resultTile == nullptr)
    {
        _selectQuery->addBindValue(tileX);
        _selectQuery->addBindValue(tileY);
        _selectQuery->addBindValue(SRTM1);

        _selectQuery->exec();
        LOG_SQL_ERROR(_selectQuery);

        while (_selectQuery->next())
        {
            resultTile = obsoleteTile;

            resultTile->x = tileX;
            resultTile->y = tileY;
            resultTile->sourceId = SRTM1;
            resultTile->tileData = _selectQuery->value(0).toByteArray();
        }

        if (resultTile == nullptr)
            return false;
    }

    resultTile->lastUsing = _usingCounter;

    const int TileRowCount = 3601;
    const int TileColCount = 1801;

    int i = int((1 - (gps_lat - trunc(gps_lat))) * (TileRowCount - 1));
    int j = int((gps_lon - trunc(gps_lon)) * (TileColCount - 1));

    short * heightData = (short*)resultTile->tileData.constData();
    short heightInPos = heightData[i * TileColCount + j];

    height = heightInPos;

    return true;
}
