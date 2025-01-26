#include "mainwindow.h"
#include <QtCore>
#include <QTime>
#include <QTabWidget>
#include <QSizePolicy>
#include <QScrollBar>
#include <QPushButton>
#include <QDesktopWidget>
#include <QStyle>
#include <QFontDatabase>
#include <QWindow>
#include <QApplication>
#include "UserInterface/Forms/ApplicationSettingsEditor.h"
#include "UserInterface/Forms/SessionSelectorWidget.h"
#include "VideoRecorder/PartitionedVideoRecorder.h"
#include "Common/CommonWidgets.h"
#include "Common/CommonUtils.h"
#include "ApplicationSettings.h"
#include "EnterProc.h"

MainWindow::MainWindow(QWidget *parent) : QWidget(parent)
{
    qRegisterMetaType<TelemetryDataFrame>("TelemetryDataFrame");

    ApplicationSettings& applicationSettings = ApplicationSettings::Instance();
    auto cameraSettings = applicationSettings.installedCameraSettings();
    auto camPreferences = applicationSettings.getCurrentCamAssemblyPreferences();

    _useMinimalisticDesign = applicationSettings.MinimalisticDesign;

    _dataStorage = new TelemetryDataStorage(this, applicationSettings.SessionsFolder,
                                            applicationSettings.VideoFileFrameCount, applicationSettings.VideoFileQuality,
                                            applicationSettings.OVRDisplayTelemetry,
                                            applicationSettings.OVRTelemetryIndicatorFontSize,
                                            applicationSettings.OVRTelemetryTimeFormat,
                                            applicationSettings.OVRDisplayTargetRectangle,
                                            applicationSettings.OVRGimbalIndicatorType,
                                            applicationSettings.OVRGimbalIndicatorAngles,
                                            applicationSettings.OVRGimbalIndicatorSize,
                                            applicationSettings.isLaserRangefinderLicensed());
    connect(_dataStorage, &TelemetryDataStorage::workModeChanged, this, &MainWindow::workModeChanged);
    connect(_dataStorage, &TelemetryDataStorage::storedDataReceived, this, &MainWindow::storedDataReceived);

    _hardwareLink = new HardwareLink(this);
    connect(_hardwareLink, &HardwareLink::dataReceived, this, &MainWindow::hardwareLinkDataReceived);
    connect(_hardwareLink, &HardwareLink::onClientCommandSent, _dataStorage, &TelemetryDataStorage::onClientCommandSent);


    _artillerySpotter = new ArtillerySpotter(this);
    if (applicationSettings.EnableArtilleryMountNotification)
    {
        _artillerySpotter->openSocket(QHostAddress(applicationSettings.ArtilleryMountAddress), applicationSettings.ArtilleryMountTCPPort);
        connect(_artillerySpotter, &ArtillerySpotter::onArtillerySpotterDataExchange, _dataStorage, &TelemetryDataStorage::onArtillerySpotterDataExchange);
    }

    auto heightMapContainer = new HeightMapContainer(this, applicationSettings.DatabaseHeightMap);
    //???? get from map tile container

    auto coordinateCalculator = new CoordinateCalculator(this, heightMapContainer);

    _imageProcessor = new ImageProcessor(this, coordinateCalculator,
                                         cameraSettings->opticalDeviceSetting(1)->UseVerticalFrameMirrororing, //todo set array for all optical systems [1, 2, 3]
                                         applicationSettings.ObjectTrackerType);
    _imageProcessor->setStabilizationType(applicationSettings.VideoStabilizationType);

    connect(_hardwareLink, &HardwareLink::onHardwareLinkStateChanged, this, &MainWindow::onHardwareLinkStateChanged);
    connect(_imageProcessor, &ImageProcessor::onDataProcessed, this, &MainWindow::onDataReceived);
    connect(_imageProcessor, &ImageProcessor::onDataProcessed, _dataStorage, &TelemetryDataStorage::onDataReceived);

    _voiceInformant = new VoiceInformant(this);
    _voiceInformant->setEnabled(applicationSettings.SoundEffectsAllowed);
    _voiceInformant->setVolume(applicationSettings.SoundLevel);

    _automaticTracer = new AutomaticTracer(this, camPreferences);
    connect(_automaticTracer, &AutomaticTracer::onTracerModeChanged, _hardwareLink, &HardwareLink::onTracerModeChanged);

    _automaticPatrol = new AutomaticPatrol(this);

    initWidgets();
    initHidController(camPreferences);
    //todo: move to settings

    TelemetryDataFrame telemetryFrame;
    telemetryFrame.VideoFrameNumber = 1;
    telemetryFrame.UavLatitude_GPS =   54;
    telemetryFrame.UavLongitude_GPS =   28;
    telemetryFrame.UavAltitude_GPS = 100;
    telemetryFrame.CamPitch = cameraSettings->FixedPosBeginingPitch;
    telemetryFrame.CamRoll =  cameraSettings->FixedPosBeginingRoll;
    //showTelemetryDataFrame(telemetryFrame);

    WorldGPSCoord uavCoord(telemetryFrame.UavLatitude_GPS, telemetryFrame.UavLongitude_GPS, telemetryFrame.UavAltitude_GPS);
    _mapView->setViewCenter(uavCoord);

    _voiceInformant->sayMessage(VoiceMessage::AnumusActivated);
    _hardwareLink->openVideoSource();
}

