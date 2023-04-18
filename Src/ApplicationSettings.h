#ifndef APPLICATIONSETTINGS_H
#define APPLICATIONSETTINGS_H

#include <QColor>
#include <QMap>
#include "ApplicationSettingsImpl.h"
#include "Common/CommonData.h"
#include "CamPreferences.h"
#include "Constants.h"

const int ApplicationRestartExitCode = 1982;

const QString DefaultCalibrationImagePath = ":/CalibrationImages/WorldMap1.jpg";

class CameraSettingsNode final : public ApplicationSettingsNode
{
    Q_OBJECT

    friend class ApplicationSettings;

    CameraSettingsNode(ApplicationSettingsImpl *parentSettings, const QString &nodeName);
    ~CameraSettingsNode();

    CameraSettingsNode(CameraSettingsNode const&) = delete;
    CameraSettingsNode& operator= (CameraSettingsNode const&) = delete;
public:
    ApplicationPreferenceString CameraDescription;
    ApplicationPreferenceEnum<CameraSuspensionTypes> CameraSuspensionType;
    ApplicationPreferenceEnum<CameraControlModes> CameraControlMode;

    ApplicationPreferenceDouble DigitalZoom;
    ApplicationPreferenceBool UseBluredBorders;
    ApplicationPreferenceBool UseGimbalTelemetryOnly;

    ApplicationPreferenceBool IsOnboardRecording;
    ApplicationPreferenceBool IsSnapshot;

    ApplicationPreferenceDouble FixedCamPitch;
    ApplicationPreferenceDouble FixedCamRoll;
    ApplicationPreferenceDouble FixedCamZoom;

    ApplicationPreferenceDouble CamPitchMax;
    ApplicationPreferenceDouble CamPitchMin;
    ApplicationPreferenceBool   CamAxisYInverse;
    ApplicationPreferenceDouble CamRollMax;
    ApplicationPreferenceDouble CamRollMin;
    ApplicationPreferenceBool   CamAxisXInverse;
    ApplicationPreferenceDouble CamZoomMax;
    ApplicationPreferenceDouble CamZoomMin;
    ApplicationPreferenceDouble EncoderAutomaticTracerMultiplier;

    ApplicationPreferenceDouble FixedPosLandingYaw;
    ApplicationPreferenceDouble FixedPosLandingPitch;
    ApplicationPreferenceDouble FixedPosLandingRoll;
    ApplicationPreferenceDouble FixedPosLandingZoom;
    ApplicationPreferenceBool   FixedPosLandingCommand;

    ApplicationPreferenceDouble FixedPosBeginingYaw;
    ApplicationPreferenceDouble FixedPosBeginingPitch;
    ApplicationPreferenceDouble FixedPosBeginingRoll;
    ApplicationPreferenceDouble FixedPosBeginingZoom;

    ApplicationPreferenceDouble FixedPosVerticalYaw;
    ApplicationPreferenceDouble FixedPosVerticalPitch;
    ApplicationPreferenceDouble FixedPosVerticalRoll;
    ApplicationPreferenceDouble FixedPosVerticalZoom;

    //Primary optical system
    ApplicationPreferenceDoubleList CamScaleCoefficient;
    ApplicationPreferenceDoubleList CamViewAnglesHorizontal;
    ApplicationPreferenceDoubleList CamViewAnglesVertical;
    ApplicationPreferenceDoubleList CamAutomaticTracerSpeedMultipliers;
    ApplicationPreferenceDoubleList CamManualSpeedMultipliers;
    ApplicationPreferenceInt CamViewSizeHorizontal;
    ApplicationPreferenceInt CamViewSizeVertical;
    ApplicationPreferenceBool CamViewSizeForceSet;
    ApplicationPreferenceBool UseVerticalFrameMirrororing;

    //Secondary optical system
    ApplicationPreferenceDoubleList CamScaleCoefficient2;
    ApplicationPreferenceDoubleList CamViewAnglesHorizontal2;
    ApplicationPreferenceDoubleList CamViewAnglesVertical2;
    ApplicationPreferenceDoubleList CamAutomaticTracerSpeedMultipliers2;
    ApplicationPreferenceDoubleList CamManualSpeedMultipliers2;
    ApplicationPreferenceInt CamViewSizeHorizontal2;
    ApplicationPreferenceInt CamViewSizeVertical2;
    ApplicationPreferenceBool CamViewSizeForceSet2;
    ApplicationPreferenceBool UseVerticalFrameMirrororing2;

