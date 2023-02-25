#ifndef GPSCOORDINDICATOR_H
#define GPSCOORDINDICATOR_H

#include <QWidget>
#include <QLineEdit>
#include <QComboBox>
#include <QLabel>
#include "TelemetryDataFrame.h"
#include "Common/CommonData.h"

class GPSCoordIndicator final : public QWidget
{
    Q_OBJECT

    GeographicalCoordinatesFormat _coordFormat;

    WorldGPSCoord _uavPos, _screenCenterPos, _targetPos;
    double _azimuth, _distance;

    QComboBox * _sourceSelector;
    QLineEdit * _edtUAVCoordLat;
    QLineEdit * _edtUAVCoordLon;

    QLabel * _label;
    QLineEdit * _edtDistance;
    QLineEdit * _edtAzimut;

    QLineEdit * createCoordEdit();
    void showData();
public:
    explicit GPSCoordIndicator(QWidget *parent);
    void processTelemetry(const TelemetryDataFrame &telemetryDataFrame);
private slots:
    void onSourceChanged(int index);
};

#endif // GPSCOORDINDICATOR_H