MainWindow::~MainWindow()
{
    //don't change order
    delete _videoWidget;
    _videoWidget = nullptr;
    delete _imageProcessor;
    _imageProcessor = nullptr;
    delete _hardwareLink;
    _hardwareLink = nullptr;
    delete _dataConsole;
    _dataConsole = nullptr;
    delete _emulatorConsole;
    _emulatorConsole = nullptr;
}

void MainWindow::SetPlayStatus(MainWindow::PlayStatus status)
{
    EnterProcStart("MainWindow::SetPlayStatus");

    _playHistoryButton->setChecked(false);
    _pauseButton->setChecked(false);
    _playRealtimeButton->setChecked(false);
    _playTimer->stop();

    switch (status)
    {
    case PlayHistory:
        _playTimer->start(100);
        _playHistoryButton->setChecked(true);
        break;
    case Pause:
        _pauseButton->setChecked(true);
        break;
    case PlayRealtime:
        _playRealtimeButton->setChecked(true);
        break;
    default:
        break;
    }

    _playStatus = status;
}

void SetSplitterSizes(QSplitter *splitter, int size1, int size2, int strech1, int strech12)
{
    QList<int> list({size1, size2});
    splitter->setSizes(list);
    splitter->setStretchFactor(0, strech1);
    splitter->setStretchFactor(1, strech12);
}

void addWidgetToTab(QTabWidget *tabTools, QWidget *widget, const QIcon& icon, const QString &toolTip, bool isVisible)
{
    if (widget == nullptr)
        return;
    const QString NO_CAPTION = "";
    int index = tabTools->count();
    int tabIdx = tabTools->insertTab(index, widget, icon, NO_CAPTION);
    tabTools->tabBar()->setTabToolTip(index, toolTip);
    if (!isVisible)
        tabTools->removeTab(tabIdx);
}

void MainWindow::showModeSpecificWidgets(bool showCameraTab, bool showInstrumentsTab, bool showMarkersTab, bool showBombingTab,
                                         bool showPatrolTab, bool showAntennaTab, bool showTimeScale)
{
    EnterProcStart("MainWindow::showModeSpecificWidgets");

    ApplicationSettings& applicationSettings = ApplicationSettings::Instance();

    auto selectedTab = _tabTools->currentWidget();
    _tabTools->clear();

    addWidgetToTab(_tabTools, _camControlsWidget, QIcon(":/tab_cam.png"), tr("Camera"), showCameraTab);
    if (applicationSettings.isBombingTabLicensed() )
        addWidgetToTab(_tabTools, _bombingWidget, QIcon(":/tab_bomb.png"), tr("Bombing"), showBombingTab);
    else if (applicationSettings.isTargetTabLicensed())
        addWidgetToTab(_tabTools, _bombingWidget, QIcon(":/tab_marker.png"), tr("Targets"), showBombingTab);
    addWidgetToTab(_tabTools, _dashboardWidget, QIcon(":/tab_instr.png"), tr("Tools"), showInstrumentsTab);
    addWidgetToTab(_tabTools, _markerListWidget, QIcon(":/tab_marker.png"), tr("Markers"), showMarkersTab);
    addWidgetToTab(_tabTools, _patrolWidget, QIcon(":/tab_patrol.png"), tr("Patrol"), showPatrolTab);

    if (applicationSettings.isAntennaLicensed())
        addWidgetToTab(_tabTools, _antennaWidget, QIcon(":/tab_antenna.png"), tr("Antenna"), showAntennaTab);

    int selectedTabIndex = _tabTools->indexOf(selectedTab);
    if (selectedTabIndex >= 0)
        _tabTools->setCurrentWidget(selectedTab);

    if (!_useMinimalisticDesign)
    {
        _timeSlider->setVisible(showTimeScale);
        _playHistoryButton->setVisible(showTimeScale);
        _playRealtimeButton->setVisible(showCameraTab);
        if (showTimeScale)
            _timeControlsLayout->removeItem(_timeSpacerItem);
        else
            _timeControlsLayout->insertItem(2, _timeSpacerItem);
    }
    updateDashboardStatuses();
}