    ApplicationPreferenceEnum<VideoFrameTrafficSources> VideoTrafficSource;
    ApplicationPreferenceString VideoFrameSourceCameraName;
    ApplicationPreferenceString VideoFrameSourceXPlaneAddress;
    ApplicationPreferenceInt VideoFrameSourceXPlanePort;
    ApplicationPreferenceInt VideoFrameSourceYurionUDPPort;
    ApplicationPreferenceString CalibrationImagePath;
    ApplicationPreferenceString VideoFilePath;
    ApplicationPreferenceString RTSPUrl;
    ApplicationPreferenceInt VideoFrameSourceMUSV2UDPPort;
    ApplicationPreferenceString BombingSightNumbers;
};

enum HIDButton
{
    hidbtnCamPitchDown = 0,
    hidbtnCamPitchUp,
    hidbtnCamRollDown,
    hidbtnCamRollUp,
    hidbtnCamZoomOut,
    hidbtnCamZoomIn,
    hidbtnNewSession,
    hidbtnDisplayOnly,
    hidbtnChangeVideo2Map,
    hidbtnSelectSessions,
    hidbtnChangeActiveCam,
    hidbtnEnableSoftwareStabilization,
    hidbtnDriversOff,
    hidbtnFixedPosLanding,
    hidbtnFixedPosBegining,
    hidbtnFixedPosVertical,
    hidbtnColorModeUp,
    hidbtnColorModeDown,
    hidbtnBombingSight,
    hidbtnScreenshot,
    hidbtnSnapshot,
    hidbtnSnapshotSeries,
    hidbtnTargetUnlock,
    hidbtnCamRecording,
    hidbtnAutomaticTracer,
    hidbtnDropBomb,
    hidbtnNewMarkerForTarget,
    hidbtnNewMarkerForLaser,
    hidbtnNewMarkerForUAV,
    hidbtnSettingsEditor,
    hidbtnDataConsole,
    hidbtnHelpViewer,
    hidbtnEmulatorConsole,
    hidbtnNormalFlight,
    hidbtnPatrolMovingTargetMode,
    hidbtnPatrolStaticTargetMode,
    hidbtnManualFlightMode,
    hidbtnLaserActivation,
    hidbtnSendHitCoordinates,
    hidbtnSendWeather,
    hidbtnMapZoomOut,
    hidbtnMapZoomIn,
    hidbtnFollowThePlane
};

//Singleton
class ApplicationSettings final: public ApplicationSettingsRoot
{
    Q_OBJECT

private:
    CamAssemblyPreferences *_camAssemblyPreferences;

    QList<CameraSettingsNode*> _cameraSettings;

    bool _hidButtonPrefsLoaded;
    QMap<HIDButton, ApplicationPreferenceString*> _hidKeyboardPrefs;
    QMap<HIDButton, ApplicationPreferenceInt*> _hidJoystickPrefs;
    QMap<HIDButton, QString> _hidCaptions;
    void ensureHIDButtonPrefsLoaded();
    void addHIDButtonPrefs(HIDButton prefIndex,
                           const QString &keyboardPrefName, const QString &keyboardPrefDefault,
                           const QString &joystickPrefName, const int joystickPrefDefult,
                           const QString &caption);

    ApplicationSettings();
    ~ApplicationSettings();

    ApplicationSettings(ApplicationSettings const&) = delete;
    ApplicationSettings& operator= (ApplicationSettings const&) = delete;

    void updateToCurrentVersion();
    void updateMapDatabaseFiles();
    void updateKeyboardShortcuts();
public:
    static ApplicationSettings& Instance();

    CamAssemblyPreferences *getCurrentCamAssemblyPreferences();

    CameraSettingsNode *installedCameraSettings();
    CameraSettingsNode *cameraSettings(int index);

    ApplicationPreferenceString *hidKeyboardPref(HIDButton prefIndex);
    ApplicationPreferenceInt *hidJoystickPref(HIDButton prefIndex);
    const QString hidCaption(HIDButton prefIndex);
    const QString hidUIHint(HIDButton prefIndex);

    ApplicationPreferenceInt InstalledCameraIndex;

