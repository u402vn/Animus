#include "AntennaHardwareLink.h"

AntennaHardwareLink::AntennaHardwareLink(QObject *parent) : QObject(parent)
{

}

double AntennaHardwareLink::antennaElevation()
{
    return _antennaElevation;
}

double AntennaHardwareLink::antennaAzimuth()
{
    return _antennaAzimuth;
}

bool AntennaHardwareLink::antennaFanEnabled()
{
    return _antennaFanEnabled;
}

bool AntennaHardwareLink::antennaHeaterEnabled()
{
    return _antennaHeaterEnabled;
}
