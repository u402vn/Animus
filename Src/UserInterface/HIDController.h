#ifndef HIDCONTROLLER_H
#define HIDCONTROLLER_H

#include <Joystick.h>
#include <QKeyEvent>
#include <QObject>
#include "ApplicationSettings.h"

class HIDController;

class HIDMapItem : public QObject
{
    friend class HIDController;
    Q_OBJECT

    int _joystickButtonIdx;
    bool _joystickButtonWasPressed;
    QKeySequence _keySequence;
    bool _processAutoRepeatKey;

    explicit HIDMapItem(HIDController *parent,
                        void(HIDController::*onPressMethod)(), void(HIDController::*onReleaseMethod)(),
                        ApplicationPreferenceInt *joystickButtonPref,
                        ApplicationPreferenceString *keyboardPref, bool processAutoRepeatKey);
public:
    void processJoystick(const QList<bool> &buttons);
    bool processKeyboard(QKeyEvent *keyEvent);
signals:
    void processPressEvent();
    void processReleaseEvent();
};

class HIDController : public QObject
{
    Q_OBJECT

    Joystick *_joystick;
    bool _keyboardUsing;
    CameraControlModes _controlMode;

    QList<HIDMapItem*> _HIDMap;

    quint32 _camZoomMin, _camZoomMax, _camZoom;

    qreal _joystickX, _joystickY, _joystickZ;
    qreal _keyboardX, _keyboardY;
    bool _joystickInZeroPoint;

    qint32 _joystickAxisZoomIndex;
    qreal _prevJoystickZ;

    qreal _joystickAxisMultiplier;
    qreal _joystickEmulationFromKeyboard;
    qreal _joystickAxisSensitivity;

    void makeHIDMapItem(HIDButton prefIndex, void(HIDController::*onPressMethod)(), void(HIDController::*onReleaseMethod)(), bool processAutoRepeatKey);

    bool processKeyboard(QKeyEvent *keyEvent, QObject *senderObj);

    void processCamZoomUp();
    void processCamZoomDown();

    void processRollUpPress();
    void processRollUpRelease();
    void processRollDownPress();
    void processRollDownRelease();
    void processPitchUpPress();
    void processPitchUpRelease();
    void processPitchDownPress();
    void processPitchDownRelease();

    void updateCamZoomInternal(quint32 zoom);
    void processAxisChanges();\
    void processPOVChanges(const QList<int> &povs);
protected:
    bool virtual eventFilter(QObject *obj, QEvent *event);
public:
    explicit HIDController(QObject *parent);
    void setCamZoomRange(quint32 camZoomMin, quint32 camZoomMax);
private slots:
    void processJoystick(const QList<int> &povs, const QList<double> &axes, const QList<bool> &buttons);
public slots:
    void doSetZoomFromUI(quint32 zoom);
signals:
    void onMapZoomInClicked();
    void onMapZoomOutClicked();

    void onOpenApplicationSettingsEditorClicked();
    void onOpenDataConsoleClicked();
    void onOpenEmulatorConsoleClicked();
    void onOpenHelpViewerClicked();
    void onForceStartNewSessionClicked();
    void onForceDisplayOnlyClicked();
    void onChangeVideo2MapClicked();
    void onSelectSessionsClicked();

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

    void onChangeBombingSightClicked();

    void onDropBombClicked();
    void onSendHitCoordinatesClicked();
    void onSendWeatherClicked();
    void onNewMarkerForTargetClicked();
    void onNewMarkerForLaserClicked();
    void onNewMarkerForUAVClicked();

    void onNormalFlightClicked();
    void onPatrolMovingTargetModeClicked();
    void onPatrolStaticTargetModeClicked();
    void onManualFlightModeClicked();

    void onRelativeCamPositionChange(float deltaRoll, float deltaPitch, float deltaYaw);
    void onAbsoluteCamZoomChange(quint32 zoom);
    void onCamMovingSpeedChange(float speedRoll, float speedPitch, float deltaYaw);

    void onJoystickStateTextChanged(const QString stateText);
    void onMapMoveClicked(int directionAngle);
};

#endif // HIDCONTROLLER_H
