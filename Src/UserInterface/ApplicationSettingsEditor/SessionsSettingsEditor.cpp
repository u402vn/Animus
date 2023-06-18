#include "SessionsSettingsEditor.h"
#include <QCheckBox>
#include <QLineEdit>
#include <QGridLayout>
#include <QSerialPortInfo>
#include <QCheckBox>
#include "ApplicationSettings.h"
#include "EnterProc.h"
#include "UserInterface/FilePathSelector.h"
#include "UserInterface/ApplicationSettingsEditor/CameraSettingsEditor.h"
#include "UserInterface/ApplicationSettingsEditor/NetworkInformationWidget.h"
#include "UserInterface/ApplicationSettingsEditor/NetworkAddressEditor.h"
#include "UserInterface/ConstantNames.h"
#include "Common/CommonWidgets.h"

const int SESSIONS_TAB_FIRST_COLUMN_WIDTH = 200;

SpoilerGrid *makeSessionsSpoilerGrid(const QString &title, QWidget *parent)
{
    auto spoilerGrid = new SpoilerGrid(title, parent);

    auto layout = spoilerGrid->gridLayout();
    layout->setColumnMinimumWidth(0, SESSIONS_TAB_FIRST_COLUMN_WIDTH);
    layout->setColumnStretch(0, 0);
    layout->setColumnStretch(1, 0);
    layout->setColumnStretch(2, 1);
    layout->setColumnStretch(3, 2);
    //layout->setColumnMinimumWidth(3, SESSIONS_TAB_FIRST_COLUMN_WIDTH);
    layout->setHorizontalSpacing(4);

    return spoilerGrid;
}

