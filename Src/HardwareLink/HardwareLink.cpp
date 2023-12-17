#include "HardwareLink.h"
#include <QtGlobal>
#include <QTimer>
#include <QtMath>
#include <QRegExp>
#include <QDebug>
#include "HardwareLink/MUSVPhotoCommandBuilder.h"
#include "HardwareLink/OtusCommonCommandBuilder.h"
#include "EnterProc.h"
#include "MUSV/protocol.h"

#define UNASSIGNED_TIMER -1

#pragma pack(push, 1)
struct UDPTelemetryMessageV4
{
    float UavRoll;
    float UavPitch;
    float UavYaw;
    float UavLatitude_GPS;
    float UavLongitude_GPS;
    float UavAltitude_GPS;
    float UavAltitude_Barometric;
    float Course_GPS;
    float CamRoll;
    float CamPitch;
    float CamYaw;
    uint32_t CamZoom;           // зум камеры

    int32_t CamEncoderRoll;
    int32_t CamEncoderPitch;
    int32_t CamEncoderYaw;
    float AirSpeed;             // воздушная скорость [m/s]
    float GroundSpeed_GPS;      // скорость относительно земли (по gps) [m/s]
    float WindDirection;        // направление ветра [deg]
    float WindSpeed;            // скорость ветра [m/s]
    float GroundSpeedNorth_GPS; // скорость на север [m/s]
    float GroundSpeedEast_GPS;  // скорость на восток [m/s]
    float VerticalSpeed;        // вертикальная скорость [m/s]
    uint8_t BombState;          // состояние гранаты, 000 - на самолете, 111 - произошел сброс
    float RangefinderDistance;  // Значение дистанции от дальномера
    uint32_t TargetCenterX;     // экранная координата центра сопровождаемой цели по оси X
    uint32_t TargetCenterY;     // экранная координата центра сопровождаемой цели по оси Y
    uint32_t TargetRectWidth;   // экранная ширина сопровождаемой цели
    uint32_t TargetRectHeight;  // экранная высота сопровождаемой цели
    uint8_t TargetState;        // статус сопровождаемой цели
};
#pragma pack(pop)

