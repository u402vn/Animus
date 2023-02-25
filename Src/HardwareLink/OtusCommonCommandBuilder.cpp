#include "OtusCommonCommandBuilder.h"

OtusDirectCommandBuilder::OtusDirectCommandBuilder(QObject *parent) : CommonCommandBuilder(parent)
{
}

BinaryContent OtusDirectCommandBuilder::SetupCamZoomCommand(float zoom)
{
    int fow[] = {10522, 10011, 9500, 8990, 8479, 7968, 7458, 6947, 6437, 5926, 5415, 4905, 4394, 3883, 3373, 2862, 2352, 1841, 1330, 820};
    int index = (int)zoom;
    index--;
    if (index < 0)
        index = 0;
    if (index > 19)
        index = 19;

    qint16 zoomInt = fow[index];

    BinaryContent command;
    command
            .appendChar(0x00)               // idRecv
            .appendChar(0x01)               // idSend
            .appendChar(0x02)               // classCode = Otusl205Cc.CameraA
            .appendChar(0x03)               // Set Horizontal Field of View
            .appendWord(quint16(zoomInt))   // arg
            .appendCharCRC_CCITT()          // CheckSum CRC-CCITT
            .byteStaffing(0x7D, 0x7D5D)
            .byteStaffing(0x7E, 0x7D5E)
            .appendChar(0x7E);              // EndFlag

    return command;
}

BinaryContent OtusDirectCommandBuilder::SetupCamPositionCommand(float roll, float pitch, float yaw)
{
    qint16 rollInt = (double)roll * 0xFFFF  / 360.0;
    qint16 pitchInt = (double)pitch * 0xFFFF  / 360.0;
    Q_UNUSED(yaw)

    BinaryContent command;
    command
            .appendChar(0x00)               // idRecv
            .appendChar(0x01)               // idSend
            .appendChar(0x01)               // classCode = Otusl205Cc.Gimbal
            .appendChar(0x05)               // msgCode, 0x07 - SetPosition
            .appendWord(quint16(rollInt))   // arg
            .appendWord(quint16(pitchInt))  // arg
            .appendCharCRC_CCITT()          // CheckSum CRC-CCITT
            .byteStaffing(0x7D, 0x7D5D)
            .byteStaffing(0x7E, 0x7D5E)
            .appendChar(0x7E);              // EndFlag

    return command;
}

BinaryContent OtusDirectCommandBuilder::SetupCamSpeedCommand(float roll, float pitch, float yaw)
{
    qint16 rollInt = (double)roll * 0xFFFF  / 360.0;
    qint16 pitchInt = (double)pitch * 0xFFFF  / 360.0;
    Q_UNUSED(yaw)

    BinaryContent command;
    command
            .appendChar(0x00)               // idRecv
            .appendChar(0x01)               // idSend
            .appendChar(0x01)               // classCode = Otusl205Cc.Gimbal
            .appendChar(0x05)               // msgCode, 0x05 - SetRate
            .appendWord(quint16(rollInt))   // arg
            .appendWord(quint16(pitchInt))  // arg
            .appendCharCRC_CCITT()          // CheckSum CRC-CCITT
            .byteStaffing(0x7D, 0x7D5D)
            .byteStaffing(0x7E, 0x7D5E)
            .appendChar(0x7E);              // EndFlag

    return command;
}

BinaryContent OtusDirectCommandBuilder::SetupHardwareCamStabilizationCommand(bool enabled)
{
    Q_UNUSED(enabled)
    return BinaryContent();
}


BinaryContent OtusDirectCommandBuilder::SetupCamMotorStatusCommand(bool enabled)
{
    BinaryContent command;
    command
            .appendChar(0x00)   // idRecv
            .appendChar(0x01)   // idSend
            .appendChar(0x01)   // classCode = Otusl205Cc.Gimbal
            .appendChar(0x03);
    if (enabled)
        command.appendChar(0x00); //0x00 - Joystic Speed Control, 0x04 - Encoder Positioning
    else
        command.appendChar(0x01);  //0x01 - Stow

    command.appendCharCRC_CCITT()
            .byteStaffing(0x7D, 0x7D5D)
            .byteStaffing(0x7E, 0x7D5E)
            .appendChar(0x7E);

    return command;
}


