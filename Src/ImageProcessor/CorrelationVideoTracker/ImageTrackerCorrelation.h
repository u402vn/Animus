#ifndef IMAGETRACKERCORRELATION_H
#define IMAGETRACKERCORRELATION_H

#include <QPoint>
#include <QRect>
#include "opencv2/core/core.hpp"
#include "opencv2/highgui.hpp"
#include "Common/CommonData.h"
#include "ImageProcessor/CorrelationVideoTracker/CorrelationVideoTracker.h"
#include "ImageProcessor/ImageTracker.h"

class ImageTrackerCorrelation : public ImageTracker
{
private:
    vtracker::CorrelationVideoTracker *_correlationTracker;
    cv::Mat _frameGray;
    int _targetSize;
public:
    explicit ImageTrackerCorrelation();
    virtual ~ImageTrackerCorrelation();

    virtual QRect processFrame(const cv::Mat &frame, const FrameShift2D &frameShift);
    virtual void lockTarget(const QPoint &targetCenter);
    virtual void unlockTarget();
    virtual void setTargetSize(int size);
};

#endif // IMAGETRACKERCORRELATION_H