HardwareLink::HardwareLink(QObject *parent) : VideoLink(parent)
{
    EnterProc("HardwareLink::HardwareLink");

    _opened = false;
    _licenseState = getAnimusLicenseState();

    ApplicationSettings& applicationSettings = ApplicationSettings::Instance();

    _uavTelemetrySourceTypes = applicationSettings.UAVTelemetrySourceType;
    _telemetryDataFormat = applicationSettings.TelemetryDataFormat;
    _udpUAVTelemetryPort = applicationSettings.UAVTelemetryUDPPort;

    _useCamTelemetryUDP = applicationSettings.UseCamTelemetryUDP;
    _udpCamTelemetryPort = applicationSettings.CamTelemetryUDPPort;

    _useExtTelemetryUDP = applicationSettings.UseExtTelemetryUDP;
    _udpExtTelemetryPort = applicationSettings.ExtTelemetryUDPPort;

    _delayTelemetryDataFrames = new TelemetryDelayLine(this, applicationSettings.VideoLagFromTelemetry);
    connect(_delayTelemetryDataFrames, &TelemetryDelayLine::dequeue, this, &HardwareLink::onTelemetryDelayLineDequeue, Qt::DirectConnection);

    _delayCameraTelemetryDataFrames  = new CameraTelemetryDelayLine(this, applicationSettings.VideoLagFromCameraTelemetry);
    connect(_delayCameraTelemetryDataFrames, &CameraTelemetryDelayLine::dequeue, this, &HardwareLink::onCameraTelemetryDelayLineDequeue, Qt::DirectConnection);

    _commandTransports = applicationSettings.CommandTransport;
    _udpCommandAddress = QHostAddress(applicationSettings.CommandUDPAddress);
    _udpCommandPort = applicationSettings.CommandUDPPort;
    _commandSerialPortName = applicationSettings.CommandSerialPortName;
    _commandSendingInterval = applicationSettings.CommandSendingInterval;
    _catapultSerialPortName = applicationSettings.UseCatapultLauncher.value() ? applicationSettings.CatapultSerialPortName.value() : "";
    _catapultCommands = applicationSettings.CatapultCommand.value().split(',');
    _catapultCommandIdx = -1;

    _enableTelemetryForwarding = applicationSettings.EnableForwarding;
    _udpTelemetryForwardingAddress = QHostAddress(applicationSettings.TelemetryForwardingAddress);
    _udpTelemetryForwardingPort = applicationSettings.TelemetryForwardingPort;

    if (applicationSettings.ObjectTrackerType == ObjectTrackerTypeEnum::External)
    {
        _trackerHardwareLink = new TrackerHardwareLink(this);
        connect(_trackerHardwareLink, &TrackerHardwareLink::onTrackerCommandSent, this, &HardwareLink::doOnCommandSent, Qt::DirectConnection);
    }
    else
        _trackerHardwareLink = nullptr;

    _antennaHardwareLink = new AntennaHardwareLink(this);

    _camTracerMode = AutomaticTracerMode::atmSleep;

    _bombingPlacePos.setIncorrect();


    connect(&_udpUAVTelemetrySocket, &QUdpSocket::readyRead, this, &HardwareLink::processUAVTelemetryPendingDatagrams, Qt::ANIMUS_CONNECTION_TYPE);

    connect(&_udpCamTelemetrySocket, &QUdpSocket::readyRead, this, &HardwareLink::processCamTelemetryPendingDatagrams, Qt::ANIMUS_CONNECTION_TYPE);

    connect(&_udpExtTelemetrySocket, &QUdpSocket::readyRead, this, &HardwareLink::processExtTelemetryPendingDatagrams, Qt::ANIMUS_CONNECTION_TYPE);

    connect(&_serialCommandPort, &QSerialPort::readyRead, this, &HardwareLink::readSerialPortMUSVData);

    _fpsTimer = new QTimer(this);
    connect(_fpsTimer, &QTimer::timeout, [&]()
    {
        _receivedVideoFrameCountPrevSec = _videoFrameNumber - _videoFrameNumberPrevSec;
        _videoFrameNumberPrevSec = _videoFrameNumber;

        _receivedTelemetryFrameCountPrevSec = _telemetryFrameNumber - _telemetryFrameNumberPrevSec;
        _telemetryFrameNumberPrevSec = _telemetryFrameNumber;
    });
    _fpsTimer->start(1000);

    _camConnectionOn = false;
    _telemetryConnectionOn = false;
    _prevCamConnectionByteCounter = 0;
    _prevTelemetryConnectionByteCounter = 0;
    _camConnectionByteCounter = 0;
    _telemetryConnectionByteCounter = 0;

    _externalDataConsoleNotificator = new ExternalDataConsoleNotificator(this, applicationSettings.ExternalDataConsoleUDPPort);
    connect(this, &HardwareLink::onClientCommandSent, _externalDataConsoleNotificator, &ExternalDataConsoleNotificator::onClientCommandSent);
    connect(this, &HardwareLink::onTelemetryReceived, _externalDataConsoleNotificator, &ExternalDataConsoleNotificator::onTelemetryReceived);

    CommandProtocols commandProtocol = applicationSettings.CommandProtocol;
    switch(commandProtocol)
    {
    case CommandProtocols::MUSV:
    {
        _commandBuilder = new MUSVPhotoCommandBuilder(this);
        break;
    }
    case CommandProtocols::Otus:
    {
        _commandBuilder = new OtusCommandBuilder(this);
        break;
    }
    case CommandProtocols::MUSVDirect:
    {
        _commandBuilder = new MUSVPhotoDirectCommandBuilder(this);
        break;
    }
    case CommandProtocols::OtusDirect:
    {
        _commandBuilder = new OtusDirectCommandBuilder(this);
        break;
    }
    } // switch(commandProtocol)

    _snapshotSeriesTimer = UNASSIGNED_TIMER;

    _camAssemblyPreferences = applicationSettings.getCurrentCamAssemblyPreferences();

    _isRangefinderEnabled = false;

    _opticalSystemId = OPTYCAL_SYSTEM_1;
    auto cameraSettings = applicationSettings.installedCameraSettings();
    _isCameraFixed = (cameraSettings->CameraSuspensionType == CameraSuspensionTypes::FixedCamera);
    _fixedCamPitch = cameraSettings->FixedCamPitch;
    _fixedCamRoll = cameraSettings->FixedCamRoll;
    _fixedCamYaw = 0; //todo
    _fixedCamZoom = cameraSettings->FixedCamZoom;
    _expectedCamZoom = 1;
    //_lastUpdatedTelemetryFrameNumber = -1;

    _emulatorTelemetryDataFrame.UavRoll = applicationSettings.EmulatorConsoleUavRoll;
    _emulatorTelemetryDataFrame.UavPitch = applicationSettings.EmulatorConsoleUavPitch;
    _emulatorTelemetryDataFrame.UavYaw = applicationSettings.EmulatorConsoleUavYaw;
    _emulatorTelemetryDataFrame.UavLatitude_GPS = applicationSettings.EmulatorConsoleGpsLat;
    _emulatorTelemetryDataFrame.UavLongitude_GPS = applicationSettings.EmulatorConsoleGpsLon;
    _emulatorTelemetryDataFrame.UavAltitude_GPS = applicationSettings.EmulatorConsoleGpsHmsl;
    _emulatorTelemetryDataFrame.UavAltitude_Barometric = applicationSettings.EmulatorConsoleGpsHmsl;
    _emulatorTelemetryDataFrame.Course_GPS = applicationSettings.EmulatorConsoleGpsCourse;

    _connectionsStatusesTimer = this->startTimer(200); //ms, to update connections statuses
    _sendCamPositionTimer = this->startTimer(_commandSendingInterval / 2); //ms, to send cam position command
}

HardwareLink::~HardwareLink()
{
    closeVideoSource();
}

bool HardwareLink::camConnectionOn()
{
    return _camConnectionOn;
}

bool HardwareLink::telemetryConnectionOn()
{
    return _telemetryConnectionOn;
}

