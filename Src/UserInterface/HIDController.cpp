#include "HIDController.h"
#include <QWindow>
#include <QtGlobal>
#include "Common/CommonWidgets.h"

constexpr uint JOYSTICK_TIME_INTERVAL = 80;
constexpr double deltaZoom = 1.0;

HIDController::HIDController(QObject *parent) : QObject(parent)
{
    ApplicationSettings& applicationSettings = ApplicationSettings::Instance();

    _joystickX = 0;
    _joystickY = 0;
    _joystickZ = 0;
    _keyboardX = 0;
    _keyboardY = 0;
    _joystickInZeroPoint = false;
    _prevJoystickZ = 0;
    setCamZoomRange(1, 1);

    _joystickAxisMultiplier = applicationSettings.JoystickAxisMultiplier;
    _joystickEmulationFromKeyboard = applicationSettings.JoystickEmulationFromKeyboard;
    _joystickAxisSensitivity = applicationSettings.JoystickAxisInsensitivity;
    _joystickAxisZoomIndex = applicationSettings.JoystickAxisZoomIndex;

    if (applicationSettings.JoystickUsing)
    {
        _joystick = new Joystick(this, applicationSettings.JoystickMapping, JOYSTICK_TIME_INTERVAL);
        connect(_joystick, &Joystick::processJoystick, this, &HIDController::processJoystick);
    }

    _keyboardUsing = applicationSettings.KeyboardUsing;
    _controlMode = applicationSettings.installedCameraSettings()->CameraControlMode;

    bool processAutoRepeatKeyForCamMoving = (_controlMode == CameraControlModes::AbsolutePosition);

    makeHIDMapItem(hidbtnCamZoomIn,          &HIDController::processCamZoomUp,                     nullptr, true);
    makeHIDMapItem(hidbtnCamZoomOut,         &HIDController::processCamZoomDown,                   nullptr, true);
    makeHIDMapItem(hidbtnCamPitchUp,         &HIDController::processPitchUpPress,               &HIDController::processPitchUpRelease,   processAutoRepeatKeyForCamMoving);
    makeHIDMapItem(hidbtnCamPitchDown,       &HIDController::processPitchDownPress,             &HIDController::processPitchDownRelease, processAutoRepeatKeyForCamMoving);
    makeHIDMapItem(hidbtnCamRollUp,          &HIDController::processRollUpPress,                &HIDController::processRollUpRelease,    processAutoRepeatKeyForCamMoving);
    makeHIDMapItem(hidbtnCamRollDown,        &HIDController::processRollDownPress,              &HIDController::processRollDownRelease,  processAutoRepeatKeyForCamMoving);

    makeHIDMapItem(hidbtnMapZoomIn,          &HIDController::onMapZoomInClicked,         nullptr, true);
    makeHIDMapItem(hidbtnMapZoomOut,         &HIDController::onMapZoomOutClicked,        nullptr, true);

    makeHIDMapItem(hidbtnSettingsEditor,     &HIDController::onOpenApplicationSettingsEditorClicked,   nullptr, false);
    makeHIDMapItem(hidbtnDataConsole,        &HIDController::onOpenDataConsoleClicked,          nullptr, false);
    makeHIDMapItem(hidbtnEmulatorConsole,    &HIDController::onOpenEmulatorConsoleClicked,      nullptr, false);
    makeHIDMapItem(hidbtnHelpViewer,         &HIDController::onOpenHelpViewerClicked,           nullptr, false);
    makeHIDMapItem(hidbtnChangeVideo2Map,    &HIDController::onChangeVideo2MapClicked,          nullptr, false);
    makeHIDMapItem(hidbtnNewSession,         &HIDController::onForceStartNewSessionClicked,     nullptr, false);
    makeHIDMapItem(hidbtnSelectSessions,     &HIDController::onSelectSessionsClicked,           nullptr, false);
    makeHIDMapItem(hidbtnDisplayOnly,        &HIDController::onForceDisplayOnlyClicked,         nullptr, false);
    makeHIDMapItem(hidbtnChangeActiveCam,    &HIDController::onChangeActiveCamClicked,          nullptr, false);
    makeHIDMapItem(hidbtnEnableSoftwareStabilization,    &HIDController::onEnableSoftwareStabilizationClicked,          nullptr, false);
    makeHIDMapItem(hidbtnDriversOff,         &HIDController::onCamDriversOffClicked,            nullptr, false);
    makeHIDMapItem(hidbtnFixedPosLanding,    &HIDController::onCamLandingPosClicked,            nullptr, false);
    makeHIDMapItem(hidbtnFixedPosBegining,   &HIDController::onCamBeginingPosClicked,           nullptr, false);
    makeHIDMapItem(hidbtnFixedPosVertical,   &HIDController::onCamVerticalPosClicked,           nullptr, false);
    makeHIDMapItem(hidbtnColorModeUp,        &HIDController::onColorModeUpClicked,              nullptr, false);
    makeHIDMapItem(hidbtnColorModeDown,      &HIDController::onColorModeDownClicked,            nullptr, false);
    makeHIDMapItem(hidbtnLaserActivation,    &HIDController::onLaserActivationClicked,          nullptr, false);

    makeHIDMapItem(hidbtnBombingSight,       &HIDController::onChangeBombingSightClicked,       nullptr, false);
    makeHIDMapItem(hidbtnScreenshot,         &HIDController::onScreenshotClicked,               nullptr, true);
    makeHIDMapItem(hidbtnSnapshot,           &HIDController::onSnapshotClicked,                 nullptr, true);
    makeHIDMapItem(hidbtnSnapshotSeries,     &HIDController::onSnapshotSeriesClicked,           nullptr, false);
    makeHIDMapItem(hidbtnTargetUnlock,       &HIDController::onTargetUnlockClicked,             nullptr, false);
    makeHIDMapItem(hidbtnCamRecording,       &HIDController::onCamRecordingClicked,             nullptr, false);
    makeHIDMapItem(hidbtnAutomaticTracer,    &HIDController::onEnableAutomaticTracerClicked,    nullptr, false);
    makeHIDMapItem(hidbtnDropBomb,           &HIDController::onDropBombClicked,                 nullptr, true);
    makeHIDMapItem(hidbtnSendHitCoordinates, &HIDController::onSendHitCoordinatesClicked,       nullptr, false);
    makeHIDMapItem(hidbtnSendWeather,        &HIDController::onSendWeatherClicked,              nullptr, false);

    makeHIDMapItem(hidbtnNewMarkerForTarget, &HIDController::onNewMarkerForTargetClicked,       nullptr, true);
    makeHIDMapItem(hidbtnNewMarkerForLaser,  &HIDController::onNewMarkerForLaserClicked,        nullptr, true);
    makeHIDMapItem(hidbtnNewMarkerForUAV,    &HIDController::onNewMarkerForUAVClicked,          nullptr, true);

    makeHIDMapItem(hidbtnNormalFlight,              &HIDController::onNormalFlightClicked,           nullptr, true);
    makeHIDMapItem(hidbtnPatrolMovingTargetMode,    &HIDController::onPatrolMovingTargetModeClicked, nullptr, true);
    makeHIDMapItem(hidbtnPatrolStaticTargetMode,    &HIDController::onPatrolStaticTargetModeClicked, nullptr, true);
    makeHIDMapItem(hidbtnManualFlightMode,          &HIDController::onManualFlightModeClicked,       nullptr, true);

    CommonWidgetUtils::installEventFilterToApplication(this);
}

