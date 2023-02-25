#include "ImageCorrector.h"
#include <opencv2\core.hpp>

ImageCorrector::ImageCorrector()
{
    _brightness = 0;
    _contrast = 0;
    _grayscale = false;
}

void contrastFilter(unsigned char *imageData, size_t dataSize, int fixContrast, int fixBrightness) // contrast (256 - normal)
{
    //b g r 255
    unsigned char buf[256];
    quint64 midBright = 0;
    for (size_t i = 0; i < dataSize; i += 4)
        midBright += imageData[i] * 77 + imageData[i + 1] * 150 + imageData[i + 2] * 29;
    midBright /= (256 * dataSize / 4);
    for (int i = 0; i < 256; i++)
    {
        int a = (((i - midBright) * fixContrast) >> 8) + midBright + fixBrightness;
        buf[i] = qBound(0, a, 255);
    }
    for (size_t i = 0; i < dataSize; i += 4)
    {
        imageData[i + 0] = buf[imageData[i + 0]];
        imageData[i + 1] = buf[imageData[i + 1]];
        imageData[i + 2] = buf[imageData[i + 2]];
        imageData[i + 3] = 255;
    }
}


unsigned char AddDoubleToByte(unsigned char bt, double d)
{
    unsigned char result = bt;
    if (double(result) + d > 255)
        result = 255;
    else if (double(result) + d < 0)
        result = 0;
    else
    {
        result += d;
    }
    return result;
}

void fillGammaExpo(unsigned char *lut, int step)
{
    for (int i = 0; i < 256; i++)
        lut[i] = AddDoubleToByte(i, std::sin(i * 0.01255) * step * 10);
}

// https://habr.com/ru/post/268115/
void gammaFilter(unsigned char *imageData, size_t dataSize, float K)
{
    if (imageData == nullptr)
        return;

    uchar lutR[256];
    uchar lutG[256];
    uchar lutB[256];
    fillGammaExpo(lutR, -K * 2);
    fillGammaExpo(lutG, K);
    fillGammaExpo(lutB, K);

    for (size_t i = 0; i < dataSize; i += 4)
    {
        imageData[i + 0] = lutR[imageData[i + 0]];
        imageData[i + 1] = lutG[imageData[i + 1]];
        imageData[i + 2] = lutB[imageData[i + 2]];
        imageData[i + 3] = 255;
    }
}

void convertToGrayscale(unsigned char *imageData, size_t dataSize)
{
    for (size_t i = 0; i < dataSize; i += 4)
    {
        int avgColor = (imageData[i + 0] + imageData[i + 1] + imageData[i + 2]) / 3;
        imageData[i + 0] = avgColor;
        imageData[i + 1] = avgColor;
        imageData[i + 2] = avgColor;
        imageData[i + 3] = 255;
    }
}

QImage ImageCorrector::ProcessFrame(const QImage &frame)
{
    QImage result = frame.convertToFormat(QImage::Format_RGB32);
    Q_ASSERT(result.format() == QImage::Format_RGB32);

    if (_contrast != 0 || _brightness != 0)
    {
        int fixContrast = _contrast * 255 + 255;
        int fixBrightness = 255 * _brightness;
        contrastFilter(result.bits(), result.byteCount(), fixContrast, fixBrightness);
    }

    if (_gamma != 0)
    {
        float fixGamma = _gamma * 10;
        gammaFilter(result.bits(), result.byteCount(), fixGamma);
    }

    if (_grayscale)
        convertToGrayscale(result.bits(), result.byteCount());
    return result;
}

void ImageCorrector::setTuneImageSettings(qreal brightness, qreal contrast, qreal gamma, bool grayscale)
{
    _brightness = brightness;
    _contrast = contrast;
    _gamma = gamma;
    _grayscale = grayscale;
}

void ImageCorrector::getTuneImageSettings(qreal &brightness, qreal &contrast, qreal &gamma, bool &grayscale)
{
    brightness = _brightness;
    contrast = _contrast;
    gamma = _gamma;
    grayscale = _grayscale;
}
