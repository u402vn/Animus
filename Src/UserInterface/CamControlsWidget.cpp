#include "CamControlsWidget.h"
#include <QPainter>
#include <QSlider>
#include <QFrame>
#include <QKeyEvent>
#include <QWindow>
#include <QVector>
#include "ConstantNames.h"
#include "EnterProc.h"

void CamControlsWidget::createTrackingButtons()
{
    //create target size menu items
    _targetSizeMenu = new QMenu(tr("Target Size"), this);
    _targetSizeGroup = new QActionGroup(this);

    QVector<int> fixedTargetSizes({10, 20, 30, 40, 50});
    QVector<int> shortCuts({Qt::Key_1, Qt::Key_2, Qt::Key_3, Qt::Key_4, Qt::Key_5});

    int currentTargetSize = 40; //todo get actual value from tracker

    for (int i = 0; i < fixedTargetSizes.count(); i++)
    {
        int size = fixedTargetSizes[i];
        QString caption = tr("Size %1x%1").arg(size);
        bool isChecked = size == currentTargetSize;
        auto acSizeItem = CommonWidgetUtils::createCheckableMenuGroupAction(caption, isChecked, _targetSizeGroup, _targetSizeMenu, size);
        connect(acSizeItem, &QAction::triggered, this, &CamControlsWidget::onTargetSizeActionTriggered, Qt::DirectConnection);

        int key = shortCuts[i];
        QKeySequence shortCut(key);
        acSizeItem->setShortcut(shortCut);

        //acSizeItem->setShortcutContext(Qt::ApplicationShortcut);

        this->addAction(acSizeItem);
    }

    //create buttons
    ApplicationSettings& applicationSettings = ApplicationSettings::Instance();
    auto cameraSettings = currCameraSettings();

    auto btnTargetSizeSelector = createButton(tr("Target Size"), false, ":/targetsizeselector.png", &CamControlsWidget::onTargetSizeSelectorClick, &CamControlsWidget::onTargetSizeSelectorClick);
    _btnTargetUnlock = createButton(applicationSettings.hidUIHint(hidbtnTargetUnlock), false, ":/targetunlock.png", &CamControlsWidget::onTargetUnlockClicked);
    _btnTargetUnlock->setEnabled(false);
    _btnCamRecording = createButton(applicationSettings.hidUIHint(hidbtnCamRecording), true, ":/camrecord.png", &CamControlsWidget::onCamRecordingClicked_Internal);
    if (!cameraSettings->IsOnboardRecording || !applicationSettings.isPhotographyLicensed())
        _btnCamRecording->setEnabled(false);
    _btnAutomaticTracer = createButton(applicationSettings.hidUIHint(hidbtnAutomaticTracer), true, ":/automatictracer.png", &CamControlsWidget::onAutomaticTracerClicked_Internal);

    //append buttons to grid
    int row = _mainLayout->rowCount();
    _mainLayout->addWidget(btnTargetSizeSelector,         row, 1, 1, 1);
    _mainLayout->addWidget(_btnTargetUnlock,              row, 2, 1, 1); // , Qt::AlignRight
    _mainLayout->addWidget(_btnAutomaticTracer,           row, 3, 1, 1);
    _mainLayout->addWidget(_btnCamRecording,              row, 4, 1, 1);

    if (!applicationSettings.MinimalisticDesign)
    {
        row++;
        auto trackingSeparator = CommonWidgetUtils::createSeparator(this);
        _mainLayout->addWidget(trackingSeparator,              row, 1, 1, 4);
        _mainLayout->setRowMinimumHeight(row, 0.5 * DEFAULT_BUTTON_HEIGHT);
    }
}

