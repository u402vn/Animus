#ifndef YURIONVIDEORECEIVER_H
#define YURIONVIDEORECEIVER_H

#include <QObject>
#include <QUdpSocket>
#include <QThread>
#include <QLibrary>
#include <QImage>

typedef void (*Inst) (unsigned char *);
typedef int (*Dec10)(
        const void *const pBufferIn,
        const unsigned int inBufferSizeBytes,
        void *const pBufferOut,
        const int black,
        const int YGain,
        const int CGain,
        const int Cif,
        const int time,
        void *const g_buff);

const int MW_DECODER_CONTEXT_SIZE_BYTES = 5006288;

class YurionVideoReceiverWorker final : public QObject
{
    Q_OBJECT

    Inst doInstallDecoder;
    Dec10 doDecodeImage;

    QUdpSocket *_udpReceiveVideoSocket;
    QByteArray *_dataArray;
    QByteArray *_outImageBytes;
    QImage *_resultImage;

    QLibrary *_yurionDecoderLibrary;

    quint8 _decodingBuffer [MW_DECODER_CONTEXT_SIZE_BYTES];

    bool _verticalMirror;
    int _udpReceivePort;

    QUdpSocket *_udpForwardingVideoSocket;

    QHostAddress _udpVideoForwardingAddress;
    quint32 _udpVideoForwardingPort;

    int _width, _height;

    void extractImageFromRawDataFrame(const QByteArray &rawDataFrame);
public:
    explicit YurionVideoReceiverWorker(QObject *parent, bool verticalMirror, int udpReceivePort);
    ~YurionVideoReceiverWorker();

    void setVideoForwarding(const QString &udpForwardingAddress, quint32 udpForwardingPort);
    void setResolution(int width, int height);
public slots:
    void process();
    void processVideoPendingDatagrams();
signals:
    void workerFrameAvailable(const QImage &frame);
};

class YurionVideoReceiver final : public QObject
{
    Q_OBJECT
    QThread *_thread;
    YurionVideoReceiverWorker *_worker;
    quint32 _videoConnectionId;
public:    
    explicit YurionVideoReceiver(QObject *parent, quint32 videoConnectionId, bool verticalMirror, int udpReceivePort);
    ~YurionVideoReceiver();

    void setVideoForwarding(const QString &udpForwardingAddress, quint32 udpForwardingPort);
    void setResolution(int width, int height);
private slots:
    void frameAvailableInternal(const QImage &frame);
signals:
    void frameAvailable(const QImage &frame, quint32 videoConnectionId);
};

#endif // YURIONVIDEORECEIVER_H
