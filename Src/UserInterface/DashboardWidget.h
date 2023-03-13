#ifndef DASHBOARDWIDGET_H
#define DASHBOARDWIDGET_H

#include <QWidget>
#include <QTableWidget>
#include <QMenu>
#include <QPushButton>
#include <QLineEdit>
#include <QMap>
#include <QString>
#include "GPSCoordIndicator.h"
#include "UserInterface/PFD.h"
#include "Common/CommonUtils.h"

enum class IndicatorTristateEnum {
    Incative = 0,
    On,
    Off,
    Disabled,
    LastValue = Disabled
};

class DashboardWidget final: public QWidget
{
    Q_OBJECT

    AnimusLicenseState _licenseState;

    PFD * _PFD;
    QTableWidget *_telemetryTable;
    QMenu *_menu;

    QPushButton *_btnCamConnectionState;
    QPushButton *_btnTelemetryConnectionState;
    QPushButton *_btnRecordingState;

    GPSCoordIndicator *_coordIndicator;

    IndicatorTristateEnum _camConnectionState, _telemetryConnectionState, _recordingState;

    enum TelemetryTableRow {RowUavRoll = 0, RowUavPitch, RowUavYaw, RowUavLatitude_GPS,
                            RowUavLongitude_GPS, RowUavAltitude_GPS, RowUavAltitude_Barometric,
                            RowAirSpeed, RowGroundSpeed_GPS, RowCourse_GPS,
                            RowCamRoll, RowCamPitch, RowCamYaw, RowCamZoom,
                            RowCamEncoderRoll, RowCamEncoderPitch, RowCamEncoderYaw,
                            RowRangefinderDistance,
                            RowWindDirection, RowWindSpeed,
                            RowStabilizedCenterX, RowStabilizedCenterY, RowStabilizedRotationAngle,
                            RowTelemetryFrameNumber, RowVideoFrameNumber, RowSessionTime,
                            RowCalculatedRangefinderGPSLat, RowCalculatedRangefinderGPSLon, RowCalculatedRangefinderGPSHmsl,
                            RowCalculatedGroundLevel, RowOpticalSystem,
                            RowRangefinderTemperature, RowAtmosphereTemperature, RowAtmospherePressure,
                            RowVideoFPS,
                            RowLast
                           };

    QMap<TelemetryTableRow, QString> _paramNames;

    void setTelemetryTableRowDouble(int rowId, double value, int precision);
    void setTelemetryTableRowDoubleOrIncorrect(int rowId, double value, int precision, double incorrectValue);
    void updateItemsVisibility();
public:
    explicit DashboardWidget(QWidget *parent);
    void showCurrentStatus(IndicatorTristateEnum camConnectionState, IndicatorTristateEnum telemetryConnectionState, IndicatorTristateEnum recordingState);
    void processTelemetry(const TelemetryDataFrame &telemetryDataFrame);
private slots:
    void onActivateCatapultClicked();
    void onTelemetryTableContextMenuRequested(const QPoint &pos);
signals:
    void activateCatapult();
};

#endif // DASHBOARDWIDGET_H
