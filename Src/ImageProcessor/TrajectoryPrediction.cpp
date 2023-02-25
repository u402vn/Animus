#include "TrajectoryPrediction.h"

void TrajectoryPrediction::UpdateKnownCoordinates(int x, int y)
{
    cv::Point2d point(x, y);

    if (KnownCoordinates.size() < _size)
        KnownCoordinates.enqueue(point);
    else
    {
        KnownCoordinates.dequeue();
        KnownCoordinates.enqueue(point);
    }
}

cv::Point2d TrajectoryPrediction::GetFilteredResult(int currTargetX, int currTargetY)
{
    if (KnownCoordinates.size() == _size)
    {
        double k = 0.7;
        double xPrev = KnownCoordinates.back().x;
        double yPrev = KnownCoordinates.back().y;

        double xFiltered = k * (double)currTargetX + (1 - k) * xPrev;
        double yFiltered = k * (double)currTargetY + (1 - k) * yPrev;

        /*double xFiltered = k * (double)currTargetX + (1 - k) * (xPrev + dx1);
        double yFiltered = k * (double)currTargetY + (1 - k) * (yPrev + dy1);*/

        /*
        if (fabs(shiftX) > 20 || fabs(shiftY) > 20)
            return 0;
        */

        return cv::Point2d(xFiltered, yFiltered);
    }

    return cv::Point2d(currTargetX, currTargetY);
}

void TrajectoryPrediction::Clear()
{
    while (!KnownCoordinates.empty())
    {
        KnownCoordinates.dequeue();
    }
}