void CamControlsWidget::createFixedPositionsButtons()
{
    ApplicationSettings& applicationSettings = ApplicationSettings::Instance();

    //create buttons
    _btnCamDriversOff = createButton(applicationSettings.hidUIHint(hidbtnDriversOff), true, ":/camdriversoff.png", &CamControlsWidget::onCamDriversOffClicked);
    _btnCamDriversOff->setChecked(true);
    _btnCamLandingPos = createButton(applicationSettings.hidUIHint(hidbtnFixedPosLanding), false, ":/camalignlanding.png", &CamControlsWidget::onCamLandingPosClicked);
    _btnCamBeginingPos = createButton(applicationSettings.hidUIHint(hidbtnFixedPosBegining), false, ":/camalignbegin.png", &CamControlsWidget::onCamBeginingPosClicked);
    _btnCamVerticalPos = createButton(applicationSettings.hidUIHint(hidbtnFixedPosVertical), false, ":/camalignvertical.png", &CamControlsWidget::onCamVerticalPosClicked);

    //append buttons to grid
    int row = _mainLayout->rowCount();
    _mainLayout->addWidget(_btnCamDriversOff,    row, 1, 1, 1);
    _mainLayout->addWidget(_btnCamLandingPos,    row, 2, 1, 1);
    _mainLayout->addWidget(_btnCamBeginingPos,   row, 3, 1, 1);
    _mainLayout->addWidget(_btnCamVerticalPos,   row, 4, 1, 1);
    _mainLayout->setRowStretch(row, 0);

    if (currCameraSettings()->CameraSuspensionType == CameraSuspensionTypes::FixedCamera)
    {
        _btnCamDriversOff->setVisible(false);
        _btnCamLandingPos->setVisible(false);
        _btnCamBeginingPos->setVisible(false);
        _btnCamVerticalPos->setVisible(false);
    }
}

void CamControlsWidget::createSnapshotButtons()
{
    ApplicationSettings& applicationSettings = ApplicationSettings::Instance();

    int row = _mainLayout->rowCount();
    _mainLayout->setRowStretch(row, 1);

    if (!applicationSettings.isPhotographyLicensed())
    {
        _sbSnapshotSeriesInterval = nullptr;
        _btnSnapshotSeries = nullptr;
        return;
    }

    //create buttons
    auto btnScreenshot = createButton(applicationSettings.hidUIHint(hidbtnScreenshot), false, ":/screenshot.png", &CamControlsWidget::onScreenshotClicked);
    auto btnSnapshot = createButton(applicationSettings.hidUIHint(hidbtnSnapshot), false, ":/snapshot.png", &CamControlsWidget::onSnapshotClicked);

    _btnSnapshotSeries = createButton(applicationSettings.hidUIHint(hidbtnSnapshotSeries), false, ":/snapshotseries.png", &CamControlsWidget::onSnapshotSeriesClicked);
    _sbSnapshotSeriesInterval = new QDoubleSpinBox(this);
    _sbSnapshotSeriesInterval->setMaximumWidth(QUARTER_BUTTON_WIDTH);
    _sbSnapshotSeriesInterval->setAlignment(Qt::AlignCenter);
    _sbSnapshotSeriesInterval->setRange(1, 100);
    _sbSnapshotSeriesInterval->setDecimals(1);
    _sbSnapshotSeriesInterval->setSingleStep(0.2);
    _sbSnapshotSeriesInterval->setMinimumHeight(_btnSnapshotSeries->minimumHeight());

    //append buttons to grid
    if (!currCameraSettings()->IsSnapshot)
    {
        btnSnapshot->setEnabled(false);
        _btnSnapshotSeries->setEnabled(false);
        _sbSnapshotSeriesInterval->setEnabled(false);
    }

    row++;
    _mainLayout->addWidget(btnScreenshot,       row, 1, 1, 1);
    _mainLayout->addWidget(btnSnapshot,         row, 2, 1, 1);
    _mainLayout->addWidget(_sbSnapshotSeriesInterval,   row, 3, 1, 1, Qt::AlignLeft);
    _mainLayout->addWidget(_btnSnapshotSeries,          row, 4, 1, 1, Qt::AlignLeft);
    _mainLayout->setRowStretch(row, 0);
}

