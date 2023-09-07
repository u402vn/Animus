#include "AntennaHardwareLink.h"
#include "ApplicationSettings.h"


AntennaHardwareLink::AntennaHardwareLink(QObject *parent) : QObject(parent)
{
    ApplicationSettings& applicationSettings = ApplicationSettings::Instance();
    QString coordStr = applicationSettings.LastAntennaCoord;
    WorldGPSCoord gpsCoord;
    gpsCoord.DecodeLatLon(coordStr);

    _fanEnabled = false;
    _heaterEnabled = false;
}

double AntennaHardwareLink::antennaElevation()
{
    return _antennaElevation;
}

double AntennaHardwareLink::antennaAzimuth()
{
    return _antennaAzimuth;
}

void AntennaHardwareLink::setFanEnabled(bool enabled)
{
    //todo send command
    _fanEnabled = enabled; //???
}

bool AntennaHardwareLink::fanEnabled()
{
    return _fanEnabled;
}

void AntennaHardwareLink::setHeaterEnabled(bool enabled)
{
    //todo send command
    _heaterEnabled = enabled; //???
}

bool AntennaHardwareLink::heaterEnabled()
{
    return _heaterEnabled;
}
