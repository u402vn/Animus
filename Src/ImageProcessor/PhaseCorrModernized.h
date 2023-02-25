#pragma once

#include "opencv2/core/core.hpp"
#include "opencv2/highgui.hpp"

#define CV_PI2   3.1415926535897932384626433832795


//using namespace std;
//using namespace cv;

struct ResultPhaseCorrelation final
{
    cv::Point2d Shift;
	double Response;
	double Max;
	int CountPeaks;
};

constexpr int BORDER_CONSTANT2 = 0;
constexpr int BORDER_REPLICATE2 = 1;
constexpr int BORDER_REFLECT2 = 2;
constexpr int BORDER_WRAP2 = 3;
constexpr int BORDER_REFLECT_101_2 = 4;
constexpr int BORDER_TRANSPARENT2 = 5;
constexpr int BORDER_ISOLATED2 = 16;
constexpr double EPSILON2 = 2.2204460492503131e-016;

constexpr int DFT_COMPLEX_INPUT_OR_OUTPUT = 512;
constexpr int DFT_NO_PERMUTE = 256;

ResultPhaseCorrelation phaseCorrelate2(const cv::InputArray &_src1, const cv::InputArray &_src2, const cv::InputArray &_window);

void createHanningWindow2(const cv::OutputArray &_dst, const cv::Size &winSize, int type);

void idft2(const cv::InputArray &src, const cv::OutputArray &dst, int flags = 0, int nonzeroRows = 0);
//void multiply2(cv::Mat src1, cv::Mat src2, cv::Mat dst);
