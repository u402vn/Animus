#ifndef ARTILLERYSPOTTER_H
#define ARTILLERYSPOTTER_H

#include <QObject>
#include <QTimerEvent>
#include <QList>
#include <QTcpSocket>
#include <QHostAddress>
#include "MarkerStorageItems.h"
#include "TelemetryDataFrame.h"

class ArtillerySpotter : public QObject
{
    Q_OBJECT
    QTcpSocket _tcpSocket;
    QByteArray _tcpBuffer;

    bool _enabled;
    QHostAddress _address;
    quint16 _port;
    int _reconnectTimerId;

    quint32 _messageId;

    //QMap<int, int> _sentMessages;

    TelemetryDataFrame _telemetryDataFrame;

    void processDataExchange(const QString &contentHEX, const QString &description, DataExchangePackageDirection direction);
protected:
    void timerEvent(QTimerEvent *event); // reconnect to socket
public:
    explicit ArtillerySpotter(QObject *parent);
    ~ArtillerySpotter();
    void openSocket(const QHostAddress address, const quint16 port);

    void processTelemetry(const TelemetryDataFrame &telemetryDataFrame);

    void sendMarkers(const QList<MapMarker *> *markers);
    void sendWeather(const QVector<WeatherDataItem> *weatherDataCollection);
private slots:
    void readSocketData();
signals:
    void onMessageExchangeInformation(const QString &information, bool isEroor);
    void onArtillerySpotterDataExchange(const DataExchangePackage &dataPackage, DataExchangePackageDirection direction);
};

#endif // ARTILLERYSPOTTER_H
