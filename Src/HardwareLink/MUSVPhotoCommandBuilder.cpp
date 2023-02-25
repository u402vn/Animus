#include "MUSVPhotoCommandBuilder.h"

BinaryContent MUSVPhotoCommandBuilder::wrapForMUSV(BinaryContent commandContent)
{
    if (commandContent.size() > 0)
    {
        commandContent.prependChar(9);                // PayloadType.MUSV
    }
    return commandContent;
}

MUSVPhotoCommandBuilder::MUSVPhotoCommandBuilder(QObject *parent) : MUSVPhotoDirectCommandBuilder(parent)
{
}

BinaryContent MUSVPhotoCommandBuilder::SetupCamPositionCommand(float roll, float pitch, float yaw)
{    
    return wrapForMUSV(MUSVPhotoDirectCommandBuilder::SetupCamPositionCommand(roll, pitch, yaw));
}

BinaryContent MUSVPhotoCommandBuilder::SetupCamSpeedCommand(float roll, float pitch, float yaw)
{
    return wrapForMUSV(MUSVPhotoDirectCommandBuilder::SetupCamSpeedCommand(roll, pitch, yaw));
}

BinaryContent MUSVPhotoCommandBuilder::SetupHardwareCamStabilizationCommand(bool enabled)
{
    return wrapForMUSV(MUSVPhotoDirectCommandBuilder::SetupHardwareCamStabilizationCommand(enabled));
}

BinaryContent MUSVPhotoCommandBuilder::SetupCamMotorStatusCommand(bool enabled)
{
    return wrapForMUSV(MUSVPhotoDirectCommandBuilder::SetupCamMotorStatusCommand(enabled));
}

BinaryContent MUSVPhotoCommandBuilder::SelectActiveCamCommand(int camId)
{
    return wrapForMUSV(MUSVPhotoDirectCommandBuilder::SelectActiveCamCommand(camId));
}

BinaryContent MUSVPhotoCommandBuilder::ParkingCommand()
{
    return wrapForMUSV(MUSVPhotoDirectCommandBuilder::ParkingCommand());
}

BinaryContent MUSVPhotoCommandBuilder::SelectCamColorModeCommand(int colorMode)
{
    return wrapForMUSV(MUSVPhotoDirectCommandBuilder::SelectCamColorModeCommand(colorMode));
}

BinaryContent MUSVPhotoCommandBuilder::SetLaserActivationCommand(bool active)
{
    return wrapForMUSV(MUSVPhotoDirectCommandBuilder::SetLaserActivationCommand(active));
}

BinaryContent MUSVPhotoCommandBuilder::SetupCamZoomCommand(float zoom)
{
    return wrapForMUSV(MUSVPhotoDirectCommandBuilder::SetupCamZoomCommand(zoom));
}

BinaryContent MUSVPhotoCommandBuilder::MakeSnapshotCommand()
{
    return wrapForMUSV(MUSVPhotoDirectCommandBuilder::MakeSnapshotCommand());
}

BinaryContent MUSVPhotoCommandBuilder::MakeSnapshotSeriesCommand(float intervalSec)
{
    return wrapForMUSV(MUSVPhotoDirectCommandBuilder::MakeSnapshotSeriesCommand(intervalSec));
}

BinaryContent MUSVPhotoCommandBuilder::StopSnapshotSeriesCommand()
{
    return wrapForMUSV(MUSVPhotoDirectCommandBuilder::StopSnapshotSeriesCommand());
}

BinaryContent MUSVPhotoCommandBuilder::StartCamRecordingCommand()
{
    return wrapForMUSV(MUSVPhotoDirectCommandBuilder::StartCamRecordingCommand());
}

BinaryContent MUSVPhotoCommandBuilder::StopCamRecordingCommand()
{
    return wrapForMUSV(MUSVPhotoDirectCommandBuilder::StopCamRecordingCommand());
}

BinaryContent MUSVPhotoCommandBuilder::DropBombCommand(int index)
{
    BinaryContent command;
    command
            .appendChar(12)                 // MesType.Commands
            .appendChar(5)                  // MessageLength
            .appendChar(12)                 // ParamSign.Motor
            .appendChar(index)              // index
            .appendCharCRC()                // CheckSum

            .prependChar(20);               // PayloadType.BuselBomb

    return command;
}

