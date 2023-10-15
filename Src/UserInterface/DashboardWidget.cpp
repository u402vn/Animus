#include "DashboardWidget.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QStandardItem>
#include <QSplitter>
#include "ApplicationSettings.h"
#include "Common/CommonWidgets.h"
#include "TelemetryDataFrame.h"
#include "Common/CommonWidgets.h"
#include "Common/CommonUtils.h"
#include "EnterProc.h"

const char *PrimaryFlightDisplayActionId = "PFD";
const char *CoordIndicatorDisplayActionId = "CI";

void DashboardWidget::setTelemetryTableRowDouble(int rowId, double value, int precision)
{
    QString strValue = QString::number(value, 'f', precision);
    auto item = _telemetryTable->item(rowId, 1);
    item->setText(strValue);
}

void DashboardWidget::setTelemetryTableRowDoubleOrIncorrect(int rowId, double value, int precision, double incorrectValue)
{
    QString strValue = value == incorrectValue ? "-" : QString::number(value, 'f', precision);
    auto item = _telemetryTable->item(rowId, 1);
    if (item != nullptr)
        item->setText(strValue);
}

DashboardWidget::DashboardWidget(QWidget *parent) : QWidget(parent)
{
    EnterProcStart("DashboardWidget::DashboardWidget");

    ApplicationSettings& applicationSettings = ApplicationSettings::Instance();

    auto instrumentsLayout = new QVBoxLayout(this);
    instrumentsLayout->setMargin(0);
    this->setLayout(instrumentsLayout);

    _PFD = new PFD(this);

    _coordIndicator = new GPSCoordIndicator(this);

    auto btnCatapultLauncher = new QPushButton(tr("Activate Catapult"), this);
    btnCatapultLauncher->setFocusPolicy(Qt::FocusPolicy::NoFocus);
    connect(btnCatapultLauncher, &QPushButton::clicked, this, &DashboardWidget::onActivateCatapultClicked);
    btnCatapultLauncher->setVisible(applicationSettings.UseCatapultLauncher.value());

    _menu = new CheckableMenu(tr("Settings"), this);
    auto action = CommonWidgetUtils::createCheckableMenuSingleAction(tr("Primary Flight Display"), true, _menu);
    action->setData(PrimaryFlightDisplayActionId);
    action = CommonWidgetUtils::createCheckableMenuSingleAction(tr("Coord Indicator"), true, _menu);
    action->setData(CoordIndicatorDisplayActionId);

    _menu->addSeparator();

    QStringList horizontalHeaderLabels = {tr("Parameter"), tr("Value")};

    _telemetryTable = new QTableWidget(this);
    _telemetryTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    _telemetryTable->horizontalHeader()->sortIndicatorOrder();
    _telemetryTable->setColumnCount(horizontalHeaderLabels.count());
    _telemetryTable->setHorizontalHeaderLabels(horizontalHeaderLabels);
    _telemetryTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    _telemetryTable->setRowCount(RowLast);
    _telemetryTable->setContextMenuPolicy(Qt::CustomContextMenu);
    _telemetryTable->setSortingEnabled(false);
    _telemetryTable->horizontalHeader()->setStretchLastSection(true);
    _telemetryTable->verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    _telemetryTable->verticalHeader()->setDefaultSectionSize(18);
    _telemetryTable->verticalHeader()->setVisible(false);
    connect(_telemetryTable, &QTableWidget::customContextMenuRequested, this, &DashboardWidget::onTelemetryTableContextMenuRequested, Qt::DirectConnection);
    connect(_PFD, &PFD::customContextMenuRequested, this, &DashboardWidget::onTelemetryTableContextMenuRequested, Qt::DirectConnection);

    auto submenuUAV = new CheckableMenu(tr("UAV"), this);
    _menu->addMenu(submenuUAV);

    auto submenuCamera = new CheckableMenu(tr("Camera"), this);
    _menu->addMenu(submenuCamera);

    auto submenuTarget = new CheckableMenu(tr("Target"), this);
    _menu->addMenu(submenuTarget);

    auto submenuWeather = new CheckableMenu(tr("Weather"), this);
    _menu->addMenu(submenuWeather);

    auto submenuSystem = new CheckableMenu(tr("System"), this);
    _menu->addMenu(submenuSystem);

    addParameter(RowUavRoll, tr("UAV Roll"), submenuUAV);
    addParameter(RowUavPitch, tr("UAV Pitch"), submenuUAV);
    addParameter(RowUavYaw, tr("UAV Yaw"), submenuUAV);
    addParameter(RowUavLatitude_GPS, tr("Latitude"), submenuUAV);
    addParameter(RowUavLongitude_GPS, tr("Longitude"), submenuUAV);
    addParameter(RowUavAltitude_GPS, tr("Altitude (GPS)"), submenuUAV);
    addParameter(RowUavAltitude_Barometric, tr("Altitude (Barometric)"), submenuUAV);
    addParameter(RowAirSpeed, tr("Air Speed"), submenuUAV);
    addParameter(RowGroundSpeed_GPS, tr("Ground Speed"), submenuUAV);
    addParameter(RowCourse_GPS, tr("Course"), submenuUAV);
    addParameter(RowCalculatedGroundLevel, tr("Ground Level"), submenuUAV);

    addParameter(RowCamRoll, tr("Cam Roll"), submenuCamera);
    addParameter(RowCamPitch, tr("Cam Pitch"), submenuCamera);
    addParameter(RowCamYaw, tr("Cam Yaw"), submenuCamera);
    addParameter(RowCamZoom, tr("Cam Zoom"), submenuCamera);
    addParameter(RowCamEncoderRoll, tr("Cam Roll (Encoder)"), submenuCamera);
    addParameter(RowCamEncoderPitch, tr("Cam Pitch (Encoder)"), submenuCamera);
    addParameter(RowCamEncoderYaw, tr("Cam Yaw (Encoder)"), submenuCamera);
    if (applicationSettings.isLaserRangefinderLicensed())
    {
        addParameter(RowRangefinderDistance, tr("Distance"), submenuCamera);
        addParameter(RowRangefinderTemperature, tr("Laser Temperature"), submenuCamera);
        addParameter(RowCalculatedRangefinderGPSLat, tr("Latitude (Rangefinder)"), submenuCamera);
        addParameter(RowCalculatedRangefinderGPSLon, tr("Longitude (Rangefinder)"), submenuCamera);
        addParameter(RowCalculatedRangefinderGPSHmsl, tr("Altitude (Rangefinder)"), submenuCamera);
    }
    addParameter(RowCalculatedTrackedTargetSpeed, tr("Target Speed"), submenuTarget);
    addParameter(RowCalculatedTrackedTargetDirection, tr("Target Direction"), submenuTarget);
    addParameter(RowCalculatedTrackedTargetGPSLat, tr("Target Latitude"), submenuTarget);
    addParameter(RowCalculatedTrackedTargetGPSLon, tr("Target Longitude"), submenuTarget);
    addParameter(RowCalculatedTrackedTargetGPSHmsl, tr("Target Altitude"), submenuTarget);

    addParameter(RowWindDirection, tr("Wind Direction"), submenuWeather);
    addParameter(RowWindSpeed, tr("Wind Speed"), submenuWeather);
    addParameter(RowAtmosphereTemperature, tr("Atmosphere Temperature"), submenuWeather);
    addParameter(RowAtmospherePressure, tr("Atmosphere Pressure"), submenuWeather);

    addParameter(RowStabilizedCenterX, tr("Stabilized Center X"), submenuSystem);
    addParameter(RowStabilizedCenterY, tr("Stabilized Center Y"), submenuSystem);
    addParameter(RowStabilizedRotationAngle, tr("Stabilized Rotation Angle"), submenuSystem);
    addParameter(RowTelemetryFrameNumber, tr("Telemetry Frame"), submenuSystem);
    addParameter(RowVideoFrameNumber, tr("Video Frame"), submenuSystem);
    addParameter(RowSessionTime, tr("Session Time"), submenuSystem);
    addParameter(RowTelemetryFPS, tr("Telemetry FPS"), submenuSystem);
    addParameter(RowVideoFPS, tr("Video FPS"), submenuSystem);
    addParameter(RowOpticalSystem, tr("Optical System"), submenuSystem);


    _telemetryTable->resizeColumnsToContents();
    updateItemsVisibility();

    auto mainSplitter = new QSplitter(Qt::Vertical, this);

    mainSplitter->addWidget(_PFD);
    mainSplitter->addWidget(_telemetryTable);

    instrumentsLayout->addWidget(_coordIndicator, 0);
    instrumentsLayout->addWidget(mainSplitter, 1);
    instrumentsLayout->addWidget(btnCatapultLauncher, 0);
}

