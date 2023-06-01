#include "ApplicationSettings.h"
#include <QSerialPortInfo>

// https://habrahabr.ru/post/149085/

const QString getDefaultSerialPortName()
{
    QString defaultName;
    auto list = QSerialPortInfo::availablePorts();
    if (!list.isEmpty())
        defaultName = list.first().portName();
    return defaultName;
}

const QString DefaultJoystickMapping = "a:b2,b:b1,x:b3,y:b0,back:b8,start:b9,leftstick:b10,rightstick:b11,leftshoulder:"
                                       "b4,rightshoulder:b5,dpup:h0.1,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,leftx:a0,lef"
                                       "ty:a1,rightx:a2,righty:a4,lefttrigger:b6,righttrigger:b7,platform:Windows,";

const QString DefaultBallisticMacro =
        "// wind_direction, wind_speed, uav_lat, uav_lon, uav_hmsl, \n"
        "// uav_groundspeed, uav_airspeed, uav_course, uav_airspeed, \n"
        "// target_lat, target_lon, target_hmsl \n"
        "// target_distance, target_azimuth, droppoint_time, droppoint_distance \n"
        "// debug_info \n"
        " \n"
        "if (uav_groundspeed > 0) {\n"
        "\t fallHeight = uav_hmsl - target_hmsl; \n"
        "\t fallTime = Math.sqrt(2 * fallHeight / 9.80665); \n"
        "\t bombOffset = uav_groundspeed * fallTime; \n"
        "\t droppoint_distance = (target_distance - bombOffset * 0.92); \n"
        "\t droppoint_time = droppoint_distance < 0 ? -1 : (droppoint_distance / uav_groundspeed + 0.5); \n"
        "} else \n"
        "\t droppoint_time = -1; \n";


void ApplicationSettings::addHIDButtonPrefs(HIDButton prefIndex, const QString &keyboardPrefName, const QString &keyboardPrefDefault,
                                            const QString &joystickPrefName, const int joystickPrefDefult, const QString &caption)
{
    if (!keyboardPrefName.isEmpty())
    {
        auto keyboardPref = new ApplicationPreferenceString(this, keyboardPrefName, keyboardPrefDefault, caption);
        _hidKeyboardPrefs.insert(prefIndex, keyboardPref);
    }

    if (!joystickPrefName.isEmpty())
    {
        auto joystickPref = new ApplicationPreferenceInt(this, joystickPrefName, joystickPrefDefult, caption);
        _hidJoystickPrefs.insert(prefIndex, joystickPref);
    }

    _hidCaptions.insert(prefIndex, caption);
}

