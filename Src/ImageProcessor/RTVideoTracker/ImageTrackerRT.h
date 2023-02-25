#ifndef IMAGETRACKERRT_H
#define IMAGETRACKERRT_H

#include <QPoint>
#include <QRect>
#include "opencv2/core/core.hpp"
#include "opencv2/highgui.hpp"
#include "Common/CommonData.h"
#include "ImageProcessor/ImageTracker.h"
#include "ImageProcessor/RTVideoTracker/RfVideoTracker.h"

class ImageTrackerRT : public ImageTracker
{
private:
    rf::RfVideoTracker *_rtTracker;
    cv::Mat _frameGray;
    int _targetSize;
public:
    explicit ImageTrackerRT();
    virtual ~ImageTrackerRT();

    QRect processFrame(const cv::Mat &frame, const FrameShift2D &frameShift);
    void lockTarget(const QPoint &targetCenter);
    void unlockTarget();
    void setTargetSize(int size);
};

#endif // IMAGETRACKERRT_H
