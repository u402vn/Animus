#ifndef ANTENNACONTROLWIDGET_H
#define ANTENNACONTROLWIDGET_H

#include <QWidget>
#include <QGridLayout>
#include "Common/CommonWidgets.h"
#include "Common/CommonUtils.h"
#include "HardwareLink/HardwareLink.h"
#include "UserInterface/GPSCoordSelector.h"
#include "TelemetryDataFrame.h"
#include "EnterProc.h"

class AntennaControlWidget final : public QWidget
{
    Q_OBJECT

    HardwareLink *_hardwareLink;

    QGridLayout *_mainLayout;
    QLabelEx *_lblAntennaCoord, *_lblUAVCoord;
    GPSCoordSelector  *_antennaCoordSelector, *_uavCoordelector;
    QPushButtonEx *_btnRotationAutoMode, *_btnRotationSetManually;
    QPushButtonEx *_btnHeatingAutoMode, *_btnHeatingOnOff;
    QPushButtonEx *_btnFanAutoMode, *_btnFanOnOff;

    WorldGPSCoord _antennaCoord, _uavCoord;


    void initWidgets();
    void showCoordValues();

    void applyState();
public:
    explicit AntennaControlWidget(QWidget *parent, HardwareLink *hardwareLink);
    ~AntennaControlWidget();

    void processTelemetry(const TelemetryDataFrame &telemetryFrame);
signals:

public slots:
};

#endif // ANTENNACONTROLWIDGET_H
