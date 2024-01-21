#ifndef APPLICATIONSETTINGS_H
#define APPLICATIONSETTINGS_H

#include <QColor>
#include <QMap>
#include <QStringList>
#include <QCameraInfo>
#include "ApplicationSettingsImpl.h"
#include "Common/CommonData.h"
#include "CamPreferences.h"
#include "Constants.h"

const int ApplicationRestartExitCode = 1982;

const QString DefaultCalibrationImagePath = ":/CalibrationImages/WorldMap1.jpg";


class VideoConnectionSettings : public QObject
{
    Q_OBJECT
public:
    VideoConnectionSettings(QObject *parent,
                            ApplicationPreferenceEnum<VideoFrameTrafficSources> *VideoTrafficSource,
                            ApplicationPreferenceString *VideoFrameSourceCameraName,
                            ApplicationPreferenceString *VideoFrameSourceXPlaneAddress,
                            ApplicationPreferenceInt *VideoFrameSourceXPlanePort,
                            ApplicationPreferenceInt *VideoFrameSourceYurionUDPPort,
                            ApplicationPreferenceString *CalibrationImagePath,
                            ApplicationPreferenceString *VideoFilePath,
                            ApplicationPreferenceString *RTSPUrl,
                            ApplicationPreferenceInt *VideoFrameSourceMUSV2UDPPort);

    ApplicationPreferenceEnum<VideoFrameTrafficSources> *VideoTrafficSource;
    ApplicationPreferenceString *VideoFrameSourceCameraName;
    ApplicationPreferenceString *VideoFrameSourceXPlaneAddress;
    ApplicationPreferenceInt *VideoFrameSourceXPlanePort;
    ApplicationPreferenceInt *VideoFrameSourceYurionUDPPort;
    ApplicationPreferenceString *CalibrationImagePath;
    ApplicationPreferenceString *VideoFilePath;
    ApplicationPreferenceString *RTSPUrl;
    ApplicationPreferenceInt *VideoFrameSourceMUSV2UDPPort;

    const QString description();
};

class OpticalDeviceSettings : public QObject
{
    Q_OBJECT
public:
    OpticalDeviceSettings(QObject *parent,
                          ApplicationPreferenceInt *DeviceLinkId,
                          ApplicationPreferenceDouble *CamZoomMax,
                          ApplicationPreferenceDouble *CamZoomMin,
                          ApplicationPreferenceDouble *FixedPosLandingZoom,
                          ApplicationPreferenceDouble *FixedPosBeginingZoom,
                          ApplicationPreferenceDouble *FixedPosVerticalZoom,
                          ApplicationPreferenceDoubleList *CamScaleCoefficient,
                          ApplicationPreferenceDoubleList *CamViewAnglesHorizontal,
                          ApplicationPreferenceDoubleList *CamViewAnglesVertical,
                          ApplicationPreferenceDoubleList *CamAutomaticTracerSpeedMultipliers,
                          ApplicationPreferenceDoubleList *CamManualSpeedMultipliers,
                          ApplicationPreferenceInt *CamViewSizeHorizontal,
                          ApplicationPreferenceInt *CamViewSizeVertical,
                          ApplicationPreferenceBool *CamViewSizeForceSet,
                          ApplicationPreferenceBool *UseVerticalFrameMirrororing,
                          ApplicationPreferenceInt *MagnifierSourceSize,
                          ApplicationPreferenceDouble *MagnifierScale);

    ApplicationPreferenceInt *DeviceLinkId;
    ApplicationPreferenceDouble *CamZoomMax;
    ApplicationPreferenceDouble *CamZoomMin;
    ApplicationPreferenceDouble *FixedPosLandingZoom;
    ApplicationPreferenceDouble *FixedPosBeginingZoom;
    ApplicationPreferenceDouble *FixedPosVerticalZoom;
    ApplicationPreferenceDoubleList *CamScaleCoefficient;
    ApplicationPreferenceDoubleList *CamViewAnglesHorizontal;
    ApplicationPreferenceDoubleList *CamViewAnglesVertical;
    ApplicationPreferenceDoubleList *CamAutomaticTracerSpeedMultipliers;
    ApplicationPreferenceDoubleList *CamManualSpeedMultipliers;
    ApplicationPreferenceInt *CamViewSizeHorizontal;
    ApplicationPreferenceInt *CamViewSizeVertical;
    ApplicationPreferenceBool *CamViewSizeForceSet;
    ApplicationPreferenceBool *UseVerticalFrameMirrororing;
    ApplicationPreferenceInt *MagnifierSourceSize;
    ApplicationPreferenceDouble *MagnifierScale;

    const QString description();
};

class CameraSettingsNode final : public ApplicationSettingsNode
{
    Q_OBJECT