ApplicationSettings::ApplicationSettings() : ApplicationSettingsRoot(),
    InstalledCameraIndex(this, "Sessions/CurrentCameraIndex", 0),

    TileReceivingMode(this, "Databases/TileReceivingMode", 0), // TileReceivingMode::DatabaseOnly == 0
    DatabaseMapDownloadCashe(this, "Databases/DatabaseDownloadCashe", "DownloadCashe.db"),
    DatabaseHeightMap(this, "Databases/DatabaseHeightMap", "HeightMap.db"),
    DatabaseGeocoder(this, "Databases/DatabaseGeocoder", "GeoCoder.db"),
    MarkerThesaurusDatabase(this, "Databases/MarkerThesaurus", "MarkerThesaurus.db"),
    MarkerStorageDatabase(this, "Databases/MarkerStorage", "MarkerStorage.db"),
    ArealObjectDatabase(this, "Databases/ArealObjectDatabase", "ArealObjects.db"),
    BallisticMacro(this, "Databases/BallisticMacro", DefaultBallisticMacro),
    SessionsFolder(this, "Sessions/SessionsFolder", "Sessions"),
    LogFolderMaxSizeMb(this, "Sessions/LogFolderMaxSizeMb", 50),
    LogFolderCleanup(this, "Sessions/LogFolderCleanup", true),
    CommandSendingInterval(this, "Sessions/CommandSendingInterval", 300),
    CommandProtocol(this, "Sessions/CommandProtocol", CommandProtocols::MUSV),
    CommandTransport(this, "Sessions/CommandTransport", CommandTransports::UDP),
    CommandUDPPort(this, "Sessions/CommandUDPPort", 45562),
    CommandUDPAddress(this, "Sessions/CommandUDPAddress", "127.0.0.1"),
    CommandSerialPortName(this, "Sessions/CommandSerialPortName", getDefaultSerialPortName()),
    UAVTelemetrySourceType(this, "Sessions/UAVTelemetrySourceType", UAVTelemetrySourceTypes::UDPChannel),
    TelemetryDataFormat(this, "Sessions/TelemetryDataFormat", UAVTelemetryDataFormats::UAVTelemetryFormatV4),
    UAVTelemetryUDPPort(this, "Sessions/TelemetryUDPPort", 45560),
    UseCamTelemetryUDP(this, "Sessions/UseCamTelemetryUDP", false),
    CamTelemetryUDPPort(this, "Sessions/CamTelemetryUDPPort", 50011),

    UseExtTelemetryUDP(this, "Sessions/UseExtTelemetryUDP", false),
    ExtTelemetryUDPPort(this, "Sessions/ExtTelemetryUDPPort", 1122),

    VideoLagFromTelemetry(this, "Sessions/VideoLagFromTelemetry", 0),
    EnableForwarding(this, "Sessions/EnableForwarding", false),
    VideoForwardingAddress(this, "Sessions/VideoForwardingAddress", "192.168.1.100"),
    VideoForwardingPort(this, "Sessions/VideoForwardingPort", 51200),
    TelemetryForwardingAddress(this, "Sessions/TelemetryForwardingAddress", "192.168.1.100"),
    TelemetryForwardingPort(this, "Sessions/TelemetryForwardingPort", 45560),
    UseCatapultLauncher(this, "Sessions/UseCatapultLauncher", false),
    CatapultSerialPortName(this, "Sessions/CatapultSerialPortName", getDefaultSerialPortName()),
    CatapultCommand(this, "Sessions/CatapultCommand", "5501305503,5501315503,55013255035501315503,5501355503"),
    EnableArtilleryMountNotification(this, "Sessions/EnableArtilleryMountNotification", false),
    ArtilleryMountAddress(this, "Sessions/ArtilleryMountAddress", "192.168.1.101"),
    ArtilleryMountTCPPort(this, "Sessions/ArtilleryMountTCPPort", 60000),
    ExternalDataConsoleUDPPort(this, "Sessions/ExternalDataConsoleUDPPort", 45570),
    ObjectTrackerType(this, "Sessions/ObjectTrackerType", ObjectTrackerTypeEnum::InternalCorrelation),
    TrackerCommandUDPPort(this, "Sessions/TrackerCommandUDPPort", 50001),
    TrackerCommandUDPAddress(this, "Sessions/TrackerCommandUDPAddress", "192.168.100.2"),
    TrackerTelemetryUDPPort(this, "Sessions/TrackerTelemetryUDPPort", 50003),
    TrackerTelemetryUDPAddress(this, "Sessions/TrackerTelemetryUDPAddress", "192.168.100.2"),
    InterfaceLanguage(this, "Interface/ApplicationLanguage", ApplicationLanguages::Russian),
    DisplayMode(this, "Interface/DisplayMode", DisplayModes::SingleDisplay),
    MinimalisticDesign(this, "Interface/MinimalisticDesign", false),
    MainFormViewPanelWidth(this, "Interface/MainFormViewPanelWidth", 720),
    MainFormToolPanelWidth(this, "Interface/MainFormToolPanelWidth", 320),
    MainFormViewPanelShowVideo(this, "Interface/MainFormViewPanelShowVideo", true),
    OSDLineWidth(this, "Interface/OSDLineWidth", 2),
    UseFixedOSDLineWidth(this, "Interface/UseFixedOSDLineWidth", true),
    OSDScreenCenterMark(this, "Interface/OSDScreenCenterMark", OSDScreenCenterMarks::SimpleCross),
    OSDGimbalIndicator(this, "Interface/OSDGimbalIndicator", OSDGimbalIndicatorType::RotatingPlane),
    OSDGimbalAngles(this, "Interface/OSDGimbalAngles", OSDGimbalIndicatorAngles::AbsoluteAngles),
    OSDGimbalIndicatorSize(this, "Interface/OSDGimbalIndicatorSize", 30),
    OSDScreenLinesColor(this, "Interface/OSDLinesColor", QColor(67, 200, 0)),
    OSDScreenCenterMarkColor(this, "Interface/OSDScreenCenterMarkColor", QColor(Qt::green)),
    OSDShowBombingSight(this, "Interface/OSDShowBombingSight", true),
    OSDShowCenterMark(this, "Interface/OSDShowCenterMark", true),
    OSDShowTelemetry(this, "Interface/OSDShowTelemetry", true),
    OSDTelemetryIndicatorFontSize(this, "Interface/OSDTelemetryIndicatorFontSize", 14),
    OSDTargetTrackerCursor(this, "Interface/OSDTargetTrackerCursor", QColor(Qt::green)),
    SoftwareStabilizationEnabled(this, "Interface/SoftwareStabilizationEnabled", true),
    VideoStabilizationType(this, "Interface/VideoStabilizationType", StabilizationType::StabilizationByFrame),
    LastTargetArtillerySpotterState(this, "Interface/LastTargetArtillerySpotterState", ArtillerySpotterState::Unspecified),
    AskAboutQuitOnSaveSettings(this, "Interface/AskAboutQuitOnSaveSettings", true),
    AskAboutQuitOnCloseApp(this, "Interface/AskAboutQuit", true),
    BombingTabAllowed(this, "Interface/UseBombingAllowed", false),
    MarkersTabAllowed(this, "Interface/UseMarkersAllowed", true),
    ToolsTabAllowed(this, "Interface/UseToolsAllowed", true),
    PatrolTabAllowed(this, "Interface/PatrolTabAllowed", true),
    CamControlsTabCoordIndicatorAllowed(this,"Interface/CamControlsTabCoordIndicatorAllowed", true),
    VisibleTelemetryTableRows(this, "Interface/VisibleTelemetryTableRows", "1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20"),

    LastUsedMapBaseSourceId(this, "Interface/LastUsedMapBaseSourceId", 3), // GoogleSatellite
    LastUsedMapHybridSourceId(this, "Interface/MapHybridSourceId", 8), // GoogleHybrid
    TargetMarkerSizes(this, "TargetMarkerSizes",                         tr("Target Marker Sizes")),
    ViewFieldLineColor(this, "Interface/ViewFieldLineColor", QColor(Qt::red)),
    ViewFieldLineWidth(this, "Interface/ViewFieldLineWidth", 3),
    TrajectoryPathLineColor(this, "Interface/TrajectoryPathLineColor", QColor(Qt::blue)),
    TrajectoryPathLineWidth(this, "Interface/TrajectoryPathLineWidth", 1),
    VisiblePathPointsPixelDistance(this, "Interface/VisiblePathPointsPixelDistance", 30),
    UseLaserRangefinderForGroundLevelCalculation(this, "Sessions/UseLaserRangefinderForGroundLevelCalculation", true),
    SoundEffectsAllowed(this, "Interface/SoundEffectsAllowed", false),
    SoundLevel(this, "Interface/SoundLevel", 1.0),

    HelpFilePath(this, "Interface/HelpFilePath", ""),

    KeyboardUsing(this, "Interface/KeyboardUsing", true),
    JoystickUsing(this, "Interface/JoystickUsing", true),
    JoystickMapping(this, "Interface/JoystickMapping", DefaultJoystickMapping),
    JoystickCameraEmulationFromKeyboard(this, "Interface/JoystickEmulationFromKeyboard", 0.125),
    JoystickCursorEmulationFromKeyboard(this, "Interface/JoystickCursorEmulationFromKeyboard", 0.125),
    JoystickCameraAxisMultiplier(this, "Interface/JoystickAxisMultiplier", 30),
    JoystickCursorAxisMultiplier(this, "Interface/JoystickCursorAxisMultiplier", 30),
    JoystickCameraAxisInsensitivity(this, "Interface/JoystickAxisSensitivity", 0.004),
    JoystickCursorAxisInsensitivity(this, "Interface/JoystickCursorAxisInsensitivity", 0.004),
    JoystickAxisZoomIndex(this, "Interface/JoystickAxisZoomIndex", 2),
    JoystickAxisCameraXIndex(this, "Interface/JoystickAxisCameraXIndex", 0),
    JoystickAxisCameraYIndex(this, "Interface/JoystickAxisCameraYIndex", 1),
    JoystickAxisCursorXIndex(this, "Interface/JoystickAxisCursorXIndex", 3),
    JoystickAxisCursorYIndex(this, "Interface/JoystickAxisCursorYIndex", 4),
    UseZoomScaleForManualMoving(this, "Interface/UseZoomScaleForManualMoving", false),

    VideoFileFrameCount(this, "Interface/VideoFileFrameCount", VIDEO_FRAMES_PER_FILE_DEFAULT),
    VideoFileQuality(this, "Interface/VideoFileQuality", VIDEO_FILE_QUALITY_DEFAULT),
    OVRDisplayTelemetry(this, "Interface/TelemetryOnVideo", true),
    OVRTelemetryIndicatorFontSize(this, "Interface/TelemetryIndicatorFontSizeOnVideo", 14),
    OVRTelemetryTimeFormat(this, "Interface/OVRTelemetryTimeFormat", OSDTelemetryTimeFormat::CurrentDateTime),
    OVRDisplayTargetRectangle(this, "Interface/DisplayTargetRectangleOnVideoRecord", true),
    OVRGimbalIndicatorType(this, "Interface/GimbalIndicatorTypeOnVideoRecord", OSDGimbalIndicatorType::RotatingPlane),
    OVRGimbalIndicatorAngles(this, "Interface/GimbalIndicatorAnglesOnVideoRecord", OSDGimbalIndicatorAngles::AbsoluteAngles),
    OVRGimbalIndicatorSize(this, "Interface/GimbalIndicatorSizeOnVideoRecord", 30),
    UIPresentationCoordSystem(this, "Interface/UIPresentationCoordSystem", GlobalCoordSystem::WGS84),

    EnableComputingStatistics(this, "Statistics/EnableComputingStatistics", false),

    EmulatorConsoleUavRoll(this, "EmulatorConsoleUavRoll", 0),
    EmulatorConsoleUavPitch(this, "EmulatorConsoleUavPitch", 0),
    EmulatorConsoleUavYaw(this, "EmulatorConsoleUavYaw", 0),
    EmulatorConsoleGpsLat(this, "EmulatorConsoleGpsLat", 53.925242),
    EmulatorConsoleGpsLon(this, "EmulatorConsoleGpsLon", 27.687746),
    EmulatorConsoleGpsHmsl(this, "EmulatorConsoleGpsHmsl", 500),
    EmulatorConsoleGpsCourse(this, "EmulatorConsoleGpsCourse", 0)

