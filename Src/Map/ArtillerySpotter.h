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
    QTcpSocket _socket;
    bool _enabled;
    QHostAddress _address;
    quint16 _port;
    int _reconnectTimerId;
protected:
    void timerEvent(QTimerEvent *event); // reconnect to socket
public:
    explicit ArtillerySpotter(QObject *parent);
    ~ArtillerySpotter();
    void openSocket(const QHostAddress address, const quint16 port);

    void sendMarkers(const QList<MapMarker *> *markers);
    void sendWeather(const QVector<TelemetryDataFrame> &telemetryDataFrames);
};

#endif // ARTILLERYSPOTTER_H