void MainWindow::updateDashboardStatuses()
{
    EnterProcStart("MainWindow::updateDashboardStatuses");

    IndicatorTristateEnum camConnectionState = IndicatorTristateEnum::Incative;
    IndicatorTristateEnum telemetryConnectionState = IndicatorTristateEnum::Incative;
    IndicatorTristateEnum recordingState = IndicatorTristateEnum::Incative;

    switch(_dataStorage->getWorkMode())
    {
    case TelemetryDataStorage::WorkMode::DisplayOnly:
        camConnectionState = _hardwareLink->camConnectionOn() ? IndicatorTristateEnum::On : IndicatorTristateEnum::Off;
        telemetryConnectionState = _hardwareLink->telemetryConnectionOn() ? IndicatorTristateEnum::On : IndicatorTristateEnum::Off;
        recordingState = IndicatorTristateEnum::Incative;
        break;
    case TelemetryDataStorage::WorkMode::PlayStored:
        camConnectionState = IndicatorTristateEnum::Incative;
        telemetryConnectionState = IndicatorTristateEnum::Incative;
        recordingState = IndicatorTristateEnum::Incative;
        break;
    case TelemetryDataStorage::WorkMode::RecordAndDisplay:
        camConnectionState = _hardwareLink->camConnectionOn() ? IndicatorTristateEnum::On : IndicatorTristateEnum::Off;
        telemetryConnectionState = _hardwareLink->telemetryConnectionOn() ? IndicatorTristateEnum::On : IndicatorTristateEnum::Off;
        recordingState = IndicatorTristateEnum::On;
        break;
    }

    _connectionsIndicator->showCurrentStatus(camConnectionState, telemetryConnectionState, recordingState);
}

void MainWindow::initForTwoDisplays(int camDisplayId, int mapDisplayId)
{
    ApplicationSettings& applicationSettings = ApplicationSettings::Instance();

    _changeVideo2MapButton->setVisible(false);

    _mainSplitter->insertWidget(0, _videoWidget);
    SetSplitterSizes(_mainSplitter, applicationSettings.MainFormViewPanelWidth, applicationSettings.MainFormToolPanelWidth, 1, 0);

    QRect camScreenRes = QApplication::desktop()->screenGeometry(camDisplayId);
    QRect mapScreenRes = QApplication::desktop()->screenGeometry(mapDisplayId);

    this->move(camScreenRes.x(), camScreenRes.y());
    _mapView->move(mapScreenRes.x(), mapScreenRes.y());

    _mapView->setGeometry(mapScreenRes);
    this->setGeometry(camScreenRes);

    _mapView->showFullScreen();
    this->showFullScreen();
}

void MainWindow::initForSingleDisplay()
{
    ApplicationSettings& applicationSettings = ApplicationSettings::Instance();

    QSize widgetRectSize(1024, 768);
    QRect widgetRect = QStyle::alignedRect(Qt::LeftToRight, Qt::AlignCenter, widgetRectSize, CommonWidgetUtils::getDesktopAvailableGeometry());
    this->setGeometry(widgetRect);

    this->setWindowState(Qt::WindowMaximized); //Qt::WindowFullScreen

    _mapView->setParent(this);

    if (applicationSettings.MainFormViewPanelShowVideo)
    {
        _mainSplitter->insertWidget(0, _videoWidget);
        _rightPanel->insertWidget(0, _mapView);
    }
    else
    {
        _mainSplitter->insertWidget(0, _mapView);
        _rightPanel->insertWidget(0, _videoWidget);
    }

    SetSplitterSizes(_rightPanel, 240, 740, 0, 1);
    SetSplitterSizes(_mainSplitter, applicationSettings.MainFormViewPanelWidth, applicationSettings.MainFormToolPanelWidth, 1, 0);

    this->show();
}

void MainWindow::initWidgets()
{
    EnterProcStart("MainWindow::InitWidgets");

    qInfo() << "Begin Init Main Form Widgets";

    QString title = tr("ANIMUS %1").arg(APP_VERSION);

    this->setWindowTitle(title);

    ApplicationSettings& applicationSettings = ApplicationSettings::Instance();

    _dataConsole = nullptr;
    _emulatorConsole = nullptr;

    _playTimer = new QTimer(this);
    connect(_playTimer, &QTimer::timeout, this, &MainWindow::playTimerTimeout);

    _mapView = new MapView(nullptr);
    _mapView->showMapMarkers();
    _mapView->showArealObjects();

    _tabTools = new QTabWidget(this);

    addTabWidgets();

    addWidgetsVideoDisplay();

    _rightPanel = new QSplitter(Qt::Vertical, this);
    _rightPanel->setContentsMargins(0, 0, 0, 0);
    _rightPanel->addWidget(_tabTools);

    _mainSplitter = new QSplitter(Qt::Horizontal, this);
    _mainSplitter->setContentsMargins(0, 0, 0, 0);
    _mainSplitter->addWidget(_rightPanel);

    _verticalWindowLayout = CommonWidgetUtils::createVBoxLayout(this, 0);
    _verticalWindowLayout->addWidget(_mainSplitter);
    _verticalWindowLayout->setSpacing(0);
    _verticalWindowLayout->setMargin(0);

    addWidgetsTime();

    if (qApp->desktop()->screenCount() == 1)
        initForSingleDisplay();
    else
    {
        switch (applicationSettings.DisplayMode)
        {
        case DisplayModes::Camera1Map2:
            initForTwoDisplays(0, 1);
            break;
        case DisplayModes::Map1Camera2:
            initForTwoDisplays(1, 0);
            break;
        default:
            initForSingleDisplay();
        }
    }
    workModeChanged();

    qInfo() << "End Init Main Form Widgets";
}

