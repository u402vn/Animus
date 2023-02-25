#pragma once
#include <QObject>
#include <QSqlDatabase>
#include <QSqlQuery>
#include "Common/CommonData.h"
#include "CamPreferences.h"
#include "TelemetryDataFrame.h"

// http://www.vhfdx.ru/karta-vyisot

enum HeightMapSource
{
    SRTM1 = 1
};

struct HeightMapTile final
{
    int x;
    int y;
    HeightMapSource sourceId;
    unsigned long lastUsing;
    QByteArray tileData;
    HeightMapTile();
    ~HeightMapTile();
};

class HeightMapContainer final : public QObject
{
    Q_OBJECT
private:
    static quint32 _heightDatabaseCounter;

    QSqlDatabase _heightTileDatabase;
    QSqlQuery * _selectQuery;

    static const int TILE_BUFFER_SIZE = 10;
    unsigned long _usingCounter;
    HeightMapTile _tiles[TILE_BUFFER_SIZE];
    int _lastTileIndex;

    bool GetHeightSRTM1(double gps_lat, double gps_lon, double &height);
public:
    explicit HeightMapContainer(QObject *parent, const QString &dbHeightMapFile);
    ~HeightMapContainer();

    bool GetHeight(double gps_lat, double gps_lon, double &height);
};
