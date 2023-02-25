#ifndef CAMCALIBRATOR_H
#define CAMCALIBRATOR_H

#include <QObject>
#include <QThread>
#include <QImage>
#include "EnterProc.h"
#include <opencv2/core/core.hpp>


struct CamCalibratorParams
{
    size_t requiredFrameCount;
    int patternSize;
    float squareLength;
    int scanInterval;
};

class CamCalibratorWorker final: public QObject
{
    friend class CamCalibrator;

    Q_OBJECT

    bool _calibrationInProgress;

    CamCalibratorParams _params;

    std::vector<std::vector<cv::Point2f> > _detectedPoints;
    ///Nubmer of squares on eachchessboard's side.
    //static const
    cv::Size _patternSize;
    ///Number of grabed chessboard's views.
    size_t _framesGrabed;
    ///Resolution of the calibrating camera(s).
    cv::Size _cameraResolution;

    double _initialFocusValue;

    ///Detected chessboard's corners.
    void orderDetectedPoints(std::vector<cv::Point2f>& points);

    void detectPoints(const QImage &frame);
    void calibrate();

    explicit CamCalibratorWorker(QObject *parent);
public:
    ~CamCalibratorWorker();
public slots:
    void processFrame(const QImage &frame);
    void start(const CamCalibratorParams params);
    void stop();

    void process();
signals:
    void progressChanged(int processedPercent);
    void calibrationCompleted();
};

class CamCalibrator final : public QObject
{
    Q_OBJECT
    QThread * _thread;

    quint32 _calibrationInterval;
    quint32 _frameCount;
public:
    explicit CamCalibrator(QObject *parent);
    ~CamCalibrator();

    void start(const CamCalibratorParams params);
    void stop();
signals:
    void frameAvailable(const QImage &frame);
    void progressChanged(int processedPercent);
    void startWorker(const CamCalibratorParams params);
    void stopWorker();

    void calibrationCompleted();
public slots:
    void processFrame(const QImage &frame);
};

#endif // CAMCALIBRATOR_H