void HardwareLink::doActivateCatapult()
{
    if (!_opened || _catapultSerialPortName.isEmpty())
        return;

    if ((_catapultCommandIdx < 0) && !(_catapultSerialPort.isOpen() && _catapultSerialPort.isWritable()))
    {
        _catapultSerialPort.setPortName(_catapultSerialPortName);
        _catapultSerialPort.setBaudRate(9600);
        _catapultSerialPort.setDataBits(QSerialPort::DataBits::Data8);
        _catapultSerialPort.setParity(QSerialPort::Parity::NoParity);
        _catapultSerialPort.setStopBits(QSerialPort::StopBits::OneStop);
        _catapultSerialPort.setFlowControl(QSerialPort::FlowControl::NoFlowControl);
        _catapultSerialPort.open(QSerialPort::WriteOnly);
        _catapultCommandIdx = 0;
        QTimer::singleShot(1000, Qt::PreciseTimer, this, &HardwareLink::doActivateCatapult);
    }
    else if (_catapultCommandIdx >= 0 && _catapultCommandIdx < _catapultCommands.count())
    {
        QString currentCommandHEX = _catapultCommands[_catapultCommandIdx].trimmed();
        if (!currentCommandHEX.isEmpty())
        {
            QByteArray command = QByteArray::fromHex(currentCommandHEX.toLatin1());
            _catapultSerialPort.write(command);
        }
        _catapultCommandIdx++;
        QTimer::singleShot(1000, Qt::PreciseTimer, this, &HardwareLink::doActivateCatapult);
    }
    else if (_catapultCommandIdx >= _catapultCommands.count())
    {
        _catapultCommandIdx = -1;
        _catapultSerialPort.close();
    }
}

void HardwareLink::onTelemetryDelayLineDequeue(const TelemetryDataFrame &value)
{
    _currentTelemetryDataFrame = value;
    notifyDataReceived();
}

void HardwareLink::onCameraTelemetryDelayLineDequeue(const CameraTelemetryDataFrame &value)
{
    _currentCameraDataFrame = value;
    _currentCameraDataFrame.applyToTelemetryDataFrame(_currentTelemetryDataFrame);
}

void HardwareLink::timerEvent(QTimerEvent *event)
{
    EnterProc("HardwareLink::timerEvent");
    Q_UNUSED(event)

    if (event->timerId() == _connectionsStatusesTimer)
    {
        bool camConnectionChanged = ((_prevCamConnectionByteCounter == _camConnectionByteCounter) == _camConnectionOn);
        bool telemetryConnectionChanged = ((_prevTelemetryConnectionByteCounter == _telemetryConnectionByteCounter) == _telemetryConnectionOn);

        _camConnectionOn = (_prevCamConnectionByteCounter != _camConnectionByteCounter);
        _telemetryConnectionOn = (_prevTelemetryConnectionByteCounter != _telemetryConnectionByteCounter);

        _prevTelemetryConnectionByteCounter = _telemetryConnectionByteCounter;
        _prevCamConnectionByteCounter = _camConnectionByteCounter;

        if (camConnectionChanged || telemetryConnectionChanged)
            emit onHardwareLinkStateChanged();
    }
    else if (event->timerId() == _snapshotSeriesTimer)
        makeSnapshot();
    else if (event->timerId() == _sendCamPositionTimer)
        tryToSendCamPosition();
}

void HardwareLink::tryToSendCamPosition()
{
    quint32 currentTime = getSessionTimeMs();
    if ( (qAbs(currentTime - _camPositionCommandTime) < _commandSendingInterval) || (_camPositionCommand.size() == 0) )
        return;

    sendCommand(_camPositionCommand, _camPositionCommandDescription);
    _camPositionCommand.clear();
    _camPositionCommandDescription.clear();
    _camPositionCommandTime = currentTime;
}

void HardwareLink::doOnCommandSent(const BinaryContent &commandContent, const QString &commandDescription)
{
    if (commandContent.size() == 0)
        return;

    DataExchangePackage clientCommand;
    clientCommand.SessionTimeMs = getSessionTimeMs();
    clientCommand.VideoFrameNumber = _currentTelemetryDataFrame.VideoFrameNumber;
    clientCommand.TelemetryFrameNumber = _currentTelemetryDataFrame.TelemetryFrameNumber;
    clientCommand.Direction = DataExchangePackageDirection::Outgoing;
    clientCommand.ContentHEX = commandContent.toHex();
    clientCommand.Description = commandDescription;
    emit onClientCommandSent(clientCommand);
}

void HardwareLink::sendCommand(const BinaryContent &commandContent, const QString &commandDescription)
{
    EnterProc("HardwareLink::sendCommand");

    if (commandContent.size() == 0)
        return;

    doOnCommandSent(commandContent, commandDescription);

    switch (_commandTransports)
    {
    case CommandTransports::UDP:
    {
        _udpCommandSocket.writeDatagram(commandContent.toByteArray(), _udpCommandAddress, _udpCommandPort);
        break;
    }
    case CommandTransports::Serial:
    {
        // result is the number of written bytes or -1 if an error occured
        qint64 result = _serialCommandPort.write(commandContent.toByteArray());
        if (result < 0)
            qDebug() << "Serial Port Error. No data send.";
        break;
    }
    }
}

