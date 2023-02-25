#include "MapTileDownloader.h"
#include <QNetworkRequest>
#include <QDebug>
#include "Map/MapTileContainer.h"
#include "Common/CommonUtils.h"

MapTileDownloader::MapTileDownloader(QObject *parent) : QObject(parent),
    _accessManager(this),
    _totalDownloads(0)
{
    connect(&_accessManager, &QNetworkAccessManager::finished, this, &MapTileDownloader::tileDownloaded, Qt::ConnectionType::QueuedConnection);
    _supportedSources << WikiMap << YandexSatellite << YandexMap << YandexHybrid << BingSatellite << GoogleHybrid << GoogleSatellite;
}

QSet<int> &MapTileDownloader::getSupportedSources()
{
    return _supportedSources;
}

int fakeRandom(int base, int low, int high) // fake random is used to duplicate the same url for repeated tile requrest
{
    return (base % ((high + 1) - low) + low);
}

//https://docs.microsoft.com/en-us/bingmaps/articles/bing-maps-tile-system
QString TileXYToQuadKey(int tileX, int tileY, int levelOfDetail)
{
    QString quadKey;
    for (int i = levelOfDetail; i > 0; i--)
    {
        char digit = '0';
        int mask = 1 << (i - 1);
        if ((tileX & mask) != 0)
        {
            digit++;
        }
        if ((tileY & mask) != 0)
        {
            digit++;
            digit++;
        }
        quadKey.append(digit);
    }
    return quadKey;
}

void MapTileDownloader::needTile(int sourceId, int scale, int x, int y)
{
    QString url;
    int base = x + y + scale + sourceId;

    switch(sourceId)
    {
    case WikiMap:
    {
        int num = x % 4 + (y % 4) * 4;
        url = QString("http://i%1.wikimapia.org/?x=%2&y=%3&zoom=%4").arg(num).arg(x).arg(y).arg(scale);
        break;
    }

    case YandexSatellite:
    {
        int servNo = fakeRandom(base, 1, 3);
        QString gagarin = QString("Gagarin").mid(0, fakeRandom(base, 1, 7));
        url = QString("http://sat0%1.maps.yandex.net/tiles?l=sat&x=%2&y=%3&z=%4&g=%5").arg(servNo).arg(x).arg(y).arg(scale).arg(gagarin);
        break;
    }

    case YandexMap:
    {
        int servNo = fakeRandom(base, 1, 3);
        QString gagarin = QString("Gagarin").mid(0, fakeRandom(base, 1, 7));
        url = QString("http://vec0%1.maps.yandex.net/tiles?l=map&x=%2&y=%3&z=%4&g=%5").arg(servNo).arg(x).arg(y).arg(scale).arg(gagarin);
        break;
    }

    case YandexHybrid:
    {
        int servNo = fakeRandom(base, 1, 3);
        QString gagarin = QString("Gagarin").mid(0, fakeRandom(base, 1, 7));
        url = QString("http://vec0%1.maps.yandex.net/tiles?l=skl&x=%2&y=%3&z=%4&g=%5").arg(servNo).arg(x).arg(y).arg(scale).arg(gagarin);
        break;
    }

    case BingSatellite:
    {
        QString quadkey = TileXYToQuadKey(x, y, scale);
        url = QString("http://ecn.t0.tiles.virtualearth.net/tiles/a%1.jpeg?g=1").arg(quadkey);
        break;
    }

    case GoogleHybrid:
    {
        int servNo = fakeRandom(base, 0, 3);
        QString galileo = QString("Galileo").mid(0, fakeRandom(base, 1, 7));
        //url = QString("http://mt%1.google.com/vt/lyrs=h@169000000&hl=ru&x=%2&y=%3&zoom=%4&s=%5").arg(servNo).arg(x).arg(y).arg(17 - scale).arg(galileo);
        url = QString("http://mt%1.google.com/vt/lyrs=h&hl=ru&x=%2&y=%3&z=%4&s=%5").arg(servNo).arg(x).arg(y).arg(scale).arg(galileo);
        break;
    }

    case GoogleSatellite:
    {
        int servNo = fakeRandom(base, 0, 3);
        QString galileo = QString("Galileo").mid(0, fakeRandom(base, 1, 7));
        url = QString("http://mt%1.google.com/vt/lyrs=s&hl=ru&x=%2&y=%3&z=%4&s=%5").arg(servNo).arg(x).arg(y).arg(scale).arg(galileo);
        break;
    }

    case JointStaffMap:
    {
        //obsolete
        //url = QString("http://vb-maps.googlecode.com/svn/trunk/tiles/z%1/%2/%3.jpg").arg(scale).arg(y).arg(x);
        break;
    }

    case MapsByMap:
    {
        //todo
        break;
    }

    case ForestMap:
    case NoBaseTile:
    case NoHybridTile:
    {
        //do nothing
        break;
    }
    }

    if (!url.isEmpty() && !_downloads.contains(url))
    {
        TileAttributes tileAttr = {sourceId, x, y, scale};
        _downloads.insert(url, tileAttr);

        QNetworkRequest request(url);
        _accessManager.get(request);
    }
    // https://wiki.openstreetmap.org/wiki/Tile_servers
    // url = "http://a.tile.openstreetmap.fr/hot/4/5/5.png";
}

void MapTileDownloader::tileDownloaded(QNetworkReply *networkReply)
{
    _totalDownloads++;

    QString url = networkReply->request().url().toString();
    if (_downloads.contains(url))
    {
        TileAttributes tileAttr = _downloads[url];
        _downloads.remove(url);
        QByteArray data = networkReply->readAll();

        emit tileReceived(tileAttr.sourceId, tileAttr.scale, tileAttr.x, tileAttr.y, data);
    }

    networkReply->deleteLater();
}
