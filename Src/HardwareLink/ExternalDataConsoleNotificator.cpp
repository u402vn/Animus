#include "ExternalDataConsoleNotificator.h"
#include <QByteArray>

ExternalDataConsoleNotificator::ExternalDataConsoleNotificator(QObject *parent, quint32 udpConsolePort): QObject(parent)
{
    _udpConsolePort = udpConsolePort;
}

ExternalDataConsoleNotificator::~ExternalDataConsoleNotificator()
{

}

void ExternalDataConsoleNotificator::onClientCommandSent(const ClientCommand &clientCommand)
{
    QByteArray datagram = clientCommand.CommandHEX.toUtf8();
    _udpConsoleSocket.writeDatagram(datagram, QHostAddress::Broadcast, _udpConsolePort);
}

void ExternalDataConsoleNotificator::onTelemetryReceived(const QString &telemetryHEX)
{
    QByteArray datagram = telemetryHEX.toUtf8();
    _udpConsoleSocket.writeDatagram(datagram, QHostAddress::Broadcast, _udpConsolePort);
    //_udpConsoleSocket.writeDatagram(datagram, QHostAddress("192.168.1.126"), 45570);
}