void CamControlsWidget::createCamMoveControls()
{
    int row = _mainLayout->rowCount();

    CameraControlModes controlMode = currCameraSettings()->CameraControlMode;

    if (controlMode == CameraControlModes::AbsolutePosition)
    {
        _camKnob = nullptr;
        _camDials = new CamControlsWidgetDials(this);
        connect(_camDials, &CamControlsWidgetDials::setCamPosition, _hardwareLink, &HardwareLink::setCamPosition, Qt::DirectConnection);
        _mainLayout->addWidget(_camDials,        row, 1, 1, 4, Qt::AlignCenter);
        _mainLayout->setRowStretch(row, 0);
    }
    else
    {
        _camDials = nullptr;
        _camKnob = new CamControlsWidgetKnob(this);
        connect(_camKnob, &CamControlsWidgetKnob::onCamMovingSpeedChange, this, &CamControlsWidget::onManualCamMovingSpeedChange, Qt::DirectConnection);
        _mainLayout->addWidget(_camKnob,        row, 1, 1, 4, Qt::AlignCenter);
        _mainLayout->setRowStretch(row, 0);
    }
}

void CamControlsWidget::createCamZoomControls()
{
    ApplicationSettings& applicationSettings = ApplicationSettings::Instance();
    auto cameraSettings = currCameraSettings();

    auto hint = QString("%1\n%2").arg(applicationSettings.hidUIHint(hidbtnCamZoomOut)).arg(applicationSettings.hidUIHint(hidbtnCamZoomIn));
    //create controls
    _camZoom = new QSlider(this);
    _camZoom->setRange(cameraSettings->opticalDeviceSetting(1)->CamZoomMin->value(), cameraSettings->opticalDeviceSetting(1)->CamZoomMax->value()); //???todo
    _camZoom->setTickPosition(QSlider::TicksBothSides);
    _camZoom->setTickInterval(1);
    _camZoom->setPageStep(1);
    _camZoom->setOrientation(Qt::Horizontal);
    connect(_camZoom, &QSlider::valueChanged, this, &CamControlsWidget::onCamZoomValueChanged, Qt::DirectConnection);
    _camZoomIndicator = new QLabel(this);

    _btnLaserActivation = createButton(applicationSettings.hidUIHint(hidbtnLaserActivation), true, ":/laseractivate.png", &CamControlsWidget::onLaserActivationClick_Internal);

    //append controls to grid
    int row = _mainLayout->rowCount();
    _mainLayout->addWidget(_camZoom,                     row, 1, 1, 2);
    _mainLayout->addWidget(_camZoomIndicator,            row, 3, 1, 1, Qt::AlignCenter);
    _mainLayout->addWidget(_btnLaserActivation,          row, 4, 1, 1, Qt::AlignCenter);

    _mainLayout->setRowStretch(row, 0);

    //init controls
    if ((cameraSettings->CameraSuspensionType == CameraSuspensionTypes::FixedCamera))
    {
        _camZoom->setValue(cameraSettings->opticalDeviceSetting(1)->FixedPosBeginingZoom->value()); //???todo
        _camZoom->setVisible(false);
        _camZoomIndicator->setVisible(false);
    }
    else
    {
        _camZoom->setValue(cameraSettings->opticalDeviceSetting(1)->FixedPosBeginingZoom->value()); //???todo
        _camZoom->setToolTip(hint);
        _camZoomIndicator->setToolTip(hint);
    }
}