void HardwareLink::notifyDataReceived()
{
    _currentTelemetryDataFrame.VideoFrameNumber = _videoFrameNumber;
    _currentTelemetryDataFrame.SessionTimeMs = getSessionTimeMs();
    _currentTelemetryDataFrame.VideoFPS = _receivedVideoFrameCountPrevSec;
    _currentTelemetryDataFrame.TelemetryFPS = _receivedTelemetryFrameCountPrevSec;

    updateTrackerValues(_currentTelemetryDataFrame);
    updateAntennaValues(_currentTelemetryDataFrame);

    if (_opened && (_licenseState != AnimusLicenseState::Expired))
        emit dataReceived(_currentTelemetryDataFrame, _videoFrame);
}

void HardwareLink::forwardTelemetryDataFrame(const char *data, qint64 len)
{
    if (_enableTelemetryForwarding)
        _udpForwardingSocket.writeDatagram(data, len, _udpTelemetryForwardingAddress, _udpTelemetryForwardingPort);
}

void HardwareLink::updateTrackerValues(TelemetryDataFrame &telemetryDataFrame)
{
    if (_trackerHardwareLink != nullptr)
    {
        telemetryDataFrame.TrackedTargetCenterX = _trackerHardwareLink->trackingRectangleCenterX();
        telemetryDataFrame.TrackedTargetCenterY = _trackerHardwareLink->trackingRectangleCenterY();
        telemetryDataFrame.TrackedTargetRectWidth = _trackerHardwareLink->trackingRectangleWidth();
        telemetryDataFrame.TrackedTargetRectHeight = _trackerHardwareLink->trackingRectangleHeight();
        telemetryDataFrame.TrackedTargetState = _trackerHardwareLink->trackingState();
    }
}

void HardwareLink::updateAntennaValues(TelemetryDataFrame &telemetryDataFrame)
{
    telemetryDataFrame.AntennaElevation = _antennaHardwareLink->antennaElevation();
    telemetryDataFrame.AntennaAzimuth = _antennaHardwareLink->antennaAzimuth();

    telemetryDataFrame.AntennaFanEnabled = _antennaHardwareLink->fanEnabled();
    telemetryDataFrame.AntennaHeaterEnabled = _antennaHardwareLink->heaterEnabled();
}

void HardwareLink::updateTelemetryValues(TelemetryDataFrame &telemetryDataFrame)
{
    // ???
    //    if (_lastUpdatedTelemetryFrameNumber == _telemetryFrameNumber)
    //        return;
    //    _lastUpdatedTelemetryFrameNumber = _telemetryFrameNumber;

    //Update camera telemetry from other UDP stream
    if (_useCamTelemetryUDP || (_commandTransports == CommandTransports::Serial)) //???todo refactor
        _currentCameraDataFrame.applyToTelemetryDataFrame(telemetryDataFrame);

    if (_useExtTelemetryUDP)
        _extendedTelemetryDataFrame.applyToTelemetryDataFrame(telemetryDataFrame);

    //??? Update active optical system
    telemetryDataFrame.OpticalSystemId = _opticalSystemId;

    //Update Laser rangefinder
    if (_isRangefinderEnabled)
    {
        if (telemetryDataFrame.RangefinderDistance == 10)
            telemetryDataFrame.RangefinderDistance = 0;
    }
    else
    {
        telemetryDataFrame.RangefinderDistance = 0;
        telemetryDataFrame.RangefinderTemperature = 0;
    }

    //update ground speed
    if (qFuzzyCompare(telemetryDataFrame.GroundSpeed_GPS, 0))
        telemetryDataFrame.GroundSpeed_GPS = qSqrt(
                    qPow(telemetryDataFrame.GroundSpeedNorth_GPS, 2) + qPow(telemetryDataFrame.GroundSpeedEast_GPS, 2));

    // update Fixed Camera Values
    if (_isCameraFixed)
    {
        telemetryDataFrame.CamPitch = _fixedCamPitch;
        telemetryDataFrame.CamRoll  = _fixedCamRoll;
        telemetryDataFrame.CamYaw   = _fixedCamYaw;
        telemetryDataFrame.CamZoom  = _fixedCamZoom;
    }

    // update FOV Angles
    if (telemetryDataFrame.CamZoom <= 0)
        telemetryDataFrame.CamZoom = _expectedCamZoom;

    auto camPreferences = _camAssemblyPreferences->opticalDevice(telemetryDataFrame.OpticalSystemId);
    telemetryDataFrame.FOVHorizontalAngle = camPreferences->fovHorizontalAngle(telemetryDataFrame.CamZoom);
    telemetryDataFrame.FOVVerticalAngle = camPreferences->fovVerticalAngle(telemetryDataFrame.CamZoom);

    telemetryDataFrame.CamTracerMode = (quint32)_camTracerMode;

    //update bombing place
    telemetryDataFrame.BombingPlacePosLat = _bombingPlacePos.lat;
    telemetryDataFrame.BombingPlacePosLon = _bombingPlacePos.lon;
    telemetryDataFrame.BombingPlacePosHmsl = _bombingPlacePos.hmsl;
}

const TelemetryDataFrame HardwareLink::getTelemetryDataFrame()
{
    return _currentTelemetryDataFrame;
}

