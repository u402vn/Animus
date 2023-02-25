#ifndef EXTERNALDATACONSOLENOTIFICATOR_H
#define EXTERNALDATACONSOLENOTIFICATOR_H

#include <QObject>
#include <QString>
#include <QUdpSocket>
#include "TelemetryDataFrame.h"

class ExternalDataConsoleNotificator : public QObject
{
    Q_OBJECT
private:
    QUdpSocket _udpConsoleSocket;
    quint32 _udpConsolePort;
public:
    explicit ExternalDataConsoleNotificator(QObject *parent, quint32 udpConsolePort);
    ~ExternalDataConsoleNotificator();
public slots:
    void onClientCommandSent(const ClientCommand &clientCommand);
    void onTelemetryReceived(const QString &telemetryHEX);
};

#endif // EXTERNALDATACONSOLENOTIFICATOR_H
