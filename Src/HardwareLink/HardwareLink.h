#ifndef HARDWARELINK_H
#define HARDWARELINK_H

#include <QObject>
#include <QUdpSocket>
#include <QSerialPort>
#include <QPoint>
#include <QTime>
#include <QQueue>
#include <QByteArray>
#include <QTimer>
#include <QTimerEvent>
#include "TelemetryDataFrame.h"
#include "HardwareLink/ExternalDataConsoleNotificator.h"
#include "HardwareLink/VideoLink.h"
#include "HardwareLink/CommonCommandBuilder.h"
#include "HardwareLink/TrackerHardwareLink.h"
#include "HardwareLink/AntennaHardwareLink.h"
#include "ApplicationSettings.h"
#include "Common/CommonUtils.h"
#include "Common/BinaryContent.h"
#include "DelayLine.h"

const int OPTYCAL_SYSTEM_1   = 1;
const int OPTYCAL_SYSTEM_2   = 2;
const int OPTYCAL_SYSTEM_3   = 3;

class HardwareLink final: public VideoLink
{
    Q_OBJECT
private:
    bool _opened;
    AnimusLicenseState _licenseState;

    QUdpSocket _udpUAVTelemetrySocket;
    QUdpSocket _udpCamTelemetrySocket;
    QUdpSocket _udpExtTelemetrySocket;
    QUdpSocket _udpCommandSocket;
    QSerialPort _serialCommandPort;

    QUdpSocket _udpForwardingSocket;
    bool _enableTelemetryForwarding;
    QHostAddress _udpTelemetryForwardingAddress;
    quint32 _udpTelemetryForwardingPort;

    CommonCommandBuilder *_commandBuilder;
    ExternalDataConsoleNotificator *_externalDataConsoleNotificator;

    quint32 _telemetryDataFormat; // TelemetryDataFormat
    quint32 _commandTransports; // CommandTransports

    UAVTelemetrySourceTypes _uavTelemetrySourceTypes;
    quint32 _udpUAVTelemetryPort;
    bool _useCamTelemetryUDP;
    quint32 _udpCamTelemetryPort;
    bool _useExtTelemetryUDP;
    quint32 _udpExtTelemetryPort;

    QHostAddress _udpCommandAddress;
    quint32 _udpCommandPort;
    QString _commandSerialPortName;
    quint32 _commandSendingInterval;

    TrackerHardwareLink *_trackerHardwareLink;

    AntennaHardwareLink *_antennaHardwareLink;

    QString _catapultSerialPortName;
    QStringList _catapultCommands;
    int _catapultCommandIdx;
    QSerialPort _catapultSerialPort;

    QTime _sessionTime;
    //quint32 _lastUpdatedTelemetryFrameNumber;

    QImage _videoFrame;

    EmulatorTelemetryDataFrame _emulatorTelemetryDataFrame;
    ExtendedTelemetryDataFrame _extendedTelemetryDataFrame;
    bool _isRangefinderEnabled;

    quint32 _telemetryFrameNumber;
    quint32 _videoFrameNumber;

    QTimer *_fpsTimer;
    quint32 _videoFrameNumberPrevSec, _receivedVideoFrameCountPrevSec;
    quint32 _telemetryFrameNumberPrevSec, _receivedTelemetryFrameCountPrevSec;

    bool _isCameraFixed;
    double _fixedCamPitch, _fixedCamRoll, _fixedCamYaw;
    double _fixedCamZoom;
    double _expectedCamZoom;


    bool _camConnectionOn, _telemetryConnectionOn;
    unsigned long long _prevCamConnectionByteCounter, _prevTelemetryConnectionByteCounter;
    unsigned long long _camConnectionByteCounter, _telemetryConnectionByteCounter;

    BinaryContent _camPositionCommand;
    quint32 _camPositionCommandTime;
    QString _camPositionCommandDescription;

    int _connectionsStatusesTimer;
    int _snapshotSeriesTimer;
    int _sendCamPositionTimer;

    AutomaticTracerMode _camTracerMode;

    WorldGPSCoord _bombingPlacePos;

    CameraTelemetryDelayLine *_delayCameraTelemetryDataFrames;
    CameraTelemetryDataFrame _currentCameraDataFrame;

    TelemetryDelayLine *_delayTelemetryDataFrames;
    TelemetryDataFrame _currentTelemetryDataFrame;

    void notifyDataReceived();

    void timerEvent(QTimerEvent *event);

    void tryToSendCamPosition();
    void sendCommand(const BinaryContent &commandContent, const QString &commandDescription);

    void forwardTelemetryDataFrame(const char *data, qint64 len);

    void updateTrackerValues(TelemetryDataFrame &telemetryDataFrame);
    void updateAntennaValues(TelemetryDataFrame &telemetryDataFrame);
    void updateTelemetryValues(TelemetryDataFrame &telemetryDataFrame);

    void processNewCameraTelemetryDataFrame(const QByteArray &rawData);

    bool processTelemetryPendingDatagramsV4();
    void processTelemetryPendingDatagramsUnknownFormat();
public:
    explicit HardwareLink(QObject *parent);
    ~HardwareLink();

    bool camConnectionOn();
    bool telemetryConnectionOn();
    const TelemetryDataFrame getTelemetryDataFrame();

    void open();
    void close();

    quint32 getSessionTimeMs();

    AntennaHardwareLink *antenna();

    // Commands
    void setHardwareCamStabilization(bool enabled);
    void setCamPosition(float roll, float pitch, float yaw);
    void setCamSpeed(float roll, float pitch, float yaw);
    void setCamZoom(float zoom);
    void setCamMotorStatus(bool enabled);
    virtual void setActiveOpticalSystemId(quint32 camId);
    void parkCamera();
    void setCamColorMode(int colorMode);
    void setLaserActivation(bool active);

    void setBombingPlacePos(double lat, double lon, double hmsl);
    void dropBomb(int index);

    void stopSnapshotSeries();
    void startSnapshotSeries(int intervalMsec);
    bool isSnapshotSeries();

    void startCamRecording();
    void stopCamRecording();
public slots:
    void lockTarget(const QPoint &targetCenter);
    void unlockTarget();
    void setTargetSize(int targetSize);

    void makeSnapshot();
    void activateCatapult();
    void onTracerModeChanged(const AutomaticTracerMode tracerMode);
    void onEmulatorTelemetryDataFrame(const EmulatorTelemetryDataFrame &emulatorTelemetryDataFrame);
private slots:
    void processUAVTelemetryPendingDatagrams();
    void processCamTelemetryPendingDatagrams();
    void processExtTelemetryPendingDatagrams();
    void readSerialPortMUSVData();
    virtual void videoFrameReceivedInternal(const QImage &frame, quint32 videoConnectionId);
    void doActivateCatapult();

    void onTelemetryDelayLineDequeue(const TelemetryDataFrame &value);
    void onCameraTelemetryDelayLineDequeue(const CameraTelemetryDataFrame &value);

    void doOnCommandSent(const BinaryContent &commandContent, const QString &commandDescription);
signals:
    void dataReceived(const TelemetryDataFrame &telemetryFrame, const QImage &videoFrame);
    void onHardwareLinkStateChanged();
    void onClientCommandSent(const DataExchangePackage &clientCommand);
    void onTelemetryReceived(const QString &telemetryHEX);
};

#endif // HARDWARELINK_H
