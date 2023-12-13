#include "CamPreferences.h"
#include "Common/CommonData.h"
#include <QtMath>

OpticalDevicePreferences::OpticalDevicePreferences(QObject *parent) : QObject(parent)
{
}

OpticalDevicePreferences::~OpticalDevicePreferences()
{

}

void OpticalDevicePreferences::init(qint32 farmeWidth, qint32 frameHeight, quint32 zoomMinValue, quint32 zoomMaxValue,
                          quint32 magnifierSize, qreal magnifierScale,
                          const QList<double> &fovHorizontalAngles, const QList<double> &fovVerticalAngles,
                          const QList<double> &automaticTracerSpeedMultipliers, const QList<double> &manualSpeedMultipliers)
{
    _frameWidth = farmeWidth;
    _frameHeight = frameHeight;
    _zoomMin = zoomMinValue;
    _zoomMax = zoomMaxValue;
    _magnifierSize = magnifierSize;
    _magnifierScale = magnifierScale;
    _fovHorizontalAngles = fovHorizontalAngles;
    _fovVerticalAngles = fovVerticalAngles;
    _automaticTracerSpeedMultipliers = automaticTracerSpeedMultipliers;
    _manualSpeedMultipliers = manualSpeedMultipliers;

    //Q_ASSERT(_fovHorizontalAngles.count() == _fovVerticalAngles.count());
    //Q_ASSERT(_fovHorizontalAngles.count() == _automaticTracerSpeedMultipliers.count());
    //Q_ASSERT(_fovHorizontalAngles.count() == _manualSpeedMultipliers.count());
}

quint32 OpticalDevicePreferences::zoomIndex(quint32 zoom)
{
    if (zoom > _zoomMax)
        zoom = _zoomMax;
    else if (zoom < _zoomMin)
        zoom = _zoomMin;
    qint32 index = zoom - _zoomMin;
    return index;
}

double OpticalDevicePreferences::fovHorizontalAngle(qint32 zoom)
{
    return _fovHorizontalAngles.at(zoomIndex(zoom));
}

double OpticalDevicePreferences::fovVerticalAngle(qint32 zoom)
{
    return _fovVerticalAngles.at(zoomIndex(zoom));
}

qint32 OpticalDevicePreferences::frameWidth()
{
    return _frameWidth;
}

qint32 OpticalDevicePreferences::frameHeight()
{
    return _frameHeight;
}

double OpticalDevicePreferences::automaticTracerSpeedMultiplier(qint32 zoom)
{
    return _automaticTracerSpeedMultipliers.at(zoomIndex(zoom));
}

double OpticalDevicePreferences::manualSpeedMultipliers(qint32 zoom)
{
    return _manualSpeedMultipliers.at(zoomIndex(zoom));
}

const QMatrix4x4 OpticalDevicePreferences::projection(qint32 zoom)
{
    float verticalAngle = fovVerticalAngle(zoom);
    float aspectRatio = _frameWidth / _frameHeight;
    float nearPlane = 1;
    float farPlane = 10000; //???

    QMatrix4x4 matrix;
    matrix.perspective(verticalAngle, aspectRatio, nearPlane, farPlane);
    return matrix;
}

quint32 OpticalDevicePreferences::zoomMax()
{
    return _zoomMax;
}

quint32 OpticalDevicePreferences::magnifierSize()
{
    return _magnifierSize;
}

qreal OpticalDevicePreferences::magnifierScale()
{
    return _magnifierScale;
}

void OpticalDevicePreferences::incMagnifierScale(qreal delta)
{
    _magnifierScale = qBound(1.1, _magnifierScale + delta, 5.0);
}

quint32 OpticalDevicePreferences::zoomMin()
{
    return _zoomMin;
}

inline double pointAngleRad(int screenSizePix, double screenSizeDegree, int screenPosPix)
{
    double screenSizeRad = screenSizeDegree * PI / 180.0;
    double screenHalfSizePix = 0.5 * screenSizePix;
    double screenPosRad = atan(tan(screenSizeRad / 2) * (screenHalfSizePix - screenPosPix) / screenHalfSizePix);
    return screenPosRad;
}

void OpticalDevicePreferences::getScreenPointAnglesRad(const quint32 zoom, const qint32 screenX, const qint32 screenY, double &angleXRad, double &angleYRad)
{
    angleXRad = pointAngleRad(_frameWidth, fovHorizontalAngle(zoom), screenX);
    angleYRad = pointAngleRad(_frameHeight, fovVerticalAngle(zoom), screenY);
}

void OpticalDevicePreferences::getScreenPointAnglesDegree(const quint32 zoom, const qint32 screenX, const qint32 screenY, double &angleXDegree, double &angleYDegree)
{
    angleXDegree = pointAngleRad(_frameWidth, fovHorizontalAngle(zoom), screenX) * 180 / PI;
    angleYDegree = pointAngleRad(_frameHeight, fovVerticalAngle(zoom), screenY) * 180 / PI;
}
/*
inline double zoomAngle(double degrees, int zoom)
{
    double theta = degrees * PI /180.0;
    return 2 * qTan(1 * qTan(theta / 2) / zoom) * 180.0 / PI;
}
*/

//--------------------------------------------------------

CamAssemblyPreferences::CamAssemblyPreferences(QObject *parent) : QObject(parent)
{

}

CamAssemblyPreferences::~CamAssemblyPreferences()
{

}

void CamAssemblyPreferences::initGimbal(double encoderAutomaticTracerMultiplier)
{
    _encoderAutomaticTracerMultiplier = encoderAutomaticTracerMultiplier;
}

void CamAssemblyPreferences::initOpticalDevice(qint32 opticalSystemId, qint32 farmeWidth, qint32 frameHeight, quint32 zoomMinValue, quint32 zoomMaxValue,
                                     quint32 magnifierSize, qreal magnifierScale,
                                     const QList<double> &fovHorizontalAngles, const QList<double> &fovVerticalAngles,
                                     const QList<double> &automaticTracerSpeedMultipliers, const QList<double> &manualSpeedMultipliers)
{
    auto opticalDevice = _opticalDevices.value(opticalSystemId, nullptr);
    if (opticalDevice == nullptr)
    {
        opticalDevice = new OpticalDevicePreferences(this);
        _opticalDevices.insert(opticalSystemId, opticalDevice);
    }
    opticalDevice->init(farmeWidth, frameHeight, zoomMinValue, zoomMaxValue, magnifierSize, magnifierScale,
                        fovHorizontalAngles, fovVerticalAngles,
                        automaticTracerSpeedMultipliers, manualSpeedMultipliers);
}

OpticalDevicePreferences *CamAssemblyPreferences::opticalDevice(qint32 opticalDeviceId)
{
    auto camPrefernces = _opticalDevices.value(opticalDeviceId, nullptr);
    return camPrefernces;
}

double CamAssemblyPreferences::encoderAutomaticTracerMultiplier()
{
    return _encoderAutomaticTracerMultiplier;
}
