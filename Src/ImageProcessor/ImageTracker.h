#ifndef IMAGETRACKER_H
#define IMAGETRACKER_H

#include <QPoint>
#include <QRect>
#include "opencv2/core/core.hpp"
#include "opencv2/highgui.hpp"
#include "Common/CommonData.h"

class ImageTracker
{
public:
    ImageTracker();
    virtual ~ImageTracker();
    virtual QRect processFrame(const cv::Mat &frame, const FrameShift2D &frameShift) = 0;
    virtual void lockTarget(const QPoint &targetCenter) = 0;
    virtual void unlockTarget() = 0;
    virtual void setTargetSize(int size) = 0;
};

#endif // IMAGETRACKER_H