void CamControlsWidget::createCamViewControls()
{
    ApplicationSettings& applicationSettings = ApplicationSettings::Instance();

    //create buttons
    _btnEnableStabilization = createButton(applicationSettings.hidUIHint(hidbtnEnableSoftwareStab), true, ":/stabilization.png",
                                           &CamControlsWidget::onEnableStabilizationClick_Internal, &CamControlsWidget::onEnableStabilizationMenuClick);
    _btnEnableStabilization->setChecked(applicationSettings.SoftwareStabilizationEnabled);
    onEnableStabilizationClick_Internal();

    //_btnEnableStabilization->setVisible(false); //???todo place on the form in nice position


    //_btnLiveViewSettings = createButton(tr("Live View Settings"), false, ":/liveviewsettings.png",
    //                                    &CamControlsWidget::onLiveViewSettingsClick, &CamControlsWidget::onLiveViewSettingsClick);

    _stabilizationTypeMenu = new QMenu(tr("Stabilization Type"), this);
    _stabilizationTypeGroup = new QActionGroup(this);

    auto stabilizationTypeCaptions = ConstantNames::StabilizationTypeCaptions();
    auto i = stabilizationTypeCaptions.begin();
    while (i != stabilizationTypeCaptions.end())
    {
        CommonWidgetUtils::createCheckableMenuGroupAction(i.value(), applicationSettings.VideoStabilizationType == i.key(),
                                                          _stabilizationTypeGroup, _stabilizationTypeMenu, i.key());
        ++i;
    }

    auto btnCam1 = createButton(tr("Camera 1"), true, ":/camera1.png", nullptr, onLiveViewSettingsClick);
    auto btnCam2 = createButton(tr("Camera 2"), true, ":/camera2.png", nullptr, onLiveViewSettingsClick);
    auto btnCam3 = createButton(tr("Camera 3"), true, ":/camera2.png", nullptr, onLiveViewSettingsClick);
    btnCam1->setChecked(true);

    _grpCamButtons = new QButtonGroup(this);
    _grpCamButtons->addButton(btnCam1, OPTYCAL_SYSTEM_1);
    _grpCamButtons->addButton(btnCam2, OPTYCAL_SYSTEM_2);
    _grpCamButtons->addButton(btnCam3, OPTYCAL_SYSTEM_3);
    connect(_grpCamButtons, static_cast<void (QButtonGroup::*)(int)>(&QButtonGroup::buttonClicked), this, &CamControlsWidget::onActiveOpticalSystemClicked, Qt::DirectConnection);

    _imageTuner = new VideoImageTuner(btnCam1);
    _imageTuner->resize(DEFAULT_BUTTON_WIDTH * 0.75, _imageTuner->height());
    connect(_imageTuner, &VideoImageTuner::tuneImageChange, this, &CamControlsWidget::tuneImageChangeInternal, Qt::DirectConnection);
    connect(_imageTuner, &VideoImageTuner::changeColorMode, this, &CamControlsWidget::onChangeColorMode, Qt::DirectConnection);

    //append buttons to grid
    int row = _mainLayout->rowCount();
    /*
    _mainLayout->addWidget(_btnEnableStabilization,    row, 1, 1, 1, Qt::AlignLeft);
    _mainLayout->addWidget(_btnLiveViewSettings,       row, 2, 1, 1, Qt::AlignLeft);
    _mainLayout->addWidget(btnCam1,                    row, 3, 1, 1, Qt::AlignLeft);
    _mainLayout->addWidget(btnCam2,                    row, 4, 1, 1, Qt::AlignLeft);
    */
    _mainLayout->addWidget(_btnEnableStabilization,    row, 1, 1, 1, Qt::AlignLeft);
    _mainLayout->addWidget(btnCam1,                    row, 2, 1, 1, Qt::AlignLeft);
    _mainLayout->addWidget(btnCam2,                    row, 3, 1, 1, Qt::AlignLeft);
    _mainLayout->addWidget(btnCam3,                    row, 4, 1, 1, Qt::AlignLeft);
    _mainLayout->setRowStretch(row, 0);
}

void CamControlsWidget::createCoordIndicator()
{
    ApplicationSettings& applicationSettings = ApplicationSettings::Instance();

    int row = _mainLayout->rowCount();
    if (applicationSettings.CamControlsTabCoordIndicatorAllowed)
    {
        _coordIndicator = new GPSCoordIndicator(this);
        _mainLayout->addWidget(_coordIndicator,                row, 1, 1, 4);
        row++;
    }
    else
        _coordIndicator = nullptr;
}

void CamControlsWidget::onAbsoluteCamPositionChange(float roll, float pitch, float yaw)
{
    EnterProcStart("CamControlsWidget::onAbsoluteCamPositionChange");
    if (_camDials != nullptr)
        _camDials->changeAbsoluteCamPosition(roll, pitch, yaw);
}

void CamControlsWidget::onAbsoluteCamZoomChange(float zoom)
{
    EnterProcStart("CamControlsWidget::onAbsoluteCamZoomChange");
    if (_camZoom->isEnabled())
        _camZoom->setValue(zoom);
    sendCamZoom(_camZoom->value());
}

