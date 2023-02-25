#include "ImageTrackerCorrelation.h"
#include "opencv2/opencv.hpp"
#include <QDebug>

using namespace std;
using namespace cv;
using namespace vtracker;


ImageTrackerCorrelation::ImageTrackerCorrelation() : ImageTracker()
{
    _correlationTracker = nullptr;
    _targetSize = 20;
}

ImageTrackerCorrelation::~ImageTrackerCorrelation()
{
    delete _correlationTracker;
}

QRect ImageTrackerCorrelation::processFrame(const Mat &frame, const FrameShift2D &frameShift)
{
    if (_correlationTracker == nullptr)
    {
        _correlationTracker = new CorrelationVideoTracker();
        _correlationTracker->SetProperty(CorrelationVideoTrackerProperty::NUM_THREADS, 1);

        _correlationTracker->ExecuteCommand(vtracker::CorrelationVideoTrackerCommand::RESET);

        //_rtTracker->SetProperty(CorrelationVideoTrackerProperty::FRAME_WIDTH, frame.cols);
        //_rtTracker->SetProperty(CorrelationVideoTrackerProperty::FRAME_HEIGHT, frame.rows);
        setTargetSize(_targetSize);
    };
    frame.convertTo(_frameGray, CV_8U);
    //_rtTracker->AddFrame(_frameGray.data);
    //_rtTracker->ExecuteCommand(CorrelationVideoTrackerCommand::PROCESS_FRAME, 1, -1, -1, nullptr);
    _correlationTracker->ProcessFrame(_frameGray.data, frame.cols, frame.rows);

    CorrelationVideoTrackerResultData trackerData = _correlationTracker->GetTrackerResultData();
    if (trackerData.mode == CVT_TRACKING_MODE_INDEX)
    {
        //QRect result(trackerData.strobe_x - (trackerData.strobe_w / 2) + trackerData.substrobe_x - (trackerData.substrobe_w / 2),
        //                     trackerData.strobe_y - (trackerData.strobe_h / 2) + trackerData.substrobe_y - trackerData.substrobe_h / 2,
        //                     trackerData.substrobe_w, trackerData.substrobe_h);
        QRect result(trackerData.trackingRectangleCenterX - trackerData.trackingRectangleWidth / 2,
                     trackerData.trackingRectangleCenterY - trackerData.trackingRectangleHeight / 2,
                     trackerData.trackingRectangleWidth, trackerData.trackingRectangleHeight);

        return result;
    }
    else
        return QRect(0, 0, 0, 0);
}

void ImageTrackerCorrelation::lockTarget(const QPoint &targetCenter)
{
    if (_correlationTracker != nullptr)
    {
        unlockTarget();
        setTargetSize(_targetSize);
        _correlationTracker->ExecuteCommand(CorrelationVideoTrackerCommand::CAPTURE, targetCenter.x(), targetCenter.y(), -1, nullptr);
    }
}

void ImageTrackerCorrelation::unlockTarget()
{
    if (_correlationTracker != nullptr)
        _correlationTracker->ExecuteCommand(CorrelationVideoTrackerCommand::RESET, -1, -1, -1, nullptr);
}

void ImageTrackerCorrelation::setTargetSize(int size)
{
    _targetSize = size;
    if (_correlationTracker != nullptr)
    {
        _correlationTracker->SetProperty(CorrelationVideoTrackerProperty::TRACKING_RECTANGLE_WIDTH, _targetSize);
        _correlationTracker->SetProperty(CorrelationVideoTrackerProperty::TRACKING_RECTANGLE_HEIGHT, _targetSize);
    }
}