void HIDController::makeHIDMapItem(HIDButton prefIndex, void (HIDController::*onPressMethod)(), void (HIDController::*onReleaseMethod)(), bool processAutoRepeatKey)
{
    ApplicationSettings& applicationSettings = ApplicationSettings::Instance();
    auto item = new HIDMapItem(this, onPressMethod, onReleaseMethod,
                               applicationSettings.hidJoystickPref(prefIndex),
                               applicationSettings.hidKeyboardPref(prefIndex), processAutoRepeatKey);
    _HIDMap.append(item);
}

void HIDController::setCamZoomRange(quint32 camZoomMin, quint32 camZoomMax)
{
    _camZoomMin = camZoomMin;
    _camZoomMax = camZoomMax;
    updateCamZoomInternal(_camZoom);
}

void HIDController::updateCamZoomInternal(quint32 zoom)
{
    quint32 prevZoom = _camZoom;
    bool useJoystickForZoom = ! qFuzzyCompare(_prevJoystickZ, _joystickZ);

    if (useJoystickForZoom)
        _camZoom = _camZoomMin + (_camZoomMax - _camZoomMin) * (_joystickZ + 1) * 0.5;
    else
        _camZoom = zoom;

    if (_camZoom > _camZoomMax)
        _camZoom = _camZoomMax;
    else if (_camZoom < _camZoomMin)
        _camZoom = _camZoomMin;

    if (prevZoom != _camZoom)
        emit onAbsoluteCamZoomChange(_camZoom);
}

