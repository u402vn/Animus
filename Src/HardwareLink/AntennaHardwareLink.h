#ifndef ANTENNAHARDWARELINK_H
#define ANTENNAHARDWARELINK_H

#include <QObject>

class AntennaHardwareLink : public QObject
{
    Q_OBJECT

private:
    double _antennaElevation;
    double _antennaAzimuth;

    bool _fanEnabled;
    bool _heaterEnabled;
public:
    explicit AntennaHardwareLink(QObject *parent);

    double antennaElevation();
    double antennaAzimuth();

    void setFanEnabled(bool enabled);
    bool fanEnabled();

    void setHeaterEnabled(bool enabled);
    bool heaterEnabled();




signals:

public slots:
};

#endif // ANTENNAHARDWARELINK_H
