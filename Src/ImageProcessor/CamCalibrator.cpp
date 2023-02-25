#include "CamCalibrator.h"
#include <QDebug>
#include <opencv2/calib3d/calib3d.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include "ImageProcessorUtils.h"

// https://www.edmundoptics.com/resources/application-notes/imaging/understanding-focal-length-and-field-of-view/
// https://docs.opencv.org/2.4/modules/calib3d/doc/camera_calibration_and_3d_reconstruction.html
// https://stackoverflow.com/questions/35942095/opencv-strange-rotation-and-translation-matrices-from-decomposehomographymat
// https://www.researchgate.net/post/How_to_decompose_homography_matrix_in_opencv
// https://docs.opencv.org/2.4/modules/imgproc/doc/geometric_transformations.html#undistortpoints

CamCalibrator::CamCalibrator(QObject *parent) : QObject(parent)
{
    EnterProcStart("CamCalibrator::CamCalibrator");

    _calibrationInterval = 15;
    _frameCount = 0;

    CamCalibratorWorker * worker = new CamCalibratorWorker(nullptr);
    connect(this, &CamCalibrator::frameAvailable, worker, &CamCalibratorWorker::processFrame, Qt::QueuedConnection);
    connect(this, &CamCalibrator::startWorker, worker, &CamCalibratorWorker::start, Qt::QueuedConnection);
    connect(this, &CamCalibrator::stopWorker, worker, &CamCalibratorWorker::stop, Qt::QueuedConnection);

    connect(worker, &CamCalibratorWorker::progressChanged, this, &CamCalibrator::progressChanged, Qt::QueuedConnection);
    connect(worker, &CamCalibratorWorker::calibrationCompleted, this, &CamCalibrator::calibrationCompleted, Qt::QueuedConnection);
    _thread = new QThread;
    worker->moveToThread(_thread);
    connect(_thread, &QThread::started, worker, &CamCalibratorWorker::process);
    connect(_thread, &QThread::finished, worker, &CamCalibratorWorker::deleteLater);

    _thread->start();
}

CamCalibrator::~CamCalibrator()
{
    EnterProcStart("CamCalibrator::~CamCalibrator");
    _thread->quit();
    _thread->wait();
    delete _thread;
}

void CamCalibrator::start(const CamCalibratorParams params)
{
    emit startWorker(params);
}

void CamCalibrator::stop()
{
    emit stopWorker();
}

void CamCalibrator::processFrame(const QImage &frame)
{
    if (_frameCount % _calibrationInterval == 0)
        emit frameAvailable(frame);
    _frameCount++;
}

//-------------------------------------------------------------------

CamCalibratorWorker::CamCalibratorWorker(QObject *parent): QObject(parent)
{

}

CamCalibratorWorker::~CamCalibratorWorker()
{

}

void CamCalibratorWorker::process()
{

}

void CamCalibratorWorker::detectPoints(const QImage &frame)
{
    cv::Mat frameMat = QImage2MatGS(frame);
    if (cv::findChessboardCorners(frameMat, _patternSize, _detectedPoints[_framesGrabed]))
    {
        _cameraResolution = frameMat.size();

        orderDetectedPoints(_detectedPoints[_framesGrabed]);
        cv::cornerSubPix( frameMat, _detectedPoints[_framesGrabed], cv::Size(11, 11), cv::Size(-1, -1),
                          cv::TermCriteria(cv::TermCriteria::EPS | cv::TermCriteria::MAX_ITER, 30, 0.1 ) );
        ++ _framesGrabed;

        int processedPercent = 100 * _framesGrabed / _params.requiredFrameCount;
        emit progressChanged(processedPercent);
        qDebug() << "CamCalibratorWorker - frame processed! ";
    }
}

