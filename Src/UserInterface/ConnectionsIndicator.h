#ifndef CONNECTIONSINDICATOR_H
#define CONNECTIONSINDICATOR_H

#include <QWidget>
#include <QObject>
#include <QPushButton>
#include "Common/CommonUtils.h"

enum class IndicatorTristateEnum {
    Incative = 0,
    On,
    Off,
    Disabled,
    LastValue = Disabled
};

class ConnectionsIndicator : public QWidget
{
    Q_OBJECT

    QPushButton *_btnCamConnectionState;
    QPushButton *_btnTelemetryConnectionState;
    QPushButton *_btnRecordingState;

    IndicatorTristateEnum _camConnectionState, _telemetryConnectionState, _recordingState;

    AnimusLicenseState _licenseState;
public:
    explicit ConnectionsIndicator(QWidget *parent);
    void showCurrentStatus(IndicatorTristateEnum camConnectionState, IndicatorTristateEnum telemetryConnectionState, IndicatorTristateEnum recordingState);
};

#endif // CONNECTIONSINDICATOR_H
