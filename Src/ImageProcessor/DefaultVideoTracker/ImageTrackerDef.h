#ifndef IMAGETRACKERDEF_H
#define IMAGETRACKERDEF_H

#include <QPoint>
#include <QRect>
#include "opencv2/core/core.hpp"
#include "opencv2/highgui.hpp"
#include "Common/CommonData.h"
#include "ImageProcessor/ImageTracker.h"

class ImageTrackerDef : public ImageTracker
{
private:
    cv::Mat _frame;
    cv::Mat _targetImage;
    QPoint _targetCenter;
    int _targetSize;
    bool _targetLocked;
public:
    explicit ImageTrackerDef();
    virtual ~ImageTrackerDef();

    QRect processFrame(const cv::Mat &frame, const FrameShift2D &frameShift);
    void lockTarget(const QPoint &targetCenter);
    void unlockTarget();
    void setTargetSize(int size);    
};

#endif // IMAGETRACKERDEF_H
