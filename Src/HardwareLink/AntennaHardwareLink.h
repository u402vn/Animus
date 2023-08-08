#ifndef ANTENNAHARDWARELINK_H
#define ANTENNAHARDWARELINK_H

#include <QObject>

class AntennaHardwareLink : public QObject
{
    Q_OBJECT

private:
    double _antennaElevation;
    double _antennaAzimuth;

    bool _antennaFanEnabled;
    bool _antennaHeaterEnabled;
public:
    explicit AntennaHardwareLink(QObject *parent);

    double antennaElevation();
    double antennaAzimuth();

    bool antennaFanEnabled();
    bool antennaHeaterEnabled();
signals:

public slots:
};

#endif // ANTENNAHARDWARELINK_H