{
    _hidButtonPrefsLoaded = false;
    _camAssemblyPreferences = nullptr;
    updateToCurrentVersion();
    checkPreferenceNames();
}

void ApplicationSettings::ensureHIDButtonPrefsLoaded()
{
    if (_hidButtonPrefsLoaded)
        return;
    _hidButtonPrefsLoaded = true;

    addHIDButtonPrefs(hidbtnCamPitchDown, "Interface/CamPitchDownKey", "S", "", 0, "");
    addHIDButtonPrefs(hidbtnCamPitchUp, "Interface/CamPitchUpKey", "W", "", -1, "");
    addHIDButtonPrefs(hidbtnCamRollDown, "Interface/CamRollDownKey", "A", "", -1, "");
    addHIDButtonPrefs(hidbtnCamRollUp, "Interface/CamRollUpKey", "D", "", -1, "");
    addHIDButtonPrefs(hidbtnCamZoomOut, "Interface/CamZoomUpKey", "Z", "CamControls/ZoomOutJoystickButton", -1, tr("Zoom Out"));
    addHIDButtonPrefs(hidbtnCamZoomIn, "Interface/CamZoomDownKey", "X",  "CamControls/ZoomInJoystickButton", -1, tr("Zoom In"));
    addHIDButtonPrefs(hidbtnMapZoomOut, "Interface/MapZoomOutKey", "-",  "CamControls/MapZoomOutJoystickButton", -1, tr("Map Zoom Out"));
    addHIDButtonPrefs(hidbtnMapZoomIn, "Interface/MapZoomInKey", "+",  "CamControls/MapZoomInJoystickButton", -1, tr("Map Zoom In"));
    addHIDButtonPrefs(hidbtnFollowThePlane, "Interface/MapFollowThePlaneKey", "",  "Interface/MapFollowThePlaneJoystickButton", -1, tr("Follow The Plane"));

    addHIDButtonPrefs(hidbtnChangeActiveCam, "CamControls/ChangeActiveCamKey", "V",  "CamControls/ChangeActiveCamJoystickButton", -1, tr("Change Active Camera"));
    addHIDButtonPrefs(hidbtnEnableSoftwareStabilization, "CamControls/EnableSoftwareStabilizationKey", "C",  "CamControls/EnableSoftwareStabilizationJoystickButton", -1, tr("Stabilization On/Off"));
    addHIDButtonPrefs(hidbtnDriversOff, "CamControls/DriversOffKey", "", "CamControls/DriversOffJoystickButton", -1, tr("Cam Drivers Off"));
    addHIDButtonPrefs(hidbtnFixedPosLanding, "CamControls/FixedPosLandingKey", "", "CamControls/FixedPosLandingJoystickButton", -1, tr("Activate Landing Position"));
    addHIDButtonPrefs(hidbtnFixedPosBegining, "CamControls/FixedPosBeginingKey", "", "CamControls/FixedPosBeginingJoystickButton", -1, tr("Activate Begining Position"));
    addHIDButtonPrefs(hidbtnFixedPosVertical, "CamControls/FixedPosVerticalKey", "", "CamControls/FixedPosVerticalJoystickButton", -1, tr("Activate Vertical Position"));
    addHIDButtonPrefs(hidbtnColorModeUp, "CamControls/ColorModeUpKey", "",  "CamControls/ColorModeUpJoystickButton", -1, tr("Color Mode +"));
    addHIDButtonPrefs(hidbtnColorModeDown, "CamControls/ColorModeDownKey", "", "CamControls/ColorModeDownJoystickButton", -1, tr("Color Mode -"));
    addHIDButtonPrefs(hidbtnLaserActivation, "CamControls/LaserActivationKey", "Ctrl+L", "CamControls/LaserActivationJoystickButton", -1, tr("Laser Activation"));

    addHIDButtonPrefs(hidbtnBombingSight, "CamControls/BombingSightKey", "N",  "CamControls/BombingSightJoystickButton", -1, tr("Bombing Sight"));
    addHIDButtonPrefs(hidbtnScreenshot, "CamControls/ScreenshotKey", "", "CamControls/ScreenshotJoystickButton", -1, tr("Screenshot"));
    addHIDButtonPrefs(hidbtnSnapshot, "CamControls/SnapshotKey", "", "CamControls/SnapshotJoystickButton", -1, tr("Snapshot"));
    addHIDButtonPrefs(hidbtnSnapshotSeries, "CamControls/SnapshotSeriesKey", "", "CamControls/SnapshotSeriesJoystickButton", -1, tr("Snapshot Series"));

    addHIDButtonPrefs(hidbtnTargetUnlock, "CamControls/TargetUnlockKey", "U",  "CamControls/TargetUnlockJoystickButton", -1, tr("Target Unlock"));
    addHIDButtonPrefs(hidbtnTargetLockInCursor, "CamControls/TargetLockInCursorKey", "",  "CamControls/TargetLockInCursorJoystickButton", 7, tr("Target Lock In Cursor"));

    addHIDButtonPrefs(hidbtnCamRecording, "CamControls/CamRecordingKey", "",  "CamControls/CamRecordingJoystickButton", -1, tr("Camera Recording"));
    addHIDButtonPrefs(hidbtnAutomaticTracer, "CamControls/AutomaticTracerKey", "T",  "CamControls/AutomaticTracerJoystickButton", -1, tr("Automatic Tracer"));
    addHIDButtonPrefs(hidbtnDropBomb,  "CamControls/DropBombKey", "", "CamControls/DropBombJoystickButton", -1, tr("Drop Bomb"));
    addHIDButtonPrefs(hidbtnSendHitCoordinates,  "CamControls/SendHitCoordinatesKey", "", "CamControls/SendHitCoordinatesJoystickButton", -1, tr("Send Hit Coordinates"));
    addHIDButtonPrefs(hidbtnSendWeather,  "CamControls/SendWeatherKey", "", "CamControls/SendWeatherJoystickButton", -1, tr("Send Weather"));

    addHIDButtonPrefs(hidbtnNormalFlight, "UAVControl/NormalFlight", "", "", -1, tr("Normal Flight Mode"));
    addHIDButtonPrefs(hidbtnPatrolMovingTargetMode, "UAVControl/PatrolMovingTargetMode", "", "", -1, tr("Patrol Moving Target Mode"));
    addHIDButtonPrefs(hidbtnPatrolStaticTargetMode, "UAVControl/PatrolStaticTargetMode", "", "", -1, tr("Patrol Static Target Mode"));
    addHIDButtonPrefs(hidbtnManualFlightMode, "UAVControl/ManualFlightMode", "", "", -1, tr("Manual Flight Mode"));

    addHIDButtonPrefs(hidbtnNewMarkerForTarget, "CamControls/AppendNewTargetKey", "", "CamControls/AppendNewTargetJoystickButton", -1, tr("New Marker (Target)"));
    addHIDButtonPrefs(hidbtnNewMarkerForLaser, "CamControls/NewMarkerForLaserKey", "", "CamControls/NewMarkerForLaserJoystickButton", -1, tr("New Marker (Laser)"));
    addHIDButtonPrefs(hidbtnNewMarkerForUAV, "CamControls/NewMarkerForUAVKey", "", "CamControls/NewMarkerForUAVJoystickButton", -1, tr("New Marker (UAV)"));

    addHIDButtonPrefs(hidbtnNewSession, "Sessions/NewSessionKey", "",  "Sessions/NewSessionJoystickButton", -1,  tr("Start New Session"));
    addHIDButtonPrefs(hidbtnDisplayOnly, "Sessions/DisplayOnlyKey", "", "Sessions/DisplayOnlyJoystickButton", -1, tr("Display Only Mode"));
    addHIDButtonPrefs(hidbtnSelectSessions, "Sessions/SelectSessionsKey", "", "Sessions/SelectSessionsJoystickButton", -1, tr("Open Select Sessions Window"));
    addHIDButtonPrefs(hidbtnChangeVideo2Map, "Sessions/ChangeVideo2MapOnlyKey", "M", "Sessions/ChangeVideo2MapJoystickButton", -1, tr("Change map and video positions"));
    addHIDButtonPrefs(hidbtnDataConsole, "Interface/DataConsoleKey", "Ctrl+Shift+C", "", -1, tr("Data Console"));
    addHIDButtonPrefs(hidbtnEmulatorConsole, "Interface/EmulatorConsoleKey", "Ctrl+Shift+E", "", -1, tr("Emulator Console"));
    addHIDButtonPrefs(hidbtnSettingsEditor, "Interface/SettingsEditorKey", "Ctrl+S", "", -1, tr("Application Settings Editor"));
    addHIDButtonPrefs(hidbtnHelpViewer, "Interface/HelpViewer", "F1", "", -1, tr("Help Viewer"));
}

