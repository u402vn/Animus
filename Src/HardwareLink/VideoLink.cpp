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
    _videoSource = nullptr;
}

VideoLink::~VideoLink()
{
    closeVideoSource();
}

void VideoLink::openVideoSource()
{
    ApplicationSettings& applicationSettings = ApplicationSettings::Instance();
    auto cameraSettings = applicationSettings.installedCameraSettings();
    auto videoConnectionSetting = cameraSettings->videoConnectionSetting(0);

    VideoFrameTrafficSources videoTrafficSource = videoConnectionSetting->VideoTrafficSource->value();

    switch(videoTrafficSource)
    {
    case VideoFrameTrafficSources::USBCamera:
    {
        // https://stackoverflow.com/questions/57352688/camera-start-error-on-qt5-5-qcamera-libv4l2-error-set-fmt-gave-us-a-differe

        auto cameraFrameGrabber = new CameraFrameGrabber(this, cameraSettings->UseVerticalFrameMirrororingA);
        QByteArray camName = cameraSettings->VideoFrameSourceCameraName1.value().toLocal8Bit();
        auto camera = new QCamera(camName, this);

        if (cameraSettings->CamViewSizeForceSetA.value())
        {
            QCameraViewfinderSettings viewfinderSettings;
            viewfinderSettings.setResolution(cameraSettings->CamViewSizeHorizontalA, cameraSettings->CamViewSizeVerticalA);
            camera->setViewfinderSettings(viewfinderSettings);
        }
        connect(camera, static_cast<void(QCamera::*)(QCamera::Error)>(&QCamera::error), this, &VideoLink::usbCameraError);
        //connect(camera, SIGNAL(error(QCamera::Error)), this, SLOT(usbCameraError(QCamera::Error)));

        camera->setViewfinder(cameraFrameGrabber);
        connect(cameraFrameGrabber, &CameraFrameGrabber::frameAvailable, this, &VideoLink::videoFrameReceivedInternal, Qt::QueuedConnection);
        _videoSource = camera;

        camera->start();
        //QCamera::supportedViewfinderResolutions()
        break;
    } // case VideoFrameTrafficSources::USBCamera
    case VideoFrameTrafficSources::Yurion:
    {
        auto yurionVideoReceiver = new YurionVideoReceiver(this, cameraSettings->UseVerticalFrameMirrororingA,
                                                           cameraSettings->VideoFrameSourceYurionUDPPort1);
        if (applicationSettings.EnableForwarding.value())
            yurionVideoReceiver->setVideoForwarding(applicationSettings.VideoForwardingAddress, applicationSettings.VideoForwardingPort);
        if (cameraSettings->CamViewSizeForceSetA.value())
            yurionVideoReceiver->setResolution(cameraSettings->CamViewSizeHorizontalA, cameraSettings->CamViewSizeVerticalA);

        connect(yurionVideoReceiver, &YurionVideoReceiver::frameAvailable, this, &VideoLink::videoFrameReceivedInternal);
        _videoSource = yurionVideoReceiver;
        break;
    } // case VideoFrameTrafficSources::Yurion
    case VideoFrameTrafficSources::XPlane:
    {
        auto xPlaneVideoReceiver = new XPlaneVideoReceiver(this, cameraSettings->UseVerticalFrameMirrororingA,
                                                           QHostAddress(cameraSettings->VideoFrameSourceXPlaneAddress1),
                                                           static_cast<quint16>(cameraSettings->VideoFrameSourceXPlanePort1));
        connect(xPlaneVideoReceiver, &XPlaneVideoReceiver::frameAvailable, this, &VideoLink::videoFrameReceivedInternal);
        _videoSource = xPlaneVideoReceiver;
        break;
    } // case VideoFrameTrafficSources::XPlane
    case VideoFrameTrafficSources::CalibrationImage:
    {
        auto calibrationImageVideoReceiver = new CalibrationImageVideoReceiver(this, cameraSettings->CalibrationImagePath1, DefaultCalibrationImagePath);
        connect(calibrationImageVideoReceiver, &CalibrationImageVideoReceiver::frameAvailable, this, &VideoLink::videoFrameReceivedInternal);
        _videoSource = calibrationImageVideoReceiver;
        break;
    } // case VideoFrameTrafficSources::CalibrationImage
    case VideoFrameTrafficSources::VideoFile:
    {
        QString filePath = cameraSettings->VideoFilePath1;
        openVideoSourceWithURL(QUrl::fromLocalFile(filePath), cameraSettings->UseVerticalFrameMirrororingA);
        break;
    } // case VideoFrameTrafficSources::VideoFile
    case VideoFrameTrafficSources::RTSP:
    {
        auto rtspVideoReceiver = new RTSPVideoReceiver(this, cameraSettings->UseVerticalFrameMirrororingA,
                                                       QUrl(cameraSettings->RTSPUrl1));
        connect(rtspVideoReceiver, &RTSPVideoReceiver::frameAvailable, this, &VideoLink::videoFrameReceivedInternal, Qt::DirectConnection);
        _videoSource = rtspVideoReceiver;
        break;
    }
    case VideoFrameTrafficSources::MUSV2:
    {
        auto musv2VideoReceiver = new MUSV2VideoReceiver(this, cameraSettings->UseVerticalFrameMirrororingA, cameraSettings->VideoFrameSourceMUSV2UDPPort1);
        _videoSource = musv2VideoReceiver;
        break;
    }
    } // switch(videoTrafficSource)
}    

void VideoLink::openVideoSourceWithURL(const QUrl &url, bool useVerticalFrameMirrororing)
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
    _videoSource = player;
    player->play();
}


void VideoLink::closeVideoSource()
{
    delete _videoSource;
    _videoSource = nullptr;
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