BinaryContent MUSVPhotoCommandBuilder::SetupRangefinderCommand(bool enabled)
{
    return wrapForMUSV(MUSVPhotoDirectCommandBuilder::SetupRangefinderCommand(enabled));
}

//------------------------------------------------------------------------------

MUSVPhotoDirectCommandBuilder::MUSVPhotoDirectCommandBuilder(QObject *parent) : CommonCommandBuilder(parent)
{
}

BinaryContent MUSVPhotoDirectCommandBuilder::SetupCamPositionCommand(float roll, float pitch, float yaw)
{
    // Is used for X-Plane and old cams
    BinaryContent command;
    command
            .appendChar(6)                  // MesType.Homing
            .appendChar(15)                 // MessageLength
            .appendFloatRev(roll)           // Roll
            .appendFloatRev(pitch)          // Pitch
            .appendFloatRev(yaw)            // Yaw
            .appendCharCRC();               // CheckSum

    return command;
}

BinaryContent MUSVPhotoDirectCommandBuilder::SetupCamSpeedCommand(float roll, float pitch, float yaw)
{
    // Is used for new cam
    BinaryContent command;
    command
            //.appendChar(10)               // MesType.AngleSpeed
            .appendChar(6)                  // MesType.Homing
            .appendChar(15)                 // MessageLength
            .appendFloatRev(roll)           // Roll
            .appendFloatRev(pitch)          // Pitch
            .appendFloatRev(yaw)            // Yaw
            .appendCharCRC();               // CheckSum

    return command;
}

BinaryContent MUSVPhotoDirectCommandBuilder::SetupHardwareCamStabilizationCommand(bool enabled)
{
    BinaryContent command;
    command
            .appendChar(8)                  // MesType.Settings
            .appendChar(5)                  // MessageLength
            .appendChar(6)                  // ParamSign.StabilizationMode
            .appendChar(enabled ? 1 : 0)    // enableStab
            .appendCharCRC();               // CheckSum

    return command;
}

BinaryContent MUSVPhotoDirectCommandBuilder::SetupCamMotorStatusCommand(bool enabled)
{
    BinaryContent command;
    command
            .appendChar(8)                  // MesType.Settings
            .appendChar(5)                  // MessageLength
            .appendChar(12)                 // ParamSign.Motor
            .appendChar(enabled ? 1 : 0)    // enableMotor
            .appendCharCRC();               // CheckSum

    return command;
}

BinaryContent MUSVPhotoDirectCommandBuilder::SelectActiveCamCommand(int camId)
{
    BinaryContent command;
    command
            .appendChar(8)                  // MesType.Settings
            .appendChar(5)                  // MessageLength
            .appendChar(9)                  // ParamSign.ChangeActive
            .appendChar(camId)              // CamId
            .appendCharCRC();               // CheckSum

    return command;
}

BinaryContent MUSVPhotoDirectCommandBuilder::ParkingCommand()
{
    BinaryContent command;
    command.appendHEX("060fc32a0000c2dc000000000000a0");
    return command;
}

BinaryContent MUSVPhotoDirectCommandBuilder::SelectCamColorModeCommand(int colorMode)
{
    BinaryContent command;
    command
            .appendChar(8)                  // MesType.Settings
            .appendChar(5)                  // MessageLength
            .appendChar(8)                  // ParamSign.Color
            .appendChar(colorMode)          // ColorMode
            .appendCharCRC();               // CheckSum

    return command;
}

BinaryContent MUSVPhotoDirectCommandBuilder::SetLaserActivationCommand(bool active)
{
    BinaryContent command;
    command
            .appendChar(8)                  // MesType.Settings
            .appendChar(5)                  // MessageLength
            .appendChar(30)                 // ParamSign.LaserActivation
            .appendChar(active ? 1 : 0)     // active laser
            .appendCharCRC();               // CheckSum

    return command;
}

BinaryContent MUSVPhotoDirectCommandBuilder::SetupCamZoomCommand(float zoom)
{
    BinaryContent command;
    command
            .appendChar(8)                  // MesType.Settings
            .appendChar(5)                  // MessageLength
            .appendChar(3)                  // ParamSign.Zoom
            .appendChar(zoom)               // ColorMode
            .appendCharCRC();               // CheckSum

    return command;
}

