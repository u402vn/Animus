#include "VideoLink.h"
#include <QDebug>
#include <QMediaPlayer>
#include "ApplicationSettings.h"
#include "VideoRecorder/CameraFrameGrabber.h"
#include "HardwareLink/YurionVideoReceiver.h"
#include "HardwareLink/XPlaneVideoReceiver.h"
#include "HardwareLink/RTSPVideoReceiver.h"
#include "HardwareLink/MUSV2VideoReceiver.h"
#include "HardwareLink/CalibrationImageVideoReceiver.h"

VideoLink::VideoLink(QObject *parent) : QObject(parent)
{

}

VideoLink::~VideoLink()
{
    closeVideoSource();
}

void VideoLink::openVideoSource()
{
    openVideoConnection(1);
    openVideoConnection(2);
}

QObject *VideoLink::openVideoConnection(int connectionId)
{
    ApplicationSettings& applicationSettings = ApplicationSettings::Instance();
    auto videoConnectionSetting = applicationSettings.installedCameraSettings()->videoConnectionSetting(connectionId);

    auto assemblyPreferences = applicationSettings.getCurrentCamAssemblyPreferences();
    auto opticalDevice = assemblyPreferences->opticalDevice(1);

    VideoFrameTrafficSources videoTrafficSource = videoConnectionSetting->VideoTrafficSource->value();
    bool mirroring = opticalDevice->useVerticalFrameMirrororing(); //???todo
    bool forceSetResolution = opticalDevice->forceSetResolution(); //???todo

    QObject *videoSource = nullptr;

    switch(videoTrafficSource)
    {
    case VideoFrameTrafficSources::USBCamera:
    {
        // https://stackoverflow.com/questions/57352688/camera-start-error-on-qt5-5-qcamera-libv4l2-error-set-fmt-gave-us-a-differe

        //auto cameraFrameGrabber = new CameraFrameGrabber(this, mirroring);
        QByteArray camName = videoConnectionSetting->VideoFrameSourceCameraName->value().toLocal8Bit();
        auto camera = new QCamera(camName, this);

        auto cameraFrameGrabber = new CameraFrameGrabber(camera, mirroring);

        if (forceSetResolution)
        {
            QCameraViewfinderSettings viewfinderSettings;
            viewfinderSettings.setResolution(opticalDevice->frameWidth(), opticalDevice->frameHeight());
            camera->setViewfinderSettings(viewfinderSettings);
        }
        connect(camera, static_cast<void(QCamera::*)(QCamera::Error)>(&QCamera::error), this, &VideoLink::usbCameraError);
        //connect(camera, SIGNAL(error(QCamera::Error)), this, SLOT(usbCameraError(QCamera::Error)));

        camera->setViewfinder(cameraFrameGrabber);
        connect(cameraFrameGrabber, &CameraFrameGrabber::frameAvailable, this, &VideoLink::videoFrameReceivedInternal, Qt::QueuedConnection);
        videoSource = cameraFrameGrabber;

        camera->start();
        //QCamera::supportedViewfinderResolutions()
        break;
    } // case VideoFrameTrafficSources::USBCamera
    case VideoFrameTrafficSources::Yurion:
    {
        auto yurionVideoReceiver = new YurionVideoReceiver(this, mirroring, videoConnectionSetting->VideoFrameSourceYurionUDPPort->value());
        if (applicationSettings.EnableForwarding.value())
            yurionVideoReceiver->setVideoForwarding(applicationSettings.VideoForwardingAddress, applicationSettings.VideoForwardingPort);
        if (forceSetResolution)
            yurionVideoReceiver->setResolution(opticalDevice->frameWidth(), opticalDevice->frameHeight());

        connect(yurionVideoReceiver, &YurionVideoReceiver::frameAvailable, this, &VideoLink::videoFrameReceivedInternal);
        videoSource = yurionVideoReceiver;
        break;
    } // case VideoFrameTrafficSources::Yurion
    case VideoFrameTrafficSources::XPlane:
    {
        auto xPlaneVideoReceiver = new XPlaneVideoReceiver(this, mirroring,
                                                           QHostAddress(videoConnectionSetting->VideoFrameSourceXPlaneAddress->value()),
                                                           static_cast<quint16>(videoConnectionSetting->VideoFrameSourceXPlanePort->value()));
        connect(xPlaneVideoReceiver, &XPlaneVideoReceiver::frameAvailable, this, &VideoLink::videoFrameReceivedInternal);
        videoSource = xPlaneVideoReceiver;
        break;
    } // case VideoFrameTrafficSources::XPlane
    case VideoFrameTrafficSources::CalibrationImage:
    {
        auto calibrationImageVideoReceiver = new CalibrationImageVideoReceiver(this, videoConnectionSetting->CalibrationImagePath->value(), DefaultCalibrationImagePath);
        connect(calibrationImageVideoReceiver, &CalibrationImageVideoReceiver::frameAvailable, this, &VideoLink::videoFrameReceivedInternal);
        videoSource = calibrationImageVideoReceiver;
        break;
    } // case VideoFrameTrafficSources::CalibrationImage
    case VideoFrameTrafficSources::VideoFile:
    {
        QString filePath = videoConnectionSetting->VideoFilePath->value();
        videoSource = openVideoSourceWithURL(QUrl::fromLocalFile(filePath), mirroring);
        break;
    } // case VideoFrameTrafficSources::VideoFile
    case VideoFrameTrafficSources::RTSP:
    {
        auto rtspVideoReceiver = new RTSPVideoReceiver(this, mirroring, QUrl(videoConnectionSetting->RTSPUrl->value()));
        connect(rtspVideoReceiver, &RTSPVideoReceiver::frameAvailable, this, &VideoLink::videoFrameReceivedInternal, Qt::DirectConnection);
        videoSource = rtspVideoReceiver;
        break;
    }
    case VideoFrameTrafficSources::MUSV2:
    {
        auto musv2VideoReceiver = new MUSV2VideoReceiver(this, mirroring, videoConnectionSetting->VideoFrameSourceMUSV2UDPPort->value());
        videoSource = musv2VideoReceiver;
        break;
    }
    } // switch(videoTrafficSource)


    _videoSources.insert(connectionId, qobject_cast<QObject*>(videoSource));

    return videoSource;
}    

