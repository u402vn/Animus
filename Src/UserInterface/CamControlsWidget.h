#ifndef CAMCONTROLSWIDGET_H
#define CAMCONTROLSWIDGET_H

#include <QWidget>
#include <QPushButton>
#include <QSpinBox>
#include <QMenu>
#include <QAction>
#include <QActionGroup>
#include <QContextMenuEvent>
#include <QGridLayout>
#include <QDoubleSpinBox>
#include <QSlider>
#include <QLabel>
#include <QButtonGroup>
#include <VoiceInformant/VoiceInformant.h>
#include "Common/CommonWidgets.h"
#include "Common/CommonData.h"
#include "HardwareLink/HardwareLink.h"
#include "VideoImageTuner.h"
#include "GPSCoordIndicator.h"
#include "ApplicationSettings.h"
#include "CamControlsWidgetDials.h"
#include "CamControlsWidgetKnob.h"
#include "AutomaticTracer.h"

class CamControlsWidget final : public QWidget
{
private:
    Q_OBJECT

    HardwareLink * _hardwareLink;

    CameraControlModes _camControlMode;
    quint32 _opticalSystemId;
    bool _camAxisXInverse, _camAxisYInverse;
    bool _useZoomScaleForManualMoving;
    bool _targetLocked;

    VideoImageTuner *_imageTuner;
    AutomaticTracer *_automaticTracer;
    VoiceInformant *_voiceInformant;
    CamAssemblyPreferences *_camAssemblyPreferences;

    QGridLayout *_mainLayout;

    GPSCoordIndicator *_coordIndicator;

    QPushButton *_btnCamDriversOff, *_btnCamLandingPos, *_btnCamBeginingPos, *_btnCamVerticalPos;

    CamControlsWidgetDials *_camDials;
    CamControlsWidgetKnob *_camKnob;
    QSlider *_camZoom;
    QLabel *_camZoomIndicator;

    QPushButton *_btnLaserActivation;

    QActionGroup *_targetSizeGroup;
    QMenu *_targetSizeMenu;
    QPushButton *_btnTargetUnlock;
    QPushButton *_btnCamRecording;
    QPushButton *_btnAutomaticTracer;

    QPushButton *_btnEnableStabilization;

    QPushButton *_btnLiveViewSettings;
    QButtonGroup *_grpCamButtons;

    QDoubleSpinBox *_sbSnapshotSeriesInterval;
    QPushButton *_btnSnapshotSeries;

    QPushButton *createButton(const QString &toolTip, bool checkable, const QString &iconName, void(CamControlsWidget::*onClickMethod)());

    void createTrackingButtons();
    void createFixedPositionsButtons();
    void createSnapshotButtons();
    void createCamMoveControls();
    void createCamZoomControls();
    void createCamViewControls();
    void createCoordIndicator();

    void initWidgets();

    void sendCamZoom(float zoom);
    void setCamControlsEnabled(bool enabled);

    void processCamMovingSpeedChange(float speedRoll, float speedPitch, float speedYaw, bool useManualSettings);
public:
    explicit CamControlsWidget(QWidget *parent, HardwareLink *hardwareLink,
                               AutomaticTracer *automaticTracer,
                               VoiceInformant *voiceInformant);
    int selectedTargetSize();

    void processTelemetry(const TelemetryDataFrame &telemetryFrame);
signals:
    void unlockTarget();
    void setTargetSize(int targetSize);
    void enableSoftwareStabilization(bool enable);
    void makeScreenshot();
    void tuneImageChange(qreal brightness, qreal contrast, qreal gamma, bool grayscale);
    void doSetZoomFromUI(quint32 zoom);
public slots:
    void onAbsoluteCamPositionChange(float roll, float pitch, float yaw);
    void onAbsoluteCamZoomChange(float zoom);

    void onRelativeCamPositionChange(float deltaRoll, float deltaPitch, float deltaYaw);

    void onManualCamMovingSpeedChange(float speedRoll, float speedPitch, float speedYaw);

    void onChangeActiveCamClicked();
    void onEnableSoftwareStabilizationClicked();

    void onCamDriversOffClicked();
    void onCamLandingPosClicked();
    void onCamBeginingPosClicked();
    void onCamVerticalPosClicked();

    void onColorModeUpClicked();
    void onColorModeDownClicked();
    void onLaserActivationClicked();

    void onScreenshotClicked();
    void onSnapshotClicked();
    void onSnapshotSeriesClicked();

    void onTargetUnlockClicked();
    void onCamRecordingClicked();
    void onEnableAutomaticTracerClicked();
private slots:
    void onLiveViewSettingsClick();
    void tuneImageChangeInternal(qreal brightness, qreal contrast, qreal gamma, bool grayscale);
    void onTargetSizeSelectorClick();
    void onTargetSizeActionTriggered();
    void onEnableStabilizationClick_Internal();

    void onLaserActivationClick_Internal();

    void onCamZoomValueChanged(int value);

    void onAutomaticCamMovingSpeedChange(float speedRoll, float speedPitch, float speedYaw);

    void onAutomaticTracerClicked_Internal();
    void onCamRecordingClicked_Internal();

    void onActiveCamClicked(int id);
    void onChangeColorMode(int colorMode);

    void onTracerModeChanged(const AutomaticTracerMode tracerMode);
};

#endif // CAMCONTROLSWIDGET_H