void DashboardWidget::processTelemetry(const TelemetryDataFrame &telemetryDataFrame)
{
    EnterProcStart("DashboardWidget::processTelemetry");

    _PFD->showTelemetryDataFrame(telemetryDataFrame);

    _coordIndicator->processTelemetry(telemetryDataFrame);

    setTelemetryTableRowDouble(RowUavRoll, telemetryDataFrame.UavRoll, 1);
    setTelemetryTableRowDouble(RowUavPitch, telemetryDataFrame.UavPitch, 1);
    setTelemetryTableRowDouble(RowUavYaw, constrainAngle360(telemetryDataFrame.UavYaw), 1);
    setTelemetryTableRowDoubleOrIncorrect(RowUavLatitude_GPS,  telemetryDataFrame.UavLatitude_GPS,   6, INCORRECT_COORDINATE);
    setTelemetryTableRowDoubleOrIncorrect(RowUavLongitude_GPS, telemetryDataFrame.UavLongitude_GPS,  6, INCORRECT_COORDINATE);
    setTelemetryTableRowDoubleOrIncorrect(RowUavAltitude_GPS,  telemetryDataFrame.UavAltitude_GPS,   1, INCORRECT_COORDINATE);
    setTelemetryTableRowDouble(RowUavAltitude_Barometric, telemetryDataFrame.UavAltitude_Barometric, 1);
    setTelemetryTableRowDouble(RowAirSpeed, telemetryDataFrame.AirSpeed, 1);
    setTelemetryTableRowDouble(RowGroundSpeed_GPS, telemetryDataFrame.GroundSpeed_GPS, 1);
    setTelemetryTableRowDouble(RowCourse_GPS, constrainAngle360(telemetryDataFrame.Course_GPS), 1);
    setTelemetryTableRowDouble(RowCamRoll, telemetryDataFrame.CamRoll, 1);
    setTelemetryTableRowDouble(RowCamPitch, telemetryDataFrame.CamPitch, 1);
    setTelemetryTableRowDouble(RowCamYaw, constrainAngle360(telemetryDataFrame.CamYaw), 1);
    setTelemetryTableRowDouble(RowCamZoom, telemetryDataFrame.CamZoom, 1);
    setTelemetryTableRowDouble(RowCamEncoderRoll, telemetryDataFrame.CamEncoderRoll, 1);
    setTelemetryTableRowDouble(RowCamEncoderPitch, telemetryDataFrame.CamEncoderPitch, 1);
    setTelemetryTableRowDouble(RowCamEncoderYaw, telemetryDataFrame.CamEncoderYaw, 1);
    setTelemetryTableRowDouble(RowRangefinderDistance, telemetryDataFrame.RangefinderDistance, 1);
    setTelemetryTableRowDouble(RowRangefinderTemperature, telemetryDataFrame.RangefinderTemperature, 1);
    setTelemetryTableRowDouble(RowWindDirection, constrainAngle360(telemetryDataFrame.WindDirection), 0);
    setTelemetryTableRowDouble(RowWindSpeed, telemetryDataFrame.WindSpeed, 1);
    setTelemetryTableRowDouble(RowAtmosphereTemperature, telemetryDataFrame.AtmosphereTemperature, 1);
    setTelemetryTableRowDouble(RowAtmospherePressure, telemetryDataFrame.AtmospherePressure, 1);
    setTelemetryTableRowDouble(RowStabilizedCenterX, telemetryDataFrame.StabilizedCenterX, 1);
    setTelemetryTableRowDouble(RowStabilizedCenterY, telemetryDataFrame.StabilizedCenterY, 1);
    setTelemetryTableRowDouble(RowStabilizedRotationAngle, telemetryDataFrame.StabilizedRotationAngle, 1);
    setTelemetryTableRowDouble(RowTelemetryFrameNumber, telemetryDataFrame.TelemetryFrameNumber, 0);
    setTelemetryTableRowDouble(RowVideoFrameNumber, telemetryDataFrame.VideoFrameNumber, 0);
    setTelemetryTableRowDouble(RowSessionTime, 0.001 * telemetryDataFrame.SessionTimeMs, 3);
    setTelemetryTableRowDouble(RowTelemetryFPS, telemetryDataFrame.TelemetryFPS, 0);
    setTelemetryTableRowDouble(RowVideoFPS, telemetryDataFrame.VideoFPS, 0);
    setTelemetryTableRowDoubleOrIncorrect(RowCalculatedRangefinderGPSLat, telemetryDataFrame.CalculatedRangefinderGPSLat, 6, INCORRECT_COORDINATE);
    setTelemetryTableRowDoubleOrIncorrect(RowCalculatedRangefinderGPSLon, telemetryDataFrame.CalculatedRangefinderGPSLon, 6, INCORRECT_COORDINATE);
    setTelemetryTableRowDoubleOrIncorrect(RowCalculatedRangefinderGPSHmsl, telemetryDataFrame.CalculatedRangefinderGPSHmsl, 1, INCORRECT_COORDINATE);
    setTelemetryTableRowDouble(RowCalculatedTrackedTargetSpeed, telemetryDataFrame.CalculatedTrackedTargetSpeed, 0);
    setTelemetryTableRowDouble(RowCalculatedTrackedTargetDirection, telemetryDataFrame.CalculatedTrackedTargetDirection, 0);
    setTelemetryTableRowDouble(RowCalculatedTrackedTargetGPSLat, telemetryDataFrame.CalculatedTrackedTargetGPSLat, 0);
    setTelemetryTableRowDouble(RowCalculatedTrackedTargetGPSLon, telemetryDataFrame.CalculatedTrackedTargetGPSLon, 0);
    setTelemetryTableRowDouble(RowCalculatedTrackedTargetGPSHmsl, telemetryDataFrame.CalculatedTrackedTargetGPSHmsl, 0);
    setTelemetryTableRowDouble(RowCalculatedGroundLevel, telemetryDataFrame.CalculatedGroundLevel, 1);
    setTelemetryTableRowDouble(RowOpticalSystem, telemetryDataFrame.OpticalSystemId, 0);
}