void MainWindow::initHidController(CamAssemblyPreferences *camAssemblyPreferences)
{
    EnterProcStart("MainWindow::initHidController");
    _hidController = new HIDController(this);
    auto camPreferences = camAssemblyPreferences->opticalDevice(OPTYCAL_SYSTEM_1);
    _hidController->setCamZoomRange(camPreferences->zoomMin(), camPreferences->zoomMax()); //???
    connect(_hidController, &HIDController::onOpenApplicationSettingsEditorClicked, this,               &MainWindow::onOpenApplicationSettingsEditorClicked);
    connect(_hidController, &HIDController::onOpenDataConsoleClicked,               this,               &MainWindow::onOpenDataConsoleClicked);
    connect(_hidController, &HIDController::onOpenEmulatorConsoleClicked,           this,               &MainWindow::onOpenEmulatorConsoleClicked);
    connect(_hidController, &HIDController::onOpenHelpViewerClicked,                this,               &MainWindow::onOpenHelpViewerClicked);
    connect(_hidController, &HIDController::onForceStartNewSessionClicked,          this,               &MainWindow::onForceStartNewSessionClicked);
    connect(_hidController, &HIDController::onForceDisplayOnlyClicked,              this,               &MainWindow::onForceDisplayOnlyClicked);
    connect(_hidController, &HIDController::onChangeVideo2MapClicked,               this,               &MainWindow::onChangeVideo2MapCicked);
    connect(_hidController, &HIDController::onSelectSessionsClicked,                this,               &MainWindow::onSelectSessionsClicked);
    connect(_hidController, &HIDController::onRelativeCamPositionChange,            _camControlsWidget, &CamControlsWidget::onRelativeCamPositionChange);
    connect(_hidController, &HIDController::onAbsoluteCamZoomChange,                _camControlsWidget, &CamControlsWidget::onAbsoluteCamZoomChange);
    connect(_hidController, &HIDController::onCamMovingSpeedChange,                 _camControlsWidget, &CamControlsWidget::onManualCamMovingSpeedChange);
    connect(_hidController, &HIDController::onChangeActiveCamClicked,               _camControlsWidget, &CamControlsWidget::onChangeActiveCamClicked);
    connect(_hidController, &HIDController::onEnableSoftwareStabClicked,            _camControlsWidget, &CamControlsWidget::onEnableSoftwareStabilizationClicked);
    connect(_hidController, &HIDController::onCamDriversOffClicked,                 _camControlsWidget, &CamControlsWidget::onCamDriversOffClicked);
    connect(_hidController, &HIDController::onCamLandingPosClicked,                 _camControlsWidget, &CamControlsWidget::onCamLandingPosClicked);
    connect(_hidController, &HIDController::onCamBeginingPosClicked,                _camControlsWidget, &CamControlsWidget::onCamBeginingPosClicked);
    connect(_hidController, &HIDController::onCamVerticalPosClicked,                _camControlsWidget, &CamControlsWidget::onCamVerticalPosClicked);
    connect(_hidController, &HIDController::onColorModeUpClicked,                   _camControlsWidget, &CamControlsWidget::onColorModeUpClicked);
    connect(_hidController, &HIDController::onColorModeDownClicked,                 _camControlsWidget, &CamControlsWidget::onColorModeDownClicked);
    connect(_hidController, &HIDController::onLaserActivationClicked,               _camControlsWidget, &CamControlsWidget::onLaserActivationClicked);

    connect(_hidController, &HIDController::onMapZoomInClicked,                     _mapView,           &MapView::onMapZoomInClicked);
    connect(_hidController, &HIDController::onMapZoomOutClicked,                    _mapView,           &MapView::onMapZoomOutClicked);
    connect(_hidController, &HIDController::onMapMoveClicked,                       _mapView,           &MapView::onMapMoveClicked);
    connect(_hidController, &HIDController::onFollowThePlaneClicked,                _mapView,           &MapView::onFollowThePlaneClicked);

    connect(_hidController, &HIDController::onScreenshotClicked,                    _camControlsWidget, &CamControlsWidget::onScreenshotClicked);
    connect(_hidController, &HIDController::onSnapshotClicked,                      _camControlsWidget, &CamControlsWidget::onSnapshotClicked);
    connect(_hidController, &HIDController::onSnapshotSeriesClicked,                _camControlsWidget, &CamControlsWidget::onSnapshotSeriesClicked);
    connect(_hidController, &HIDController::onTargetUnlockClicked,                  _camControlsWidget, &CamControlsWidget::onTargetUnlockClicked);
    connect(_hidController, &HIDController::onCamRecordingClicked,                  _camControlsWidget, &CamControlsWidget::onCamRecordingClicked);
    connect(_hidController, &HIDController::onEnableAutomaticTracerClicked,         _camControlsWidget, &CamControlsWidget::onEnableAutomaticTracerClicked);
    if (_bombingWidget != nullptr)
    {
        connect(_hidController, &HIDController::onDropBombClicked05,                _bombingWidget,     &BombingWidget::onDropBombClicked05);
        connect(_hidController, &HIDController::onDropBombClicked10,                _bombingWidget,     &BombingWidget::onDropBombClicked10);
        connect(_hidController, &HIDController::onSendHitCoordinatesClicked,        _bombingWidget,     &BombingWidget::onSendHitCoordinatesClicked);
        connect(_hidController, &HIDController::onSendWeatherClicked,               _bombingWidget,     &BombingWidget::onSendWeatherClicked);
        connect(_hidController, &HIDController::onNewMarkerForTargetClicked,        _bombingWidget,     &BombingWidget::onNewMarkerForTargetClicked);
        connect(_hidController, &HIDController::onNewMarkerForLaserClicked,         _bombingWidget,     &BombingWidget::onNewMarkerForLaserClicked);
        connect(_hidController, &HIDController::onNewMarkerForUAVClicked,           _bombingWidget,     &BombingWidget::onNewMarkerForUAVClicked);
    }
    connect(_hidController, &HIDController::onChangeBombingSightClicked,            _videoWidget,       &VideoDisplayWidget::onChangeBombingSightClicked);
    connect(_hidController, &HIDController::onTargetLockCursorSpeedChange,          _videoWidget,       &VideoDisplayWidget::onTargetLockCursorSpeedChange);
    connect(_hidController, &HIDController::onTargetLockInCursorClick,              _videoWidget,       &VideoDisplayWidget::onTargetLockInCursorClick);
    connect(_hidController, &HIDController::onMagnifierClick,                       _videoWidget,       &VideoDisplayWidget::onMagnifierClick);

    connect(_camControlsWidget, &CamControlsWidget::doSetZoomFromUI,                _hidController,     &HIDController::doSetZoomFromUI);
}