void CamControlsWidget::onRelativeCamPositionChange(float deltaRoll, float deltaPitch, float deltaYaw)
{
    if (_camDials != nullptr)
        _camDials->changeRelativeCamPosition(deltaRoll, deltaPitch, deltaYaw);
}

void CamControlsWidget::processCamMovingSpeedChange(float speedRoll, float speedPitch, float speedYaw, bool useManualSettings)
{
    if (useManualSettings)
    {
        if (_camAxisXInverse)
            speedYaw = -speedYaw;
        if (_camAxisYInverse)
            speedPitch = -speedPitch;
        if (_useZoomScaleForManualMoving)
        {
            // https://planetcalc.ru/5992/
            //qreal v = 0.207665288L - 0.160719196L * _camZoom->value();
            //qreal zoomScale = qExp(v);
            qreal zoomScale = _camAssemblyPreferences->opticalDevice(_opticalSystemId)->manualSpeedMultipliers(_camZoom->value());
            speedRoll = speedRoll * zoomScale;
            speedPitch = speedPitch * zoomScale;
            speedYaw = speedYaw * zoomScale;
        }
    }

    _hardwareLink->setCamSpeed(speedRoll, speedPitch, speedYaw);
}

void CamControlsWidget::onManualCamMovingSpeedChange(float speedRoll, float speedPitch, float speedYaw)
{
    _automaticTracer->sleep();
    processCamMovingSpeedChange(speedRoll, speedPitch, speedYaw, true);
}

void CamControlsWidget::onChangeActiveCamClicked()
{
    auto checkedButton = _grpCamButtons->checkedButton();
    auto buttons = _grpCamButtons->buttons();
    int i = buttons.indexOf(checkedButton);
    if (i < buttons.count() - 1)
        i++;
    else
        i = 0;
    buttons[i]->click();
}

void CamControlsWidget::onEnableSoftwareStabilizationClicked()
{
    EnterProcStart("CamControlsWidget::onEnableSoftwareStabilizationClicked");
    _btnEnableStabilization->click();
}

void CamControlsWidget::onAutomaticCamMovingSpeedChange(float speedRoll, float speedPitch, float speedYaw)
{
    processCamMovingSpeedChange(speedRoll, speedPitch, speedYaw, false);
}

CamControlsWidget::CamControlsWidget(QWidget *parent, HardwareLink *hardwareLink,
                                     AutomaticTracer *automaticTracer, VoiceInformant *voiceInformant) : QWidget(parent),
    _hardwareLink(hardwareLink),
    _automaticTracer(automaticTracer),
    _voiceInformant(voiceInformant)
{
    EnterProcStart("CamControlsWidget::CamControlsWidget");

    ApplicationSettings& applicationSettings = ApplicationSettings::Instance();
    auto cameraSettings = applicationSettings.installedCameraSettings();
    _camAssemblyPreferences = applicationSettings.getCurrentCamAssemblyPreferences();
    _opticalSystemId = OPTYCAL_SYSTEM_1;

    connect(_automaticTracer, &AutomaticTracer::onAutomaticCamMovingSpeedChange, this, &CamControlsWidget::onAutomaticCamMovingSpeedChange);
    connect(_automaticTracer, &AutomaticTracer::onTracerModeChanged, this, &CamControlsWidget::onTracerModeChanged);

    _camControlMode = cameraSettings->CameraControlMode;
    _camAxisXInverse = cameraSettings->CamAxisXInverse;
    _camAxisYInverse = cameraSettings->CamAxisYInverse;
    _useZoomScaleForManualMoving = applicationSettings.UseZoomScaleForManualMoving;

    initWidgets();
}

int CamControlsWidget::selectedTargetSize()
{
    auto actions = _targetSizeGroup->actions();

    foreach (auto targetSizeAction, actions)
    {
        if (targetSizeAction->isChecked())
        {
            int targetSize = targetSizeAction->data().toInt();
            return targetSize;
        }
    }
    return 20;
}