QObject *VideoLink::openVideoSourceWithURL(const QUrl &url, bool useVerticalFrameMirrororing)
{
    auto player = new QMediaPlayer(this);
    auto frameGrabber = new CameraFrameGrabber(player, useVerticalFrameMirrororing);

    player->setMedia(url);
    player->setVideoOutput(frameGrabber);
    player->setMuted(true);
    connect(frameGrabber, &CameraFrameGrabber::frameAvailable, this, &VideoLink::videoFrameReceivedInternal);
    // loop video
    connect(player, &QMediaPlayer::mediaStatusChanged, [player](QMediaPlayer::MediaStatus status)
    {
        if (status == QMediaPlayer::EndOfMedia)
        {
            qDebug() << "Restart the video file/stream playback.";
            player->stop();
            player->play();
        }
    });
    player->play();
    return frameGrabber;
}


void VideoLink::closeVideoSource()
{
    return;

    for (int i = 1; i <= 2; i++)
    {
        auto videoSource = _videoSources[i];
        _videoSources.remove(i);
        delete videoSource;
    }

    //delete _videoSources[1];
    //delete _videoSources[2];
    //_videoSources.clear();
}

void VideoLink::selectActiveCam(int camId)
{
    _opticalSystemId = camId;
    //todo process forceSetResolution???
}

void VideoLink::usbCameraError(QCamera::Error value)
{
    qDebug() << "USB camera error: " << value;
}

SimpleVideoLink::SimpleVideoLink(QObject *parent) : VideoLink(parent)
{

}

SimpleVideoLink::~SimpleVideoLink()
{

}

void SimpleVideoLink::videoFrameReceivedInternal(const QImage &frame)
{
    emit videoFrameReceived(frame);
}
