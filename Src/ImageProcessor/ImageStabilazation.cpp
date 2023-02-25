#include "ImageStabilazation.h"
#include <QtGlobal>
#include "opencv2/opencv.hpp"
#include "opencv2/core/core.hpp"
#include "opencv2/video/tracking.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/calib3d.hpp"
#include <QDebug>

using namespace cv;
using namespace std;

#define CORNER_RETRACK_FRAMES 30
#define EXPECTED_POINTS_COUNT 10
#define MAXIMAL_POINTS_COUNT 20
#define MINIMAL_DISTANCE_BETWEEN_POINTS 50
#define MAXIMAL_FRAME_WIDTH 720.0
#define MAXIMAL_OFFSET_COMPENSATION MAXIMAL_FRAME_WIDTH / 4
#define MAXIMAL_ROTATION_COMPENSATION 20
#define FILTER_UPDATE_K 0.9

ImageStabilazation::ImageStabilazation(bool verticalMirror) :
    _filterX(-MAXIMAL_OFFSET_COMPENSATION,   +MAXIMAL_OFFSET_COMPENSATION,   FILTER_UPDATE_K),
    _filterY(-MAXIMAL_OFFSET_COMPENSATION,   +MAXIMAL_OFFSET_COMPENSATION,   FILTER_UPDATE_K),
    _filterA(-MAXIMAL_ROTATION_COMPENSATION, +MAXIMAL_ROTATION_COMPENSATION, FILTER_UPDATE_K),
    _verticalMirror(verticalMirror)
{
    _frameNumber = 0;

    _correctionShift = {.X = 0, .Y = 0, .A = 0};
    _lastShift = {.X = 0, .Y = 0, .A = 0};
}

ImageStabilazation::~ImageStabilazation(void)
{

}

//https://answers.opencv.org/question/29665/getting-subpixel-with-matchtemplate/
void ImageStabilazation::ProcessFrame(const cv::Mat &sourceFrame)
{
    Q_ASSERT(sourceFrame.channels() == 1);

    Mat curFrame;
    double scale_down  = 1;

    if (!sourceFrame.empty())
    {
        double scale_down = MAXIMAL_FRAME_WIDTH / sourceFrame.cols;
        if (scale_down < 1)
            resize(sourceFrame, curFrame, Size(), scale_down, scale_down, INTER_LINEAR);
        else
            curFrame = sourceFrame;

        //qDebug() << sourceFrame.cols << curFrame.cols << scale_down;
    }
    else
        curFrame = sourceFrame;

    vector<cv::Point2f> curPoints, prevPointsFiltered, curPointsFiltered;
    vector<float> err;
    vector<uchar> status;

    if (_prevFrame.empty())
        curFrame.copyTo(_prevFrame);

    if ( (_frameNumber % CORNER_RETRACK_FRAMES == 0) || (_prevPoints.size() < EXPECTED_POINTS_COUNT))
    {
        _prevPoints.clear();
        goodFeaturesToTrack(_prevFrame, _prevPoints, MAXIMAL_POINTS_COUNT, 0.01, MINIMAL_DISTANCE_BETWEEN_POINTS);
    }

    if (_prevPoints.size() >= EXPECTED_POINTS_COUNT)
        calcOpticalFlowPyrLK(_prevFrame, curFrame, _prevPoints, curPoints, status, err);
    else
        _prevPoints.clear();

    for (size_t i = 0; i < status.size(); i++)
        if (status[i])
        {
            prevPointsFiltered.push_back(_prevPoints[i]);
            curPointsFiltered.push_back(curPoints[i]);
        }

    _prevPoints = curPointsFiltered;

    Mat T;
    if ( (prevPointsFiltered.size() > 2) && (curPointsFiltered.size() > 2) )
        T = estimateRigidTransform(prevPointsFiltered, curPointsFiltered, false);

    bool noTransformation = T.data == nullptr;

    double dx = noTransformation ? _lastShift.X * scale_down: T.at<double>(0, 2);
    double dy = noTransformation ? _lastShift.Y * scale_down: T.at<double>(1, 2);
    double da = noTransformation ? _lastShift.A: rad2deg(atan2(T.at<double>(1, 0), T.at<double>(0, 0)));

    _lastShift = {.X = dx  / scale_down, .Y = dy / scale_down, .A = da};
    _correctionShift = {.X = _filterX.process(dx) / scale_down, .Y = _filterY.process(dy) / scale_down, .A = _filterA.process(da)};

    curFrame.copyTo(_prevFrame); // _prevFrame = curFrame;

    _frameNumber++;
}

FrameShift2D ImageStabilazation::getLastFrameCorrectionShift() const
{
    return _correctionShift;
}

FrameShift2D ImageStabilazation::getLastFrameShift() const
{
    return _lastShift;
}

//------------------------------------------------------------------

FloatFilter::FloatFilter(double minValue, double maxValue, double k)
{
    _minValue = minValue;
    _maxValue = maxValue;
    _k = k;
    _sum = 0;
}

double FloatFilter::process(double value)
{
    value = qBound(_minValue, value, _maxValue);
    _sum = _sum * _k + value;
    return _sum;
}