const QString makeJoystickStateText(const QList<int> &povs, const QList<double> &axes, const QList<bool> &buttons)
{
    QString povsText, axesText, buttonsText;
    for (int i = 0; i < povs.count(); i++)
        povsText += QString("\tPOV %1: %2\n").arg(i).arg(povs[i]);

    for (int i = 0; i < axes.count(); i++)
        axesText += QString("\tAxis %1: %2\n").arg(i).arg(axes[i]);

    for (int i = 0; i < buttons.count(); i++)
        buttonsText += QString("\tButton %1: %2\n").arg(i + 1).arg(buttons[i]);

    QString result = QString("POVs:\n%1\nAxes:\n%2\nButtons:\n%3").arg(povsText).arg(axesText).arg(buttonsText);

    return result;
}

void HIDController::processJoystick(const QList<int> &povs, const QList<double> &axes, const QList<bool> &buttons)
{
    Q_UNUSED(povs)

    _prevJoystickZ = _joystickZ;

    _joystickX = axes[0];
    _joystickY = axes[1];

    if ( (_joystickAxisZoomIndex > 0)  && (_joystickAxisZoomIndex < axes.count()) )
        _joystickZ = axes[_joystickAxisZoomIndex];
    else
        _joystickZ = 0;

    foreach (auto hidMapItem, _HIDMap)
        hidMapItem->processJoystick(buttons);

    emit onJoystickStateTextChanged( makeJoystickStateText(povs, axes, buttons) );

    processAxisChanges();
    processPOVChanges(povs);
}

void HIDController::doSetZoomFromUI(quint32 zoom)
{
    updateCamZoomInternal(zoom);
}

bool HIDController::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::KeyPress || event->type() == QEvent::KeyRelease)
    {
        QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
        if (processKeyboard(keyEvent, obj))
            return true;
    }

    return QObject::eventFilter(obj, event);
}

bool HIDController::processKeyboard(QKeyEvent *keyEvent, QObject *senderObj)
{
    if (!_keyboardUsing)
        return false;

    //Exclude events from other windows
    const QString MainWindowSender = "MainWindowClassWindow";
    const QString MapViewSender = "MapViewClassWindow";

    if (QWindow * sender = dynamic_cast<QWindow*> (senderObj))
    {
        QString senderName = sender->objectName();
        if (senderName != MainWindowSender && senderName != MapViewSender)
            return false;
    }
    else
        return false;

    bool keyProcessed = false;

    foreach (auto hidMapItem, _HIDMap)
        keyProcessed = keyProcessed || hidMapItem->processKeyboard(keyEvent);

    return keyProcessed;
}

void HIDController::processCamZoomUp()
{
    updateCamZoomInternal(_camZoom + deltaZoom);
}

void HIDController::processCamZoomDown()
{
    updateCamZoomInternal(_camZoom - deltaZoom);
}

void HIDController::processRollUpPress()
{
    if (qFuzzyCompare(_keyboardX, 0))
        _keyboardX = _joystickEmulationFromKeyboard;
    processAxisChanges();
}

void HIDController::processRollUpRelease()
{
    if (qFuzzyCompare(_keyboardX, _joystickEmulationFromKeyboard))
        _keyboardX = 0;
    processAxisChanges();
}

void HIDController::processRollDownPress()
{
    if (qFuzzyCompare(_keyboardX, 0))
        _keyboardX = -_joystickEmulationFromKeyboard;
    processAxisChanges();
}

void HIDController::processRollDownRelease()
{
    if (qFuzzyCompare(_keyboardX, -_joystickEmulationFromKeyboard))
        _keyboardX = 0;
    processAxisChanges();
}

void HIDController::processPitchUpPress()
{
    if (qFuzzyCompare(_keyboardY, 0))
        _keyboardY = -_joystickEmulationFromKeyboard;
    processAxisChanges();
}

void HIDController::processPitchUpRelease()
{
    if (qFuzzyCompare(_keyboardY, -_joystickEmulationFromKeyboard))
        _keyboardY = 0;
    processAxisChanges();
}

void HIDController::processPitchDownPress()
{
    if (qFuzzyCompare(_keyboardY, 0))
        _keyboardY = _joystickEmulationFromKeyboard;
    processAxisChanges();
}

