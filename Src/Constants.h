#ifndef CONSTANTS
#define CONSTANTS

enum TileReceivingMode
{
    DatabaseOnly = 0,
    NetworkOnly,
    DatabaseAndNetwork
};

enum CameraSuspensionTypes
{
    FixedCamera = 0,
    RotatingCamera
};

enum CameraControlModes
{
    AbsolutePosition = 0,
    RotationSpeed
};

enum ApplicationLanguages
{
    English = 0,
    Russian,
    Belarusian,
    Arabic
};

enum DisplayModes
{
    SingleDisplay = 0,
    Camera1Map2,
    Map1Camera2
};

enum UAVTelemetryDataFormats
{
    UAVTelemetryFormatV1 = 0,
    UAVTelemetryFormatV2,
    UAVTelemetryFormatV3,
    UAVTelemetryFormatV4,
    UAVTelemetryFormatV4_1,
    UAVTelemetryFormatUnknown
};

enum VideoFrameTrafficSources
{
    USBCamera = 0,
    XPlane,
    Yurion,
    CalibrationImage,
    VideoFile,
    RTSP,
    MUSV2,
    LastEelemet = MUSV2
};

enum UAVTelemetrySourceTypes
{
    UDPChannel,
    Emulator
};

enum CommandProtocols
{
    MUSV = 0,
    Otus,
    MUSVDirect,
    OtusDirect
};

enum CommandTransports
{
    UDP = 0,
    Serial
};

enum OSDScreenCenterMarks
{
    SimpleCross = 0,
    CircleCross,
    CrossAndRulers,
    Cross2,
    Cross3
};

enum OSDGimbalIndicatorType
{
    NoGimbal = 0,
    StaticPlane,
    RotatingPlane,
    CombinedPresentation
};

enum OSDGimbalIndicatorAngles
{
    NoAngles = 0,
    AbsoluteAngles,
    RelativeAngles
};

enum AutomaticTracerMode
{
    atmSleep = 0,
    atmScreenPoint,
    atmEncoderValues
};

enum VoiceMessage
{
    AnumusActivated = 0,
    DropBomb,
    TurnLeft,
    TurnRight,
    Lapel,
    TargetLocked,
    TargetDropped,
    ZoomChange
};

enum ObjectTrackerTypeEnum
{
    InternalCorrelation,
    InternalRT,
    InternalDefault,
    External
};

enum OSDTelemetryTimeFormat
{
    NoDateTime,
    CurrentDateTime,
    SessionTime
};


enum ArtillerySpotterState
{
    Unspecified = 0,
    DefeatRequired = 1,
    TrialShot = 2,
    RealShot = 3,
    EllipseCenter = 10
};


enum StabilizationType
{
    StabilizationByFrame = 0,
    StabilizationByTarget = 1
};
#endif // CONSTANTS
