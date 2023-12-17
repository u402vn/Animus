#include "YurionVideoReceiver.h"
#include <QDebug>
#include <QFile>
#include <QtEndian>
#include <QLibrary>
#include "EnterProc.h"

YurionVideoReceiver::YurionVideoReceiver(QObject *parent, quint32 videoConnectionId, bool verticalMirror, int udpReceivePort) : QObject(parent)
{
    EnterProcStart("UrionVideoReceiver::UrionVideoReceiver");
    _videoConnectionId = videoConnectionId;

    _worker = new YurionVideoReceiverWorker(nullptr, verticalMirror, udpReceivePort);
    connect(_worker, &YurionVideoReceiverWorker::workerFrameAvailable, this, &YurionVideoReceiver::frameAvailableInternal, Qt::QueuedConnection);
    _thread = new QThread;
    _worker->moveToThread(_thread);
    connect(_thread, &QThread::started, _worker, &YurionVideoReceiverWorker::process);
    connect(_thread, &QThread::finished, _worker, &YurionVideoReceiverWorker::deleteLater);

    _thread->start();
}

YurionVideoReceiver::~YurionVideoReceiver()
{
    EnterProcStart("UrionVideoReceiver::~UrionVideoReceiver");
    _thread->quit();
    _thread->wait();
    delete _thread;
}

void YurionVideoReceiver::setVideoForwarding(const QString &udpForwardingAddress, quint32 udpForwardingPort)
{
    _worker->setVideoForwarding(udpForwardingAddress, udpForwardingPort);
}

void YurionVideoReceiver::setResolution(int width, int height)
{
    _worker->setResolution(width, height);
}

void YurionVideoReceiver::frameAvailableInternal(const QImage & frame)
{
    EnterProcStart("UrionVideoReceiver::frameAvailableInternal");
    emit frameAvailable(frame, _videoConnectionId);
}


YurionVideoReceiverWorker::YurionVideoReceiverWorker(QObject *parent, bool verticalMirror, int udpReceivePort) : QObject(parent)
{
    _verticalMirror = verticalMirror;
    _udpReceivePort = udpReceivePort;
    _udpVideoForwardingPort = 0;

    _width = 0;
    _height = 0;

    _udpForwardingVideoSocket = nullptr;
    _udpReceiveVideoSocket = nullptr;
    _dataArray = nullptr;
    _outImageBytes = nullptr;
    _yurionDecoderLibrary = nullptr;
    _resultImage = nullptr;
}

YurionVideoReceiverWorker::~YurionVideoReceiverWorker()
{
    delete _udpForwardingVideoSocket;
    if (_udpReceiveVideoSocket != nullptr)
    {
        _udpReceiveVideoSocket->close();
        delete _udpReceiveVideoSocket;
    }
    delete _dataArray;
    delete _outImageBytes;
    delete _yurionDecoderLibrary;
    delete _resultImage;
}

void YurionVideoReceiverWorker::setVideoForwarding(const QString &udpForwardingAddress, quint32 udpForwardingPort)
{
    _udpVideoForwardingAddress = QHostAddress(udpForwardingAddress);
    _udpVideoForwardingPort = udpForwardingPort;
}

void YurionVideoReceiverWorker::setResolution(int width, int height)
{
    _width = width;
    _height = height;
}

void YurionVideoReceiverWorker::process()
{
    _yurionDecoderLibrary = new QLibrary("mw11dec", this);
    if (!_yurionDecoderLibrary->load())
    {
        qDebug() << "Library 'mw11dec' is not loaded";
        return;
    }

    doInstallDecoder = (Inst)(_yurionDecoderLibrary->resolve(
                              #ifdef Q_OS_LINUX
                                  "MWDec_Install"
                              #else
                                  "_MWDec_Install"
                              #endif
                                  ));

    doDecodeImage = (Dec10)(_yurionDecoderLibrary->resolve("MwDecodeFrameV10Checked"));
    if (!doInstallDecoder || !doDecodeImage)
    {
        qDebug() << "Resolving yurion's functions has failed";
        return;
    }
    doInstallDecoder(_decodingBuffer);

    _dataArray = new QByteArray();
    _outImageBytes = new QByteArray();
    _udpReceiveVideoSocket = new QUdpSocket(this);
    //_udpVideoSocket->setSocketOption(QAbstractSocket::ReceiveBufferSizeSocketOption, 1000000); // -1 == Unlimited ???
    _udpReceiveVideoSocket->bind(_udpReceivePort, QUdpSocket::DefaultForPlatform);
    connect(_udpReceiveVideoSocket, &QUdpSocket::readyRead, this, &YurionVideoReceiverWorker::processVideoPendingDatagrams, Qt::ANIMUS_CONNECTION_TYPE);
    _udpForwardingVideoSocket = new QUdpSocket(this);
}

void SaveByteArrayToFile(const QByteArray & array, const QString fileName)
{
    if (array.size() > 0)
    {
        QFile file(fileName);
        if (file.open(QIODevice::WriteOnly))
        {
            file.write(array);
            file.close();
        }
    }
}

