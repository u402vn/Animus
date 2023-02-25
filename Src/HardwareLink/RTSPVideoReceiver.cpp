#include "RTSPVideoReceiver.h"
#include "EnterProc.h"
#include <QProcessEnvironment>
#include <QDebug>
#include "opencv2/opencv.hpp"

RTSPVideoReceiver::RTSPVideoReceiver(QObject *parent, bool verticalMirror, const QUrl url) : QObject(parent)
{
    EnterProcStart("RTSPVideoReceiver::RTSPVideoReceiver");

    //QProcessEnvironment::systemEnvironment().insert("OPENCV_FFMPEG_CAPTURE_OPTIONS", "timeout;3000");

    auto worker = new RTSPVideoReceiverWorker(nullptr, verticalMirror, url);
    connect(worker, &RTSPVideoReceiverWorker::frameAvailable, this, &RTSPVideoReceiver::frameAvailableInternal, Qt::QueuedConnection);
    _thread = new QThread;
    worker->moveToThread(_thread);
    connect(_thread, &QThread::started,  worker, &RTSPVideoReceiverWorker::startProcessing);
    connect(_thread, &QThread::finished, worker, &RTSPVideoReceiverWorker::deleteLater);

    _thread->start();
}

RTSPVideoReceiver::~RTSPVideoReceiver()
{
    EnterProcStart("RTSPVideoReceiver::~RTSPVideoReceiver");
    _thread->requestInterruption();
    _thread->quit();
    _thread->wait();
    delete _thread;
}

void RTSPVideoReceiver::frameAvailableInternal(const QImage &frame)
{
    EnterProcStart("RTSPVideoReceiver::frameAvailableInternal");
    emit frameAvailable(frame);
}

//--------------------------------------------------------------------------------

RTSPVideoReceiverWorker::RTSPVideoReceiverWorker(QObject *parent, bool verticalMirror, const QUrl url) : QObject(parent)
{
    _verticalMirror = verticalMirror;
    _url = url;
}

RTSPVideoReceiverWorker::~RTSPVideoReceiverWorker()
{

}

void RTSPVideoReceiverWorker::startProcessing()
{
    qint64 frameCount = 0;
    //cv::VideoCapture capture = cv::VideoCapture("rtsp://192.168.100.64:13554/live3.sdp");
    //???cv::VideoCapture capture = cv::VideoCapture("rtsp://admin:123456@xx.xx.x.xxx:7070", cv::CAP_FFMPEG);
    QString path = _url.toString();


    //path = "rtspsrc location=rtsp://192.168.100.64:13554/live3.sdp latency=0 tcp-timeout=1 ! rtph264depay ! h264parse ! avdec_h264 output-corrupt=false ! videoconvert ! appsink";
    //path = "rtspsrc location=rtsp://192.168.100.64:13554/live3.sdp";

    qDebug() << "Open RTSP video stream: " << path;

    //cv::VideoCapture capture = cv::VideoCapture(path.toLatin1().data(), cv::CAP_FFMPEG);
    cv::VideoCapture capture = cv::VideoCapture(path.toLatin1().data());
    cv::Mat frameMat, frameMatRGB;
    while (capture.isOpened() && !QThread::currentThread()->isInterruptionRequested())
    {
        if (capture.read(frameMat))
        {
            frameCount++;

            cv::cvtColor(frameMat, frameMatRGB, CV_RGB2RGBA);
            QImage rgb32Image((uchar*)frameMatRGB.data, frameMatRGB.cols, frameMatRGB.rows, frameMatRGB.step1(), QImage::Format_RGB32);

            if (_verticalMirror)
                rgb32Image = rgb32Image.mirrored(false, true);
            else
                rgb32Image = rgb32Image.copy();

            if (frameCount > 50)
                emit frameAvailable(rgb32Image);
        }
        else
            qDebug() << "Cannot read RTSP video frame: " << path;
    }
    if (!capture.isOpened())
        qDebug() << "RTSP video stream is closed: " << path;
}

