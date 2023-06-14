#include "ConstantNames.h"
#include "Constants.h"


ConstantNames::ConstantNames(QObject *parent) : QObject(parent)
{

}

ConstantNames::~ConstantNames()
{

}

const QMap<int, QString> ConstantNames::ArtillerySpotterStateCaptions()
{
    static const QMap<int, QString> mapArtillerySpotterStateCaptions {
        { ArtillerySpotterState::Unspecified,       tr("Unspecified") },
        { ArtillerySpotterState::DefeatRequired,    tr("Defeat Required") },
        { ArtillerySpotterState::TrialShot,         tr("Trial Shot") },
        { ArtillerySpotterState::RealShot,          tr("Real Shot") } };

    return mapArtillerySpotterStateCaptions;
}

const QMap<int, QString> ConstantNames::TileReceivingModeCaptions()
{
    static const QMap<int, QString> mapTileReceivingModeCaptions {
        { TileReceivingMode::DatabaseOnly, tr("Use Tiles from Database Only") },
        { TileReceivingMode::NetworkOnly, tr("Use Tiles from Network Only") },
        { TileReceivingMode::DatabaseAndNetwork, tr("Use Tiles from Database and Network")}
    };
    return mapTileReceivingModeCaptions;
}

const QMap<int, QString> ConstantNames::ColorModeCaptions()
{
    static const QMap<int, QString> mapColorModeCaptions = {
        { 0,    tr("Whitehot") },
        { 1,    tr("Blackhot") },
        { 2,    tr("Ranbow") },
        { 3,    tr("Three-primary colors") },
        { 4,    tr("Blue-red-yellow") },
        { 5,    tr("Blue-purple-red") },
        { 6,    tr("Mixed colors") },
        { 7,    tr("Blue-green-red") },
        { 8,    tr("Blackish green-red") },
        { 9,    tr("Lava") }
    };
    return mapColorModeCaptions;
}

const QMap<int, QString> ConstantNames::OVRGimbalIndicatorTypeCaptions()
{
    static const QMap<int, QString> mapOVRGimbalIndicatorTypeCaptions {
        { OSDGimbalIndicatorType::NoGimbal, tr("None") },
        { OSDGimbalIndicatorType::StaticPlane, tr("Static Plane") },
        { OSDGimbalIndicatorType::RotatingPlane, tr("Rotating Plane") },
        { OSDGimbalIndicatorType::CombinedPresentation, tr("Combined Presentation") }
    };
    return mapOVRGimbalIndicatorTypeCaptions;
}


const QMap<int, QString> ConstantNames::UAVTelemetryFormatCaptions()
{
    static const QMap<int, QString> mapUAVTelemetryFormatCaptions {
        { UAVTelemetryDataFormats::UAVTelemetryFormatV4, tr("V4") }
    };
    return mapUAVTelemetryFormatCaptions;
}

const QMap<int, QString> ConstantNames::CommandProtocolCaptions()
{
    static const QMap<int, QString> mapCommandProtocolCaptions {
        { CommandProtocols::MUSV, tr("MUSV") },
        { CommandProtocols::Otus, tr("Otus") },
        { CommandProtocols::MUSVDirect, tr("MUSV Direct") },
        { CommandProtocols::OtusDirect, tr("Otus Direct") }
    };
    return mapCommandProtocolCaptions;
}

const QMap<int, QString> ConstantNames::ObjectTrackerTypeCaptions()
{
    static const QMap<int, QString> mapObjectTrackerTypeCaptions {
        { ObjectTrackerTypeEnum::InternalCorrelation,   tr("Internal Tracker #1") },
        { ObjectTrackerTypeEnum::InternalRT,            tr("Internal Tracker #2") },
        { ObjectTrackerTypeEnum::InternalDefault,       tr("Internal Tracker #3") },
        { ObjectTrackerTypeEnum::External,              tr("External Tracker")    }
    };
    return mapObjectTrackerTypeCaptions;
}


const QMap<int, QString> ConstantNames::ApplicationLanguageCaptions()
{
    static const QMap<int, QString> mapApplicationLanguageCaptions {
        { ApplicationLanguages::English,    tr("English") },
        { ApplicationLanguages::Russian,    tr("Russian") },
        { ApplicationLanguages::Belarusian, tr("Belarusian") },
        { ApplicationLanguages::Arabic,     tr("Arabic") }
    };
    return mapApplicationLanguageCaptions;
}

const QMap<int, QString> ConstantNames::ApplicationStyleCaptions()
{
    static const QMap<int, QString> mapApplicationStyleCaptions {
        { DisplayModes::SingleDisplay, tr("Single Display") },
        { DisplayModes::Camera1Map2,   tr("Camera (1) & Map (2)") },
        { DisplayModes::Map1Camera2,   tr("Map (1) & Camera (2)") }
    };
    return mapApplicationStyleCaptions;
}

const QMap<int, QString> ConstantNames::CameraControlModeCaptions()
{
    static const QMap <int, QString> mapCameraControlModeCaptions {
        { CameraControlModes::AbsolutePosition, tr("Position") },
        { CameraControlModes::RotationSpeed, tr("Speed")}
    };
    return mapCameraControlModeCaptions;
}


const QMap<int, QString> ConstantNames::StabilizationTypeCaptions()
{
    static const QMap <int, QString> mapStabilizationTypeCaptions {
        { StabilizationType::StabilizationByFrame,  tr("Stabilization by Frame") },
        { StabilizationType::StabilizationByTarget, tr("Stabilization by Target")}
    };
    return mapStabilizationTypeCaptions;
}

const QMap<int, QString> ConstantNames::OSDScreenCenterMarkCaptions()
{
    static const QMap <int, QString> mapOSDScreenCenterMarkCaptions {
        { OSDScreenCenterMarks::SimpleCross,    tr("Cross in Center") },
        { OSDScreenCenterMarks::CircleCross,    tr("Circle in Center")},
        { OSDScreenCenterMarks::CrossAndRulers, tr("Cross and Rulers")}
    };
    return mapOSDScreenCenterMarkCaptions;
}

const QMap<int, QString> ConstantNames::VideoFrameTrafficSourceCaptions()
{
    static const QMap <int, QString> videoFrameTrafficSourceCaptions {
        { VideoFrameTrafficSources::USBCamera,    tr("USB Camera") },
        { VideoFrameTrafficSources::XPlane,    tr("X-Plane") },
        { VideoFrameTrafficSources::Yurion,    tr("Yurion") },
        { VideoFrameTrafficSources::CalibrationImage,    tr("Image File") },
        { VideoFrameTrafficSources::VideoFile,    tr("Video File") },
        { VideoFrameTrafficSources::RTSP,    tr("RTSP") },
        { VideoFrameTrafficSources::MUSV2,    tr("MUSV-2") }
    };
    return videoFrameTrafficSourceCaptions;
}