    friend class ApplicationSettings;

    CameraSettingsNode(ApplicationSettingsImpl *parentSettings, const QString &nodeName);
    ~CameraSettingsNode();

    CameraSettingsNode(CameraSettingsNode const&) = delete;
    CameraSettingsNode& operator= (CameraSettingsNode const&) = delete;

    // Video Connection 1
    ApplicationPreferenceEnum<VideoFrameTrafficSources> VideoTrafficSource1;
    ApplicationPreferenceString VideoFrameSourceCameraName1;
    ApplicationPreferenceString VideoFrameSourceXPlaneAddress1;
    ApplicationPreferenceInt VideoFrameSourceXPlanePort1;
    ApplicationPreferenceInt VideoFrameSourceYurionUDPPort1;
    ApplicationPreferenceString CalibrationImagePath1;
    ApplicationPreferenceString VideoFilePath1;
    ApplicationPreferenceString RTSPUrl1;
    ApplicationPreferenceInt VideoFrameSourceMUSV2UDPPort1;
    // Video Connection 2
    ApplicationPreferenceEnum<VideoFrameTrafficSources> VideoTrafficSource2;
    ApplicationPreferenceString VideoFrameSourceCameraName2;
    ApplicationPreferenceString VideoFrameSourceXPlaneAddress2;
    ApplicationPreferenceInt VideoFrameSourceXPlanePort2;
    ApplicationPreferenceInt VideoFrameSourceYurionUDPPort2;
    ApplicationPreferenceString CalibrationImagePath2;
    ApplicationPreferenceString VideoFilePath2;
    ApplicationPreferenceString RTSPUrl2;
    ApplicationPreferenceInt VideoFrameSourceMUSV2UDPPort2;

    // Optical system A
    ApplicationPreferenceInt DeviceLinkIdA;
    ApplicationPreferenceDouble CamZoomMaxA;
    ApplicationPreferenceDouble CamZoomMinA;
    ApplicationPreferenceDouble FixedPosLandingZoomA;
    ApplicationPreferenceDouble FixedPosBeginingZoomA;
    ApplicationPreferenceDouble FixedPosVerticalZoomA;
    ApplicationPreferenceDoubleList CamScaleCoefficientA;
    ApplicationPreferenceDoubleList CamViewAnglesHorizontalA;
    ApplicationPreferenceDoubleList CamViewAnglesVerticalA;
    ApplicationPreferenceDoubleList CamAutomaticTracerSpeedMultipliersA;
    ApplicationPreferenceDoubleList CamManualSpeedMultipliersA;
    ApplicationPreferenceInt CamViewSizeHorizontalA;
    ApplicationPreferenceInt CamViewSizeVerticalA;
    ApplicationPreferenceBool CamViewSizeForceSetA;
    ApplicationPreferenceBool UseVerticalFrameMirrororingA;
    ApplicationPreferenceInt MagnifierSourceSizeA;
    ApplicationPreferenceDouble MagnifierScaleA;
    // Optical system B
    ApplicationPreferenceInt DeviceLinkIdB;
    ApplicationPreferenceDouble CamZoomMaxB;
    ApplicationPreferenceDouble CamZoomMinB;
    ApplicationPreferenceDouble FixedPosLandingZoomB;
    ApplicationPreferenceDouble FixedPosBeginingZoomB;
    ApplicationPreferenceDouble FixedPosVerticalZoomB;
    ApplicationPreferenceDoubleList CamScaleCoefficientB;
    ApplicationPreferenceDoubleList CamViewAnglesHorizontalB;
    ApplicationPreferenceDoubleList CamViewAnglesVerticalB;
    ApplicationPreferenceDoubleList CamAutomaticTracerSpeedMultipliersB;
    ApplicationPreferenceDoubleList CamManualSpeedMultipliersB;
    ApplicationPreferenceInt CamViewSizeHorizontalB;
    ApplicationPreferenceInt CamViewSizeVerticalB;
    ApplicationPreferenceBool CamViewSizeForceSetB;
    ApplicationPreferenceBool UseVerticalFrameMirrororingB;
    ApplicationPreferenceInt MagnifierSourceSizeB;
    ApplicationPreferenceDouble MagnifierScaleB;
    // Optical system C
    ApplicationPreferenceInt DeviceLinkIdC;
    ApplicationPreferenceDouble CamZoomMaxC;
    ApplicationPreferenceDouble CamZoomMinC;
    ApplicationPreferenceDouble FixedPosLandingZoomC;
    ApplicationPreferenceDouble FixedPosBeginingZoomC;
    ApplicationPreferenceDouble FixedPosVerticalZoomC;
    ApplicationPreferenceDoubleList CamScaleCoefficientC;
    ApplicationPreferenceDoubleList CamViewAnglesHorizontalC;
    ApplicationPreferenceDoubleList CamViewAnglesVerticalC;
    ApplicationPreferenceDoubleList CamAutomaticTracerSpeedMultipliersC;
    ApplicationPreferenceDoubleList CamManualSpeedMultipliersC;
    ApplicationPreferenceInt CamViewSizeHorizontalC;
    ApplicationPreferenceInt CamViewSizeVerticalC;
    ApplicationPreferenceBool CamViewSizeForceSetC;
    ApplicationPreferenceBool UseVerticalFrameMirrororingC;
    ApplicationPreferenceInt MagnifierSourceSizeC;
    ApplicationPreferenceDouble MagnifierScaleC;

