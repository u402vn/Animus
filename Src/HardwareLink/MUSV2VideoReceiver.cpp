#include "MUSV2VideoReceiver.h"
#include <QDebug>
#include <QDataStream>
#include <QByteArray>
#include "EnterProc.h"

MUSV2VideoReceiver::MUSV2VideoReceiver(QObject *parent, bool verticalMirror, int udpReceivePort) : QObject(parent)
{
    EnterProcStart("MUSV2VideoReceiver::MUSV2VideoReceiver");
    _socket = new QLocalSocket(this);

    connect(_socket, &QLocalSocket::readyRead, this, &MUSV2VideoReceiver::onReadyRead);
    //connect(_socket, QOverload<QLocalSocket::LocalSocketError>::of(&QLocalSocket::error), this, &MUSV2VideoReceiver::onError);

    connect(_socket, static_cast<void (QLocalSocket::*)(QLocalSocket::LocalSocketError)>(&QLocalSocket::error), this, &MUSV2VideoReceiver::onError);

    _socket->abort();
    _socket->connectToServer("fortune");



    /// https://stackoverflow.com/questions/71756200/how-to-transfer-qimage-from-qlocalserver-to-qlocalsocket
}

MUSV2VideoReceiver::~MUSV2VideoReceiver()
{
    EnterProcStart("MUSV2VideoReceiver::~MUSV2VideoReceiver");

}

void MUSV2VideoReceiver::onError(QLocalSocket::LocalSocketError socketError)
{
    switch (socketError) {
    case QLocalSocket::ServerNotFoundError:
        qDebug() << "The host was not found. Please check the  host name and port settings.";
        break;
    case QLocalSocket::ConnectionRefusedError:
        qDebug() << "The connection was refused by the peer. Make sure the server is running, "
                    "and check that the host name and port settings are correct.";
        break;
    case QLocalSocket::PeerClosedError:
        break;
    default:
        qDebug() << QString("The following error occurred: %1.").arg(_socket->errorString());
    }
}

void MUSV2VideoReceiver::onReadyRead()
{
    QByteArray jsonData;
    QDataStream socketStream(_socket);
    socketStream.setVersion(QDataStream::Qt_4_0);
    //socketStream.setVersion(QDataStream::Qt_5_5);
    qDebug() << _socket->bytesAvailable();

    quint16 blockSize = 0;
    if (blockSize == 0)
    {
        if (_socket->bytesAvailable() < (int)sizeof(quint16))
            return;
        socketStream >> blockSize;
    }


    if (socketStream.atEnd())
        return;


    QString nextFortune;
    socketStream >> nextFortune;

    qDebug() << nextFortune;

    //socketStream >> jsonData;
    //qDebug() << jsonData.count();
    //qDebug() << jsonData.data();


}

