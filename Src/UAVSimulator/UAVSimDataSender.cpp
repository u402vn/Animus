#include "UAVSimDataSender.h"
#include "HardwareLink/lz4.h"
#include "DataAccess/csv.h"
#include <QFileInfo>
#include <QImage>
#include <QStringList>
#include <QtMath>

const int CUDPPACKETVIDEODATALENGTH = 720;

#pragma pack(push, 1)
struct UDPSimpleVideoMessage {
    unsigned frameNumber;
    unsigned short line;
    unsigned short part;
    unsigned char frameData[CUDPPACKETVIDEODATALENGTH];
};
#pragma pack(pop)

const int  XPLANE_PACKET_CHUNKSIZE = 1400;
const int  XPLANE_IMAGE_WIDTH      =  720;
const int  XPLANE_IMAGE_HEIGHT     =  576;
const int  XPLANE_IMAGE_BPP        =    4;
const int  XPLANE_IMAGE_DATA_SIZE  = XPLANE_IMAGE_WIDTH * XPLANE_IMAGE_HEIGHT * XPLANE_IMAGE_BPP;

#pragma pack(push,1)
struct XPLANE_PACKET {
    unsigned       frameId        = 0;
    int            frameTotalSize = 0;
    unsigned short framePartNo    = 0;
    quint8         frameData[XPLANE_PACKET_CHUNKSIZE];
};
#pragma pack(pop)

void UAVSimDataSender::updateCamPitchRoll()
{

    if (_controlMode == AbsolutePosition)
    {
        if (_camX < _camXTarget)
            _camX++;
        else if (_camX > _camXTarget)
            _camX--;
        if (qAbs(_camX - _camXTarget) < 1)
            _camX = _camXTarget;

        if (_camY < _camYTarget)
            _camY++;
        else if (_camY > _camYTarget)
            _camY--;
        if (qAbs(_camY - _camYTarget) < 1)
            _camY = _camYTarget;
    }
    else
    {
        _camX += _camXTarget;
        _camY += _camYTarget;
    }

    if (_camZoom < _camZoomTarget)
        _camZoom++;
    else if (_camZoom > _camZoomTarget)
        _camZoom--;
    if (qAbs(_camZoom - _camZoomTarget) < 1)
        _camZoom = _camZoomTarget;
}

bool UAVSimDataSender::loadFromCSV(const QString &fileName)
{
    _currentMessageNumber = 5000; //???

    QFileInfo checkFile(fileName);
    if (!checkFile.exists() || !checkFile.isFile())
        return false;

    double roll, pitch, yaw, gps_lat, gps_lon, gps_hmsl, gps_course, gps_Vnorth, gps_Veast, airspeed, windSpd, windHdg;

    io::CSVReader<12> telemetryFile(fileName.toLocal8Bit().constData());
    telemetryFile.read_header(io::ignore_extra_column, "roll", "pitch", "yaw", "gps_lat", "gps_lon", "gps_hmsl", "gps_course", "gps_Vnorth", "gps_Veast", "airspeed", "windSpd", "windHdg");
    while(telemetryFile.read_row(roll, pitch, yaw, gps_lat, gps_lon, gps_hmsl, gps_course, gps_Vnorth, gps_Veast, airspeed, windSpd, windHdg))
    {
        UDPSimulatorTelemetryMessageV4 telemetryMessage;

        telemetryMessage.UavRoll = roll;
        telemetryMessage.UavPitch = pitch;
        telemetryMessage.UavYaw = yaw;
        telemetryMessage.UavLatitude_GPS = gps_lat;
        telemetryMessage.UavLongitude_GPS = gps_lon;
        telemetryMessage.UavAltitude_GPS = gps_hmsl;
        telemetryMessage.Course_GPS = gps_course;
        telemetryMessage.GroundSpeed_GPS = 0;
        telemetryMessage.AirSpeed = airspeed;

        telemetryMessage.GroundSpeedNorth_GPS = gps_Vnorth;
        telemetryMessage.GroundSpeedEast_GPS = gps_Veast;

        telemetryMessage.WindSpeed = windSpd;
        telemetryMessage.WindDirection = windHdg;

        telemetryMessage.CamRoll = 0;
        telemetryMessage.CamPitch = 0;
        telemetryMessage.CamYaw = 0;
        telemetryMessage.CamZoom = 1;

        telemetryMessage.RangefinderDistance = 0;

        _telemetryMessages.append(telemetryMessage);
    }
    return true;
}

