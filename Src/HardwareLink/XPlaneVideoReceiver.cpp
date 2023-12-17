#include "XPlaneVideoReceiver.h"
#include <QDebug>
#include "lz4.h"
#include "EnterProc.h"

XPlaneVideoReceiver::XPlaneVideoReceiver(QObject *parent, quint32 videoConnectionId, bool verticalMirror, QHostAddress addr, quint16 udpPort) : QObject(parent)
{
    EnterProcStart("XPlaneVideoReceiver::XPlaneVideoReceiver");
    _videoConnectionId = videoConnectionId;

    XPlaneVideoReceiverWorker * worker = new XPlaneVideoReceiverWorker(nullptr, verticalMirror, addr, udpPort);
    connect(worker, &XPlaneVideoReceiverWorker::workerFrameAvailable, this, &XPlaneVideoReceiver::frameAvailableInternal, Qt::QueuedConnection);
    _thread = new QThread;
    worker->moveToThread(_thread);
    connect(_thread, &QThread::started, worker, &XPlaneVideoReceiverWorker::startProcessing);
    connect(_thread, &QThread::finished, worker, &XPlaneVideoReceiverWorker::deleteLater);

    _thread->start();
}

XPlaneVideoReceiver::~XPlaneVideoReceiver()
{
    EnterProcStart("XPlaneVideoReceiver::~XPlaneVideoReceiver");
    _thread->quit();
    _thread->wait();
    delete _thread;
}

void XPlaneVideoReceiver::frameAvailableInternal(const QImage & frame)
{
    EnterProcStart("XPlaneVideoReceiver::frameAvailableInternal");
    emit frameAvailable(frame, _videoConnectionId);
}

XPlaneVideoReceiverWorker::XPlaneVideoReceiverWorker(QObject *parent, bool verticalMirror, QHostAddress addr, quint16 port): QObject(parent)
{
    _verticalMirror = verticalMirror;
    _address = addr;
    _port = port;
    _tcpSocket = nullptr;
    _compressedData = nullptr;
}

XPlaneVideoReceiverWorker::~XPlaneVideoReceiverWorker()
{
    if (_reconnectionTimer != nullptr)
        _reconnectionTimer->stop();

    if (_tcpSocket != nullptr)
        _tcpSocket->close();

    delete _compressedData;
    delete _imageData;
}

void XPlaneVideoReceiverWorker::startProcessing()
{
    _compressedDataLength = LZ4_compressBound(XPLANE_IMAGE_DATA_SIZE);
    _compressedData = new quint8[_compressedDataLength];
    _imageData = new quint8[XPLANE_IMAGE_DATA_SIZE];

    _tcpSocket = new QTcpSocket(this);
    _tcpSocket->setSocketOption(QAbstractSocket::ReceiveBufferSizeSocketOption, 4000000); // -1 == Unlimited ???

    connect(_tcpSocket, &QTcpSocket::readyRead, this, &XPlaneVideoReceiverWorker::readVideoData, Qt::ANIMUS_CONNECTION_TYPE);
    connect(_tcpSocket, &QTcpSocket::stateChanged, this, &XPlaneVideoReceiverWorker::socketStateChanged);
    connect(_tcpSocket, static_cast<void (QTcpSocket::*)(QAbstractSocket::SocketError)>(&QTcpSocket::error), this, &XPlaneVideoReceiverWorker::socketError);
    connect(_tcpSocket, &QTcpSocket::connected, this, &XPlaneVideoReceiverWorker::socketConnected);
    connect(_tcpSocket, &QTcpSocket::disconnected, this, &XPlaneVideoReceiverWorker::socketDisconnected);

    _reconnectionTimer = new QTimer(this);

    _tcpSocket->connectToHost(_address, _port, QIODevice::ReadOnly);
}

#pragma pack(1)
struct XPLANE_PACKET {
    quint32 frameId;
    quint32 frameTotalSize;
    quint16 framePartNo;
    quint8  frameData[XPLANE_PACKET_CHUNKSIZE];
};
#pragma pack()

void XPlaneVideoReceiverWorker::readVideoData()
{
    XPLANE_PACKET* udpPacket;

    do {
        _tcpBuffer.append(_tcpSocket->readAll());

        while (static_cast<quint32>(_tcpBuffer.size()) >= sizeof(XPLANE_PACKET))
        {
            udpPacket = reinterpret_cast<XPLANE_PACKET*>(_tcpBuffer.data());

            int framePartCount = static_cast<int>(std::ceil(static_cast<float>(udpPacket->frameTotalSize) / XPLANE_PACKET_CHUNKSIZE));
            bool isLastChunk = (udpPacket->framePartNo + 1) >= framePartCount;

            int chunkDataOffset = udpPacket->framePartNo * XPLANE_PACKET_CHUNKSIZE;
            int chunkDataSize = isLastChunk ? udpPacket->frameTotalSize % XPLANE_PACKET_CHUNKSIZE : XPLANE_PACKET_CHUNKSIZE;

            if ((chunkDataOffset + chunkDataSize) < _compressedDataLength)
                memcpy(_compressedData + chunkDataOffset, udpPacket->frameData, static_cast<size_t>(chunkDataSize));

            if (isLastChunk)
            {
                LZ4_decompress_safe(reinterpret_cast<char*>(_compressedData), reinterpret_cast<char*>(_imageData),
                                    static_cast<int>(udpPacket->frameTotalSize), XPLANE_IMAGE_DATA_SIZE);

                QImage resultImage{_imageData, XPLANE_IMAGE_WIDTH, XPLANE_IMAGE_HEIGHT, QImage::Format_ARGB32};
                QImage rgb32Image = resultImage.convertToFormat(QImage::Format_RGB32);

                if (_verticalMirror)
                    rgb32Image = rgb32Image.mirrored(false, true);

                emit workerFrameAvailable(rgb32Image);
            };

            _tcpBuffer.remove(0, sizeof(XPLANE_PACKET));
        }
    } while (_tcpSocket->bytesAvailable() > 0);
}

void XPlaneVideoReceiverWorker::socketStateChanged(QAbstractSocket::SocketState socketState)
{
    qDebug() << "XPlane Video Socket state: " << socketState;
}

void XPlaneVideoReceiverWorker::socketError(QAbstractSocket::SocketError socketError)
{
    qDebug() << "XPlane Video Socket error: " << socketError
             << "; Description: " << _tcpSocket->errorString();

    _reconnectionTimer->singleShot(RECONNECTION_DELAY, [&](){
        _tcpSocket->connectToHost(_address, _port, QIODevice::ReadOnly);
    });
}

void XPlaneVideoReceiverWorker::socketConnected()
{
    qDebug() << "XPlane Video Socket connected";
}

void XPlaneVideoReceiverWorker::socketDisconnected()
{
    qDebug() << "XPlane Video Socket disconnected";
    _tcpBuffer.clear();
}
