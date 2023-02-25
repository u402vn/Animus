#ifndef IMAGETRACKER_H
#define IMAGETRACKER_H

#include <QPoint>
#include <QRect>
#include "opencv2/core/core.hpp"
#include "opencv2/highgui.hpp"
#include "Common/CommonData.h"

class ImageTracker final
{
private:
    cv::Mat _frame;
    cv::Mat _targetImage;
    QPoint _targetCenter;
    int _targetSize;
    bool _targetLocked;
public:
    ImageTracker();
    ~ImageTracker();

    QRect processFrame(const cv::Mat &frame, const FrameShift2D &frameShift);
    void lockTarget(const QPoint &targetCenter);
    void unlockTarget();
    void setTargetSize(int size);    
};

#endif // IMAGETRACKER_H
