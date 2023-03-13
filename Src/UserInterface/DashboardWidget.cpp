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

    _licenseState = getAnimusLicenseState();

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

    _paramNames[RowUavRoll] = tr("UAV Roll");
    _paramNames[RowUavPitch] = tr("UAV Pitch");
    _paramNames[RowUavYaw] = tr("UAV Yaw");
    _paramNames[RowUavLatitude_GPS] = tr("Latitude");
    _paramNames[RowUavLongitude_GPS] = tr("Longitude");
    _paramNames[RowUavAltitude_GPS] = tr("Altitude (GPS)");
    _paramNames[RowUavAltitude_Barometric] = tr("Altitude (Barometric)");
    _paramNames[RowAirSpeed] = tr("Air Speed");
    _paramNames[RowGroundSpeed_GPS] = tr("Ground Speed");
    _paramNames[RowCourse_GPS] = tr("Course");
    _paramNames[RowCamRoll] = tr("Cam Roll");
    _paramNames[RowCamPitch] = tr("Cam Pitch");
    _paramNames[RowCamYaw] = tr("Cam Yaw");
    _paramNames[RowCamZoom] = tr("Cam Zoom");
    _paramNames[RowCamEncoderRoll] = tr("Cam Roll (Encoder)");
    _paramNames[RowCamEncoderPitch] = tr("Cam Pitch (Encoder)");
    _paramNames[RowCamEncoderYaw] = tr("Cam Yaw (Encoder)");
    if (applicationSettings.isLaserRangefinderLicensed())
    {
        _paramNames[RowRangefinderDistance] = tr("Distance");
        _paramNames[RowRangefinderTemperature] = tr("Laser Temperature");
        _paramNames[RowCalculatedRangefinderGPSLat] = tr("Latitude (Rangefinder)");
        _paramNames[RowCalculatedRangefinderGPSLon] = tr("Longitude (Rangefinder)");
        _paramNames[RowCalculatedRangefinderGPSHmsl] = tr("Altitude (Rangefinder)");
    }
    _paramNames[RowWindDirection] = tr("Wind Direction");
    _paramNames[RowWindSpeed] = tr("Wind Speed");
    _paramNames[RowAtmosphereTemperature] = tr("Atmosphere Temperature");
    _paramNames[RowAtmospherePressure] = tr("Atmosphere Pressure");
    _paramNames[RowStabilizedCenterX] = tr("Stabilized Center X");
    _paramNames[RowStabilizedCenterY] = tr("Stabilized Center Y");
    _paramNames[RowStabilizedRotationAngle] = tr("Stabilized Rotation Angle");
    _paramNames[RowTelemetryFrameNumber] = tr("Telemetry Frame");
    _paramNames[RowVideoFrameNumber] = tr("Video Frame");
    _paramNames[RowSessionTime] = tr("Session Time");
    _paramNames[RowVideoFPS] = tr("Video FPS");
    _paramNames[RowCalculatedGroundLevel] = tr("Ground Level");
    _paramNames[RowOpticalSystem] = tr("Optical System");

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

    QMapIterator<TelemetryTableRow, QString> paramName(_paramNames);
    while (paramName.hasNext())
    {
        paramName.next();
        TelemetryTableRow tableRow = paramName.key();
        QString paramName = _paramNames[tableRow];
        _telemetryTable->setItem(tableRow, 0, new QTableWidgetItem(paramName));
        auto item = new QTableWidgetItem("");
        item->setTextAlignment(Qt::AlignRight);
        _telemetryTable->setItem(tableRow, 1, item);

        auto action = CommonWidgetUtils::createCheckableMenuSingleAction(paramName, true, _menu);
        action->setData(tableRow);
    }
    _telemetryTable->resizeColumnsToContents();
    updateItemsVisibility();

    _btnCamConnectionState = new QPushButton(tr("Camera"), this);
    _btnCamConnectionState->setEnabled(false);
    _btnTelemetryConnectionState = new QPushButton(tr("Telemetry"), this);
    _btnTelemetryConnectionState->setEnabled(false);
    _btnRecordingState = new QPushButton(tr("Record"), this);
    _btnRecordingState->setEnabled(false);

    auto connectionsLayout = new QHBoxLayout();
    connectionsLayout->addWidget(_btnCamConnectionState, 0);
    connectionsLayout->addWidget(_btnTelemetryConnectionState, 0);
    connectionsLayout->addWidget(_btnRecordingState, 0);

    auto mainSplitter = new QSplitter(Qt::Vertical, this);

    mainSplitter->addWidget(_PFD);
    mainSplitter->addWidget(_telemetryTable);

    instrumentsLayout->addWidget(_coordIndicator, 0);
    instrumentsLayout->addWidget(mainSplitter, 1);
    instrumentsLayout->addWidget(btnCatapultLauncher, 0);
    instrumentsLayout->addLayout(connectionsLayout, 0);

    _camConnectionState = IndicatorTristateEnum::Off;
    _telemetryConnectionState = IndicatorTristateEnum::Off;
    _recordingState = IndicatorTristateEnum::Off;

    showCurrentStatus(IndicatorTristateEnum::Incative, IndicatorTristateEnum::Incative, IndicatorTristateEnum::Incative);
}

static const QString stateStyles[static_cast<int>(IndicatorTristateEnum::LastValue) + 1] = {
        QString("background-color: #31363b; border-color: #76797C; color: #eff0f1;"),  // Incative
        QString("background-color: #009900; border-color: #76797C; color: #eff0f1;"),  // On
        QString("background-color: #990000; border-color: #76797C; color: #eff0f1;"),  // Off
        QString("background-color: #000099; border-color: #76797C; color: #eff0f1;")   // Disabled
        };

void DashboardWidget::showCurrentStatus(IndicatorTristateEnum camConnectionState, IndicatorTristateEnum telemetryConnectionState, IndicatorTristateEnum recordingState)
{
    EnterProcStart("DashboardWidget::showCurrentStatus");

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
    setTelemetryTableRowDouble(RowVideoFPS, telemetryDataFrame.VideoFPS, 0);
    setTelemetryTableRowDoubleOrIncorrect(RowCalculatedRangefinderGPSLat, telemetryDataFrame.CalculatedRangefinderGPSLat, 6, INCORRECT_COORDINATE);
    setTelemetryTableRowDoubleOrIncorrect(RowCalculatedRangefinderGPSLon, telemetryDataFrame.CalculatedRangefinderGPSLon, 6, INCORRECT_COORDINATE);
    setTelemetryTableRowDoubleOrIncorrect(RowCalculatedRangefinderGPSHmsl, telemetryDataFrame.CalculatedRangefinderGPSHmsl, 1, INCORRECT_COORDINATE);
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

    foreach (auto action, _menu->actions())
    {
        auto actionRowStr = action->data().toString();
        action->setChecked(visibleRowsStr.contains(actionRowStr));
    }

    _menu->exec(_telemetryTable->mapToGlobal(pos));

    visibleRowsStr.clear();
    foreach (auto action, _menu->actions())
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