void CamControlsWidget::processTelemetry(const TelemetryDataFrame &telemetryFrame)
{    
    EnterProcStart("CamControlsWidget::processTelemetry");

    _opticalSystemId = telemetryFrame.OpticalSystemId;

    _automaticTracer->processTelemetry(telemetryFrame, telemetryFrame.targetRect().center(), telemetryFrame.targetIsVisible());
    if (_coordIndicator != nullptr)
        _coordIndicator->processTelemetry(telemetryFrame);

    _camZoomIndicator->setText(QString("%1 x").arg(telemetryFrame.CamZoom));

    if (_camDials != nullptr)
        _camDials->showTelemetryDataFrame(telemetryFrame);

    if (telemetryFrame.TelemetryFrameNumber == 1)
    {
        _targetLocked = telemetryFrame.targetIsVisible();
        _btnTargetUnlock->setEnabled(_targetLocked);
        _camZoom->setValue(telemetryFrame.CamZoom);
    }

    bool targetLockedNow = telemetryFrame.targetIsVisible();
    if (_targetLocked != targetLockedNow)
    {
        _targetLocked = targetLockedNow;
        if (_btnTargetUnlock->isEnabled() != _targetLocked)
        {
            _btnTargetUnlock->setEnabled(_targetLocked);
            _voiceInformant->sayMessage(_targetLocked ? VoiceMessage::TargetLocked : VoiceMessage::TargetDropped);
        }
    }
}

QPushButtonEx *CamControlsWidget::createButton(const QString &toolTip, bool checkable, const QString &iconName,
                                               void(CamControlsWidget::*onClickMethod)(), void(CamControlsWidget::*onRightClick)())
{
    auto button = CommonWidgetUtils::createButton(this, NO_CAPTION, toolTip, checkable, QUARTER_BUTTON_WIDTH, DEFAULT_BUTTON_HEIGHT, iconName);
    if (onClickMethod != nullptr)
        connect(button, &QPushButtonEx::clicked, this, onClickMethod, Qt::DirectConnection);
    if (onRightClick != nullptr)
        connect(button, &QPushButtonEx::onRightClick, this, onRightClick, Qt::DirectConnection);
    return button;
}

void CamControlsWidget::initWidgets()
{
    EnterProcStart("CamControlsWidget::initWidgets");

    _mainLayout = new QGridLayout(this);
    _mainLayout->setContentsMargins(0, 0, 0, 0);
    _mainLayout->setColumnStretch(0, 1);
    _mainLayout->setColumnStretch(1, 0);
    _mainLayout->setColumnStretch(2, 0);
    _mainLayout->setColumnStretch(3, 0);
    _mainLayout->setColumnStretch(4, 0);
    _mainLayout->setColumnStretch(5, 1);

    createTrackingButtons();
    createCoordIndicator();
    createFixedPositionsButtons();
    createCamMoveControls();
    createCamZoomControls();
    createCamViewControls();
    createSnapshotButtons();

    setCamControlsEnabled(true);
}

void CamControlsWidget::onLiveViewSettingsClick()
{
    auto btnCamN = qobject_cast<QPushButtonEx *>(sender());
    auto buttonId = _grpCamButtons->id(btnCamN);
    _imageTuner->activate(buttonId);
}

void CamControlsWidget::tuneImageChangeInternal(qreal brightness, qreal contrast, qreal gamma, bool grayscale)
{
    emit tuneImageChange(brightness, contrast, gamma, grayscale);
}

void CamControlsWidget::onTargetSizeSelectorClick()
{
    auto btnTargetSizeSelector = qobject_cast<QPushButtonEx *>(sender());
    _targetSizeMenu->exec(btnTargetSizeSelector->mapToGlobal(QPoint(0, btnTargetSizeSelector->height())));
}

void CamControlsWidget::onTargetSizeActionTriggered()
{
    EnterProcStart("CamControlsWidget::onTargetSizeActionTriggered");
    auto targetSizeAction = qobject_cast<QAction*>(QObject::sender());
    if (targetSizeAction == nullptr)
        return;
    int targetSize = targetSizeAction->data().toInt();
    emit setTargetSize(targetSize);
}

