#ifndef COMMONCOMMANDBUILDER_H
#define COMMONCOMMANDBUILDER_H

#include <QObject>
#include "Common/BinaryContent.h"

class CommonCommandBuilder : public QObject
{
    Q_OBJECT
public:
    CommonCommandBuilder(QObject *parent);

    virtual BinaryContent SetupCamZoomCommand(float zoom) = 0;
    virtual BinaryContent SetupCamPositionCommand(float roll, float pitch, float yaw) = 0;
    virtual BinaryContent SetupCamSpeedCommand(float roll, float pitch, float yaw) = 0;
    virtual BinaryContent SetupHardwareCamStabilizationCommand(bool enabled) = 0;
    virtual BinaryContent SetupCamMotorStatusCommand(bool enabled) = 0;
    virtual BinaryContent SelectActiveCamCommand(int camId) = 0;
    virtual BinaryContent ParkingCommand() = 0;
    virtual BinaryContent SelectCamColorModeCommand(int colorMode) = 0;
    virtual BinaryContent SetLaserActivationCommand(bool active) = 0;
    virtual BinaryContent MakeSnapshotCommand() = 0;
    virtual BinaryContent MakeSnapshotSeriesCommand(float intervalSec) = 0;
    virtual BinaryContent StopSnapshotSeriesCommand() = 0;
    virtual BinaryContent StartCamRecordingCommand() = 0;
    virtual BinaryContent StopCamRecordingCommand() = 0;
    virtual BinaryContent DropBombCommand(int index) = 0;
    virtual BinaryContent SetupRangefinderCommand(bool enabled) = 0;
};

#endif // COMMONCOMMANDBUILDER_H