void MainWindow::addTabWidgets()
{
    EnterProcStart("MainWindow::addTabWidgets");

    ApplicationSettings& applicationSettings = ApplicationSettings::Instance();

    _camControlsWidget = new CamControlsWidget(_tabTools, _hardwareLink, _automaticTracer, _voiceInformant);
    connect(_camControlsWidget, &CamControlsWidget::makeScreenshot,            this,               &MainWindow::makeScreenshot);

    connect(_camControlsWidget, &CamControlsWidget::tuneImageChange,           this,               &MainWindow::tuneImageChange);

    connect(_camControlsWidget,  &CamControlsWidget::setStabilizationType,     _imageProcessor,  &ImageProcessor::setStabilizationType);

    if (applicationSettings.ObjectTrackerType == ObjectTrackerTypeEnum::External)
    {
        connect(_camControlsWidget,  &CamControlsWidget::unlockTarget,     _hardwareLink,    &HardwareLink::unlockTarget);
        connect(_camControlsWidget,  &CamControlsWidget::setTargetSize,    _hardwareLink,    &HardwareLink::setTargetSize);
    }
    else
    {
        connect(_camControlsWidget,  &CamControlsWidget::unlockTarget,     _imageProcessor,  &ImageProcessor::unlockTarget);
        connect(_camControlsWidget,  &CamControlsWidget::setTargetSize,    _imageProcessor,  &ImageProcessor::setTargetSize);
    }

    _imageProcessor->setTargetSize(_camControlsWidget->selectedTargetSize());

    _dashboardWidget = new DashboardWidget(_tabTools);
    connect(_dashboardWidget, &DashboardWidget::activateCatapult,           _hardwareLink,     &HardwareLink::activateCatapult);

    _bombingWidget = applicationSettings.isBombingTabLicensed() || applicationSettings.isTargetTabLicensed() ?
                new BombingWidget(_tabTools, _hardwareLink, _artillerySpotter, _dataStorage) : nullptr;

    _patrolWidget = applicationSettings.isPatrolTabLicensed() ?
                new PatrolWidget(_tabTools) : nullptr;

    _antennaWidget = applicationSettings.isAntennaLicensed() ?
                new AntennaControlWidget(_tabTools, _hardwareLink) : nullptr;

    _markerListWidget = applicationSettings.isMarkersTabLicensed() ?
                new MarkerListWidget(_tabTools) : nullptr;
}

QToolButton *MainWindow::createToolButton(const QString &caption, const QString &toolTip, bool checkable, QStyle::StandardPixmap icon, void (MainWindow::*onClickMethod)())
{
    auto button = new QToolButton(this);
    if (icon < QStyle::StandardPixmap::SP_CustomBase)
        button->setIcon(this->style()->standardIcon(icon));
    button->setCheckable(checkable);
    if (!caption.isEmpty())
        button->setText(caption);
    button->setToolTip(toolTip);
    connect(button, &QToolButton::clicked, this, onClickMethod);
    return button;
}