void UAVSimDataSender::loadYurionVideo(const QString &fileName)
{
    QFile file(fileName);
    if (file.open(QIODevice::ReadOnly))
        _urionVideo = file.readAll();
    _urionVideoPosition = 0;

    const unsigned char preamble[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x57, 0x44};
    const QByteArray prbyte(reinterpret_cast<const char*>(preamble), 8);

    qDebug() << "URION Frame Postions: " << endl;
    int startFrameIdx = 0, prevPos = 0;
    do
    {
        startFrameIdx = _urionVideo.indexOf(prbyte, startFrameIdx);
        qDebug() << "---- Size: " << (startFrameIdx - prevPos + 1) << endl;
        qDebug() << "Pos: " << startFrameIdx << endl;
        prevPos = startFrameIdx;
        if (startFrameIdx >= 0)
            startFrameIdx += prbyte.size();
    } while (startFrameIdx >= 0);
}

void UAVSimDataSender::loadXPlaneVideo(const QString &fileName)
{
    _frameNumber = 0;

    QImage frame;
    frame.load(fileName);

    int compressedSizeEstimation = LZ4_compressBound(XPLANE_IMAGE_DATA_SIZE);
    _compressedXPlaneData = new quint8[compressedSizeEstimation];

    _compressedXPlaneDataSize = LZ4_compress_default((char*)frame.bits(),
                                                     (char*)_compressedXPlaneData,
                                                     XPLANE_IMAGE_DATA_SIZE,
                                                     compressedSizeEstimation);
}

void UAVSimDataSender::sendTelemetryMassage()
{
    UDPSimulatorTelemetryMessageV4 telemetryMessage = _telemetryMessages.at(_currentMessageNumber);

    telemetryMessage.UavRoll =              _isFreezedUavRoll ?        _freezedUavRoll :        telemetryMessage.UavRoll;
    telemetryMessage.UavPitch =             _isFreezedUavPitch ?       _freezedUavPitch :       telemetryMessage.UavPitch;
    telemetryMessage.UavYaw =               _isFreezedUavYaw ?         _freezedUavYaw :         telemetryMessage.UavYaw;
    telemetryMessage.UavLatitude_GPS =      _isFreezedGPSLat ?         _freezedGPSLat :         telemetryMessage.UavLatitude_GPS;
    telemetryMessage.UavLongitude_GPS =     _isFreezedGPSLon ?         _freezedGPSLon :         telemetryMessage.UavLongitude_GPS;
    telemetryMessage.UavAltitude_GPS =      _isFreezedGPSHmsl ?        _freezedGPSHmsl :        telemetryMessage.UavAltitude_GPS;
    telemetryMessage.Course_GPS =           _isFreezedGPSCourse ?      _freezedGPSCourse :      telemetryMessage.Course_GPS;
    telemetryMessage.WindDirection =        _isFreezedWindDirection ?  _freezedWindDirection :  telemetryMessage.WindDirection;
    telemetryMessage.WindSpeed =            _isFreezedWindSpeed ?      _freezedWindSpeed :      telemetryMessage.WindSpeed;

    telemetryMessage.CamPitch =             _camY;
    telemetryMessage.CamYaw =               _camX;
    telemetryMessage.CamRoll =              _camZ;
    telemetryMessage.CamZoom =              _camZoom;

    telemetryMessage.CamEncoderPitch = telemetryMessage.CamPitch / 360.0 * 16384.0;
    telemetryMessage.CamEncoderYaw = telemetryMessage.CamYaw / 360.0 * 16384.0;
    telemetryMessage.CamEncoderRoll = telemetryMessage.CamRoll / 360.0 * 16384.0;

    telemetryMessage.BombState =            _bombState ? 000 : 111;

    telemetryMessage.RangefinderDistance =  _isRangefinderDistance ? _freezedRangefinderDistance : telemetryMessage.RangefinderDistance;

    // https://forum.qt.io/topic/37774/endianess-of-qbytearray/11
    // https://stackoverflow.com/questions/2782725/converting-float-values-from-big-endian-to-little-endian
    // https://betterexplained.com/articles/understanding-big-and-little-endian-byte-order/

    _udpTelemetrySocket.writeDatagram((char*)&telemetryMessage, sizeof(telemetryMessage),
                                      QHostAddress::LocalHost, _telemetryUDPPort);

    if (!_pause)
    {
        _currentMessageNumber++;
        if (_currentMessageNumber == _telemetryMessages.count())
            _currentMessageNumber = 0;
    }

    QByteArray msg = QByteArray::fromHex("0f0b00000000000000001a");
    _udpTelemetrySocket.writeDatagram(msg, QHostAddress::LocalHost, 50011);
    msg = QByteArray::fromHex("050f0000000041980000421000003f");
    _udpTelemetrySocket.writeDatagram(msg, QHostAddress::LocalHost, 50011);
    msg = QByteArray::fromHex("030fbf28c0003eca8000c09b640000");
    _udpTelemetrySocket.writeDatagram(msg, QHostAddress::LocalHost, 50011);
    msg = QByteArray::fromHex("100f06000606000000000000000031");
    _udpTelemetrySocket.writeDatagram(msg, QHostAddress::LocalHost, 50011);
}

