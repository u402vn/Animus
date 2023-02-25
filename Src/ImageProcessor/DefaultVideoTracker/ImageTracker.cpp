#include "ImageTracker.h"
#include "opencv2/opencv.hpp"
#include <QDebug>

using namespace std;
using namespace cv;


ImageTracker::ImageTracker()
{
    _targetSize = 20;
    _targetLocked = false;
}


ImageTracker::~ImageTracker()
{

}

void PlaceRectInsideFrame(cv::Rect &rect, int frameWidth, int frameHeight)
{
    int dx = 0;
    int dy = 0;

    if (rect.x < 0)
        dx = -rect.x;
    else if (rect.x + rect.width >= frameWidth)
        dx = frameWidth - (rect.x + rect.width);

    if (rect.y < 0)
        dy = -rect.y;
    else if (rect.y + rect.height >= frameHeight)
        dy = frameHeight - (rect.y + rect.height);

    rect.x += dx;
    rect.y += dy;
}

QRect ImageTracker::processFrame(const cv::Mat &frame, const FrameShift2D &frameShift)
{
    _frame = frame;

    if (!_targetLocked)
        return QRect(0, 0, 0, 0);


    int searchCenterX = _targetCenter.x() + frameShift.X;
    int searchCenterY = _targetCenter.y() + frameShift.Y;
    int searchWindowSize = qMax(100, _targetSize * 5);
    cv::Rect searchWindowRect = cv::Rect(searchCenterX - searchWindowSize / 2,
                                         searchCenterY - searchWindowSize / 2,
                                         searchWindowSize, searchWindowSize);

    PlaceRectInsideFrame(searchWindowRect, _frame.size().width, _frame.size().height);

    cv::Mat searchWindow = _frame(searchWindowRect);
    Mat matchResult;
    double minVal, maxVal;
    Point minLoc, maxLoc;

    cv::matchTemplate(searchWindow, _targetImage, matchResult, CV_TM_CCOEFF_NORMED);
    minMaxLoc( matchResult, &minVal, &maxVal, &minLoc, &maxLoc);


    if (maxVal < 0.5)
        return QRect(0, 0, 0, 0);

    if (maxVal > 0.9)
    {
        cv::Mat currentObject = searchWindow(cv::Rect(maxLoc.x, maxLoc.y, _targetSize, _targetSize));
        cv::addWeighted(_targetImage, 0.9, currentObject, 0.1, 0, _targetImage);
    }

    _targetCenter.setX(searchWindowRect.x + maxLoc.x + _targetSize / 2);
    _targetCenter.setY(searchWindowRect.y + maxLoc.y + _targetSize / 2);

    QRect result(_targetCenter.x() -_targetSize / 2,
                 _targetCenter.y() -_targetSize / 2,
                 _targetSize, _targetSize);

    return result;
/*
    cv::imwrite("E:/Projects/Animus/testdata/frame.jpg", _frame);
    cv::imwrite("E:/Projects/Animus/testdata/searchWindow.jpg", searchWindow);
    cv::imwrite("E:/Projects/Animus/testdata/targetImage.jpg", _targetImage);
    cv::imwrite("E:/Projects/Animus/testdata/matchResult.jpg", matchResult);
*/
}

void ImageTracker::lockTarget(const QPoint &targetCenter)
{
    cv::Rect targetRect(targetCenter.x() -_targetSize / 2,
                        targetCenter.y() -_targetSize / 2,
                        _targetSize, _targetSize);

    PlaceRectInsideFrame(targetRect, _frame.size().width, _frame.size().height);

    _targetCenter = targetCenter;
   _targetImage = _frame(targetRect);
   _targetLocked = true;
}

void ImageTracker::unlockTarget()
{
    _targetLocked = false;
}

void ImageTracker::setTargetSize(int size)
{
    _targetSize = size;
    if (_targetLocked)
        lockTarget(_targetCenter);
    //todo re-lock target with new size
}
