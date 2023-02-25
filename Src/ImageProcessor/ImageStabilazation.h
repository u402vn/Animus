#ifndef IMAGESTABILAZATION_H
#define IMAGESTABILAZATION_H

#include <QPoint>
#include "opencv2/core/core.hpp"
#include "opencv2/highgui.hpp"
#include "Common/CommonData.h"

// http://recog.ru/blog/opencv/209.html
// https://gist.github.com/anonymous/7ae54f90fa923d055a5edcc29c9cc3fd

class FloatFilter final
{
    double _minValue, _maxValue, _k;
    double _sum;
public:
    FloatFilter(double minValue, double maxValue, double k);
    double process(double value);
};

class ImageStabilazation final
{
private:
    quint32 _frameNumber;
    FloatFilter _filterX, _filterY, _filterA;
    FrameShift2D _lastShift, _correctionShift;
    cv::Mat _prevFrame;
    std::vector<cv::Point2f> _prevPoints;

    bool _verticalMirror;
public:
    ImageStabilazation(bool verticalMirror);
    ~ImageStabilazation();

    void ProcessFrame(const cv::Mat &sourceFrame);
    FrameShift2D getLastFrameCorrectionShift() const;
    FrameShift2D getLastFrameShift() const;
};

#endif // IMAGESTABILAZATION_H