    QMap<qint32, VideoConnectionSettings*> _videoConnectionSettings;
    QMap<qint32, OpticalDeviceSettings*> _opticalDeviceSettings;
public:
    ApplicationPreferenceString UserDescription;
    ApplicationPreferenceInt PhisycalLensCount;
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
    ApplicationPreferenceDouble EncoderAutomaticTracerMultiplier;

    ApplicationPreferenceDouble FixedPosLandingYaw;
    ApplicationPreferenceDouble FixedPosLandingPitch;
    ApplicationPreferenceDouble FixedPosLandingRoll;
    ApplicationPreferenceBool   FixedPosLandingCommand;

    ApplicationPreferenceDouble FixedPosBeginingYaw;
    ApplicationPreferenceDouble FixedPosBeginingPitch;
    ApplicationPreferenceDouble FixedPosBeginingRoll;

    ApplicationPreferenceDouble FixedPosVerticalYaw;
    ApplicationPreferenceDouble FixedPosVerticalPitch;
    ApplicationPreferenceDouble FixedPosVerticalRoll;

    ApplicationPreferenceString BombingSightNumbers;

    VideoConnectionSettings *videoConnectionSetting(int connectionId);
    OpticalDeviceSettings *opticalDeviceSetting(int deviceId);

    const QString description();
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
    hidbtnEnableSoftwareStab,
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
    hidbtnFollowThePlane,
    hidbtnTargetLockInCursor,
    hidbtnMagnifier
};

//Singleton
class ApplicationSettings final: public ApplicationSettingsRoot
{
    Q_OBJECT

private:
    CamAssemblyPreferences *_camAssemblyPreferences;

    QList<CameraSettingsNode*> _cameraSettings;

    QStringList _licensedModules;

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
    CameraSettingsNode *cameraPreferences(int index);

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
    ApplicationPreferenceInt VideoLagFromCameraTelemetry;
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
    ApplicationPreferenceBool ShowExternalTrackerRectangle;
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
    ApplicationPreferenceColor OSDTargetTrackerCursor;
    ApplicationPreferenceBool OSDCursorMarkVisibility;
    ApplicationPreferenceInt OSDCursorMarkVisibilityTimeout;
    ApplicationPreferenceBool SoftwareStabilizationEnabled;
    ApplicationPreferenceEnum<StabilizationType> VideoStabilizationType;
    ApplicationPreferenceEnum<ArtillerySpotterState> LastTargetArtillerySpotterState;
    ApplicationPreferenceInt CoordCalulationHistoryMs;

    ApplicationPreferenceBool AskAboutQuitOnSaveSettings;
    ApplicationPreferenceBool AskAboutQuitOnCloseApp;

    ApplicationPreferenceBool BombingTabAllowed;
    ApplicationPreferenceBool MarkersTabAllowed;
    ApplicationPreferenceBool ToolsTabAllowed;
    ApplicationPreferenceBool PatrolTabAllowed;
    ApplicationPreferenceBool AntennaTabAllowed;
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
    ApplicationPreferenceDouble JoystickCameraEmulationFromKeyboard;
    ApplicationPreferenceDouble JoystickCursorEmulationFromKeyboard;
    ApplicationPreferenceDouble JoystickCameraAxisMultiplier;
    ApplicationPreferenceDouble JoystickCursorAxisMultiplier;
    ApplicationPreferenceDouble JoystickCameraAxisInsensitivity;
    ApplicationPreferenceDouble JoystickCursorAxisInsensitivity;
    ApplicationPreferenceInt JoystickAxisZoomIndex;
    ApplicationPreferenceInt JoystickAxisCameraXIndex;
    ApplicationPreferenceInt JoystickAxisCameraYIndex;
    ApplicationPreferenceInt JoystickAxisCursorXIndex;
    ApplicationPreferenceInt JoystickAxisCursorYIndex;
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

    ApplicationPreferenceString LastAntennaCoord;

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
    bool isAntennaLicensed();
};

CameraSettingsNode *currCameraSettings();

#endif // APPLICATIONSETTINGS_H
