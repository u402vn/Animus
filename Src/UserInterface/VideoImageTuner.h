#ifndef VIDEOIMAGETUNER_H
#define VIDEOIMAGETUNER_H

#include <QWidget>
#include <QGridLayout>
#include <QSlider>
#include <QFrame>
#include <QCheckBox>
#include "Common/CommonWidgets.h"

class VideoImageTuner final : public QFrame
{
    Q_OBJECT
    QGridLayout * _gridLayout;
    QSlider * _brightnessSlider;
    QSlider * _contrastSlider;
    QSlider * _gammaSlider;
    QCheckBox * _chkGrayscale;
    QComboBoxExt *_cbColorMode;

    QSlider *addSlider(const QString &iconName, void (VideoImageTuner::*)(), int row);

    void processTuneImageChange();
protected:
    void focusOutEvent(QFocusEvent * event);
public:
    explicit VideoImageTuner(QWidget *parent);
    ~VideoImageTuner();

    void setTuneImageSettings(qreal brightness, qreal contrast, qreal gamma);
    void activate();
    void changeCamMode(int delta);
private slots:
    void onSliderValueChanged(int value);
    void onSetDefaultBrightnessClicked();
    void onSetDefaultContrastClicked();
    void onSetDefaultGammaClicked();
    void onGrayscaleChecked(bool checked);
    void onColorModeChanged(int index);
signals:
    void tuneImageChange(qreal brightness, qreal contrast, qreal gamma, bool grayscale);
    void changeColorMode(int colorMode);
};

#endif // VIDEOIMAGETUNER_H