void HardwareLink::open()
{
    EnterProc("HardwareLink::open");
    _sessionTime.start();
    _videoFrameNumber = 0;
    _telemetryFrameNumber = 0;
    _delayTelemetryDataFrames->clear();
    _currentTelemetryDataFrame.clear();
    _delayCameraTelemetryDataFrames->clear();
    _currentCameraDataFrame.clear();

    _videoFrameNumberPrevSec = 0;
    _receivedVideoFrameCountPrevSec = 0;
    _telemetryFrameNumberPrevSec = 0;
    _receivedTelemetryFrameCountPrevSec = 0;

    _emulatorTelemetryDataFrame.applyToTelemetryDataFrame(_currentTelemetryDataFrame);
    updateTelemetryValues(_currentTelemetryDataFrame);
    _camPositionCommandTime = 0;

    switch (_commandTransports)
    {
    case CommandTransports::UDP:
    {
        _udpCommandSocket.setSocketOption(QAbstractSocket::SendBufferSizeSocketOption, 1000000);

        if (_useCamTelemetryUDP)
        {
            _udpCamTelemetrySocket.bind(_udpCamTelemetryPort, QUdpSocket::DefaultForPlatform);
            _udpCamTelemetrySocket.setSocketOption(QAbstractSocket::ReceiveBufferSizeSocketOption, 10000); // -1 == Unlimited ???
        }

        if (_useExtTelemetryUDP)
        {
            _udpExtTelemetrySocket.bind(_udpExtTelemetryPort, QUdpSocket::DefaultForPlatform);
            _udpExtTelemetrySocket.setSocketOption(QAbstractSocket::ReceiveBufferSizeSocketOption, 10000); // -1 == Unlimited ???
        }

        if (_uavTelemetrySourceTypes == UAVTelemetrySourceTypes::UDPChannel)
        {
            _udpUAVTelemetrySocket.bind(_udpUAVTelemetryPort, QUdpSocket::DefaultForPlatform);
            _udpUAVTelemetrySocket.setSocketOption(QAbstractSocket::ReceiveBufferSizeSocketOption, 10000); // -1 == Unlimited ???
        }

        break;
    }
    case CommandTransports::Serial:
    {
        if (!_serialCommandPort.isOpen())
        {
            _serialCommandPort.setPortName(_commandSerialPortName);
            _serialCommandPort.setBaudRate(QSerialPort::Baud115200);
            _serialCommandPort.open(QSerialPort::ReadWrite);
        }
        break;
    }
    }

    if (_trackerHardwareLink != nullptr)
        _trackerHardwareLink->open();

    _opened = true;
}

void HardwareLink::close()
{
    _opened = false;
    if (_serialCommandPort.isOpen())
        _serialCommandPort.close();
    if (_catapultSerialPort.isOpen())
        _catapultSerialPort.close();

    _udpCommandSocket.close();
    _udpUAVTelemetrySocket.close();
    //???_udpCamTelemetrySocket.close();

    if (_trackerHardwareLink != nullptr)
        _trackerHardwareLink->close();
}

quint32 HardwareLink::getSessionTimeMs()
{
    quint32 elapsedTime = (quint32)_sessionTime.elapsed();
    return elapsedTime;
}

AntennaHardwareLink *HardwareLink::antenna()
{
    return _antennaHardwareLink;
}

void HardwareLink::setHardwareCamStabilization(bool enabled)
{
    EnterProc("HardwareLink::setHardwareCamStabilization");
    auto description = QString("SetHardwareCamStabilization: %1").arg(enabled);
    sendCommand(_commandBuilder->SetupHardwareCamStabilizationCommand(enabled), description);
}

void HardwareLink::setCamPosition(float roll, float pitch, float yaw)
{
    _camPositionCommandDescription = QString("setCamPosition: Y=%1, P=%2, R=%3").arg(yaw, 0, 'f', 2).arg(pitch, 0, 'f', 2).arg(roll, 0, 'f', 2);
    _camPositionCommand = _commandBuilder->SetupCamPositionCommand(roll, pitch, yaw);
    tryToSendCamPosition();
}

void HardwareLink::setCamSpeed(float roll, float pitch, float yaw)
{
    _camPositionCommandDescription = QString("setCamSpeed: Y=%1, P=%2, R=%3").arg(yaw, 0, 'f', 2).arg(pitch, 0, 'f', 2).arg(roll, 0, 'f', 2);
    _camPositionCommand = _commandBuilder->SetupCamSpeedCommand(roll, pitch, yaw);
    tryToSendCamPosition();
}

void HardwareLink::setCamZoom(float zoom)
{
    auto description = QString("setCamZoom: %1").arg(zoom);
    sendCommand(_commandBuilder->SetupCamZoomCommand(zoom), description);
    _expectedCamZoom = zoom;
}

void HardwareLink::setCamMotorStatus(bool enabled)
{
    EnterProc("HardwareLink::setCamMotorStatus");
    auto description = QString("setCamMotorStatus: %1").arg(enabled);
    sendCommand(_commandBuilder->SetupCamMotorStatusCommand(enabled), description);
}

void HardwareLink::selectActiveCam(int camId)
{
    EnterProc("HardwareLink::selectActiveCam");
    auto description = QString("selectActiveCam: %1").arg(camId);
    sendCommand(_commandBuilder->SelectActiveCamCommand(camId), description);

    VideoLink::selectActiveCam(camId);
    //_opticalSystemId = camId;
}