void YurionVideoReceiverWorker::processVideoPendingDatagrams()
{
    const int PREAMBLE_SZIE = 8;
    const unsigned char preamble[PREAMBLE_SZIE] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x57, 0x44};
    const QByteArray prbyte(reinterpret_cast<const char*>(preamble), PREAMBLE_SZIE);
    //const unsigned char fin[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
    //const QByteArray fnbyte(reinterpret_cast<const char*>(fin), 6);
    const int MAX_DATA_ARRAY_SIZE = 16 * 1024 * 1024;

    QByteArray rxData;
    while (_udpReceiveVideoSocket->hasPendingDatagrams())
    {
        int pendingSize = _udpReceiveVideoSocket->pendingDatagramSize();
        rxData.resize(pendingSize);

        _udpReceiveVideoSocket->readDatagram(rxData.data(), pendingSize);
        _dataArray->append(rxData);

        if (_dataArray->size() > MAX_DATA_ARRAY_SIZE)
        {
            _dataArray->clear();
            qDebug() << "Yurion buffer overflow";
        }

        if (_udpVideoForwardingPort > 0)
            _udpForwardingVideoSocket->writeDatagram(rxData, _udpVideoForwardingAddress, _udpVideoForwardingPort);
    }

    int startFrameIdx, endFrameIdx;

    do
    {
        startFrameIdx = _dataArray->indexOf(prbyte);
        if (startFrameIdx < 0)
            return;

        endFrameIdx = _dataArray->indexOf(prbyte, startFrameIdx + PREAMBLE_SZIE);
        if (endFrameIdx < 0)
            return;

        QByteArray rawDataFrame = _dataArray->mid(startFrameIdx, endFrameIdx - startFrameIdx);
        extractImageFromRawDataFrame(rawDataFrame);
        _dataArray->remove(0, endFrameIdx);
    } while (startFrameIdx >= 0);
}


void YurionVideoReceiverWorker::extractImageFromRawDataFrame(const QByteArray &rawDataFrame)
{
#pragma pack(push, 1)
    struct YurionFrameData
    {
        quint8 PreamblePrefix[8];       // WORD0 .. WORD3
        quint32 FrameSizeWithPreamble;  // WORD4 .. WORD5
        quint32 DataThreadId;           // WORD6 .. WORD7
        quint32 FrameNumber;            // WORD8 .. WORD9
        quint16 ALARM0;                 // WORD10
        quint16 ALARM1;                 // WORD11
        quint16 ALARM2;                 // WORD12
        quint16 ALARM3;                 // WORD13
        quint8 AudioChannelCount;       // WORD14(0)
        quint8 VideoInformation;        // WORD14(1)
        quint16 FrameFrequencyHz;       // WORD15;
        quint16 AudioFrequencyHz;       // WORD16;
        quint16 VideoOffset;            // WORD17;
    };

    struct YurionVideoInformationData
    {
        quint8 Time_SecondFractions;    // WORD NV+0 (0)
        quint8 Time_Seconds;            // WORD NV+0 (1)
        quint8 Time_Minutes;            // WORD NV+1 (0)
        quint8 Time_Hours;              // WORD NV+1 (1)
        quint8 Time_Days;               // WORD NV+2 (0)
        quint8 Time_Months;             // WORD NV+2 (1)
        quint16 Time_Years;             // WORD NV+3
        quint8 CompressionDetails;      // WORD NV+4 (0)
        quint8 VaweletBlockCount;       // WORD NV+4 (1)
        quint16 CompressionLevel;       // WORD NV+5
        quint8 VaweletBlockTable[220];  // WORD NV+6
    };
#pragma pack(pop)

    const quint8 VIDEO_INFORMATION_SD     = 0b00010000;
    const quint8 VIDEO_INFORMATION_HD     = 0b00100000;
    const quint8 VIDEO_INFORMATION_FULLHD = 0b01000000;

    size_t rawDataFrameSize = rawDataFrame.size();

    if (rawDataFrameSize < sizeof(YurionFrameData))
        return; // data frame is broken. Incorrect size (less than the size of a preamble)

    const YurionFrameData * udf = (const YurionFrameData*)(rawDataFrame.data());

    if (udf->FrameSizeWithPreamble != rawDataFrameSize)
        return; // data frame is broken. Incorrect size (doesn't correspond specified in a preamble)

    // UrionVideoInformationData * uvi = (UrionVideoInformationData*)(rawDataFrame.data() + udf->VideoOffset);

    int encodedVideoFrameSize = rawDataFrameSize - udf->VideoOffset;
    if (encodedVideoFrameSize <= 0)
        return; // no encoded video

    const char * encodedVideoFrameDataPtr = rawDataFrame.data() + udf->VideoOffset;

    int width = 0;
    int height = 0;
    int bpp = 4;
    QImage::Format imageFormat = QImage::Format_ARGB32;

    if (_width > 0 && _height > 0)
    {
        width = _width;
        height = _height;
    }
    else if ((udf->VideoInformation & VIDEO_INFORMATION_SD) != 0)
    {
        width = 704;
        height = 576;
    }
    else if ((udf->VideoInformation & VIDEO_INFORMATION_HD) != 0)
    {
        width = 1280;
        height = 720;
    }
    else if ((udf->VideoInformation & VIDEO_INFORMATION_FULLHD) != 0)
    {
        width = 1920;
        height = 1080;
    }
    else
    {
        width = 704;
        height = 576;
    }
    //qDebug() << "YURION Error. Unsupported resolution. VideoInformation: " << udf->VideoInformation;


    int outImageBytesSize = _outImageBytes->size();
    if (outImageBytesSize == 0)
    {
        _outImageBytes->resize(width * height * bpp);
        _resultImage = new QImage((const uchar*)_outImageBytes->data(), width, height, imageFormat);
    }

    int decodeResult = doDecodeImage(encodedVideoFrameDataPtr, encodedVideoFrameSize, \
                                     _outImageBytes->data(), 0, 0, 0, 0, 0, _decodingBuffer);
    if (decodeResult)
    {
        QImage rgb32Image = _resultImage->convertToFormat(QImage::Format_RGB32);
        if (_verticalMirror)
            rgb32Image = rgb32Image.mirrored(false, true);
        emit workerFrameAvailable(rgb32Image);
    }
    else
        qDebug() << "YURION Error. Decoding Error. VideoInformation: " << udf->VideoInformation << ". DecodeResult: " << decodeResult;
}
