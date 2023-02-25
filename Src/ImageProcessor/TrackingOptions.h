#pragma once

struct TrackingOptions final
{
    int TargetSize = 20;
    int TargetSearchAreaSize = 100;
    int CountStepsForUpdateTarget = 50; //100
    int ThresholdCountFrameNoFindTarget = 15;//5;

    double ThresholdResponse = 0.2;
    double ThresholdCountPeaks = 1;

    double ThresholdNoise = 0;

    double CoefficientIncreaseTarget = 1.1;
    double CoefficientDecreaseTarget = 0.9;
    int ThresholdCountFramesForLargelerTarget = 5;
    int ThresholdCountFramesForSmallerTarget = 6;
};