void HIDController::processPitchDownRelease()
{
    if (qFuzzyCompare(_keyboardY, _joystickEmulationFromKeyboard))
        _keyboardY = 0;
    processAxisChanges();
}

void HIDController::processAxisChanges()
{
    updateCamZoomInternal(_camZoom);

    float x = qFuzzyCompare(_keyboardX, 0) ? _joystickX : _keyboardX;
    float y = qFuzzyCompare(_keyboardY, 0) ? _joystickY : _keyboardY;

    bool inZeroPoint = (qAbs(x) < _joystickAxisSensitivity) && (qAbs(y) < _joystickAxisSensitivity);

    if (_joystickInZeroPoint && inZeroPoint)
        return;
    _joystickInZeroPoint = inZeroPoint;

    if (inZeroPoint)
    {
        x = 0;
        y = 0;
    }

    if (_controlMode == CameraControlModes::AbsolutePosition)
    {
        float deltaRoll = x * _joystickAxisMultiplier;
        float deltaPitch = y * _joystickAxisMultiplier;
        float deltaYaw = 0;

        emit onRelativeCamPositionChange(deltaRoll, deltaPitch, deltaYaw);
    }
    else if (_controlMode == CameraControlModes::RotationSpeed)
    {
        float speedYaw = x * _joystickAxisMultiplier;
        float speedPitch = y * _joystickAxisMultiplier;
        float speedRoll = 0;

        emit onCamMovingSpeedChange(speedRoll, speedPitch, speedYaw);
    }
}

void HIDController::processPOVChanges(const QList<int> &povs)
{
    if (povs.count() == 0)
        return;

    if (povs[0] >= 0)
        emit onMapMoveClicked(povs[0]);
}


// ---------------------------------------------

HIDMapItem::HIDMapItem(HIDController *parent, void(HIDController::*onPressMethod)(),
                       void(HIDController::*onReleaseMethod)(),
                       ApplicationPreferenceInt *joystickButtonPref,
                       ApplicationPreferenceString *keyboardPref, bool processAutoRepeatKey) : QObject(parent)
{
    if (keyboardPref != nullptr)
    {
        QString keyPref = keyboardPref->value();
        _keySequence = QKeySequence::fromString(keyPref);
    }

    _processAutoRepeatKey = processAutoRepeatKey;

    if (joystickButtonPref != nullptr)
        _joystickButtonIdx = joystickButtonPref->value();
    else
        _joystickButtonIdx = -1;
    _joystickButtonWasPressed = false;

    if (onPressMethod != nullptr)
        connect(this, &HIDMapItem::processPressEvent, parent, onPressMethod);
    if (onReleaseMethod != nullptr)
        connect(this, &HIDMapItem::processReleaseEvent, parent, onReleaseMethod);
}

void HIDMapItem::processJoystick(const QList<bool> &buttons)
{
    if (_joystickButtonIdx < 0 || _joystickButtonIdx >= buttons.count())
        return;

    bool joystickButtonIsPressed = buttons[_joystickButtonIdx];
    bool isClicked = !_joystickButtonWasPressed && joystickButtonIsPressed;
    bool isUnclicked = _joystickButtonWasPressed && !joystickButtonIsPressed;
    bool isAutorepeat = joystickButtonIsPressed && _processAutoRepeatKey;
    _joystickButtonWasPressed = joystickButtonIsPressed;
    if (isClicked || isAutorepeat)
        emit processPressEvent();
    if (isUnclicked)
        emit processReleaseEvent();
}

bool HIDMapItem::processKeyboard(QKeyEvent *keyEvent)
{
    if (_keySequence.count() != 1)
        return false;
    if (!_processAutoRepeatKey && keyEvent->isAutoRepeat())
        return false;
    int key = keyEvent->key();
    if (key == 0)
        return false;

    int elemet = _keySequence[0];
    int modifiers = keyEvent->modifiers();
    modifiers = modifiers & (~ 0x20000000); // Qt::KeypadModifier
    key = key | modifiers;
    QEvent::Type eventType = keyEvent->type();

    if (elemet == key)
    {
        if (eventType == QEvent::KeyPress)
        {
            emit processPressEvent();
            return true;
        }
        else if (eventType == QEvent::KeyRelease)
        {
            emit processReleaseEvent();
            return true;
        }

    }
    return false;
}
