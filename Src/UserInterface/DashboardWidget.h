#ifndef DASHBOARDWIDGET_H
#define DASHBOARDWIDGET_H

#include <QWidget>
#include <QTableWidget>
#include <QMenu>
#include <QPushButton>
#include <QLineEdit>
#include <QMap>
#include <QString>
#include <QList>
#include "GPSCoordIndicator.h"
#include "UserInterface/PFD.h"

class DashboardWidget final: public QWidget
{
    Q_OBJECT

    PFD * _PFD;
    QTableWidget *_telemetryTable;
    QMenu *_menu;

    GPSCoordIndicator *_coordIndicator;

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
                            RowVideoFPS, RowTelemetryFPS,
                            RowCalculatedTrackedTargetSpeed, RowCalculatedTrackedTargetDirection,
                            RowCalculatedTrackedTargetGPSLat, RowCalculatedTrackedTargetGPSLon, RowCalculatedTrackedTargetGPSHmsl,
                            RowTrackedTargetCenterX, RowTrackedTargetCenterY, RowTrackedTargetRectWidth, RowTrackedTargetRectHeight,
                            RowTrackedTargetState,
                            //insert items before this line. Don't change the order of the items
                            RowLast
                           };

    QList<QAction*> _checkableItems;

    void setTelemetryTableRowDouble(int rowId, double value, int precision);
    void setTelemetryTableRowDoubleOrIncorrect(int rowId, double value, int precision, double incorrectValue);
    void updateItemsVisibility();

    void addParameter(TelemetryTableRow tableRow, const QString &name, QMenu *menu);
public:
    explicit DashboardWidget(QWidget *parent);
    void processTelemetry(const TelemetryDataFrame &telemetryDataFrame);
private slots:
    void onActivateCatapultClicked();
    void onTelemetryTableContextMenuRequested(const QPoint &pos);
signals:
    void activateCatapult();
};

#endif // DASHBOARDWIDGET_H