ApplicationSettings::~ApplicationSettings()
{
}

void ApplicationSettings::updateToCurrentVersion()
{
    QString IniFileVersion = "System/Version";

    int lastVersion = getStoredVarValue(IniFileVersion, 1).toInt();

    if (lastVersion < 2)
        updateMapDatabaseFiles();

    if (lastVersion < 3)
        updateKeyboardShortcuts();

    setStoredVarValue(IniFileVersion, 3); //Increment this number after adding new update
}

ApplicationSettings &ApplicationSettings::Instance()
{
    static ApplicationSettings s;
    return s;
}

CamAssemblyPreferences *ApplicationSettings::getCurrentCamAssemblyPreferences()
{
    if (_camAssemblyPreferences == nullptr)
    {
        auto cameraSettings = installedCameraSettings();
        _camAssemblyPreferences = new CamAssemblyPreferences(this);
        _camAssemblyPreferences->initGimbal(cameraSettings->EncoderAutomaticTracerMultiplier);

        _camAssemblyPreferences->initCam(1, // PRIMARY_OPTYCAL_SYSTEM_ID
                                         cameraSettings->CamViewSizeHorizontal, cameraSettings->CamViewSizeVertical,
                                         cameraSettings->CamZoomMin, cameraSettings->CamZoomMax,
                                         cameraSettings->CamViewAnglesHorizontal, cameraSettings->CamViewAnglesVertical,
                                         cameraSettings->CamAutomaticTracerSpeedMultipliers,
                                         cameraSettings->CamManualSpeedMultipliers);
        _camAssemblyPreferences->initCam(2, // SECONDARY_OPTYCAL_SYSTEM_ID
                                         cameraSettings->CamViewSizeHorizontal2, cameraSettings->CamViewSizeVertical2,
                                         cameraSettings->CamZoomMin, cameraSettings->CamZoomMax,
                                         cameraSettings->CamViewAnglesHorizontal2, cameraSettings->CamViewAnglesVertical2,
                                         cameraSettings->CamAutomaticTracerSpeedMultipliers2,
                                         cameraSettings->CamManualSpeedMultipliers2);
    }

    return _camAssemblyPreferences;
}

