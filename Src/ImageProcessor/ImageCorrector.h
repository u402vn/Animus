#ifndef IMAGECORRECTOR_H
#define IMAGECORRECTOR_H

#include <QImage>

class ImageCorrector final
{
    bool _grayscale;
    qreal _brightness; // -1 ... 0 ... +1
    qreal _contrast;   // -1 ... 0 ... +1
    qreal _gamma;   // -1 ... 0 ... +1
public:
    ImageCorrector();
    QImage ProcessFrame(const QImage &frame);
    void setTuneImageSettings(qreal brightness, qreal contrast, qreal gamma, bool grayscale);
    void getTuneImageSettings(qreal &brightness, qreal &contrast, qreal &gamma, bool &grayscale);
};

#endif // IMAGECORRECTOR_H
