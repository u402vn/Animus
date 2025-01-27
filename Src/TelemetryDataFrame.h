#pragma once
#ifndef TELEMETRYDATAFRAME_H
#define TELEMETRYDATAFRAME_H

#include <QString>
#include <QQuaternion>
#include <QRect>
#include <QPoint>

#define ViewFieldBorderPointsCount 40

struct TelemetryDataFrame final
{
    quint32 Time;                   // ms
    double UavRoll;
    double UavPitch;
    double UavYaw;
    double UavLatitude_GPS;
    double UavLongitude_GPS;
    double UavAltitude_GPS;
    double UavAltitude_Barometric;
    float AirSpeed;                 // воздушная скорость [m/s]
    float GroundSpeed_GPS;          // скорость относительно земли (по gps) [m/s]
    float Course_GPS;
    float VerticalSpeed;            // вертикальная скорость [m/s]
    float WindDirection;            // направление ветра [deg]
    float WindSpeed;                // скорость ветра [m/s]
    float GroundSpeedNorth_GPS;     // скорость на север [m/s]
    float GroundSpeedEast_GPS;      // скорость на восток [m/s]    
    float AtmospherePressure;
    float AtmosphereTemperature;
    double CamRoll;
    double CamPitch;
    double CamYaw;
    double CamZoom;
    qint32 CamEncoderRoll;
    qint32 CamEncoderPitch;
    qint32 CamEncoderYaw;

    double OpticalSystemId;
    double FOVVerticalAngle;
    double FOVHorizontalAngle;

    float StabilizedCenterX;
    float StabilizedCenterY;
    float StabilizedRotationAngle;

    float TrackedTargetCenterX;
    float TrackedTargetCenterY;
    float TrackedTargetRectWidth;
    float TrackedTargetRectHeight;
    quint32 TrackedTargetState;
    double CalculatedTrackedTargetGPSLat;
    double CalculatedTrackedTargetGPSLon;
    double CalculatedTrackedTargetGPSHmsl;
    float CalculatedTrackedTargetSpeed;
    float CalculatedTrackedTargetDirection;

    qint32 VideoFPS;
    qint32 TelemetryFPS;

    static bool UseGimbalTelemetryOnlyForCalculation;

    float RangefinderDistance;      // Значение дистанции от дальномера
    float RangefinderTemperature;
    double CalculatedRangefinderGPSLat;
    double CalculatedRangefinderGPSLon;
    double CalculatedRangefinderGPSHmsl;
    double CalculatedGroundLevel;

    double ViewFieldBorderPointsLat[ViewFieldBorderPointsCount];
    double ViewFieldBorderPointsLon[ViewFieldBorderPointsCount];
    double ViewFieldBorderPointsHmsl[ViewFieldBorderPointsCount];

    quint32 CamTracerMode;

    double BombingPlacePosLat;
    double BombingPlacePosLon;
    double BombingPlacePosHmsl;

    double DistanceToBombingPlace;
    double AzimuthToBombingPlace;
    double AzimuthUAVToBombingPlace; // -180..+180
    double RemainingTimeToDropBomb; // [m/s]

    quint32 BombState;


    double AntennaLatitude_GPS;
    double AntennaLongitude_GPS;
    double AntennaAltitude_GPS;

    double AntennaElevation;
    double AntennaAzimuth;

    bool AntennaFanEnabled;
    bool AntennaHeaterEnabled;

    quint32 TelemetryFrameNumber;
    quint32 VideoFrameNumber;
    quint32 SessionTimeMs;

    void clear()
    {
        memset(this, 0, sizeof(TelemetryDataFrame));
    }

    TelemetryDataFrame()
    {
        clear();
    }

    double recalculatedCamYaw() const;
    const QQuaternion getCamQuaternion() const;
    const QRect targetRect() const;
    bool targetIsVisible() const;
    const QPoint stabilizedCenter() const;
};

// Q_DECLARE_METATYPE(TelemetryDataFrame)

enum DataExchangePackageDirection {Incoming, Outgoing};

struct DataExchangePackage final
{
    quint32 TelemetryFrameNumber;
    quint32 VideoFrameNumber;
    quint32 SessionTimeMs;

    QString ContentHEX;
    QString Description;
    DataExchangePackageDirection Direction;

    DataExchangePackage()
    {
        TelemetryFrameNumber = 0;
        VideoFrameNumber = 0;
        SessionTimeMs = 0;
    }

    void init(const TelemetryDataFrame &telemetryDataFrame)
    {
        VideoFrameNumber = telemetryDataFrame.VideoFrameNumber;
        TelemetryFrameNumber = telemetryDataFrame.TelemetryFrameNumber;
        SessionTimeMs = telemetryDataFrame.SessionTimeMs;
    }
};

struct WeatherDataItem final
{
    qint16 Altitude;
    double WindDirection;
    double WindSpeed;
    double AtmospherePressure;
    double AtmosphereTemperature;

    WeatherDataItem()
    {
        memset(this, 0, sizeof(WeatherDataItem));
    }
};

struct EmulatorTelemetryDataFrame final
{
    double UavRoll;
    double UavPitch;
    double UavYaw;
    double UavLatitude_GPS;
    double UavLongitude_GPS;
    double UavAltitude_GPS;
    double UavAltitude_Barometric;
    float Course_GPS;

    EmulatorTelemetryDataFrame()
    {
        memset(this, 0, sizeof(EmulatorTelemetryDataFrame));
    }

    void applyToTelemetryDataFrame(TelemetryDataFrame &telemetryDataFrame);
};


struct CameraTelemetryDataFrame final
{
    double CamRoll;
    double CamPitch;
    double CamYaw;
    double CamZoom;
    qint32 CamEncoderRoll;
    qint32 CamEncoderPitch;
    qint32 CamEncoderYaw;
    float RangefinderDistance;
    float RangefinderTemperature;

    void clear()
    {
        memset(this, 0, sizeof(CameraTelemetryDataFrame));
    }

    CameraTelemetryDataFrame()
    {
        clear();
    }

    void applyToTelemetryDataFrame(TelemetryDataFrame &telemetryDataFrame);
};


struct ExtendedTelemetryDataFrame final
{
    double AtmosphereTemperature;
    double AtmospherePressure;

    void clear()
    {
        memset(this, 0, sizeof(ExtendedTelemetryDataFrame));
    }

    ExtendedTelemetryDataFrame()
    {
        clear();
    }

    void applyToTelemetryDataFrame(TelemetryDataFrame &telemetryDataFrame);
};


double encoderAngleDegree(qint32 camEncoderValue);

#endif // TELEMETRYDATAFRAME_H