void MainWindow::addWidgetsTime()
{
    EnterProcStart("MainWindow::AddWidgetsTime");
    ApplicationSettings& applicationSettings = ApplicationSettings::Instance();

    _timeControlsLayout = new QHBoxLayout();
    _timeControlsLayout->setMargin(4);
    if (!_useMinimalisticDesign)
        _verticalWindowLayout->addLayout(_timeControlsLayout);

    const QString NO_CAPTION = "";

    auto selectSessionButton = createToolButton(tr("Sessions"), applicationSettings.hidUIHint(hidbtnSelectSessions),
                                                false, QStyle::SP_CustomBase, &MainWindow::onSelectSessionsClicked);
    _changeVideo2MapButton = createToolButton(NO_CAPTION, applicationSettings.hidUIHint(hidbtnChangeVideo2Map),
                                              false, QStyle::SP_TitleBarNormalButton, &MainWindow::onChangeVideo2MapCicked);

    // http://thesmithfam.org/blog/2010/03/10/fancy-qslider-stylesheet/
    _timeSlider = new QSlider(this);
    _timeSlider->setOrientation(Qt::Horizontal);
    _timeSlider->setMinimum(0);
    connect(_timeSlider, &QSlider::valueChanged, this, &MainWindow::timeSliderValueChanged);

    _timeFromStartIndicator = new QLCDNumber(this);
    _timeFromStartIndicator->setSegmentStyle(QLCDNumber::Filled);
    _timeFromStartIndicator->setDigitCount(12);
    _timeFromStartIndicator->setToolTip(tr("Session Time from Start"));

    _connectionsIndicator = new ConnectionsIndicator(this);

    _playHistoryButton = createToolButton(NO_CAPTION, tr("Play stored session data"), true, QStyle::SP_MediaPlay, &MainWindow::playHistoryClicked);
    _pauseButton = createToolButton(NO_CAPTION, tr("Pause showed data"), true, QStyle::SP_MediaPause, &MainWindow::pauseClicked);
    _playRealtimeButton = createToolButton(NO_CAPTION, tr("Show realtime data"), true, QStyle::SP_MediaSkipForward, &MainWindow::playRealtimeClicked);

    _timeSpacerItem = new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Minimum);

    QList<QWidget*> timeWidgets {selectSessionButton, _changeVideo2MapButton, _timeSlider,_timeFromStartIndicator,
                _playHistoryButton, _pauseButton, _playRealtimeButton, _connectionsIndicator};

    foreach (auto widget, timeWidgets)
    {
        _timeControlsLayout->addWidget(widget);
        widget->setVisible(!_useMinimalisticDesign);
    }
}

void MainWindow::addWidgetsVideoDisplay()
{
    EnterProcStart("MainWindow::addWidgetsVideoDisplay");

    ApplicationSettings& applicationSettings = ApplicationSettings::Instance();

    _videoWidget = new VideoDisplayWidget(this, _voiceInformant);

    if (applicationSettings.ObjectTrackerType == ObjectTrackerTypeEnum::External)
    {
        connect(_videoWidget,       &VideoDisplayWidget::lockTarget,  _hardwareLink,   &HardwareLink::lockTarget);
    }
    else
    {
        connect(_videoWidget,       &VideoDisplayWidget::lockTarget,  _imageProcessor, &ImageProcessor::lockTarget);
    }

    connect(_camControlsWidget, &CamControlsWidget::setTargetSize,               _videoWidget, &VideoDisplayWidget::setTargetSize);
    _videoWidget->setTargetSize(_camControlsWidget->selectedTargetSize());

    connect(_camControlsWidget, &CamControlsWidget::enableSoftwareStabilization, _videoWidget, &VideoDisplayWidget::onEnableStabilization);
    if (_useMinimalisticDesign)
        _videoWidget->setMinimumSize(200, 50);
    else
        _videoWidget->setMinimumSize(200, 200);

    _videoWidget->onEnableStabilization(applicationSettings.SoftwareStabilizationEnabled);
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    EnterProcStart("MainWindow::closeEvent");

    if (_dataConsole != nullptr)
        _dataConsole->hide();
    if (_emulatorConsole != nullptr)
        _emulatorConsole->hide();

    ApplicationSettings& applicationSettings = ApplicationSettings::Instance();

    bool needExit = !(applicationSettings.AskAboutQuitOnCloseApp);
    if (!needExit)
        needExit = CommonWidgetUtils::showConfirmDialog(tr("Are you sure you want to quit?"), true);
    if (needExit)
    {
        auto mainSplitterSizes =_mainSplitter->sizes();
        applicationSettings.MainFormViewPanelWidth = mainSplitterSizes[0];
        applicationSettings.MainFormToolPanelWidth = mainSplitterSizes[1];

        applicationSettings.MainFormViewPanelShowVideo = (_videoWidget->parentWidget() == _mainSplitter);

        delete _mapView;
        _mapView = nullptr;
        event->accept();

        applicationSettings.savePreferences();
    }
    else
        event->ignore();
    //QWidget::closeEvent(event);
}