void CamControlsWidget::onEnableStabilizationClick_Internal()
{
    EnterProcStart("CamControlsWidget::onEnableStabilizationClick");
    ApplicationSettings& applicationSettings = ApplicationSettings::Instance();

    bool isSoftwareEnabled = _btnEnableStabilization->isChecked();
    applicationSettings.SoftwareStabilizationEnabled = isSoftwareEnabled;
    emit enableSoftwareStabilization(isSoftwareEnabled);
}

void CamControlsWidget::onEnableStabilizationMenuClick()
{
    EnterProcStart("CamControlsWidget::onEnableStabilizationMenuClick");
    ApplicationSettings& applicationSettings = ApplicationSettings::Instance();

    auto btnStabilization = qobject_cast<QPushButtonEx *>(sender());
    _stabilizationTypeMenu->exec(btnStabilization->mapToGlobal(QPoint(0, btnStabilization->height())));
    auto acSelectedStabilizationType = _stabilizationTypeGroup->checkedAction();

    if (acSelectedStabilizationType != nullptr)
    {
        StabilizationType stabType = StabilizationType::StabilizationByFrame;
        stabType = StabilizationType(acSelectedStabilizationType->data().toInt());
        applicationSettings.VideoStabilizationType = stabType;
        emit setStabilizationType(stabType);
    }
}

void CamControlsWidget::onLaserActivationClick_Internal()
{
    EnterProcStart("CamControlsWidget::onCamRecordingClicked_Internal");
    _hardwareLink->setLaserActivation(_btnLaserActivation->isChecked());
}

void CamControlsWidget::onTargetUnlockClicked()
{
    emit unlockTarget();
}

void CamControlsWidget::onCamRecordingClicked()
{
    EnterProcStart("CamControlsWidget::onCamRecordingClicked");
    _btnCamRecording->click();
}

void CamControlsWidget::onCamRecordingClicked_Internal()
{
    EnterProcStart("CamControlsWidget::onCamRecordingClicked_Internal");
    if(_btnCamRecording->isChecked())
        _hardwareLink->startCamRecording();
    else
        _hardwareLink->stopCamRecording();
}

void CamControlsWidget::onEnableAutomaticTracerClicked()
{
    EnterProcStart("CamControlsWidget::onAutomaticTracerClicked");
    _btnAutomaticTracer->click();
}

void CamControlsWidget::onAutomaticTracerClicked_Internal()
{
    EnterProcStart("CamControlsWidget::onAutomaticTracerClicked_Internal");
    if (_btnAutomaticTracer->isChecked())
        _automaticTracer->followScreenPoint();
    else
        _automaticTracer->sleep();
}

void CamControlsWidget::onCamZoomValueChanged(int value)
{
    Q_UNUSED(value)
    EnterProcStart("CamControlsWidget::onCamZoomValueChanged");
    sendCamZoom(_camZoom->value());
}

void CamControlsWidget::onCamDriversOffClicked()
{
    EnterProcStart("CamControlsWidget::onCamDriversOffClicked");

    if (qobject_cast<QPushButtonEx*>(sender()) == nullptr )
        _btnCamDriversOff->setChecked(! _btnCamDriversOff->isChecked());

    bool driversEnabled = _btnCamDriversOff->isChecked();
    _hardwareLink->setCamMotorStatus(driversEnabled);
}

void CamControlsWidget::setCamControlsEnabled(bool enabled)
{
    EnterProcStart("CamControlsWidget::setCamControlsEnabled");
    if (_camDials != nullptr)
        _camDials->setEnabled(enabled);
    if (_camKnob != nullptr)
        _camKnob->setEnabled(enabled);
    _camZoom->setEnabled(enabled);


    auto cameraSettings = currCameraSettings();
    bool rotatingCamera = (cameraSettings->CameraSuspensionType == CameraSuspensionTypes::RotatingCamera);

    _btnCamLandingPos->setEnabled(rotatingCamera && enabled);
    _btnCamBeginingPos->setEnabled(rotatingCamera);
    _btnCamVerticalPos->setEnabled(rotatingCamera && enabled);
    _btnAutomaticTracer->setEnabled(rotatingCamera && enabled);
    _btnEnableStabilization->setEnabled(rotatingCamera && enabled);
}