void HardwareLink::parkCamera()
{
    EnterProc("HardwareLink::parkCamera");
    auto description = QString("parkCamera");
    sendCommand(_commandBuilder->ParkingCommand(), description);
}

void HardwareLink::setCamColorMode(int colorMode)
{
    auto description = QString("setCamColorMode: %1").arg(colorMode);
    sendCommand(_commandBuilder->SelectCamColorModeCommand(colorMode), description);
}

void HardwareLink::setLaserActivation(bool active)
{
    auto description = QString("setLaserActivation: %1").arg(active);
    sendCommand(_commandBuilder->SetLaserActivationCommand(active), description);
    _isRangefinderEnabled = active;
}

void HardwareLink::setBombingPlacePos(double lat, double lon, double hmsl)
{    
    //todo real sending and receiving
    _bombingPlacePos.lat = lat;
    _bombingPlacePos.lon = lon;
    _bombingPlacePos.hmsl = hmsl;
}

void HardwareLink::dropBomb(int index)
{
    EnterProc("HardwareLink::dropBomb");
    auto description = QString("dropBomb: %1").arg(index);
    sendCommand(_commandBuilder->DropBombCommand(index), description);
}

void HardwareLink::makeSnapshot()
{
    EnterProc("HardwareLink::makeSnapshot");
    auto description = QString("makeSnapshot");
    sendCommand(_commandBuilder->MakeSnapshotCommand(), description);
}

void HardwareLink::activateCatapult()
{
    EnterProc("HardwareLink::activateCatapult");
    if (_catapultCommandIdx <= 0)
        doActivateCatapult();
}

void HardwareLink::onTracerModeChanged(const AutomaticTracerMode tracerMode)
{
    _camTracerMode = tracerMode;
}

void HardwareLink::onEmulatorTelemetryDataFrame(const EmulatorTelemetryDataFrame &emulatorTelemetryDataFrame)
{
    _emulatorTelemetryDataFrame = emulatorTelemetryDataFrame;
    _emulatorTelemetryDataFrame.applyToTelemetryDataFrame(_currentTelemetryDataFrame);
}

void HardwareLink::startSnapshotSeries(int intervalMsec)
{
    EnterProc("HardwareLink::startSnapshotSeries");
    //sendCommand(_commandBuilder->MakeSnapshotSeriesCommand(intervalSec));

    stopSnapshotSeries();
    _snapshotSeriesTimer = this->startTimer(intervalMsec);
    makeSnapshot();
}

bool HardwareLink::isSnapshotSeries()
{
    return _snapshotSeriesTimer != UNASSIGNED_TIMER;
}

void HardwareLink::startCamRecording()
{
    EnterProc("HardwareLink::startCamRecording");
    auto description = QString("startCamRecording");
    sendCommand(_commandBuilder->StartCamRecordingCommand(), description);
}

void HardwareLink::stopCamRecording()
{
    EnterProc("HardwareLink::stopCamRecording");
    auto description = QString("startCamRecording");
    sendCommand(_commandBuilder->StopCamRecordingCommand(), description);
}

void HardwareLink::lockTarget(const QPoint &targetCenter)
{
    EnterProc("HardwareLink::lockTarget");
    if (_trackerHardwareLink != nullptr)
        _trackerHardwareLink->lockTarget(targetCenter);
}

void HardwareLink::unlockTarget()
{
    EnterProc("HardwareLink::unlockTarget");
    if (_trackerHardwareLink != nullptr)
        _trackerHardwareLink->unlockTarget();
}

void HardwareLink::setTargetSize(int targetSize)
{
    EnterProc("HardwareLink::setTargetSize");
    if (_trackerHardwareLink != nullptr)
        _trackerHardwareLink->setTargetSize(targetSize);
}

void HardwareLink::stopSnapshotSeries()
{
    EnterProc("HardwareLink::stopSnapshotSeries");
    //sendCommand(_commandBuilder->StopSnapshotSeriesCommand());

    if (_snapshotSeriesTimer != UNASSIGNED_TIMER)
    {
        killTimer(_snapshotSeriesTimer);
        _snapshotSeriesTimer = UNASSIGNED_TIMER;
    }
}

void HardwareLink::processUAVTelemetryPendingDatagrams()
{
    bool isProcessed = false;

    if ((_telemetryDataFormat == UAVTelemetryDataFormats::UAVTelemetryFormatV4) ||
            (_telemetryDataFormat == UAVTelemetryDataFormats::UAVTelemetryFormatV4_1))
    {
        isProcessed = processTelemetryPendingDatagramsV4();
        if (!isProcessed)
        {
            qInfo() << "Incorrect Telemetry Pending Datagram V4 Size.";
            _telemetryDataFormat = UAVTelemetryDataFormats::UAVTelemetryFormatUnknown;
        }
    }
    else
        _telemetryDataFormat = UAVTelemetryDataFormats::UAVTelemetryFormatUnknown;

    if (_telemetryDataFormat == UAVTelemetryDataFormats::UAVTelemetryFormatUnknown)
        processTelemetryPendingDatagramsUnknownFormat();
}

