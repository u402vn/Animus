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
#include "ApplicationSettings.h"
#include "Common/CommonUtils.h"
#include "Common/BinaryContent.h"

const int PRIMARY_OPTYCAL_SYSTEM_ID   = 1;
const int SECONDARY_OPTYCAL_SYSTEM_ID = 2;

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

    QString _catapultSerialPortName;
    QStringList _catapultCommands;
    int _catapultCommandIdx;
    QSerialPort _catapultSerialPort;

    QTime _sessionTime;
    //quint32 _lastUpdatedTelemetryFrameNumber;

    QQueue<TelemetryDataFrame> _incommingFrames;
    TelemetryDataFrame _currentTelemetryDataFrame;
    QImage _videoFrame;

    CameraTelemetryDataFrame _cameraTelemetryDataFrame;
    EmulatorTelemetryDataFrame _emulatorTelemetryDataFrame;
    ExtendedTelemetryDataFrame _extendedTelemetryDataFrame;

    quint32 _telemetryFrameNumber;
    quint32 _videoFrameNumber;

    quint32 _videoLagFromTelemetry;

    QTimer *_fpsTimer;
    quint32 _videoFrameNumberPrevSec;
    quint32 _receivedFrameCountPrevSec;

    bool _isCameraFixed;
    double _fixedCamPitch, _fixedCamRoll, _fixedCamYaw;
    double _fixedCamZoom;
    double _expectedCamZoom;
    quint32 _opticalSystemId;

    bool _camConnectionOn, _telemetryConnectionOn;
    unsigned long long _prevCamConnectionByteCounter, _prevTelemetryConnectionByteCounter;
    unsigned long long _camConnectionByteCounter, _telemetryConnectionByteCounter;

    BinaryContent _camPositionCommand;
    quint32 _camPositionCommandTime;
    QString _camPositionCommandDescription;

    int _connectionsStatusesTimer;
    int _snapshotSeriesTimer;
    int _sendCamPositionTimer;

    CamAssemblyPreferences *_camAssemblyPreferences;
    AutomaticTracerMode _camTracerMode;

    WorldGPSCoord _bombingPlacePos;

    void timerEvent(QTimerEvent *event);

    void tryToSendCamPosition();
    void sendCommand(const BinaryContent &commandContent, const QString &commandDescription);

    void delayedTelemetryDataFrameProcessing(const TelemetryDataFrame &telemetryDataFrame);
    void notifyDataReceived();

    void forwardTelemetryDataFrame(const char *data, qint64 len);

    void updateTrackerValues(TelemetryDataFrame &telemetryDataFrame);
    void updateTelemetryValues(TelemetryDataFrame &telemetryDataFrame);

    void updateCameraTelemetryDataFrame(const QByteArray &rawData);

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

    // Commands
    void setHardwareCamStabilization(bool enabled);
    void setCamPosition(float roll, float pitch, float yaw);
    void setCamSpeed(float roll, float pitch, float yaw);
    void setCamZoom(float zoom);
    void setCamMotorStatus(bool enabled);
    void selectActiveCam(int camId);
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
    virtual void videoFrameReceivedInternal(const QImage &frame);
    void doActivateCatapult();
    void doProcessTelemetryDataFrameQueue();

    void doOnCommandSent(const BinaryContent &commandContent, const QString &commandDescription);
signals:
    void dataReceived(const TelemetryDataFrame &telemetryFrame, const QImage &videoFrame);
    void onHardwareLinkStateChanged();
    void onClientCommandSent(const ClientCommand &clientCommand);
    void onTelemetryReceived(const QString &telemetryHEX);
};

#endif // HARDWARELINK_H
