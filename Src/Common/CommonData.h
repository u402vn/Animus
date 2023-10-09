#pragma once
#ifndef COMMONDATA_H
#define COMMONDATA_H

#include <QString>
#include <QPointF>
#include <QMatrix4x4>

constexpr double PI = 3.14159265358979323846;
constexpr double EARTH_RADIUS_M = 6371200; //m. Alternative values: 6378245, 6356863
constexpr double INCORRECT_COORDINATE = -100000;
constexpr double INCORRECT_DISTANCE = -100000;
constexpr double INCORRECT_TIME = -100000;

constexpr quint32 VIDEO_FRAMES_PER_FILE_DEFAULT = 750;
constexpr quint32 VIDEO_FILE_FRAME_FREQUENCY = 25;
constexpr quint32 VIDEO_FILE_QUALITY_DEFAULT = 50;

enum GlobalCoordSystem
{
    WGS84,
    SK42
};

enum GeographicalCoordinatesFormat
{
    Degree,
    DegreeMinutes,
    DegreeMinutesSecondsF,
    DegreeMinutesSeconds
};

struct FrameShift2D final
{
    double X, Y, A;
};

struct WorldGPSCoord final
{
    double lat, lon, hmsl;
    GlobalCoordSystem CoordSystem;

    WorldGPSCoord(const double Lat, const double Lon, const double Hmsl = 0);
    WorldGPSCoord();
    bool isIncorrect() const;
    void setIncorrect();

    bool getDistanceAzimuthTo(const WorldGPSCoord &toGPSCoord, double &distance, double &azimuth) const;
    const WorldGPSCoord getBiasPoint(double distance, double azimut) const;
    const QMatrix4x4 lookAt(const WorldGPSCoord &toGPSCoord) const;

    const WorldGPSCoord convertSK42toWGS84() const;
    const WorldGPSCoord convertWGS84toSK42() const;
    const QPointF getSK42() const;

    const QString EncodeLatitude(GeographicalCoordinatesFormat format) const;
    const QString EncodeLongitude(GeographicalCoordinatesFormat format) const;
    QString EncodeLatLon(GeographicalCoordinatesFormat format, bool safeString) const;


    static const QString postfixN();
    static const QString postfixS();
    static const QString postfixW();
    static const QString postfixE();

    bool DecodeLatLon(const QString &coordStr);

    bool operator!=(const WorldGPSCoord &coord);
    bool operator==(const WorldGPSCoord &coord);
};

inline double deg2rad(double angleDegree)
{
    return (angleDegree * PI) / 180.0;
}

inline double rad2deg(double angleRad)
{
    return (angleRad * 180.0) / PI;
}

#endif // COMMONDATA_H