CameraSettingsNode *ApplicationSettings::installedCameraSettings()
{
    return cameraSettings(InstalledCameraIndex);
}

CameraSettingsNode *ApplicationSettings::cameraSettings(int index)
{
    if (_cameraSettings.count() == 0)
    {
        for (int i = 0; i < 10; i++)
        {
            auto cameraSettingsNode = new CameraSettingsNode(this, QString("CameraSettions_%1").arg(i));
            _cameraSettings.append(cameraSettingsNode);
        }
    }

    return _cameraSettings[index];
}

ApplicationPreferenceString *ApplicationSettings::hidKeyboardPref(HIDButton prefIndex)
{
    ensureHIDButtonPrefsLoaded();
    return _hidKeyboardPrefs[prefIndex];
}

ApplicationPreferenceInt *ApplicationSettings::hidJoystickPref(HIDButton prefIndex)
{
    ensureHIDButtonPrefsLoaded();
    return _hidJoystickPrefs[prefIndex];
}

const QString ApplicationSettings::hidCaption(HIDButton prefIndex)
{
    ensureHIDButtonPrefsLoaded();
    return _hidCaptions[prefIndex];
}

const QString ApplicationSettings::hidUIHint(HIDButton prefIndex)
{
    auto hint = hidCaption(prefIndex);
    auto keyboardPref = hidKeyboardPref(prefIndex);
    auto joystickPref = hidJoystickPref(prefIndex);

    if (keyboardPref != nullptr && !keyboardPref->value().isEmpty())
        hint += tr("\n  Key %1").arg(keyboardPref->value());
    if (joystickPref != nullptr && (joystickPref->value() >= 0) )
        hint += tr("\n  Button# %1").arg(joystickPref->value() + 1);

    return hint;
}