SessionsSettingsEditor::SessionsSettingsEditor(QWidget *parent):
    QScrollArea(parent),
    _association(this)
{
    EnterProcStart("InterfaceSettingsEditor::InterfaceSettingsEditor");
    ApplicationSettings& applicationSettings = ApplicationSettings::Instance();

    auto videoSourceSettingsGrid = CommonWidgetUtils::createGridLayoutForScrollArea(this);

    const int ROW_SEPARATOR_HEIGHT = 20;

    // Init Computer Info

    auto fpsSessions = new FilePathSelector(this, tr("Sessions Folder"), tr("Select Sessions Folder"), QString(""));
    fpsSessions->setLabelWidth(SESSIONS_TAB_FIRST_COLUMN_WIDTH);
    fpsSessions->setUseFolder(true);

    auto chkLogFolderCleanup = new QCheckBox(tr("Log Folder Cleanup"), this);
    auto sbLogFolderMaxSizeMb = CommonWidgetUtils::createRangeSpinbox(this, 10, 1000);
    auto lblLogFolderMaxSizeMb = new QLabel(tr("Mb (Maximal Log Folder Size)"), this);

    auto netInfo = new NetworkInformationWidget(this);

    auto computerInfoLayout = new QGridLayout();

    int row = 0;
    computerInfoLayout->addWidget(fpsSessions,                row, 0, 1, 4);
    row++;

    computerInfoLayout->addWidget(chkLogFolderCleanup,        row, 0, 1, 1);
    computerInfoLayout->addWidget(sbLogFolderMaxSizeMb,       row, 1, 1, 1, Qt::AlignLeft);
    computerInfoLayout->addWidget(lblLogFolderMaxSizeMb,      row, 2, 1, 1, Qt::AlignLeft);
    row++;

    computerInfoLayout->addWidget(netInfo,                    row, 0, 1, 4);
    row++;

    auto spoilerComputerInfo = new Spoiler(tr("Computer Info"), this);
    spoilerComputerInfo->setContentLayout(computerInfoLayout);

    // Init Video record section

    auto lblVideoFileFrameCount = new QLabel(tr("Frame Number in Video File"), this);
    auto cbVideoFileFrameCount = new QComboBoxExt(this);
    cbVideoFileFrameCount->addItem(tr("Unlimited"), -1);
    for (quint32 i = 1; i < 6; i++)
    {
        quint32 frameNaumber = VIDEO_FRAMES_PER_FILE_DEFAULT * i / 2;
        cbVideoFileFrameCount->addItem(QString::number(frameNaumber), frameNaumber);
    }

    auto lblVideoFileQuality = new QLabel(tr("Video File Quality"), this);
    auto sbVideoFileQuality = CommonWidgetUtils::createRangeSpinbox(this, 30, 100);

    auto lblVideoRecord  = new QLabel(tr("Video Record"), this);

    auto chkOVRDisplayTelemetry = new QCheckBox(tr("Telemetry On Video"), this);
    auto chkOVRDisplayTargetRectangle = new QCheckBox(tr("Target Rectangle On Video"), this);

    auto cbOVRGimbalIndicatorType = new QComboBoxExt(this, ConstantNames::OVRGimbalIndicatorTypeCaptions());

    auto lblOVRGimbalIndicatorSize = new QLabel(tr("Gimbal Indicator Size"), this);
    auto sbOVRGimbalIndicatorSize = CommonWidgetUtils::createRangeSpinbox(this, 20, 200);

    auto lblOVRTelemetryIndicatorFontSize = new QLabel(tr("Telemetry Indicator Font Size"), this);
    auto sbOVRTelemetryIndicatorFontSize = CommonWidgetUtils::createRangeSpinbox(this, 2, 40);

    auto spoilerVideoRecord = makeSessionsSpoilerGrid(tr("Video Record"), this);
    auto videoRecordLayout = spoilerVideoRecord->gridLayout();

    row = 0;

    videoRecordLayout->addWidget(lblVideoFileFrameCount,                       row, 0, 1, 1);
    videoRecordLayout->addWidget(cbVideoFileFrameCount,                        row, 1, 1, 1);
    row++;

    videoRecordLayout->addWidget(lblVideoFileQuality,                          row, 0, 1, 1);
    videoRecordLayout->addWidget(sbVideoFileQuality,                           row, 1, 1, 1);
    row++;

    videoRecordLayout->addWidget(lblVideoRecord,                               row, 0, 1, 1);
    videoRecordLayout->addWidget(chkOVRDisplayTelemetry,                       row, 1, 1, 4);
    row++;

    videoRecordLayout->addWidget(chkOVRDisplayTargetRectangle,                  row, 1, 1, 4);
    row++;

    videoRecordLayout->addWidget(cbOVRGimbalIndicatorType,                      row, 1, 1, 4);
    row++;

    videoRecordLayout->addWidget(lblOVRGimbalIndicatorSize,                     row, 0, 1, 1);
    videoRecordLayout->addWidget(sbOVRGimbalIndicatorSize,                      row, 1, 1, 1);
    row++;

    videoRecordLayout->addWidget(lblOVRTelemetryIndicatorFontSize,              row, 0, 1, 1);
    videoRecordLayout->addWidget(sbOVRTelemetryIndicatorFontSize,               row, 1, 1, 1);
    row++;

    // Init Data Reception Section

    auto gbUAVTelemetrySourceType = new QButtonGroupExt(this);
    auto rbUAVTelemetryUDPChannel = gbUAVTelemetrySourceType->appendButton(tr("UDP Channel"),  UAVTelemetrySourceTypes::UDPChannel);
    auto rbUAVTelemetryEmulator   = gbUAVTelemetrySourceType->appendButton(tr("Emulator"),     UAVTelemetrySourceTypes::Emulator);


    auto cbUAVTelemetryFormat = new QComboBoxExt(this, ConstantNames::UAVTelemetryFormatCaptions());

    auto edUAVTelemetryUDPPort = CommonWidgetUtils::createPortEditor(this);

    auto chkExtTelemetryUDP = new QCheckBox(tr("Extended Telemetry UDP"), this);
    auto edExtTelemetryUDPPort = CommonWidgetUtils::createPortEditor(this);

    auto chkCamTelemetryUDP = new QCheckBox(tr("Camera Telemetry UDP"), this);
    auto edCamTelemetryUDPPort = CommonWidgetUtils::createPortEditor(this);

    auto lblCurrentCamera = new QLabel(tr("Camera"), this);
    _cbCurrentCamera = CameraSettingsEditor::createCamListCombo(this);
    connect(_cbCurrentCamera, static_cast<void(QComboBoxExt::*)(int)>(&QComboBoxExt::currentIndexChanged), this, &SessionsSettingsEditor::onCurrentCameraChanged);
    _lblCurrentCameraInfo = new QLabel(this);

    auto lblTelemetryLag = new QLabel(tr("Telemetry Lag (ms)"), this);
    auto sbTelemetryLag = CommonWidgetUtils::createRangeSpinbox(this, 0, 5000);

    auto lblCameraTelemetryLag = new QLabel(tr("Camera Telemetry Lag (ms)"), this);
    auto sbCameraTelemetryLag = CommonWidgetUtils::createRangeSpinbox(this, 0, 5000);

    auto spoilerDataReception = makeSessionsSpoilerGrid(tr("Data Reception"), this);
    auto dataReceptionLayout = spoilerDataReception->gridLayout();

    dataReceptionLayout->addWidget(CommonWidgetUtils::createSeparator(this),     row, 0, 1, 4);
    row++;

    dataReceptionLayout->addWidget(rbUAVTelemetryUDPChannel,        row, 0, 1, 1);
    dataReceptionLayout->addWidget(cbUAVTelemetryFormat,            row, 1, 1, 1);
    dataReceptionLayout->addWidget(edUAVTelemetryUDPPort,           row, 2, 1, 1);
    row++;

    dataReceptionLayout->addWidget(rbUAVTelemetryEmulator,          row, 0, 1, 1);
    row++;

    dataReceptionLayout->addWidget(CommonWidgetUtils::createSeparator(this),     row, 0, 1, 4);
    row++;

    dataReceptionLayout->addWidget(chkExtTelemetryUDP,              row, 0, 1, 1);
    dataReceptionLayout->addWidget(edExtTelemetryUDPPort,           row, 2, 1, 1);
    row++;

    dataReceptionLayout->addWidget(chkCamTelemetryUDP,              row, 0, 1, 1);
    dataReceptionLayout->addWidget(edCamTelemetryUDPPort,           row, 2, 1, 1);
    row++;

    dataReceptionLayout->addWidget(lblCurrentCamera,                row, 0, 1, 1, Qt::AlignTop | Qt::AlignLeft);
    dataReceptionLayout->addWidget(_cbCurrentCamera,                row, 1, 1, 1, Qt::AlignTop | Qt::AlignLeft);
    dataReceptionLayout->addWidget(_lblCurrentCameraInfo,           row, 2, 1, 1, Qt::AlignTop | Qt::AlignLeft);
    dataReceptionLayout->setRowMinimumHeight(row++, 3 * ROW_SEPARATOR_HEIGHT);
    row++;

    dataReceptionLayout->addWidget(lblTelemetryLag,                 row, 0, 1, 1);
    dataReceptionLayout->addWidget(sbTelemetryLag,                  row, 1, 1, 1);
    row++;

    dataReceptionLayout->addWidget(lblCameraTelemetryLag,           row, 0, 1, 1);
    dataReceptionLayout->addWidget(sbCameraTelemetryLag,            row, 1, 1, 1);
    row++;

    // Init Data Sending Section

    auto gbCmdTransport = new QButtonGroupExt(this);
    auto rbCmdUdpTransport    = gbCmdTransport->appendButton(tr("Commands over UDP/IP"),        CommandTransports::UDP);
    auto rbCmdSerialTransport = gbCmdTransport->appendButton(tr("Commands over a serial port"), CommandTransports::Serial);
    auto naeCommandUDP = new NetworkAddressEditor(this, &_association, &applicationSettings.CommandUDPAddress, &applicationSettings.CommandUDPPort);

    auto cbCmdSerialPortName = new QComboBoxExt(this);
    foreach (const QSerialPortInfo &info, QSerialPortInfo::availablePorts())
        cbCmdSerialPortName->addItem(info.portName(), info.portName());

    auto lblCommandSendingInterval = new QLabel(tr("Command Sending Interval"), this);
    auto sbCommandSendingInterval = CommonWidgetUtils::createRangeSpinbox(this, 10, 1000);

    auto lblCommandProtocol = new QLabel(tr("Command Protocol:"), this);

    auto cbCommandProtocol = new QComboBoxExt(this, ConstantNames::CommandProtocolCaptions());

    auto spoilerDataSending = makeSessionsSpoilerGrid(tr("Data Sending"), this);
    auto dataSendingLayout = spoilerDataSending->gridLayout();

    dataSendingLayout->addWidget(rbCmdUdpTransport,               row, 0, 1, 1);
    dataSendingLayout->addWidget(naeCommandUDP,                   row, 1, 1, 2);
    row++;

    dataSendingLayout->addWidget(rbCmdSerialTransport,            row, 0, 1, 1);
    dataSendingLayout->addWidget(cbCmdSerialPortName,             row, 1, 1, 1);
    row++;

    dataSendingLayout->addWidget(CommonWidgetUtils::createSeparator(this),     row, 0, 1, 4);
    row++;

    dataSendingLayout->addWidget(lblCommandSendingInterval,       row, 0, 1, 1);
    dataSendingLayout->addWidget(sbCommandSendingInterval,        row, 1, 1, 1);
    row++;

    dataSendingLayout->addWidget(lblCommandProtocol,              row, 0, 1, 1);
    dataSendingLayout->addWidget(cbCommandProtocol,               row, 1, 1, 1);
    row++;

    // Init Data Forwarding section
    SpoilerGrid *spoilerDataForwarding = nullptr;
    if (applicationSettings.isDataForwardingLicensed())
    {
        auto chkEnableForwarding = new QCheckBox(tr("Forwarding"), this);
        auto naeVideoForwarding = new NetworkAddressEditor(this, &_association, &applicationSettings.VideoForwardingAddress, &applicationSettings.VideoForwardingPort);
        auto naeTelemetryForwarding = new NetworkAddressEditor(this, &_association, &applicationSettings.TelemetryForwardingAddress, &applicationSettings.TelemetryForwardingPort);

        auto lblExternalDataConsole = new QLabel(tr("External Data Console (UDP)"), this);
        auto naeExternalDataConsole = new NetworkAddressEditor(this, &_association, nullptr, &applicationSettings.ExternalDataConsoleUDPPort);

        spoilerDataForwarding = makeSessionsSpoilerGrid(tr("Data Forwarding"), this);
        auto dataForwardingLayout = spoilerDataForwarding->gridLayout();

        dataForwardingLayout->addWidget(chkEnableForwarding,             row, 0, 1, 1);
        dataForwardingLayout->addWidget(naeVideoForwarding,              row, 1, 1, 2);
        row++;
        dataForwardingLayout->addWidget(naeTelemetryForwarding,          row, 1, 1, 2);
        row++;

        dataForwardingLayout->addWidget(lblExternalDataConsole,          row, 0, 1, 1);
        dataForwardingLayout->addWidget(naeExternalDataConsole,          row, 1, 1, 1);
        row++;

        _association.addBinding(&applicationSettings.EnableForwarding,                  chkEnableForwarding);
    }

    // Init Artillery Spotter Tools

    SpoilerGrid *spoilerArtillerySpotter = nullptr;
    if (applicationSettings.isArtillerySpotterLicensed())
    {
        auto chkEnableArtilleryMountNotification = new QCheckBox(tr("Enable Artillery Mount Notification"), this);

        auto lblArtilleryMount = new QLabel(tr("Artillery Mount (TCP)"), this);
        auto naeArtilleryMount = new NetworkAddressEditor(this, &_association, &applicationSettings.ArtilleryMountAddress, &applicationSettings.ArtilleryMountTCPPort);

        spoilerArtillerySpotter = makeSessionsSpoilerGrid(tr("Artillery Spotter"), this);
        auto artillerySpotterLayout = spoilerArtillerySpotter->gridLayout();
        row = 0;

        artillerySpotterLayout->addWidget(chkEnableArtilleryMountNotification,  row, 0, 1, 1);
        row++;

        artillerySpotterLayout->addWidget(lblArtilleryMount,                    row, 0, 1, 1);
        artillerySpotterLayout->addWidget(naeArtilleryMount,                    row, 1, 1, 1);
        row++;

        _association.addBinding(&applicationSettings.EnableArtilleryMountNotification,  chkEnableArtilleryMountNotification);
    }

    // Init Other section

    SpoilerGrid *spoilerOther = nullptr;
    if (applicationSettings.isCatapultLicensed())
    {
        auto chkUseCatapultLauncher = new QCheckBox(tr("Catapult Launcher"), this);
        auto cbCatapultSerialPortName = new QComboBoxExt(this);
        foreach (const QSerialPortInfo &info, QSerialPortInfo::availablePorts())
            cbCatapultSerialPortName->addItem(info.portName(), info.portName());
        auto edCatapultCommand = new QLineEdit(this);

        spoilerOther = makeSessionsSpoilerGrid(tr("Other"), this);
        auto otherLayout = spoilerOther->gridLayout();
        row = 0;

        otherLayout->addWidget(chkUseCatapultLauncher,          row, 0, 1, 1);
        otherLayout->addWidget(cbCatapultSerialPortName,        row, 1, 1, 1);
        otherLayout->addWidget(edCatapultCommand,               row, 2, 1, 2);
        row++;

        _association.addBinding(&applicationSettings.UseCatapultLauncher,               chkUseCatapultLauncher);
        _association.addBinding(&applicationSettings.CatapultSerialPortName,            cbCatapultSerialPortName);
        _association.addBinding(&applicationSettings.CatapultCommand,                   edCatapultCommand);
    }

    // Init Tracker section

    auto cmbObjectTrackerType = new QComboBoxExt(this, ConstantNames::ObjectTrackerTypeCaptions());
    auto chkShowExternalTrackerRectangle = new QCheckBox(tr("Show External Tracker Cursor"), this);

    auto lblTrackerCommandUDP = new QLabel(tr("External Tracker Commands (UDP/IP)"), this);
    auto naeTrackerCommandUDP = new NetworkAddressEditor(this, &_association, &applicationSettings.TrackerCommandUDPAddress, &applicationSettings.TrackerCommandUDPPort);

    auto lblTrackerTelemetryUDP = new QLabel(tr("External Tracker Telemetry (UDP/IP)"), this);
    auto naeTrackerTelemetryUDP = new NetworkAddressEditor(this, &_association, &applicationSettings.TrackerTelemetryUDPAddress, &applicationSettings.TrackerTelemetryUDPPort);


    auto spoilerTracker = makeSessionsSpoilerGrid(tr("Tracker"), this);
    auto trackerLayout = spoilerTracker->gridLayout();
    row = 0;

    trackerLayout->addWidget(cmbObjectTrackerType,                      row, 0, 1, 1);
    trackerLayout->addWidget(chkShowExternalTrackerRectangle,           row, 1, 1, 2);
    row++;

    trackerLayout->addWidget(lblTrackerCommandUDP,                      row, 0, 1, 1);
    trackerLayout->addWidget(naeTrackerCommandUDP,                      row, 1, 1, 1);
    row++;


    trackerLayout->addWidget(lblTrackerTelemetryUDP,                    row, 0, 1, 1);
    trackerLayout->addWidget(naeTrackerTelemetryUDP,                    row, 1, 1, 1);
    row++;

    // Fill main layout
    row = 0;

    videoSourceSettingsGrid->addWidget(spoilerComputerInfo,             row, 0, 1, 4);
    row++;

    videoSourceSettingsGrid->addWidget(spoilerVideoRecord,              row, 0, 1, 4);
    row++;

    videoSourceSettingsGrid->addWidget(spoilerDataReception,            row, 0, 1, 4);
    row++;

    videoSourceSettingsGrid->addWidget(spoilerDataSending,              row, 0, 1, 4);
    row++;

    if (spoilerDataForwarding != nullptr)
    {
        videoSourceSettingsGrid->addWidget(spoilerDataForwarding,       row, 0, 1, 4);
        row++;
    }

    videoSourceSettingsGrid->addWidget(spoilerTracker,                  row, 0, 1, 4);
    row++;

    if (spoilerArtillerySpotter != nullptr)
    {
        videoSourceSettingsGrid->addWidget(spoilerArtillerySpotter,     row, 0, 1, 4);
        row++;
    }

    if (spoilerOther != nullptr)
    {
        videoSourceSettingsGrid->addWidget(spoilerOther,                row, 0, 1, 4);
        row++;
    }

    videoSourceSettingsGrid->setRowStretch(row++, 1);

    _association.addBinding(&applicationSettings.SessionsFolder,                    fpsSessions);
    _association.addBinding(&applicationSettings.LogFolderCleanup,                  chkLogFolderCleanup);
    _association.addBinding(&applicationSettings.LogFolderMaxSizeMb,                sbLogFolderMaxSizeMb);
    _association.addBinding(&applicationSettings.VideoFileFrameCount,               cbVideoFileFrameCount);
    _association.addBinding(&applicationSettings.VideoFileQuality,                  sbVideoFileQuality);
    _association.addBinding(&applicationSettings.OVRDisplayTelemetry,               chkOVRDisplayTelemetry);
    _association.addBinding(&applicationSettings.OVRDisplayTargetRectangle,         chkOVRDisplayTargetRectangle);
    _association.addBinding(&applicationSettings.OVRGimbalIndicatorType,            cbOVRGimbalIndicatorType);
    _association.addBinding(&applicationSettings.OVRGimbalIndicatorSize,            sbOVRGimbalIndicatorSize);
    _association.addBinding(&applicationSettings.OVRTelemetryIndicatorFontSize,     sbOVRTelemetryIndicatorFontSize);
    _association.addBinding(&applicationSettings.UAVTelemetrySourceType,            gbUAVTelemetrySourceType);
    _association.addBinding(&applicationSettings.TelemetryDataFormat,               cbUAVTelemetryFormat);
    _association.addBinding(&applicationSettings.UAVTelemetryUDPPort,               edUAVTelemetryUDPPort);
    _association.addBinding(&applicationSettings.UseExtTelemetryUDP,                chkExtTelemetryUDP);
    _association.addBinding(&applicationSettings.ExtTelemetryUDPPort,               edExtTelemetryUDPPort);
    _association.addBinding(&applicationSettings.UseCamTelemetryUDP,                chkCamTelemetryUDP);
    _association.addBinding(&applicationSettings.CamTelemetryUDPPort,               edCamTelemetryUDPPort);

    _association.addBinding(&applicationSettings.VideoLagFromTelemetry,             sbTelemetryLag);
    _association.addBinding(&applicationSettings.VideoLagFromCameraTelemetry,       sbCameraTelemetryLag);

    _association.addBinding(&applicationSettings.InstalledCameraIndex,              _cbCurrentCamera);
    _association.addBinding(&applicationSettings.CommandSendingInterval,            sbCommandSendingInterval);
    _association.addBinding(&applicationSettings.CommandTransport,                  gbCmdTransport);
    _association.addBinding(&applicationSettings.CommandSerialPortName,             cbCmdSerialPortName);
    _association.addBinding(&applicationSettings.CommandProtocol,                   cbCommandProtocol);

    _association.addBinding(&applicationSettings.ObjectTrackerType,                 cmbObjectTrackerType);
    _association.addBinding(&applicationSettings.ShowExternalTrackerRectangle,      chkShowExternalTrackerRectangle);

}

void SessionsSettingsEditor::onCurrentCameraChanged(int index)
{
    Q_UNUSED(index)
    auto camIdx = _cbCurrentCamera->currentData().toInt();
    _lblCurrentCameraInfo->setText(CameraSettingsEditor::getCameraInfo(camIdx));
}

void SessionsSettingsEditor::loadSettings()
{
    EnterProcStart("SessionsSettingsEditor::loadSettings");
    _association.toEditor();

    onCurrentCameraChanged(0);
}

void SessionsSettingsEditor::saveSettings()
{
    EnterProcStart("SessionsSettingsEditor::saveSettings");
    _association.fromEditor();
}
