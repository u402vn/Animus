#include "ImageProcessor.h"
#include "opencv2/core/core.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/imgproc.hpp"
#include "ImageProcessorUtils.h"
#include "ImageProcessor/CorrelationVideoTracker/ImageTrackerCorrelation.h"
#include "ImageProcessor/DefaultVideoTracker/ImageTrackerDef.h"
#include "ImageProcessor/RTVideoTracker/ImageTrackerRT.h"

using namespace cv;

// Пример "Blocking Fortune Client"
// http://doc.crossplatform.ru/qt/4.3.5/network-blockingfortuneclient.html

// Поддержка потоков в Qt
// http://doc.crossplatform.ru/qt/4.3.5/threads.html

// Queued Custom Type Example
// http://doc.qt.io/qt-5/qtcore-threads-queuedcustomtype-example.html

// Пример использования QtConcurrent
// http://itnotesblog.ru/note.php?id=148#sthash.9iW7YuaJ.Y6xedreF.dpbs

ImageProcessor::ImageProcessor(QObject *parent, CoordinateCalculator *coordinateCalculator, bool verticalMirror, ObjectTrackerTypeEnum trackerType) : QObject(parent),
    _coordinateCalculator(coordinateCalculator)
{    
    _procThread = new ImageProcessorThread(nullptr, verticalMirror, trackerType);

    connect(_procThread, &ImageProcessorThread::dataProcessedInThread, this, &ImageProcessor::dataProcessedInThread);

    qInfo() << "OpenCV version: \n" << getBuildInformation().c_str();
}

ImageProcessor::~ImageProcessor()
{
    delete _procThread;
}

void ImageProcessor::processDataAsync(const TelemetryDataFrame &telemetryFrame, const QImage &videoFrame)
{
    _procThread->processData(telemetryFrame, videoFrame);
}

void ImageProcessor::lockTarget(const QPoint &targetCenter)
{    
    _procThread->lockTarget(targetCenter);
}

void ImageProcessor::unlockTarget()
{
    _procThread->unlockTarget();
}

void ImageProcessor::setTargetSize(int targetSize)
{
    _procThread->setTargetSize(targetSize);
}

void ImageProcessor::setTuneImageSettings(qreal brightness, qreal contrast, qreal gamma, bool grayscale)
{
    _procThread->setTuneImageSettings(brightness, contrast, gamma, grayscale);
}

void ImageProcessor::getTuneImageSettings(qreal &brightness, qreal &contrast, qreal &gamma, bool &grayscale)
{
    _procThread->getTuneImageSettings(brightness, contrast, gamma, grayscale);
}

void ImageProcessor::dataProcessedInThread(const TelemetryDataFrame &telemetryFrame, const QImage &videoFrame)
{    
    TelemetryDataFrame frame = telemetryFrame;
    _coordinateCalculator->processTelemetryDataFrame(&frame);

    emit onDataProcessed(frame, videoFrame);
}

ImageProcessorThread::ImageProcessorThread(QObject *parent, bool verticalMirror, ObjectTrackerTypeEnum trackerType): QThread(parent)
{
    _quit = false;
    _videoFrames = new QQueue<QImage>();
    _telemetryFrames = new QQueue<TelemetryDataFrame>();
    _imageCorrector = new ImageCorrector();
    _imageStabilazation = new ImageStabilazation(verticalMirror);

    switch (trackerType)
    {
    case ObjectTrackerTypeEnum::InternalCorrelation:
        _imageTracker = new ImageTrackerCorrelation();
        break;
    case ObjectTrackerTypeEnum::InternalRT:
        _imageTracker = new ImageTrackerRT();
        break;
    case ObjectTrackerTypeEnum::InternalDefault:
        _imageTracker = new ImageTrackerDef();
        break;
    case ObjectTrackerTypeEnum::External:
        _imageTracker = nullptr;
        break;
    default:
        _imageTracker = nullptr;
    }
}

ImageProcessorThread::~ImageProcessorThread()
{
    _quit = true;
    _waitCondition.wakeOne();
    wait();
    delete _videoFrames;
    delete _telemetryFrames;
    delete _imageCorrector;
    delete _imageStabilazation;
    if ( _imageTracker != nullptr)
        delete _imageTracker;
}

void ImageProcessorThread::run()
{
    while (!_quit)
    {
        _mutex.lock();
        if (_videoFrames->isEmpty())
            _waitCondition.wait(&_mutex);
        if (_videoFrames->isEmpty())
        {
            _mutex.unlock();
            break;
        }

        QImage videoFrame = _videoFrames->dequeue();
        TelemetryDataFrame telemetryFrame = _telemetryFrames->dequeue();

        _mutex.unlock();

        if ( !videoFrame.isNull() )
        {
            videoFrame = _imageCorrector->ProcessFrame(videoFrame);

            const Mat frameMat = QImage2MatGS(videoFrame);
            _imageStabilazation->ProcessFrame(frameMat);
            FrameShift2D correctionFrameShift = _imageStabilazation->getLastFrameCorrectionShift();
            FrameShift2D frameShift = _imageStabilazation->getLastFrameShift();

            telemetryFrame.StabilizedCenterX = frameMat.cols / 2 + correctionFrameShift.X;
            telemetryFrame.StabilizedCenterY = frameMat.rows / 2 + correctionFrameShift.Y;
            telemetryFrame.StabilizedRotationAngle = correctionFrameShift.A;

            if (_imageTracker != nullptr)
            {
                QRect targetRect = _imageTracker->processFrame(frameMat, frameShift);
                telemetryFrame.TrackedTargetCenterX = targetRect.center().x();
                telemetryFrame.TrackedTargetCenterY = targetRect.center().y();
                telemetryFrame.TrackedTargetRectWidth = targetRect.width();
                telemetryFrame.TrackedTargetRectHeight = targetRect.height();
            }
        }
        emit dataProcessedInThread(telemetryFrame, videoFrame);
    }
}

void ImageProcessorThread::processData(const TelemetryDataFrame &telemetryFrame, const QImage &videoFrame)
{
    _mutex.lock();
    _videoFrames->enqueue(videoFrame);
    _telemetryFrames->enqueue(telemetryFrame);
    _mutex.unlock();
    if (!isRunning())
        start();
    else
        _waitCondition.wakeOne();
}

void ImageProcessorThread::lockTarget(const QPoint &targetCenter)
{
    if (_imageTracker != nullptr)
    {
        _mutex.lock();
        _imageTracker->lockTarget(targetCenter);
        _mutex.unlock();
    }
}

void ImageProcessorThread::unlockTarget()
{
    if (_imageTracker != nullptr)
    {
        _mutex.lock();
        _imageTracker->unlockTarget();
        _mutex.unlock();
    }
}

void ImageProcessorThread::setTargetSize(int targetSize)
{
    if (_imageTracker != nullptr)
    {
        _mutex.lock();
        _imageTracker->setTargetSize(targetSize);
        _mutex.unlock();
    }
}

void ImageProcessorThread::setTuneImageSettings(qreal brightness, qreal contrast, qreal gamma, bool grayscale)
{
    _mutex.lock();
    _imageCorrector->setTuneImageSettings(brightness, contrast, gamma, grayscale);
    _mutex.unlock();
}

void ImageProcessorThread::getTuneImageSettings(qreal &brightness, qreal &contrast, qreal &gamma, bool &grayscale)
{
    _mutex.lock();
    _imageCorrector->getTuneImageSettings(brightness, contrast, gamma, grayscale);
    _mutex.unlock();
}