void CamControlsWidget::sendCamZoom(float zoom)
{
    EnterProcStart("CamControlsWidget::sendCamZoom");
    emit doSetZoomFromUI(zoom);
    _hardwareLink->setCamZoom(zoom);
    _voiceInformant->sayMessage(VoiceMessage::ZoomChange);
}

void CamControlsWidget::onCamLandingPosClicked()
{
    EnterProcStart("CamControlsWidget::onCamLandingPosClicked");

    _automaticTracer->sleep();

    auto cameraSettings = currCameraSettings();

    if (cameraSettings->FixedPosLandingCommand.value())
        _hardwareLink->parkCamera();
    else
    {
        _btnAutomaticTracer->setChecked(false);
        _automaticTracer->followEncoderValues(cameraSettings->FixedPosLandingRoll, cameraSettings->FixedPosLandingPitch, cameraSettings->FixedPosLandingYaw);
        onAbsoluteCamZoomChange(cameraSettings->opticalDeviceSetting(1)->FixedPosLandingZoom->value()); //???todo
    }

    setCamControlsEnabled(false);

    emit unlockTarget();
    _btnEnableStabilization->setChecked(false);
}

void CamControlsWidget::onCamBeginingPosClicked()
{
    EnterProcStart("CamControlsWidget::onCamBeginingPosClicked");

    setCamControlsEnabled(true);

    _btnAutomaticTracer->setChecked(false);
    auto cameraSettings = currCameraSettings();
    _automaticTracer->followEncoderValues(cameraSettings->FixedPosBeginingRoll, cameraSettings->FixedPosBeginingPitch, cameraSettings->FixedPosBeginingYaw);
}

void CamControlsWidget::onCamVerticalPosClicked()
{
    EnterProcStart("CamControlsWidget::onCamVerticalPosClicked");

    _btnAutomaticTracer->setChecked(false);
    auto cameraSettings = currCameraSettings();
    _automaticTracer->followEncoderValues(cameraSettings->FixedPosVerticalRoll, cameraSettings->FixedPosVerticalPitch, cameraSettings->FixedPosVerticalYaw);
    //    onAbsoluteCamZoomChange(cameraSettings->FixedPosVerticalZoom);
}

void CamControlsWidget::onColorModeUpClicked()
{
    _imageTuner->changeCamMode(-1);
}

void CamControlsWidget::onColorModeDownClicked()
{
    _imageTuner->changeCamMode(+1);
}

void CamControlsWidget::onLaserActivationClicked()
{
    _btnLaserActivation->click();
}

void CamControlsWidget::onActiveOpticalSystemClicked(quint32 id)
{
    EnterProcStart("CamControlsWidget::onActiveOpticalSystemClicked");
    if (_hardwareLink->activeOpticalSystemId() == id)
        _imageTuner->activate(id);

    _hardwareLink->setActiveOpticalSystemId(id);
}

void CamControlsWidget::onChangeColorMode(int colorMode)
{
    _hardwareLink->setCamColorMode(colorMode);
}

void CamControlsWidget::onTracerModeChanged(const AutomaticTracerMode tracerMode)
{
    if (tracerMode == AutomaticTracerMode::atmScreenPoint)
        _btnAutomaticTracer->setChecked(true);
    else
        _btnAutomaticTracer->setChecked(false);
}

void CamControlsWidget::onScreenshotClicked()
{
    EnterProcStart("CamControlsWidget::onScreenshotClicked");
    emit makeScreenshot();
}

void CamControlsWidget::onSnapshotSeriesClicked()
{
    EnterProcStart("CamControlsWidget::onSnapshotSeriesClicked");

    bool needStart = !_hardwareLink->isSnapshotSeries();

    _btnSnapshotSeries->setDown(needStart);
    _sbSnapshotSeriesInterval->setEnabled(!needStart);
    if (needStart)
    {
        int intervalMsec = 1000 * _sbSnapshotSeriesInterval->value();
        _hardwareLink->startSnapshotSeries(intervalMsec);
    }
    else
        _hardwareLink->stopSnapshotSeries();
}

void CamControlsWidget::onSnapshotClicked()
{
    _hardwareLink->makeSnapshot();
}