void MainWindow::onOpenApplicationSettingsEditorClicked()
{
    EnterProcStart("MainWindow::onOpenApplicationSettingsEditorClicked");
    if (_dataConsole != nullptr)
        _dataConsole->hide();
    if (_emulatorConsole != nullptr)
        _emulatorConsole->hide();

    _hardwareLink->closeVideoSource();
    ApplicationSettingsEditor applicationSettingsEditor(this, _hidController);
    int result = applicationSettingsEditor.exec();
    if (result == QDialog::Rejected)
        _hardwareLink->openVideoSource();
}

void MainWindow::onOpenDataConsoleClicked()
{
    EnterProcStart("MainWindow::onOpenDataConsoleClicked");
    if (_dataConsole == nullptr)
    {
        _dataConsole = new DataConsole(nullptr);
        connect(_hardwareLink, &HardwareLink::onClientCommandSent, _dataConsole, &DataConsole::onHardwareLinkCommandSent);
        connect(_hardwareLink, &HardwareLink::onTelemetryReceived, _dataConsole, &DataConsole::onHardwareLinkTelemetryReceived);
    }
    _dataConsole->show();
}

void MainWindow::onOpenEmulatorConsoleClicked()
{
    EnterProcStart("MainWindow::onOpenEmulatorConsoleClicked");
    if (_emulatorConsole == nullptr)
    {
        _emulatorConsole = new EmulatorConsole(nullptr);
        connect(_emulatorConsole, &EmulatorConsole::onEmulatorTelemetryDataFrame, _hardwareLink, &HardwareLink::onEmulatorTelemetryDataFrame);
    }
    _emulatorConsole->show();
}

void MainWindow::onOpenHelpViewerClicked()
{
    EnterProcStart("MainWindow::onOpenHelpViewerClicked");
    ApplicationSettings& applicationSettings = ApplicationSettings::Instance();
    QString helpFilePath = applicationSettings.HelpFilePath;

    //    helpFilePath = "E:\\Projects\\MapTelemetryViewDemo\\Help\\Animus.rtf";
    HelpViewer helpViewer(this);
    if (helpViewer.load(helpFilePath))
        helpViewer.exec();
}

void MainWindow::onSelectSessionsClicked()
{
    auto sessionSelector = new SessionSelectorWidget(_dataStorage, this);
    sessionSelector->showNormal();
}

void MainWindow::timeSliderValueChanged(int value)
{
    EnterProcStart("MainWindow::timeSliderValueChanged");

    if (value < 0)
        return;

    if (_dataStorage->getTelemetryDataFrameCount() == 0) // for first frame in RecordAndDisplay mode
        return;

    if (_playStatus == PlayRealtime)
        SetPlayStatus(Pause);

    const TelemetryDataFrame telemetryFrame = _dataStorage->getTelemetryDataFrameByIndex(value);
    _dataStorage->showStoredDataAsync(telemetryFrame);
    _timeFromStartIndicator->display(_dataStorage->getTelemetryFrameTimeAsString(telemetryFrame));
}

void MainWindow::playHistoryClicked()
{
    SetPlayStatus(PlayHistory);
}

void MainWindow::pauseClicked()
{
    SetPlayStatus(Pause);
}

void MainWindow::playRealtimeClicked()
{
    SetPlayStatus(PlayRealtime);
}

void MainWindow::onChangeVideo2MapCicked()
{
    EnterProcStart("MainWindow::changeVideo2MapCicked");

    QList<int> rightPanelSizes = _rightPanel->sizes();
    QList<int> mainSplitterSizes = _mainSplitter->sizes();

    if (_mainSplitter->widget(0) == _mapView)
    {
        _mainSplitter->insertWidget(0, _videoWidget);
        _rightPanel->insertWidget(0, _mapView);
    }
    else
    {
        _mainSplitter->insertWidget(0, _mapView);
        _rightPanel->insertWidget(0, _videoWidget);
    }

    _rightPanel->setSizes(rightPanelSizes);
    _mainSplitter->setSizes(mainSplitterSizes);
}

void MainWindow::playTimerTimeout()
{
    EnterProcStart("MainWindow::playTimerTimout");

    const int TIMER_STEP_IN_FRAMES = 3;
    int timerValue = _timeSlider->value();
    int maxValue = _timeSlider->maximum();

    int deltaTime = maxValue - timerValue;
    if (deltaTime > TIMER_STEP_IN_FRAMES)
        deltaTime = TIMER_STEP_IN_FRAMES;

    if (deltaTime > 0)
        _timeSlider->setValue(timerValue + deltaTime);
    else
        SetPlayStatus(Pause);
}

void MainWindow::makeScreenshot()
{
    _videoWidget->saveScreenshot(_dataStorage->getScreenshotFolder());
}

void MainWindow::onForceStartNewSessionClicked()
{
    if (_dataStorage->getWorkMode() != TelemetryDataStorage::WorkMode::RecordAndDisplay)
        _dataStorage->newSession();
}

void MainWindow::onForceDisplayOnlyClicked()
{
    _dataStorage->stopSession();
}

