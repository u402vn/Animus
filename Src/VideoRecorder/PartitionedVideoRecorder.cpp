#include "PartitionedVideoRecorder.h"
#include <QTime>
#include <QDebug>
#include <QImage>
#include "opencv2/core/core.hpp"
#include "opencv2/imgproc.hpp"
#include "Common/CommonData.h"

using namespace cv;

void PartitionedVideoRecorder::swapVideoFiles()
{
    QString fileName = getVideoFileNameForFrame(_fileDirecory, _recordName, _videoFileFrameCount, _frameNumber);

    int fps = VIDEO_FILE_FRAME_FREQUENCY;
    //int codec = CV_FOURCC('X', 'V', 'I', 'D');
    int codec = CV_FOURCC('F', 'M', 'P', '4');
    std::string utf8_text_fileName = fileName.toUtf8().constData();

    qDebug() << "Write Video File '" << fileName <<
                "' (" << _frameWidth << " x " << _frameHeight << ") with quality " << _videoFileQuality;

    _videoWriter->open(utf8_text_fileName, codec, fps, cvSize(_frameWidth, _frameHeight), true);
    //_videoWriter->set(VIDEOWRITER_PROP_QUALITY, _videoFileQuality); //may be this property sometimes brokes recording

    if (!_videoWriter->isOpened())
        qDebug() << "Video File is not opened!";

}

PartitionedVideoRecorder::PartitionedVideoRecorder(QObject *parent) : QObject(parent)
{
    _frameNumber = 0;
    _frameWidth = 0;
    _frameHeight = 0;
    _videoFileQuality = VIDEO_FILE_QUALITY_DEFAULT;

    _videoWriter = new VideoWriter();
}

PartitionedVideoRecorder::~PartitionedVideoRecorder()
{
    stop();
    delete _videoWriter;
}

void PartitionedVideoRecorder::start(const QString &fileDirecory, const QString &recordName, const quint32 videoFileFrameCount, const quint32 videoFileQuality)
{
    if (_frameNumber > 0)
        stop();

    _frameNumber = 0;
    _fileDirecory = fileDirecory;
    _recordName = recordName;
    _videoFileFrameCount = videoFileFrameCount;
    _videoFileQuality = videoFileQuality;
}

void PartitionedVideoRecorder::stop()
{
    _videoWriter->release();
}

Mat QImage2MatColor(QImage const& srcImage)
{
    Q_ASSERT(srcImage.format() == QImage::Format_RGB32);
    Mat tempMat(srcImage.height(), srcImage.width(), CV_8UC4, (uchar*)srcImage.bits(), srcImage.bytesPerLine());
    Mat resultMat;
    cvtColor(tempMat, resultMat, CV_RGBA2RGB);
    return resultMat;
}

void PartitionedVideoRecorder::saveFrame(const QImage &frame)
{
    if (_frameNumber == 0)
    {
        _frameWidth = frame.width();
        _frameHeight = frame.height();
        qDebug() << "Frame Size: " << _frameWidth << " x " << _frameHeight;
    }

    if (_frameNumber == 0 || (_videoFileFrameCount > 0 && _frameNumber % _videoFileFrameCount == 0))
        swapVideoFiles();

    _frameNumber++;

    Mat frameMat = QImage2MatColor(frame);
    _videoWriter->write(frameMat);
}

quint32 PartitionedVideoRecorder::frameWidth()
{
    return _frameWidth;
}

quint32 PartitionedVideoRecorder::frameHeight()
{
    return _frameHeight;
}

QString getVideoFileNameForFrame(const QString &fileDirecory, const QString &recordName, quint32 videoFileFrameCount, quint32 frameNumber)
{
    quint32 partNumber = videoFileFrameCount > 0 ? frameNumber / videoFileFrameCount + 1 : 1;
    QString fileName = QString("%1/%2_%3.avi").arg(fileDirecory).arg(recordName).arg(partNumber);
    return fileName;
}
