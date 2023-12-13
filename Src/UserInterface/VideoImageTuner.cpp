#include "VideoImageTuner.h"
#include <QDesktopWidget>
#include <QDialogButtonBox>
#include <QStyle>
#include <QComboBox>
#include "ConstantNames.h"
#include "EnterProc.h"

constexpr int SLIDER_MIN_VALUE = 0;
constexpr int SLIDER_MAX_VALUE = 512;
constexpr float SLIDER_MIDDLE_VALUE = (SLIDER_MAX_VALUE - SLIDER_MIN_VALUE) / 2;

//popup windows: https://stackoverflow.com/questions/7421699/close-widget-window-if-mouse-clicked-outside-of-it
//move with mouse: https://stackoverflow.com/questions/11172420/moving-object-with-mouse
VideoImageTuner::VideoImageTuner(QWidget *parent) : QFrame(parent)
{
    EnterProcStart("VideoImageTuner::VideoImageTuner");

    //setAttribute(Qt::WA_DeleteOnClose);
    setWindowFlags(Qt::Popup);
    setFocusPolicy(Qt::ClickFocus);

    _gridLayout = new QGridLayout(this);
    _gridLayout->setHorizontalSpacing(15);
    _gridLayout->setVerticalSpacing(15);

    int row = 0;

    _brightnessSlider = addSlider(":/settings_brightness.png", &VideoImageTuner::onSetDefaultBrightnessClicked, row++);
    _contrastSlider = addSlider(":/settings_contrast.png", &VideoImageTuner::onSetDefaultContrastClicked, row++);
    _gammaSlider = addSlider(":/settings_color.png", &VideoImageTuner::onSetDefaultGammaClicked, row++);
    // _colorSlider = addSlider(":/settings_color.png", row++);

    _chkGrayscale = new QCheckBox(tr("Grayscale Image"), this);
    connect(_chkGrayscale, &QCheckBox::toggled, this, &VideoImageTuner::onGrayscaleChecked);
    _gridLayout->addWidget(_chkGrayscale,        row++, 1, 1, 2);

    _cbColorMode = new QComboBoxExt(this, ConstantNames::ColorModeCaptions());

    connect(_cbColorMode, static_cast<void(QComboBoxExt::*)(int)>(&QComboBoxExt::currentIndexChanged), this, &VideoImageTuner::onColorModeChanged);
    _gridLayout->addWidget(_cbColorMode,        row++, 1, 1, 2);
}

VideoImageTuner::~VideoImageTuner()
{
}

void VideoImageTuner::setTuneImageSettings(qreal brightness, qreal contrast, qreal gamma)
{
    _brightnessSlider->setValue(brightness * SLIDER_MIDDLE_VALUE + SLIDER_MIDDLE_VALUE);
    _contrastSlider->setValue(contrast * SLIDER_MIDDLE_VALUE / 2.0 + SLIDER_MIDDLE_VALUE);
    _gammaSlider->setValue(gamma * SLIDER_MIDDLE_VALUE + SLIDER_MIDDLE_VALUE);
}

void VideoImageTuner::activate()
{
    QWidget *parentWidget = (QWidget *)parent();
    QRect desktopGeometry = CommonWidgetUtils::getDesktopAvailableGeometry();
    QPoint parentLeftBottom = parentWidget->mapToGlobal(QPoint(0, parentWidget->height()));
    adjustSize();
    if (parentLeftBottom.x() + width() > desktopGeometry.right()
            || parentLeftBottom.y() + height() > desktopGeometry.bottom())
    {
        QPoint parentTopRight = parentWidget->mapToGlobal(QPoint(parentWidget->width(), 0));
        parentTopRight -= QPoint(width(), height());
        move(parentTopRight);
    }
    else
        move(parentLeftBottom);
    show();
}

void VideoImageTuner::changeCamMode(int delta)
{
    int modeIdx = _cbColorMode->currentIndex() + delta;
    int count = _cbColorMode->count();

    modeIdx = fmod(modeIdx, count);
    if (modeIdx < 0)
        modeIdx += count;

    _cbColorMode->setCurrentIndex(modeIdx);
}

void VideoImageTuner::processTuneImageChange()
{
    qreal brightness = (_brightnessSlider->value() - SLIDER_MIDDLE_VALUE) / SLIDER_MIDDLE_VALUE;
    qreal contrast = 2.0 * (_contrastSlider->value() - SLIDER_MIDDLE_VALUE) / SLIDER_MIDDLE_VALUE;
    qreal gamma = (_gammaSlider->value() - SLIDER_MIDDLE_VALUE) / SLIDER_MIDDLE_VALUE;

    bool grayscale = _chkGrayscale->isChecked();
    emit tuneImageChange(brightness, contrast, gamma, grayscale);
}

void VideoImageTuner::onSliderValueChanged(int value)
{
    Q_UNUSED(value)
    processTuneImageChange();
}

void VideoImageTuner::onGrayscaleChecked(bool checked)
{
    Q_UNUSED(checked)
    processTuneImageChange();
}

void VideoImageTuner::onColorModeChanged(int index)
{
    Q_UNUSED(index)
    int selectedMode = _cbColorMode->currentData().toInt();
    emit changeColorMode(selectedMode);
}

void VideoImageTuner::onSetDefaultBrightnessClicked()
{
    _brightnessSlider->setValue(SLIDER_MIDDLE_VALUE);
}

void VideoImageTuner::onSetDefaultContrastClicked()
{
    _contrastSlider->setValue(SLIDER_MIDDLE_VALUE);
}

void VideoImageTuner::onSetDefaultGammaClicked()
{
    _gammaSlider->setValue(SLIDER_MIDDLE_VALUE);
}

QSlider *VideoImageTuner::addSlider(const QString &iconName, void (VideoImageTuner::*onIconClick)(), int row)
{
    auto slider = new QSlider(this);
    slider->setOrientation(Qt::Horizontal);
    slider->setRange(SLIDER_MIN_VALUE, SLIDER_MAX_VALUE);
    slider->setValue(SLIDER_MIDDLE_VALUE);
    connect(slider, &QSlider::valueChanged, this, &VideoImageTuner::onSliderValueChanged);

    QPixmap pixmap(iconName);
    auto labelIcon = new QLabelEx(this);
    labelIcon->setObjectName("VideoImageTunerIcon"); //used for stylesheet
    labelIcon->setPixmap(pixmap);
    connect(labelIcon, &QLabelEx::clicked, this, onIconClick);

    _gridLayout->addWidget(labelIcon,     row, 1, 1, 1);
    _gridLayout->addWidget(slider,        row, 2, 1, 1);

    return slider;
}

void VideoImageTuner::focusOutEvent(QFocusEvent *event)
{
    close();
}