void ApplicationSettings::updateMapDatabaseFiles()
{
    QStringList databseFiles;

    const int obsoleteKeysCount = 9;
    QString obsoleteKeys[obsoleteKeysCount] = {
        "Databases/DatabaseYandexSatellite",
        "Databases/DatabaseJointStaffMap",
        "Databases/DatabaseGoogleSatellite",
        "Databases/DatabaseWikiMap",
        "Databases/DatabaseBingSatellite",
        "Databases/DatabaseMapsByMap",
        "Databases/DatabaseYandexMap",
        "Databases/DatabaseYandexHybrid",
        "Databases/DatabaseGoogleHybrid"
    };

    bool obsoleteSettings = false;
    for (int i = 0; i < obsoleteKeysCount; i++)
    {
        QString filePath = getStoredVarValue(obsoleteKeys[i++], "").toString();
        if (filePath != "" && !databseFiles.contains(filePath))
        {
            databseFiles.append(filePath);
            obsoleteSettings = true;
        }
    }

    if (obsoleteSettings)
    {
        setMapDatabaseFiles(databseFiles);
        for (int i = 0; i < obsoleteKeysCount; i++)
            removeStoredVarValue(obsoleteKeys[i]);
    }
}

void ApplicationSettings::updateKeyboardShortcuts()
{
    removeStoredVarValue("Interface/CamPitchDownKey");
    removeStoredVarValue("Interface/CamPitchUpKey");
    removeStoredVarValue("Interface/CamRollDownKey");
    removeStoredVarValue("Interface/CamRollUpKey");
    removeStoredVarValue("Interface/CamZoomDownKey");
    removeStoredVarValue("Interface/CamZoomDownKey");
}