void UAVSimDataSender::sendYurionMessage()
{
    if (_urionVideo.size() == 0)
        return;

    const int VIDEO_PACKET_SIZE = 1024;

    for (int i = 0; i < 50; i++)
    {
        int endVideoPos = _urionVideoPosition + VIDEO_PACKET_SIZE;
        if (endVideoPos >= _urionVideo.size())
            endVideoPos = _urionVideo.size();

        QByteArray videoPacket = _urionVideo.mid(_urionVideoPosition, endVideoPos - _urionVideoPosition);
        _udpVideoSocket.writeDatagram(videoPacket, QHostAddress::LocalHost, _videoUDPPort);

        _urionVideoPosition = endVideoPos;
        if (_urionVideoPosition == _urionVideo.size())
            _urionVideoPosition = 0;
    }
}

void UAVSimDataSender::sendXPLaneVideoMessage()
{
    _frameNumber++;

    XPLANE_PACKET videoPacket;
    int beginPos = 0, endPos = 0;
    videoPacket.framePartNo = 0;
    videoPacket.frameTotalSize = _compressedXPlaneDataSize;
    videoPacket.frameId = _frameNumber;

    int framePartCount = qCeil((float)videoPacket.frameTotalSize / XPLANE_PACKET_CHUNKSIZE);
    //int framePartCount = (videoPacket.frameTotalSize / XPLANE_PACKET_CHUNKSIZE);

    for (beginPos = 0; endPos < _compressedXPlaneDataSize - 1; beginPos += XPLANE_PACKET_CHUNKSIZE)
    {
        endPos = beginPos + XPLANE_PACKET_CHUNKSIZE - 1;
        if (endPos >= _compressedXPlaneDataSize)
            endPos = _compressedXPlaneDataSize - 1;
        int frameSize = endPos - beginPos + 1;
        memcpy(videoPacket.frameData, _compressedXPlaneData + beginPos, frameSize);

        _udpVideoSocket.writeDatagram((char*)&videoPacket, sizeof(videoPacket), QHostAddress::LocalHost, _videoUDPPort);
        videoPacket.framePartNo++;
    }
}

void UAVSimDataSender::timerEvent(QTimerEvent *event)
{
    if (_telemetryMessages.isEmpty())
        return;

    if (event->timerId() == _telemetryTimerId)
    {
        updateCamPitchRoll();
        sendTelemetryMassage();
    }
    else if (event->timerId() == _videoTimerId)
    {
        if (_trafficSource == Sim_XPlane)
            sendXPLaneVideoMessage();
        else
            sendYurionMessage();
    }
}

