#include "ImageTrackerRT.h"
#include "opencv2/opencv.hpp"
#include <QDebug>

using namespace std;
using namespace cv;
using namespace rf;


ImageTrackerRT::ImageTrackerRT() : ImageTracker()
{
    _rtTracker = nullptr;
    _targetSize = 20;
}

ImageTrackerRT::~ImageTrackerRT()
{
    delete _rtTracker;
}

QRect ImageTrackerRT::processFrame(const Mat &frame, const FrameShift2D &frameShift)
{
    if (_rtTracker == nullptr)
    {
        _rtTracker = new RfVideoTracker();
        _rtTracker->SetProperty(RfVideoTrackerProperty::FRAME_WIDTH, frame.cols);
        _rtTracker->SetProperty(RfVideoTrackerProperty::FRAME_HEIGHT, frame.rows);
        setTargetSize(_targetSize);
    };
    frame.convertTo(_frameGray, CV_8U);
    _rtTracker->AddFrame(_frameGray.data);
    _rtTracker->ExecuteCommand(RfVideoTrackerCommand::PROCESS_FRAME, 1, -1, -1, nullptr);

    RfVideoTrackerData trackerData = _rtTracker->GetTrackerData();
    if (trackerData.mode == RF_TRACKING_MODE_INDEX)
    {
        QRect result(trackerData.strobe_x - (trackerData.strobe_w / 2) + trackerData.substrobe_x - (trackerData.substrobe_w / 2),
                     trackerData.strobe_y - (trackerData.strobe_h / 2) + trackerData.substrobe_y - trackerData.substrobe_h / 2,
                     trackerData.substrobe_w, trackerData.substrobe_h);
        return result;
    }
    else
        return QRect(0, 0, 0, 0);
}

void ImageTrackerRT::lockTarget(const QPoint &targetCenter)
{
    if (_rtTracker != nullptr)
    {
        unlockTarget();
        setTargetSize(_targetSize);
        _rtTracker->ExecuteCommand(RfVideoTrackerCommand::CAPTURE, targetCenter.x(), targetCenter.y(), -1, nullptr);
    }
}

void ImageTrackerRT::unlockTarget()
{
    if (_rtTracker != nullptr)
        _rtTracker->ExecuteCommand(RfVideoTrackerCommand::RESET, -1, -1, -1, nullptr);
}

void ImageTrackerRT::setTargetSize(int size)
{
    _targetSize = size;
    if (_rtTracker != nullptr)
    {
        _rtTracker->SetProperty(RfVideoTrackerProperty::STROBE_WIDTH, _targetSize);
        _rtTracker->SetProperty(RfVideoTrackerProperty::STROBE_HEIGHT, _targetSize);
    }
}
