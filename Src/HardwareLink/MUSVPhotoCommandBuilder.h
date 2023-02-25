#ifndef MUSVPHOTOCOMMANDBUILDER_H
#define MUSVPHOTOCOMMANDBUILDER_H

#include <QObject>
#include "HardwareLink/CommonCommandBuilder.h"

class MUSVPhotoDirectCommandBuilder : public CommonCommandBuilder
{
    Q_OBJECT
public:
    explicit MUSVPhotoDirectCommandBuilder(QObject *parent);

    virtual BinaryContent SetupCamZoomCommand(float zoom);
    virtual BinaryContent SetupCamPositionCommand(float roll, float pitch, float yaw);
    virtual BinaryContent SetupCamSpeedCommand(float roll, float pitch, float yaw);
    virtual BinaryContent SetupHardwareCamStabilizationCommand(bool enabled);
    virtual BinaryContent SetupCamMotorStatusCommand(bool enabled);
    virtual BinaryContent SelectActiveCamCommand(int camId);
    virtual BinaryContent ParkingCommand();
    virtual BinaryContent SelectCamColorModeCommand(int colorMode);
    virtual BinaryContent SetLaserActivationCommand(bool active);
    virtual BinaryContent MakeSnapshotCommand();
    virtual BinaryContent MakeSnapshotSeriesCommand(float intervalSec);
    virtual BinaryContent StopSnapshotSeriesCommand();
    virtual BinaryContent StartCamRecordingCommand();
    virtual BinaryContent StopCamRecordingCommand();
    virtual BinaryContent DropBombCommand(int index);
    virtual BinaryContent SetupRangefinderCommand(bool enabled);
};

class MUSVPhotoCommandBuilder final : public MUSVPhotoDirectCommandBuilder
{
    Q_OBJECT
    BinaryContent wrapForMUSV(BinaryContent commandContent);
public:
    explicit MUSVPhotoCommandBuilder(QObject *parent);

    virtual BinaryContent SetupCamZoomCommand(float zoom);
    virtual BinaryContent SetupCamPositionCommand(float roll, float pitch, float yaw);
    virtual BinaryContent SetupCamSpeedCommand(float roll, float pitch, float yaw);
    virtual BinaryContent SetupHardwareCamStabilizationCommand(bool enabled);
    virtual BinaryContent SetupCamMotorStatusCommand(bool enabled);
    virtual BinaryContent SelectActiveCamCommand(int camId);
    virtual BinaryContent ParkingCommand();
    virtual BinaryContent SelectCamColorModeCommand(int colorMode);
    virtual BinaryContent SetLaserActivationCommand(bool active);

    virtual BinaryContent MakeSnapshotCommand();
    virtual BinaryContent MakeSnapshotSeriesCommand(float intervalSec);
    virtual BinaryContent StopSnapshotSeriesCommand();
    virtual BinaryContent StartCamRecordingCommand();
    virtual BinaryContent StopCamRecordingCommand();
    virtual BinaryContent DropBombCommand(int index);
    virtual BinaryContent SetupRangefinderCommand(bool enabled);
};

#endif // MUSVPHOTOCOMMANDBUILDER_H