void CamCalibratorWorker::calibrate()
{
    ///Positions of chessboard's corners in the camera's coordinate system.
    std::vector<std::vector<cv::Point3f> > _realPoints;

    _realPoints.resize(_params.requiredFrameCount);
    const int pointsCount = _patternSize.width * _patternSize.height;
    float temp = _params.squareLength * (_patternSize.width >> 1);
    int j;
    for(size_t i = 0; i < _params.requiredFrameCount; ++i)
    {
        _realPoints[i].resize(pointsCount);
        for (j = 0; j < pointsCount; ++j)
        {
            _realPoints[i][j].x = _params.squareLength * (j % _patternSize.width);
            _realPoints[i][j].x -= temp;
            _realPoints[i][j].y = _params.squareLength * (j / _patternSize.height);
            _realPoints[i][j].y -= temp;
            _realPoints[i][j].z = 0.0f;
        }
    }

    std::vector<cv::Mat> rvecs(_realPoints.size());
    std::vector<cv::Mat> tvecs(_realPoints.size());


    cv::Mat _distortionCoefficients;
    _distortionCoefficients = cv::Scalar(0);
    cv::Mat _cameraMatrix;
    _cameraMatrix = cv::Scalar(0);

    int flags = cv::CALIB_FIX_K3;
    if (_initialFocusValue > 0.0)
    {
        _cameraMatrix.at<double>(0, 0) = _initialFocusValue;
        _cameraMatrix.at<double>(0, 2) = _cameraResolution.width >> 1;
        _cameraMatrix.at<double>(1, 1) = _initialFocusValue;
        _cameraMatrix.at<double>(1, 2) = _cameraResolution.height >> 1;
        _cameraMatrix.at<double>(2, 2) = 1.0;
        flags |= cv::CALIB_USE_INTRINSIC_GUESS;
    }
    cv::calibrateCamera(_realPoints, _detectedPoints,
                        _cameraResolution, _cameraMatrix, _distortionCoefficients,
                        rvecs, tvecs, flags);
    cv::Mat _R = rvecs[0];
    cv::Mat _T = tvecs[0];

    _calibrationInProgress = false;

    emit calibrationCompleted();
}

void CamCalibratorWorker::processFrame(const QImage &frame)
{
    if (!_calibrationInProgress)
        return;
    if (_framesGrabed < _params.requiredFrameCount)
        detectPoints(frame);
    else
        calibrate();
}

void CamCalibratorWorker::start(const CamCalibratorParams params)
{
    _params = params;

    _framesGrabed = 0;
    _initialFocusValue = -1.0;

    _patternSize = cv::Size(params.patternSize, params.patternSize);

    _detectedPoints.resize(params.requiredFrameCount);
    for(size_t i = 0; i < params.requiredFrameCount; ++i)
        _detectedPoints[i].resize(params.patternSize * params.patternSize);

    _calibrationInProgress = true;
}

void CamCalibratorWorker::stop()
{
    _calibrationInProgress = false;
}

void CamCalibratorWorker::orderDetectedPoints(std::vector<cv::Point2f> &points)
{
    cv::Point2f temp;
    int lastIndex=_patternSize.width-1;
    int maxJ , j, i = 0;
    if (points[0].x > points[points.size() - 1].x)
    {
        if (points[0].y > points[points.size() - 1].y)
        {
            for( ; i < lastIndex; ++i)
            {
                maxJ = lastIndex - i;
                for (j = i; j < maxJ; ++j)
                {
                    std::swap(points[i * _patternSize.width +             j], points[(lastIndex - i) * _patternSize.width + lastIndex - j]);
                    std::swap(points[j * _patternSize.width + lastIndex - i], points[(lastIndex - j) * _patternSize.width +             i]);
                }
            }
        }
        else
        {
            for( ; i < lastIndex; ++i)
            {
                maxJ = lastIndex - i;
                for(j = i; j < maxJ; ++j)
                {
                    temp = points[i * _patternSize.width + j];
                    points[i * _patternSize.width + j] = points[(lastIndex - j) * _patternSize.width + i];
                    points[(lastIndex - j) * _patternSize.width +             i] = points[(lastIndex - i) * _patternSize.width + lastIndex - j];
                    points[(lastIndex - i) * _patternSize.width + lastIndex - j] = points[              j * _patternSize.width + lastIndex - i];
                    points[j * _patternSize.width + lastIndex - i] = temp;
                }
            }
        }
    }
    else if (points[0].y > points[points.size() - 1].y)
    {
        for ( ; i < lastIndex; ++i)
        {
            maxJ = lastIndex - i;
            for(j = i; j < maxJ; ++j)
            {
                temp = points[i * _patternSize.width + j];
                points[i * _patternSize.width +             j] = points[              j * _patternSize.width + lastIndex - i];
                points[j * _patternSize.width + lastIndex - i] = points[(lastIndex - i) * _patternSize.width + lastIndex - j];
                points[(lastIndex - i) * _patternSize.width + lastIndex - j] = points[(lastIndex - j) * _patternSize.width + i];
                points[(lastIndex - j) * _patternSize.width +             i] = temp;
            }
        }
    }
}