UAVSimDataSender::UAVSimDataSender(QObject *parent,
                                   const QString &telemetryFile, int telemetryUDPPort,
                                   const QString &videoFile, SimulatorVideoFrameTrafficSource trafficSource, int videoUDPPort, CameraControlModes controlMode) : QObject(parent)
{
    _telemetryFile = telemetryFile;
    _telemetryUDPPort = telemetryUDPPort;
    _videoFile = videoFile;
    _trafficSource = trafficSource;
    _videoUDPPort = videoUDPPort;

    _controlMode = controlMode;

    _camX = 0;
    _camY = 0;
    _camZ = 0;
    _camZoom = 0;
    _camYTarget = 0;
    _camXTarget = 0;
    _camZoomTarget = 0;

    _bombState = true;

    _isFreezedUavRoll = false;
    _isFreezedUavPitch = false;
    _isFreezedUavYaw = false;
    _isFreezedGPSLat = false;
    _isFreezedGPSLon = false;
    _isFreezedGPSHmsl = false;
    _isFreezedGPSCourse = false;
    _isFreezedWindDirection = false;
    _isFreezedWindSpeed = false;
    _isRangefinderDistance = false;

    _pause = false;

    _udpTelemetrySocket.setSocketOption(QAbstractSocket::SendBufferSizeSocketOption, 2000000);

    loadFromCSV(telemetryFile);

    if (_trafficSource == Sim_XPlane)
        loadXPlaneVideo(videoFile);
    else
        loadYurionVideo(videoFile);

    _telemetryTimerId = this->startTimer(100);
    _videoTimerId = this->startTimer(40);
}

void UAVSimDataSender::setPause(bool pause)
{
    _pause = pause;
}

double UAVSimDataSender::camXTarget()
{
    return _camX;
}

double UAVSimDataSender::camYTarget()
{
    return _camY;
}

void UAVSimDataSender::setCamXTarget(double value)
{
    _camXTarget = value;
}

void UAVSimDataSender::setCamYTarget(double value)
{
    _camYTarget = value;
}

void UAVSimDataSender::setCamZoomTarget(double value)
{
    _camZoomTarget = value;
}

void UAVSimDataSender::setBombState(bool state)
{
    _bombState = state;
}

void UAVSimDataSender::setFreezedUavRoll(bool freezed, double value)
{
    _isFreezedUavRoll = freezed;
    _freezedUavRoll = value;
}

void UAVSimDataSender::setFreezedUavPitch(bool freezed, double value)
{
    _isFreezedUavPitch = freezed;
    _freezedUavPitch = value;
}

void UAVSimDataSender::setFreezedUavYaw(bool freezed, double value)
{
    _isFreezedUavYaw = freezed;
    _freezedUavYaw = value;
}

void UAVSimDataSender::setFreezedGPSLat(bool freezed, double value)
{
    _isFreezedGPSLat = freezed;
    _freezedGPSLat = value;
}

void UAVSimDataSender::setFreezedGPSLon(bool freezed, double value)
{
    _isFreezedGPSLon = freezed;
    _freezedGPSLon = value;
}

void UAVSimDataSender::setFreezedGPSHmsl(bool freezed, double value)
{
    _isFreezedGPSHmsl = freezed;
    _freezedGPSHmsl = value;
}

void UAVSimDataSender::setFreezedGPSCourse(bool freezed, double value)
{
    _isFreezedGPSCourse = freezed;
    _freezedGPSCourse = value;
}

void UAVSimDataSender::setFreezedWindDirection(bool freezed, double value)
{
    _isFreezedWindDirection = freezed;
    _freezedWindDirection = value;
}

void UAVSimDataSender::setFreezedWindSpeed(bool freezed, double value)
{
    _isFreezedWindSpeed = freezed;
    _freezedWindSpeed = value;
}

void UAVSimDataSender::setFreezeRangefinderDistance(bool freezed, double value)
{
    _isRangefinderDistance = freezed;
    _freezedRangefinderDistance = value;
}
