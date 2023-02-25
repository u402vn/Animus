#ifndef UAVSIMDATASENDER_H
#define UAVSIMDATASENDER_H

#include <QObject>
#include <QUdpSocket>
#include <QTimerEvent>
#include <QVector>
#include "ApplicationSettings.h"

#pragma pack(push, 1)
struct UDPSimulatorTelemetryMessageV4
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
    uint32_t TargetWidth;       // экранная ширина сопровождаемой цели
    uint32_t TargetHeight;      // экранная высота сопровождаемой цели
    uint8_t TargetState;        // статус сопровождаемой цели

    UDPSimulatorTelemetryMessageV4()
    {
        memset(this, 0, sizeof(UDPSimulatorTelemetryMessageV4));
    }
};
#pragma pack(pop)

enum SimulatorVideoFrameTrafficSource
{
    Sim_XPlane,
    Sim_Yurion
};

class UAVSimDataSender : public QObject
{
    Q_OBJECT

    QString _telemetryFile;
    int _telemetryUDPPort;
    QString _videoFile;
    SimulatorVideoFrameTrafficSource _trafficSource;
    int _videoUDPPort;

    int _currentMessageNumber;
    QVector<UDPSimulatorTelemetryMessageV4> _telemetryMessages;
    QByteArray _urionVideo;
    int _urionVideoPosition;

    quint8 * _compressedXPlaneData;
    int _compressedXPlaneDataSize;
    int _frameNumber;

    QUdpSocket _udpTelemetrySocket;
    QUdpSocket _udpVideoSocket;
    QUdpSocket _udpCommandsSocket;

    int _telemetryTimerId;
    int _videoTimerId;

    bool _pause;

    CameraControlModes _controlMode;
    double _camX, _camY, _camZ, _camZoom, _camYTarget, _camXTarget, _camZoomTarget;

    bool _bombState;

    bool _isFreezedUavRoll;
    bool _isFreezedUavPitch;
    bool _isFreezedUavYaw;
    bool _isFreezedGPSLat;
    bool _isFreezedGPSLon;
    bool _isFreezedGPSHmsl;
    bool _isFreezedGPSCourse;
    bool _isFreezedWindDirection;
    bool _isFreezedWindSpeed;
    bool _isRangefinderDistance;

    double _freezedUavRoll;
    double _freezedUavPitch;
    double _freezedUavYaw;
    double _freezedGPSLat;
    double _freezedGPSLon;
    double _freezedGPSHmsl;
    double _freezedGPSCourse;
    double _freezedWindDirection;
    double _freezedWindSpeed;
    double _freezedRangefinderDistance;

    void updateCamPitchRoll();

    bool loadFromCSV(const QString &fileName);
    void loadYurionVideo(const QString &fileName);
    void loadXPlaneVideo(const QString &fileName);

    void sendTelemetryMassage();
    void sendYurionMessage();
    void sendXPLaneVideoMessage();

    void timerEvent(QTimerEvent *event);
public:
    explicit UAVSimDataSender(QObject *parent,
                              const QString &telemetryFile, int telemetryUDPPort,
                              const QString &videoFile, SimulatorVideoFrameTrafficSource trafficSource, int videoUDPPort,
                              CameraControlModes controlMode);

    void setPause(bool pause);


    double camXTarget();
    double camYTarget();
    void setCamXTarget(double value);
    void setCamYTarget(double value);
    void setCamZoomTarget(double value);

    void setBombState(bool state);

    void setFreezedUavRoll(bool freezed, double value);
    void setFreezedUavPitch(bool freezed, double value);
    void setFreezedUavYaw(bool freezed, double value);
    void setFreezedGPSLat(bool freezed, double value);
    void setFreezedGPSLon(bool freezed, double value);
    void setFreezedGPSHmsl(bool freezed, double value);
    void setFreezedGPSCourse(bool freezed, double value);
    void setFreezedWindDirection(bool freezed, double value);
    void setFreezedWindSpeed(bool freezed, double value);
    void setFreezeRangefinderDistance(bool freezed, double value);

    // roll, pitch, yaw, gps_lat, gps_lon, gps_hmsl, gps_course

signals:

public slots:
};

#endif // UAVSIMDATASENDER_H
