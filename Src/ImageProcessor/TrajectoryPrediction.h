#pragma once

#include "opencv2/core/core.hpp"
#include "opencv2/highgui.hpp"
#include <QQueue>

class TrajectoryPrediction final
{
private:
    QQueue<cv::Point2d> KnownCoordinates;

    int *offsetX;
    int *offsetY;

    int _size = 2;
public:
    void UpdateKnownCoordinates(int x, int y);
    void Clear();
    cv::Point2d GetFilteredResult(int _targetX, int _targetY);
};