    ApplicationPreferenceInt TileReceivingMode;
    ApplicationPreferenceString DatabaseMapDownloadCashe;
    ApplicationPreferenceString DatabaseHeightMap;
    ApplicationPreferenceString DatabaseGeocoder;
    ApplicationPreferenceString MarkerThesaurusDatabase;
    ApplicationPreferenceString MarkerStorageDatabase;
    ApplicationPreferenceString ArealObjectDatabase;
    ApplicationPreferenceString BallisticMacro;
    ApplicationPreferenceString SessionsFolder;
    ApplicationPreferenceInt LogFolderMaxSizeMb;
    ApplicationPreferenceBool LogFolderCleanup;
    ApplicationPreferenceInt CommandSendingInterval;
    ApplicationPreferenceEnum<CommandProtocols> CommandProtocol;
    ApplicationPreferenceEnum<CommandTransports> CommandTransport;
    ApplicationPreferenceInt CommandUDPPort;
    ApplicationPreferenceString CommandUDPAddress;
    ApplicationPreferenceString CommandSerialPortName;
    ApplicationPreferenceEnum<UAVTelemetrySourceTypes> UAVTelemetrySourceType;
    ApplicationPreferenceEnum<UAVTelemetryDataFormats> TelemetryDataFormat;
    ApplicationPreferenceInt UAVTelemetryUDPPort;
    ApplicationPreferenceBool UseCamTelemetryUDP;
    ApplicationPreferenceInt CamTelemetryUDPPort;
    ApplicationPreferenceBool UseExtTelemetryUDP;
    ApplicationPreferenceInt ExtTelemetryUDPPort;
    ApplicationPreferenceInt VideoLagFromTelemetry;
    ApplicationPreferenceBool EnableForwarding;
    ApplicationPreferenceString VideoForwardingAddress;
    ApplicationPreferenceInt VideoForwardingPort;
    ApplicationPreferenceString TelemetryForwardingAddress;
    ApplicationPreferenceInt TelemetryForwardingPort;
    ApplicationPreferenceBool UseCatapultLauncher;
    ApplicationPreferenceString CatapultSerialPortName;
    ApplicationPreferenceString CatapultCommand;
    ApplicationPreferenceBool EnableArtilleryMountNotification;
    ApplicationPreferenceString ArtilleryMountAddress;
    ApplicationPreferenceInt ArtilleryMountTCPPort;
    ApplicationPreferenceInt ExternalDataConsoleUDPPort;
    ApplicationPreferenceEnum<ObjectTrackerTypeEnum> ObjectTrackerType;
    ApplicationPreferenceInt TrackerCommandUDPPort;
    ApplicationPreferenceString TrackerCommandUDPAddress;
    ApplicationPreferenceInt TrackerTelemetryUDPPort;
    ApplicationPreferenceString TrackerTelemetryUDPAddress;
    ApplicationPreferenceEnum<ApplicationLanguages> InterfaceLanguage;
    ApplicationPreferenceEnum<DisplayModes> DisplayMode;
    ApplicationPreferenceBool MinimalisticDesign;
    ApplicationPreferenceInt MainFormViewPanelWidth;
    ApplicationPreferenceInt MainFormToolPanelWidth;
    ApplicationPreferenceBool MainFormViewPanelShowVideo;
    ApplicationPreferenceInt OSDLineWidth;
    ApplicationPreferenceBool UseFixedOSDLineWidth;
    ApplicationPreferenceEnum<OSDScreenCenterMarks> OSDScreenCenterMark;
    ApplicationPreferenceEnum<OSDGimbalIndicatorType> OSDGimbalIndicator;
    ApplicationPreferenceEnum<OSDGimbalIndicatorAngles> OSDGimbalAngles;
    ApplicationPreferenceInt OSDGimbalIndicatorSize;
    ApplicationPreferenceColor OSDScreenLinesColor;
    ApplicationPreferenceColor OSDScreenCenterMarkColor;
    ApplicationPreferenceBool OSDShowBombingSight;
    ApplicationPreferenceBool OSDShowCenterMark;
    ApplicationPreferenceBool OSDShowTelemetry;
    ApplicationPreferenceInt OSDTelemetryIndicatorFontSize;
    ApplicationPreferenceBool SoftwareStabilizationEnabled;
    ApplicationPreferenceEnum<ArtillerySpotterState> LastTargetArtillerySpotterState;

