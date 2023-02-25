#ifndef OTUSCOMMONCOMMANDBUILDER_H
#define OTUSCOMMONCOMMANDBUILDER_H

#include <QObject>
#include "HardwareLink/CommonCommandBuilder.h"

class OtusDirectCommandBuilder : public CommonCommandBuilder
{    
public:
    explicit OtusDirectCommandBuilder(QObject *parent);

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

class OtusCommandBuilder final : public OtusDirectCommandBuilder
{
private:
    BinaryContent wrapForOtus(BinaryContent commandContent);
public:
    explicit OtusCommandBuilder(QObject *parent);

    virtual BinaryContent SetupCamZoomCommand(float zoom);
    virtual BinaryContent SetupCamPositionCommand(float roll, float pitch, float yaw);
    virtual BinaryContent SetupCamSpeedCommand(float roll, float pitch, float yaw);
    virtual BinaryContent SetupCamMotorStatusCommand(bool enable);
    virtual BinaryContent SelectActiveCamCommand(int camId);
    virtual BinaryContent DropBombCommand(int index);
};

#endif // OTUSCOMMONCOMMANDBUILDER_H
