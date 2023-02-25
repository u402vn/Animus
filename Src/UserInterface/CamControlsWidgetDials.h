#ifndef CAMCONTROLSWIDGETDIALS_H
#define CAMCONTROLSWIDGETDIALS_H

#include <QWidget>
#include "Common/CommonWidgets.h"
#include "Common/CommonData.h"
#include "ApplicationSettings.h"
#include "TelemetryDataFrame.h"

class CamControlsWidgetDials final : public QWidget
{
private:
    Q_OBJECT

    int _camControlsMoving;

    bool _camAxisXInverse;
    bool _camAxisYInverse;

    QDialEx * _camPitch;
    QDialEx * _camRoll;

    void doSetCamPostion();
public:
    explicit CamControlsWidgetDials(QWidget *parent);

    void showTelemetryDataFrame(const TelemetryDataFrame &telemetryFrame);
    void changeRelativeCamPosition(float deltaRoll, float deltaPitch, float deltaYaw);
    void changeAbsoluteCamPosition(float roll, float pitch, float yaw);
    void changeAbsoluteCamPositionToLanding();
    void changeAbsoluteCamPositionToBegining();
    void changeAbsoluteCamPositionToVertical();
private slots:
    void onCamPitchValueChanged(double value);
    void onCamRollValueChanged(double value);
signals:
    void setCamPosition(float roll, float pitch, float yaw);
};


#endif // CAMCONTROLSWIDGETDIALS_H