void HardwareLink::processCamTelemetryPendingDatagrams()
{
    while (_udpCamTelemetrySocket.hasPendingDatagrams())
    {
        qint64 messageSize = _udpCamTelemetrySocket.pendingDatagramSize();
        QByteArray rawData(messageSize, 0);
        _udpCamTelemetrySocket.readDatagram(rawData.data(), messageSize);
        processNewCameraTelemetryDataFrame(rawData);

        _telemetryConnectionByteCounter += rawData.size();
        emit onTelemetryReceived(rawData.toHex());
    }

    if (_uavTelemetrySourceTypes == UAVTelemetrySourceTypes::Emulator)
    {
        TelemetryDataFrame telemetryDataFrame;
        telemetryDataFrame.clear();
        telemetryDataFrame.TelemetryFrameNumber = ++_telemetryFrameNumber;

        //magic numbers for camera testing
        _emulatorTelemetryDataFrame.applyToTelemetryDataFrame(telemetryDataFrame);
        updateTelemetryValues(telemetryDataFrame);
        _delayTelemetryDataFrames->enqueue(telemetryDataFrame);
    }
}

void HardwareLink::processExtTelemetryPendingDatagrams()
{
    QByteArray datagram;
    qint64 messageSize = 0;

    while (_udpExtTelemetrySocket.hasPendingDatagrams())
    {
        messageSize = _udpExtTelemetrySocket.pendingDatagramSize();
        datagram.resize(messageSize);
        _udpExtTelemetrySocket.readDatagram(datagram.data(), messageSize);

        QString datagramAsString = QString(datagram);
        QRegExp rx("Temperature = (.+) C; Pressure = (.+) hPa");
        rx.setMinimal(true);

        int i = rx.indexIn(datagramAsString);

        while(i != -1)
        {
            bool ok = false;

            QString strValue = rx.cap(1);
            double value = strValue.toDouble(&ok);
            if (ok)
                _extendedTelemetryDataFrame.AtmosphereTemperature = value;

            strValue = rx.cap(2);
            value = strValue.toDouble(&ok);
            if (ok)
                _extendedTelemetryDataFrame.AtmospherePressure = value * 0.75006157584566; //convert hPa 2 mmHg

            i = rx.indexIn(datagramAsString, i) + rx.cap(0).length();
        }
    }
}

void HardwareLink::processNewCameraTelemetryDataFrame(const QByteArray &rawData)
{
    static protocolName::protocol protocolMUSV;
    protocolMUSV.SetData(rawData);
    protocolMUSV.DecodingData();
    protocolName::InputData *info = protocolMUSV.GetData();

    CameraTelemetryDataFrame cameraDataFrame;

    cameraDataFrame.CamRoll = static_cast<double>(info->angles.roll);
    cameraDataFrame.CamPitch = static_cast<double>(info->angles.pitch);
    cameraDataFrame.CamYaw = static_cast<double>(info->angles.yaw);
    cameraDataFrame.CamZoom = static_cast<double>(info->ZoomAll);

    cameraDataFrame.CamEncoderRoll = info->Encoders[0];
    cameraDataFrame.CamEncoderPitch = info->Encoders[1];
    cameraDataFrame.CamEncoderYaw = info->Encoders[2];

    if (_isRangefinderEnabled)
    {
        cameraDataFrame.RangefinderDistance = info->LdDistance;
        cameraDataFrame.RangefinderTemperature = info->LdTemperature;
    }
    else
    {
        cameraDataFrame.RangefinderDistance = 0;
        cameraDataFrame.RangefinderTemperature = 0;
    }

    _delayCameraTelemetryDataFrames->enqueue(cameraDataFrame);
}

void HardwareLink::readSerialPortMUSVData()
{
    auto rawData = _serialCommandPort.readAll();
    processNewCameraTelemetryDataFrame(rawData);

    TelemetryDataFrame telemetryDataFrame;
    telemetryDataFrame.clear();
    telemetryDataFrame.TelemetryFrameNumber = ++_telemetryFrameNumber;

    //magic numbers for camera testing
    _emulatorTelemetryDataFrame.applyToTelemetryDataFrame(telemetryDataFrame);
    updateTelemetryValues(telemetryDataFrame);

    _telemetryConnectionByteCounter += rawData.size();

    _delayTelemetryDataFrames->enqueue(telemetryDataFrame);
    emit onTelemetryReceived(rawData.toHex());
}