BinaryContent OtusDirectCommandBuilder::SelectActiveCamCommand(int camId)
{
    camId--;

    BinaryContent command;
    command
            .appendChar(0x00)                       // idRecv
            .appendChar(0x01)                       // idSend
            .appendChar(0x06)                       // classCode = Otusl205Cc.VideoOutput
            .appendChar(0x04)                       // msgCode = Otusl205VideoOutMc.SetVidSource
            .appendChar(static_cast<quint8>(camId)) // arg = source = [0, 1]
            .appendCharCRC_CCITT()                  // CheckSum CRC-CCITT
            .byteStaffing(0x7D, 0x7D5D)
            .byteStaffing(0x7E, 0x7D5E)
            .appendChar(0x7E);                      // EndFlag

    return command;
}


BinaryContent OtusDirectCommandBuilder::ParkingCommand()
{
    return BinaryContent();
}

BinaryContent OtusDirectCommandBuilder::SelectCamColorModeCommand(int colorMode)
{
    Q_UNUSED(colorMode);
    return BinaryContent();
}

BinaryContent OtusDirectCommandBuilder::SetLaserActivationCommand(bool active)
{
    Q_UNUSED(active);
    return BinaryContent();
}

BinaryContent OtusDirectCommandBuilder::MakeSnapshotCommand()
{
    return BinaryContent();
}


BinaryContent OtusDirectCommandBuilder::MakeSnapshotSeriesCommand(float intervalSec)
{
    Q_UNUSED(intervalSec);
    return BinaryContent();
}


BinaryContent OtusDirectCommandBuilder::StopSnapshotSeriesCommand()
{
    return BinaryContent();
}

BinaryContent OtusDirectCommandBuilder::StartCamRecordingCommand()
{
    return BinaryContent();
}

BinaryContent OtusDirectCommandBuilder::StopCamRecordingCommand()
{
    return BinaryContent();
}

BinaryContent OtusDirectCommandBuilder::DropBombCommand(int index)
{
    Q_UNUSED(index);
    return BinaryContent();
}

BinaryContent OtusDirectCommandBuilder::SetupRangefinderCommand(bool enabled)
{
    Q_UNUSED(enabled);
    return BinaryContent();
}

//===OtusCommandBuilder===========================================
BinaryContent OtusCommandBuilder::wrapForOtus(BinaryContent commandContent)
{
    int size = commandContent.size();
    if (size > 0)
    {
        commandContent
                .prependChar(size)
                .prependChar(8); // PayloadType.OtusL205
    }
    return commandContent;
}

OtusCommandBuilder::OtusCommandBuilder(QObject *parent) : OtusDirectCommandBuilder(parent)
{
}

BinaryContent OtusCommandBuilder::SetupCamZoomCommand(float zoom)
{
    return wrapForOtus(OtusDirectCommandBuilder::SetupCamZoomCommand(zoom));
}

BinaryContent OtusCommandBuilder::SetupCamPositionCommand(float roll, float pitch, float yaw)
{
    return wrapForOtus(OtusDirectCommandBuilder::SetupCamPositionCommand(roll, pitch, yaw));
}

BinaryContent OtusCommandBuilder::SetupCamSpeedCommand(float roll, float pitch, float yaw)
{
    return wrapForOtus(OtusDirectCommandBuilder::SetupCamSpeedCommand(roll, pitch, yaw));
}

BinaryContent OtusCommandBuilder::SetupCamMotorStatusCommand(bool enable)
{
    return wrapForOtus(OtusDirectCommandBuilder::SetupCamMotorStatusCommand(enable));;
}

BinaryContent OtusCommandBuilder::SelectActiveCamCommand(int camId)
{
    return wrapForOtus(OtusDirectCommandBuilder::SelectActiveCamCommand(camId));
}

BinaryContent OtusCommandBuilder::DropBombCommand(int index)
{
    BinaryContent command;
    command
            .appendChar(12)                          // MesType.Commands
            .appendChar(5)                           // MessageLength
            .appendChar(12)                          // ParamSign.Motor
            .appendChar(static_cast<quint8>(index))  // index
            .appendCharCRC()                         // CheckSum
            .prependChar(20);                        // PayloadType.BuselBomb

    return command;
}