void MainWindow::onHardwareLinkStateChanged()
{
    EnterProcStart("MainWindow::hardwareLinkStateChanged");
    updateDashboardStatuses();
}

void MainWindow::onDataReceived(const TelemetryDataFrame &telemetryFrame, const QImage &videoFrame)
{
    EnterProcStart("MainWindow::onDataReceived");

    if (_playStatus == PlayStatus::PlayRealtime)
    {
        _videoWidget->setData(telemetryFrame, videoFrame);
        _artillerySpotter->processTelemetry(telemetryFrame);
        if (_mapView != nullptr)
            _mapView->processTelemetry(telemetryFrame);
        _camControlsWidget->processTelemetry(telemetryFrame);
        _dashboardWidget->processTelemetry(telemetryFrame);
        if (_bombingWidget != nullptr)
            _bombingWidget->processTelemetry(telemetryFrame);
        if (_antennaWidget != nullptr)
            _antennaWidget->processTelemetry(telemetryFrame);
    }
}

void MainWindow::tuneImageChange(qreal brightness, qreal contrast, qreal gamma, bool grayscale)
{
    _imageProcessor->setTuneImageSettings(brightness, contrast, gamma, grayscale);
}

void MainWindow::hardwareLinkDataReceived(const TelemetryDataFrame &telemetryFrame, const QImage &videoFrame)
{
    EnterProcStart("MainWindow::hardwareLinkDataReceived");

    if (_dataStorage->getWorkMode() == TelemetryDataStorage::WorkMode::PlayStored)
        return;

    // 1. Video
    //    if (videoFrame.isNull())
    {
        _imageProcessor->processDataAsync(telemetryFrame, videoFrame);
        if (_playStatus == PlayRealtime)
            _timeFromStartIndicator->display(_dataStorage->getLastTelemetryFrameTimeAsString());
    }

    // 2. Telemetry
    if (telemetryFrame.TelemetryFrameNumber > 0)
        if (_mapView != nullptr)
            _mapView->appendTrajectoryPoint(telemetryFrame);

    int timeSliderMax = _dataStorage->getTelemetryDataFrameCount() - 1;
    if (timeSliderMax < 0)
        timeSliderMax = 0;
    else if (timeSliderMax > 10)
        timeSliderMax = (timeSliderMax / 10) * 10;
    _timeSlider->setMaximum(timeSliderMax);
}

void MainWindow::storedDataReceived(const TelemetryDataFrame &telemetryFrame, const QImage &videoFrame)
{
    EnterProcStart("MainWindow::storedVideoFrameReceived");

    if (_playStatus == Pause || _playStatus == PlayHistory)
    {
        _videoWidget->setData(telemetryFrame, videoFrame);
    }
}

void MainWindow::workModeChanged()
{
    EnterProcStart("MainWindow::workModeChanged");

    SetPlayStatus(Unknown);

    if (_hardwareLink != nullptr)
        _hardwareLink->close();
    if (_mapView != nullptr)
        _mapView->clearTrajectory();
    _timeFromStartIndicator->display(getTimeAsString(0));
    _timeSlider->setValue(0);
    if (_videoWidget != nullptr)
        _videoWidget->clear();

    ApplicationSettings& applicationSettings = ApplicationSettings::Instance();
    bool showBombingTab = (applicationSettings.isBombingTabLicensed() || applicationSettings.isTargetTabLicensed()) &&
            applicationSettings.BombingTabAllowed;
    bool showMarkersTab = applicationSettings.isMarkersTabLicensed() && applicationSettings.MarkersTabAllowed;
    bool showToolsTab   = applicationSettings.ToolsTabAllowed;
    bool showPatrolTab  = applicationSettings.isPatrolTabLicensed() && applicationSettings.PatrolTabAllowed;
    bool showAntennaTab = applicationSettings.isAntennaLicensed() && applicationSettings.AntennaTabAllowed;

    switch (_dataStorage->getWorkMode())
    {
    case TelemetryDataStorage::WorkMode::DisplayOnly:
        showModeSpecificWidgets(true, showToolsTab, showMarkersTab, showBombingTab, showPatrolTab, showAntennaTab, false);
        SetPlayStatus(PlayRealtime);
        _hardwareLink->open();
        break;
    case TelemetryDataStorage::WorkMode::PlayStored:
        showModeSpecificWidgets(false, showToolsTab, showMarkersTab, false, false, false, true);
        SetPlayStatus(PlayHistory);
        _timeSlider->setMaximum(_dataStorage->getTelemetryDataFrameCount() - 1);
        if (_mapView != nullptr)
            _mapView->loadTrajectory(_dataStorage->getTelemetryDataFrames());
        break;
    case TelemetryDataStorage::WorkMode::RecordAndDisplay:
        bool showTimeScale = applicationSettings.VideoFileFrameCount > 0;
        showModeSpecificWidgets(true, showToolsTab, showMarkersTab, showBombingTab, showPatrolTab, showAntennaTab, showTimeScale);
        SetPlayStatus(PlayRealtime);
        _hardwareLink->open();
        break;
    }
}