const QStringList ApplicationSettings::getMapDatabaseFiles()
{

    QStringList databseFiles;

    int i = 0;
    while(true)
    {
        QString key = QString("Databases/TileDataFile%1").arg(i++);
        QString filePath = getStoredVarValue(key, "").toString();
        if (filePath.isEmpty())
            break;
        databseFiles.append(filePath);
    }

    return databseFiles;
}

void ApplicationSettings::setMapDatabaseFiles(const QStringList &files)
{
    int k = 0;
    foreach (auto filePath, files)
    {
        QString key = QString("Databases/TileDataFile%1").arg(k++);
        setStoredVarValue(key, filePath);
    }

    QStringList obsoleteMapFiles = getMapDatabaseFiles();
    int deleteFrom = files.count();
    int deleteTo = obsoleteMapFiles.count();
    for (int i = deleteFrom; i < deleteTo; i++)
    {
        QString key = QString("Databases/TileDataFile%1").arg(i);
        removeStoredVarValue(key);
    }
}

const bool totalLicensed = true;

bool ApplicationSettings::isStatisticViewLicensed()
{
    return totalLicensed;
}

bool ApplicationSettings::isLaserRangefinderLicensed()
{
    return true;
}

bool ApplicationSettings::isPhotographyLicensed()
{
    return totalLicensed;
}

bool ApplicationSettings::isBombingTabLicensed()
{
    return totalLicensed;
}

bool ApplicationSettings::isTargetTabLicensed()
{
    return true;
}

bool ApplicationSettings::isPatrolTabLicensed()
{
    return totalLicensed;
}

bool ApplicationSettings::isMarkersTabLicensed()
{
    return totalLicensed;
}

bool ApplicationSettings::isDataForwardingLicensed()
{
    return totalLicensed;
}

bool ApplicationSettings::isCatapultLicensed()
{
    return totalLicensed;
}

bool ApplicationSettings::isArtillerySpotterLicensed()
{
    return totalLicensed;
}

//----------------------------------------------------------------------------------------------------------