void DashboardWidget::onActivateCatapultClicked()
{
    bool needActivate = CommonWidgetUtils::showConfirmDialog(tr("Do you want to activate catapult?"), false);
    if (needActivate)
        emit activateCatapult();
}

void DashboardWidget::onTelemetryTableContextMenuRequested(const QPoint &pos)
{
    ApplicationSettings& applicationSettings = ApplicationSettings::Instance();

    auto visibleRowsStr = applicationSettings.VisibleTelemetryTableRows.value().split(",", QString::SkipEmptyParts);

    foreach (auto action, _checkableItems)
    {
        auto actionRowStr = action->data().toString();
        action->setChecked(visibleRowsStr.contains(actionRowStr));
    }

    _menu->exec(_telemetryTable->mapToGlobal(pos));

    visibleRowsStr.clear();
    foreach (auto action, _checkableItems)
        if (action->isCheckable())
        {
            if (action->isChecked())
                visibleRowsStr.append(action->data().toString());
        }

    applicationSettings.VisibleTelemetryTableRows = visibleRowsStr.join(",");

    updateItemsVisibility();
}

void DashboardWidget::updateItemsVisibility()
{
    ApplicationSettings& applicationSettings = ApplicationSettings::Instance();
    auto visibleRowsStr = applicationSettings.VisibleTelemetryTableRows.value().split(",", QString::SkipEmptyParts);

    for (int i = 0; i < TelemetryTableRow::RowLast; i++)
    {
        if (visibleRowsStr.contains(QString::number(i)))
            _telemetryTable->showRow(i);
        else
            _telemetryTable->hideRow(i);
    }

    _PFD->setVisible(visibleRowsStr.contains(PrimaryFlightDisplayActionId));
    _coordIndicator->setVisible(visibleRowsStr.contains(CoordIndicatorDisplayActionId));
}

void DashboardWidget::addParameter(DashboardWidget::TelemetryTableRow tableRow, const QString &name, QMenu *menu)
{
    auto itemName = new QTableWidgetItem(name);
    itemName->setToolTip(name);
    _telemetryTable->setItem(tableRow, 0, itemName);

    auto itemValue = new QTableWidgetItem("");
    itemValue->setTextAlignment(Qt::AlignRight);
    _telemetryTable->setItem(tableRow, 1, itemValue);

    auto action = CommonWidgetUtils::createCheckableMenuSingleAction(name, true, menu);
    action->setData(tableRow);

    _checkableItems.append(action);
}