BinaryContent MUSVPhotoDirectCommandBuilder::MakeSnapshotCommand()
{
    BinaryContent command;
    command
            .appendChar(8)                  // MesType.Settings
            .appendChar(7)                  // MessageLength
            .appendChar(2)                  // ParamSign.PhotoParams
            .appendChar(3)                  // PhotoMode
            .appendWord(0)                  // SnapshotInterval
            .appendCharCRC();               // CheckSum

    return command;
}

BinaryContent MUSVPhotoDirectCommandBuilder::MakeSnapshotSeriesCommand(float intervalSec)
{
    BinaryContent command;
    command
            .appendChar(8)                  // MesType.Settings
            .appendChar(7)                  // MessageLength
            .appendChar(2)                  // ParamSign.PhotoParams
            .appendChar(5)                  // PhotoMode
            .appendWord(intervalSec * 100)  // SnapshotInterval
            .appendCharCRC();               // CheckSum

    return command;
}

BinaryContent MUSVPhotoDirectCommandBuilder::StopSnapshotSeriesCommand()
{
    BinaryContent command;
    command
            .appendChar(8)                  // MesType.Settings
            .appendChar(7)                  // MessageLength
            .appendChar(2)                  // ParamSign.PhotoParams
            .appendChar(0)                  // PhotoMode
            .appendWord(0)                  // SnapshotInterval
            .appendCharCRC();               // CheckSum

    return command;
}

BinaryContent MUSVPhotoDirectCommandBuilder::StartCamRecordingCommand()
{
    BinaryContent command;
    command.appendHEX("08070207000018");
    return command;
}

BinaryContent MUSVPhotoDirectCommandBuilder::StopCamRecordingCommand()
{
    BinaryContent command;
    command.appendHEX("08070200000011");
    return command;
}

BinaryContent MUSVPhotoDirectCommandBuilder::DropBombCommand(int index)
{
    BinaryContent command;
    //cannot be used in direct connection
    return command;
}

BinaryContent MUSVPhotoDirectCommandBuilder::SetupRangefinderCommand(bool enabled)
{
    BinaryContent command;
    command.appendHEX("08070200000011"); //??? enabled
    return command;
}

/*
const char msg_length = 5;
QByteArray command(msg_length, 0);
command[0] = 8;                 // MesType.Settings
command[1] = msg_length;        // MsgLength
command[2] = 3;                 // ParamSign.Zoom
command[3] = char(zoom);        // zoom value
command[4] = 0;                 // CheckSum
updateCheckSum(command);

return command;
*/
/*
  byte[] data = MusSettings.ZoomMsg(zoom);

    public static byte[] ZoomMsg(byte zoom)
    {
        byte[] x = new byte[2];
        x[0] = (byte)ParamSign.Zoom;
        x[1] = zoom;
        return x;
    }

    GVU.Payload.Protocols.MusMsg(data, GVU.Payload.Protocols.MesType.Settings);


//перечисление признаков настроек
enum ParamSign
{
    PowerMode = 1,
    PhotoParams = 2,
    Zoom = 3,
    Exposition = 4,
    Frequency = 5,
    StabilizationMode = 6,
    PWMMode = 7,
    ColorMode = 8,
    ChangeActive = 9,
    Focus = 10,
    Calibration = 11,
    Motor = 12,
    Format=13,
    wifi = 14,
    zero = 15

    LaserActivation = 30

    TargetSize = 20
    LockTarget = 21
    UnlockTarget = 22
}

enum MesType
{
    ParamInf = 2,
    CurStateReq = 4,
    Homing = 6,
    Settings = 8,
    AngleSpeed = 10,
    Establish = 11,
    Commands = 12,
    SensorAdjust = 14,
    Report = 1,
    AnglePosition = 3
}

    public MusMsg(byte[] bytes, MesType mt)
    {
        MsgType = (byte)mt;
        MsgLength = (byte)(bytes.Length+3);
        MsgData = bytes;
        CheckSum = CalcCheckSum();
    }

    public byte[] ToBytes()
    {
        byte[] x = new byte[MsgLength];
        x[0] = MsgType;
        x[1] = MsgLength;
        MsgData.CopyTo(x, 2);
        x[MsgLength-1] = CheckSum;
        return x;
    }

    private byte CalcCheckSum()
    {
        byte sum = (byte)(+MsgType+MsgLength);
        for (int i = 0; i < MsgLength-3; i++)
        {
            sum += MsgData[i];
        }
        return sum;
    }

 */