CameraSettingsNode::CameraSettingsNode(ApplicationSettingsImpl *parentSettings, const QString &nodeName) :
    ApplicationSettingsNode(parentSettings, nodeName),

    CameraDescription(this, "CameraDescription",            "",     tr("Description")),
    CameraSuspensionType(this, "CamControls/CameraSuspensionType",  CameraSuspensionTypes::RotatingCamera),
    CameraControlMode(this, "CamControls/CameraControlMode",        CameraControlModes::AbsolutePosition),

    DigitalZoom(this, "DigitalZoom",                        1,      tr("Digital Zoom")),
    UseBluredBorders(this, "UseBluredBorders",              true),
    UseGimbalTelemetryOnly(this, "UseGimbalTelemetryOnly",  false,  tr("Use Gimbal Telemetry Only")),

    IsOnboardRecording(this, "IsOnboardRecording",          true),
    IsSnapshot(this, "IsSnapshot",                          true),

    FixedCamPitch(this, "FixedCamPitch",                    -90,    tr("Value of Fixed Camera Pich")),
    FixedCamRoll(this, "FixedCamRoll",                      0,      tr("Value of Fixed Camera Roll")),
    FixedCamZoom(this, "FixedCamZoom",                      1,      tr("Value of Fixed Camera Zoom")),

    CamPitchMax(this, "CamPitchMax",                        +90,    tr("Maximum Value of Camera Pitch")),
    CamPitchMin(this, "CamPitchMin",                        -90,    tr("Minimum Value of Camera Pitch")),
    CamAxisYInverse(this, "CamAxisYInverse",                false),
    CamRollMax(this, "CamRollMax",                          90,     tr("Maximum Value of Camera Roll")),
    CamRollMin(this, "CamRollMin",                          -90,    tr("Minimum Value of Camera Roll")),
    CamAxisXInverse(this, "CamAxisXInverse",                true),
    CamZoomMax(this, "CamZoomMax",                          1,      tr("Maximum Value of Camera Zoom")),
    CamZoomMin(this, "CamZoomMin",                          1,      tr("Minimum Value of Camera Zoom")),
    EncoderAutomaticTracerMultiplier(this, "AutomaticTracerMultiplier", 1, tr("Automatic Tracer Multiplier")),

    FixedPosLandingYaw(this, "FixedPosLandingYaw",          -90,    tr("Value of Camera Yaw in Landing Position")),
    FixedPosLandingPitch(this, "FixedPosLandingPitch",      -90,    tr("Value of Camera Pich in Landing Position")),
    FixedPosLandingRoll(this, "FixedPosLandingRoll",        -170,   tr("Value of Camera Roll in Landing Position")),
    FixedPosLandingZoom(this, "FixedPosLandingZoom",        1,      tr("Value of Camera Zoom in Landing Position")),
    FixedPosLandingCommand(this, "FixedPosLandingCommand",  false,  tr("Use special command")),

    FixedPosBeginingYaw(this, "FixedPosBeginingYaw",       -90,     tr("Value of Camera Yaw in Begining Position")),
    FixedPosBeginingPitch(this, "FixedPosBeginingPitch",    -45,    tr("Value of Camera Pich in Begining Position")),
    FixedPosBeginingRoll(this, "FixedPosBeginingRoll",      0,      tr("Value of Camera Roll in Begining Position")),
    FixedPosBeginingZoom(this, "FixedPosBeginingZoom",      1,      tr("Value of Camera Zoom in Begining Position")),

    FixedPosVerticalYaw(this, "FixedPosVerticalYaw",        -90,    tr("Value of Camera Yaw in Vertical Position")),
    FixedPosVerticalPitch(this, "FixedPosVerticalPitch",    -90,    tr("Value of Camera Pich in Vertical Position")),
    FixedPosVerticalRoll(this, "FixedPosVerticalRoll",      0,      tr("Value of Camera Roll in Vertical Position")),
    FixedPosVerticalZoom(this, "FixedPosVerticalZoom",      1,      tr("Value of Camera Zoom in Vertical Position")),

    CamScaleCoefficient(this, "CamScaleCoefficient",                         tr("Camera Scale Coefficient")),
    CamViewAnglesHorizontal(this, "CamViewAnglesHorizontal",                 tr("Horizontal View Angle of Camera")),
    CamViewAnglesVertical(this,   "CamViewAnglesVertical",                   tr("Vertical View Angle of Camera")),
    CamAutomaticTracerSpeedMultipliers(this, "CamSpeedMultipliers",          tr("Automatic Tracer Speed Multiplier")),
    CamManualSpeedMultipliers(this, "CamManualSpeedMultipliers",             tr("Manual Moving Speed Multipliers")),
    CamViewSizeHorizontal(this, "CamViewSizeHorizontal",    720,    tr("Horizontal View Size of Camera")),
    CamViewSizeVertical(this, "CamViewSizeVertical",        576,    tr("Vertical View Size of Camera")),
    CamViewSizeForceSet(this, "CamViewSizeForceSet",        false),
    UseVerticalFrameMirrororing(this, "UseVerticalImageMirror", true, tr("Vertical Mirroring")),

    CamScaleCoefficient2(this, "CamScaleCoefficient2",                         tr("Camera Scale Coefficient")),
    CamViewAnglesHorizontal2(this, "CamViewAnglesHorizontal2",                 tr("Horizontal View Angle of Camera")),
    CamViewAnglesVertical2(this,   "CamViewAnglesVertical2",                   tr("Vertical View Angle of Camera")),
    CamAutomaticTracerSpeedMultipliers2(this, "CamSpeedMultipliers2",          tr("Automatic Tracer Speed Multiplier")),
    CamManualSpeedMultipliers2(this, "CamManualSpeedMultipliers2",             tr("Manual Moving Speed Multipliers")),
    CamViewSizeHorizontal2(this, "CamViewSizeHorizontal2",    720,    tr("Horizontal View Size of Camera")),
    CamViewSizeVertical2(this, "CamViewSizeVertical2",        576,    tr("Vertical View Size of Camera")),
    CamViewSizeForceSet2(this, "CamViewSizeForceSet2",        false),
    UseVerticalFrameMirrororing2(this, "UseVerticalImageMirror2", true, tr("Vertical Mirroring")),

    VideoTrafficSource(this, "VideoTrafficSource", VideoFrameTrafficSources::USBCamera),
    VideoFrameSourceCameraName(this, "VideoFrameSourceCameraName", ""),
    VideoFrameSourceXPlaneAddress(this, "VideoFrameSourceXPlaneAddress", "192.168.1.235"),
    VideoFrameSourceXPlanePort(this, "VideoFrameSourceXPlanePort", 51200),
    VideoFrameSourceYurionUDPPort(this, "VideoFrameSourceDigitalChannelUDPPort", 5000),
    CalibrationImagePath(this, "CalibrationImagePath", DefaultCalibrationImagePath),
    VideoFilePath(this, "VideoFilePath", QString()),
    RTSPUrl(this, "RTSPUrl", QString("rtsp://127.0.0.1:8899/UAV")),

    VideoFrameSourceMUSV2UDPPort(this, "VideoFrameSourceMUSV2UDPPort", 50006),

    BombingSightNumbers(this, "BombingSightNumbers", "")
{

}

CameraSettingsNode::~CameraSettingsNode()
{

}

//-----------------------------------------------------------------------------------
CameraSettingsNode *currCameraSettings()
{
    ApplicationSettings& applicationSettings = ApplicationSettings::Instance();
    auto cameraSettings = applicationSettings.installedCameraSettings();
    return cameraSettings;
}
