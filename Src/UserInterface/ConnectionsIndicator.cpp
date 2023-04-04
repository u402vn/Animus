#include "ConnectionsIndicator.h"
#include <QHBoxLayout>
#include "EnterProc.h"
#include "Common/CommonWidgets.h"

ConnectionsIndicator::ConnectionsIndicator(QWidget *parent) : QWidget(parent)
{
    EnterProcStart("DashboardWidget::DashboardWidget");

    this->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);

    _btnCamConnectionState = CommonWidgetUtils::createButton(this,  "C", tr("Camera"), false, 0, 0, "");
    _btnCamConnectionState->setEnabled(false);
    _btnTelemetryConnectionState = CommonWidgetUtils::createButton(this,  "T", tr("Telemetry"), false, 0, 0, "");
    _btnTelemetryConnectionState->setEnabled(false);
    _btnRecordingState = CommonWidgetUtils::createButton(this,  "R", tr("Record"), false, 0, 0, "");
    _btnRecordingState->setEnabled(false);

    auto connectionsLayout = new QHBoxLayout(this);
    connectionsLayout->setMargin(0);
    this->setLayout(connectionsLayout);
    connectionsLayout->addWidget(_btnCamConnectionState, 0);
    connectionsLayout->addWidget(_btnTelemetryConnectionState, 0);
    connectionsLayout->addWidget(_btnRecordingState, 0);

    _licenseState = getAnimusLicenseState();

    _camConnectionState = IndicatorTristateEnum::Off;
    _telemetryConnectionState = IndicatorTristateEnum::Off;
    _recordingState = IndicatorTristateEnum::Off;

    showCurrentStatus(IndicatorTristateEnum::Incative, IndicatorTristateEnum::Incative, IndicatorTristateEnum::Incative);
}

static const QString stateStyles[static_cast<int>(IndicatorTristateEnum::LastValue) + 1] = {
        QString("QPushButton {background-color: #31363b; border-color: #76797C; color: #eff0f1;}"),  // Incative
        QString("QPushButton {background-color: #009900; border-color: #76797C; color: #eff0f1;}"),  // On
        QString("QPushButton {background-color: #990000; border-color: #76797C; color: #eff0f1;}"),  // Off
        QString("QPushButton {background-color: #000099; border-color: #76797C; color: #eff0f1;}")   // Disabled
        };

void ConnectionsIndicator::showCurrentStatus(IndicatorTristateEnum camConnectionState, IndicatorTristateEnum telemetryConnectionState, IndicatorTristateEnum recordingState)
{
    this->setUpdatesEnabled(false);
    //this->blockSignals(true);
    if (_camConnectionState != camConnectionState)
    {
        _btnCamConnectionState->setStyleSheet(stateStyles[static_cast<int>(camConnectionState)]);
        _camConnectionState = camConnectionState;
    }
    if (_telemetryConnectionState != telemetryConnectionState)
    {
        if (_licenseState != AnimusLicenseState::Expired)
            _btnTelemetryConnectionState->setStyleSheet(stateStyles[static_cast<int>(telemetryConnectionState)]);
        else
            _btnTelemetryConnectionState->setStyleSheet(stateStyles[static_cast<int>(IndicatorTristateEnum::Disabled)]);
        _telemetryConnectionState = telemetryConnectionState;
    }
    if (_recordingState != recordingState)
    {
        _btnRecordingState->setStyleSheet(stateStyles[static_cast<int>(recordingState)]);
        _recordingState = recordingState;
    }
    //this->blockSignals(false);
    this->setUpdatesEnabled(true);
}
