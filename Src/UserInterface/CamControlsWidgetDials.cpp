#include "CamControlsWidgetDials.h"
#include <QPainter>
#include <QLabel>
#include <QKeyEvent>
#include "EnterProc.h"

void CamControlsWidgetDials::doSetCamPostion()
{
    EnterProcStart("CamPositionControlsWidget::doSetCamPostion");

    float roll = _camRoll->currentValue();
    float pitch = _camPitch->currentValue();
    float yaw = 0;

    if (_camAxisXInverse)
        roll = -roll;
    if (_camAxisYInverse)
        pitch = -pitch;

    emit setCamPosition(roll, pitch, yaw);
}

CamControlsWidgetDials::CamControlsWidgetDials(QWidget *parent) : QWidget(parent)
{
    //initialize fields
    _camControlsMoving = 0;

    auto cameraSettings = currCameraSettings();
    _camAxisXInverse = cameraSettings->CamAxisXInverse;
    _camAxisYInverse = cameraSettings->CamAxisYInverse;

    //create controls
    _camPitch = CommonWidgetUtils::createDialEx(this, 0.45 * DEFAULT_BUTTON_WIDTH, cameraSettings->CamPitchMin, cameraSettings->CamPitchMax);
    connect(_camPitch, &QDialEx::currentValueChanged, this, &CamControlsWidgetDials::onCamPitchValueChanged);
    _camRoll = CommonWidgetUtils::createDialEx(this, 0.45 * DEFAULT_BUTTON_WIDTH, cameraSettings->CamRollMin, cameraSettings->CamRollMax);
    connect(_camRoll, &QDialEx::currentValueChanged, this, &CamControlsWidgetDials::onCamRollValueChanged);

    //append controls to grid

    QGridLayout * controlsGridLayout = new QGridLayout(this);
    controlsGridLayout->setContentsMargins(0, 0, 0, 0);
    controlsGridLayout->setColumnStretch(0, 0);
    controlsGridLayout->setColumnStretch(1, 0);


    int row = 0;
    controlsGridLayout->addWidget(_camPitch,            row, 1, 1, 2, Qt::AlignLeft);
    controlsGridLayout->addWidget(_camRoll,             row, 3, 1, 2, Qt::AlignRight);
    controlsGridLayout->setRowStretch(row, 0);

    /*
    ApplicationSettings& applicationSettings = ApplicationSettings::Instance();
    if (!applicationSettings.MinimalisticDesign)
    {
        row++;
        QLabel * camPitchLabel = new QLabel(tr("Pitch"), this);
        QLabel * camRollLabel = new QLabel(tr("Roll"), this);
        controlsGridLayout->addWidget(camPitchLabel,       row, 1, 1, 2, Qt::AlignCenter);
        controlsGridLayout->addWidget(camRollLabel,        row, 3, 1, 2, Qt::AlignCenter);
        controlsGridLayout->setRowStretch(row, 0);
    }
    */
    //init controls
    _camControlsMoving++;
    if ((cameraSettings->CameraSuspensionType == CameraSuspensionTypes::FixedCamera))
    {
        _camPitch->setCurrentValueForced(cameraSettings->FixedCamPitch);
        _camRoll->setCurrentValueForced(cameraSettings->FixedCamRoll);
        _camPitch->setVisible(false);
        _camRoll->setVisible(false);
    }
    else
    {
        _camPitch->setValue(cameraSettings->FixedPosBeginingPitch);
        _camRoll->setValue(cameraSettings->FixedPosBeginingRoll);
    }
    _camControlsMoving--;
}

void CamControlsWidgetDials::showTelemetryDataFrame(const TelemetryDataFrame &telemetryFrame)
{
    double roll = telemetryFrame.CamRoll;
    double pitch = telemetryFrame.CamPitch;
    if (_camAxisXInverse)
        roll = -roll;
    if (_camAxisYInverse)
        pitch = -pitch;

    _camRoll->setIndicatorValue(roll);
    _camPitch->setIndicatorValue(pitch);

    if (telemetryFrame.TelemetryFrameNumber == 1)
    {
        _camControlsMoving++;
        _camRoll->setCurrentValue(roll);
        _camPitch->setCurrentValue(pitch);
        _camControlsMoving--;
    }
}

void CamControlsWidgetDials::changeRelativeCamPosition(float deltaRoll, float deltaPitch, float deltaYaw)
{
    Q_UNUSED(deltaYaw)
    float roll = _camRoll->currentValue() + deltaRoll;
    float pitch = _camPitch->currentValue() + deltaPitch;
    float yaw = 0;
    changeAbsoluteCamPosition(roll, pitch, yaw);
}

void CamControlsWidgetDials::changeAbsoluteCamPosition(float roll, float pitch, float yaw)
{
    _camControlsMoving++;
    if (_camPitch->isEnabled())
        _camPitch->setCurrentValue(pitch);
    if (_camRoll->isEnabled())
        _camRoll->setCurrentValue(roll);
    _camControlsMoving--;

    doSetCamPostion();
}

void CamControlsWidgetDials::changeAbsoluteCamPositionToLanding()
{
    auto cameraSettings = currCameraSettings();
    float fixedPosLandingYaw = 0;
    changeAbsoluteCamPosition(cameraSettings->FixedPosLandingRoll, cameraSettings->FixedPosLandingPitch, fixedPosLandingYaw);
}

void CamControlsWidgetDials::changeAbsoluteCamPositionToBegining()
{
    auto cameraSettings = currCameraSettings();
    float fixedPosBeginingYaw = 0;
    changeAbsoluteCamPosition(cameraSettings->FixedPosBeginingRoll, cameraSettings->FixedPosBeginingPitch, fixedPosBeginingYaw);
}

void CamControlsWidgetDials::changeAbsoluteCamPositionToVertical()
{
    auto cameraSettings = currCameraSettings();
    float fixedPosVerticalYaw = 0;
    changeAbsoluteCamPosition(cameraSettings->FixedPosVerticalRoll, cameraSettings->FixedPosVerticalPitch, fixedPosVerticalYaw);
}

void CamControlsWidgetDials::onCamPitchValueChanged(double value)
{
    Q_UNUSED(value)
    EnterProcStart("CamPositionControlsWidget::onCamPitchValueChanged");
    if (_camControlsMoving == 0)
        doSetCamPostion();
}

void CamControlsWidgetDials::onCamRollValueChanged(double value)
{
    Q_UNUSED(value)
    EnterProcStart("CamPositionControlsWidget::onCamRollValueChanged");
    if (_camControlsMoving == 0)
        doSetCamPostion();
}
