#ifndef MAPTILEDOWNLOADER_H
#define MAPTILEDOWNLOADER_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QPixmap>
#include <QMap>
#include <QSet>

struct TileAttributes
{
    int sourceId, x, y, scale;
};

class MapTileDownloader final : public QObject
{
    Q_OBJECT
private:
    QNetworkAccessManager _accessManager;
    QMap<QString, TileAttributes> _downloads;
    quint32 _totalDownloads, _successDownloads;

    QSet<int> _supportedSources;
public:
    explicit MapTileDownloader(QObject *parent);
    QSet<int> &getSupportedSources();
signals:
    void tileReceived(int sourceId, int scale, int x, int y, const QByteArray &tileImageRawData);
public slots:
    void needTile(int sourceId, int scale, int x, int y);
private slots:
    void tileDownloaded(QNetworkReply* networkReply);
};

#endif // MAPTILEDOWNLOADER_H
