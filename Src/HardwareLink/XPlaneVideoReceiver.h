#ifndef XPLANEVIDEORECEIVER_H
#define XPLANEVIDEORECEIVER_H

#include <QObject>
#include <QTcpSocket>
#include <QThread>
#include <QLibrary>
#include <QImage>
#include <QHostAddress>
#include <QTimer>

const int XPLANE_PACKET_CHUNKSIZE = 1400;
const int XPLANE_IMAGE_WIDTH      =  720;
const int XPLANE_IMAGE_HEIGHT     =  576;
const int XPLANE_IMAGE_BPP        =    4;
const int XPLANE_IMAGE_DATA_SIZE  = XPLANE_IMAGE_WIDTH * XPLANE_IMAGE_HEIGHT * XPLANE_IMAGE_BPP;
const int RECONNECTION_DELAY      = 5000;

class XPlaneVideoReceiverWorker final : public QObject
{
    Q_OBJECT

    QTcpSocket * _tcpSocket;
    QByteArray _tcpBuffer;
    QTimer * _reconnectionTimer;

    quint8 * _compressedData;
    qint32 _compressedDataLength;
    quint8 * _imageData;

    bool _verticalMirror;
    QHostAddress _address;
    quint16 _port;
public:
    explicit XPlaneVideoReceiverWorker(QObject *parent, bool verticalMirror, QHostAddress addr, quint16 port);
    ~XPlaneVideoReceiverWorker();
public slots:
    void startProcessing();

    void readVideoData();
    void socketStateChanged(QAbstractSocket::SocketState socketState);
    void socketError(QAbstractSocket::SocketError socketError);
    void socketConnected();
    void socketDisconnected();
signals:
    void workerFrameAvailable(const QImage &frame);
};

class XPlaneVideoReceiver final : public QObject
{
    Q_OBJECT
    QThread * _thread;
    quint32 _videoConnectionId;
public:
    explicit XPlaneVideoReceiver(QObject *parent, quint32 videoConnectionId, bool verticalMirror, QHostAddress addr, quint16 port);
    ~XPlaneVideoReceiver();
private slots:
    void frameAvailableInternal(const QImage &frame);
signals:
    void frameAvailable(const QImage &frame, quint32 videoConnectionId);
};

#endif // XPLANEVIDEORECEIVER_H