    ApplicationPreferenceBool AskAboutQuitOnSaveSettings;
    ApplicationPreferenceBool AskAboutQuitOnCloseApp;

    ApplicationPreferenceBool BombingTabAllowed;
    ApplicationPreferenceBool MarkersTabAllowed;
    ApplicationPreferenceBool ToolsTabAllowed;
    ApplicationPreferenceBool PatrolTabAllowed;
    ApplicationPreferenceBool CamControlsTabCoordIndicatorAllowed;
    ApplicationPreferenceString VisibleTelemetryTableRows;

    ApplicationPreferenceInt LastUsedMapBaseSourceId;
    ApplicationPreferenceInt LastUsedMapHybridSourceId;
    ApplicationPreferenceDoubleList TargetMarkerSizes;
    ApplicationPreferenceColor ViewFieldLineColor;
    ApplicationPreferenceInt ViewFieldLineWidth;
    ApplicationPreferenceColor TrajectoryPathLineColor;
    ApplicationPreferenceInt TrajectoryPathLineWidth;
    ApplicationPreferenceInt VisiblePathPointsPixelDistance;
    ApplicationPreferenceBool UseLaserRangefinderForGroundLevelCalculation;
    ApplicationPreferenceBool SoundEffectsAllowed;
    ApplicationPreferenceDouble SoundLevel;

    ApplicationPreferenceString HelpFilePath;
    ApplicationPreferenceBool KeyboardUsing;
    ApplicationPreferenceBool JoystickUsing;
    ApplicationPreferenceString JoystickMapping;
    ApplicationPreferenceDouble JoystickEmulationFromKeyboard;
    ApplicationPreferenceDouble JoystickAxisMultiplier;
    ApplicationPreferenceDouble JoystickAxisInsensitivity;
    ApplicationPreferenceInt JoystickAxisZoomIndex;
    ApplicationPreferenceInt JoystickAxisCameraXIndex;
    ApplicationPreferenceInt JoystickAxisCameraYIndex;
    ApplicationPreferenceInt JoystickAxisTrackerXIndex;
    ApplicationPreferenceInt JoystickAxisTrackerYIndex;
    ApplicationPreferenceBool UseZoomScaleForManualMoving;
    ApplicationPreferenceInt VideoFileFrameCount;
    ApplicationPreferenceInt VideoFileQuality;
    ApplicationPreferenceBool OVRDisplayTelemetry;
    ApplicationPreferenceInt OVRTelemetryIndicatorFontSize;
    ApplicationPreferenceEnum<OSDTelemetryTimeFormat> OVRTelemetryTimeFormat;
    ApplicationPreferenceBool OVRDisplayTargetRectangle;
    ApplicationPreferenceEnum<OSDGimbalIndicatorType> OVRGimbalIndicatorType;
    ApplicationPreferenceEnum<OSDGimbalIndicatorAngles> OVRGimbalIndicatorAngles;
    ApplicationPreferenceInt OVRGimbalIndicatorSize;
    ApplicationPreferenceEnum<GlobalCoordSystem> UIPresentationCoordSystem;
    ApplicationPreferenceBool EnableComputingStatistics;

    ApplicationPreferenceDouble EmulatorConsoleUavRoll;
    ApplicationPreferenceDouble EmulatorConsoleUavPitch;
    ApplicationPreferenceDouble EmulatorConsoleUavYaw;
    ApplicationPreferenceDouble EmulatorConsoleGpsLat;
    ApplicationPreferenceDouble EmulatorConsoleGpsLon;
    ApplicationPreferenceDouble EmulatorConsoleGpsHmsl;
    ApplicationPreferenceDouble EmulatorConsoleGpsCourse;

    // https://forum.qt.io/topic/72107/save-qlist-qstringlist-using-qsettings/3
    const QStringList getMapDatabaseFiles();
    void setMapDatabaseFiles(const QStringList &files);

    bool isStatisticViewLicensed();
    bool isLaserRangefinderLicensed();
    bool isPhotographyLicensed();
    bool isBombingTabLicensed();
    bool isTargetTabLicensed();
    bool isPatrolTabLicensed();
    bool isMarkersTabLicensed();
    bool isDataForwardingLicensed();
    bool isCatapultLicensed();
    bool isArtillerySpotterLicensed();
};

CameraSettingsNode *currCameraSettings();

#endif // APPLICATIONSETTINGS_H