bool HardwareLink::processTelemetryPendingDatagramsV4()
{
    UDPTelemetryMessageV4 udpTelemetryMessage;

    qint64 messageSize = sizeof(UDPTelemetryMessageV4);
    while (_udpUAVTelemetrySocket.hasPendingDatagrams())
    {
        if (messageSize != _udpUAVTelemetrySocket.pendingDatagramSize())
            return false;

        _udpUAVTelemetrySocket.readDatagram((char*)&udpTelemetryMessage, messageSize);

        TelemetryDataFrame telemetryDataFrame;

        telemetryDataFrame.TelemetryFrameNumber = ++_telemetryFrameNumber;

        telemetryDataFrame.UavRoll =                 udpTelemetryMessage.UavRoll;
        telemetryDataFrame.UavPitch =                udpTelemetryMessage.UavPitch;
        telemetryDataFrame.UavYaw =                  udpTelemetryMessage.UavYaw;

        telemetryDataFrame.UavLatitude_GPS =         udpTelemetryMessage.UavLatitude_GPS;
        telemetryDataFrame.UavLongitude_GPS =        udpTelemetryMessage.UavLongitude_GPS;
        telemetryDataFrame.UavAltitude_GPS =         udpTelemetryMessage.UavAltitude_GPS;
        telemetryDataFrame.UavAltitude_Barometric =  udpTelemetryMessage.UavAltitude_Barometric;
        telemetryDataFrame.Course_GPS =              udpTelemetryMessage.Course_GPS;

        //Fix Error in transfer order
        if (_telemetryDataFormat == UAVTelemetryDataFormats::UAVTelemetryFormatV4_1)
        {
            telemetryDataFrame.UavYaw =              udpTelemetryMessage.Course_GPS;
            telemetryDataFrame.Course_GPS =          udpTelemetryMessage.UavYaw;
        }

        telemetryDataFrame.CamPitch =                udpTelemetryMessage.CamPitch;
        telemetryDataFrame.CamRoll =                 udpTelemetryMessage.CamRoll;
        telemetryDataFrame.CamYaw =                  udpTelemetryMessage.CamYaw;
        telemetryDataFrame.CamZoom =                 udpTelemetryMessage.CamZoom;

        telemetryDataFrame.CamEncoderRoll =          udpTelemetryMessage.CamEncoderRoll;
        telemetryDataFrame.CamEncoderPitch =         udpTelemetryMessage.CamEncoderPitch;
        telemetryDataFrame.CamEncoderYaw =           udpTelemetryMessage.CamEncoderYaw;

        telemetryDataFrame.AirSpeed =                 udpTelemetryMessage.AirSpeed;
        telemetryDataFrame.GroundSpeed_GPS =          udpTelemetryMessage.GroundSpeed_GPS;
        telemetryDataFrame.WindDirection =            udpTelemetryMessage.WindDirection;
        telemetryDataFrame.WindSpeed =                udpTelemetryMessage.WindSpeed;
        telemetryDataFrame.GroundSpeedNorth_GPS =     udpTelemetryMessage.GroundSpeedNorth_GPS;
        telemetryDataFrame.GroundSpeedEast_GPS =      udpTelemetryMessage.GroundSpeedEast_GPS;
        telemetryDataFrame.VerticalSpeed =            udpTelemetryMessage.VerticalSpeed;
        telemetryDataFrame.BombState =                udpTelemetryMessage.BombState;

        if (_isRangefinderEnabled) //???
            telemetryDataFrame.RangefinderDistance =      udpTelemetryMessage.RangefinderDistance;
        else
            telemetryDataFrame.RangefinderDistance =      0;

        telemetryDataFrame.TrackedTargetCenterX =     udpTelemetryMessage.TargetCenterX;
        telemetryDataFrame.TrackedTargetCenterY =     udpTelemetryMessage.TargetCenterY;
        telemetryDataFrame.TrackedTargetRectWidth =   udpTelemetryMessage.TargetRectWidth;
        telemetryDataFrame.TrackedTargetRectHeight =  udpTelemetryMessage.TargetRectHeight;
        telemetryDataFrame.TrackedTargetState =       udpTelemetryMessage.TargetState;

        updateTelemetryValues(telemetryDataFrame);

        _telemetryConnectionByteCounter += sizeof(udpTelemetryMessage);

        _delayTelemetryDataFrames->enqueue(telemetryDataFrame);

        QByteArray rawData = QByteArray((char*)&udpTelemetryMessage, messageSize);
        emit onTelemetryReceived(rawData.toHex());

        forwardTelemetryDataFrame((char*)&udpTelemetryMessage, messageSize);
    }
    return true;
}

void HardwareLink::processTelemetryPendingDatagramsUnknownFormat()
{
    QByteArray datagram;
    qint64 messageSize = 0;

    //Ignore all datagrams
    while (_udpUAVTelemetrySocket.hasPendingDatagrams())
    {
        messageSize = _udpUAVTelemetrySocket.pendingDatagramSize();
        datagram.resize(messageSize);
        _udpUAVTelemetrySocket.readDatagram(datagram.data(), messageSize);
    }

    if (messageSize == sizeof(UDPTelemetryMessageV4) &&
            (_telemetryDataFormat != UAVTelemetryDataFormats::UAVTelemetryFormatV4_1) )
        _telemetryDataFormat = UAVTelemetryDataFormats::UAVTelemetryFormatV4;
    //else the same UnknownFormat
}

void HardwareLink::videoFrameReceivedInternal(const QImage &frame)
{
    auto videoConnectionId = _camAssemblyPreferences->opticalDevice(_opticalSystemId)->videoConnectionId();
    auto s = _videoSources.value(videoConnectionId, nullptr);
    if (s == nullptr)
        return;

    QObject *activeVideoSource = qobject_cast<QObject*>(s);
    QObject *senderVideoSource = qobject_cast<QObject*>(sender());

    if (senderVideoSource != activeVideoSource)
        return;

    _camConnectionByteCounter += frame.byteCount();
    _videoFrameNumber++;
    _videoFrame = frame;

    //???    updateCameraTelemetry();

    notifyDataReceived();
}
