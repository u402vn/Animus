#include <QImage>
#include "opencv2/core/core.hpp"
#include "opencv2/imgproc.hpp"

inline cv::Mat QImage2MatGS(QImage const& srcImage)
{
    QImage::Format imageFormat = srcImage.format();
    Q_ASSERT(imageFormat == QImage::Format_RGB32);
    cv::Mat tempMat(srcImage.height(), srcImage.width(), CV_8UC4, (uchar*)srcImage.bits(), srcImage.bytesPerLine());
    cv::Mat resultGS;
    cv::cvtColor(tempMat, resultGS, CV_RGBA2GRAY);
    return resultGS;
}
