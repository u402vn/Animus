#include "GPSCoordIndicator.h"
#include <QHBoxLayout>
#include <QLabel>
#include "ApplicationSettings.h"
#include "Common/CommonUtils.h"
#include "Map/MapTileContainer.h"

QLineEdit * GPSCoordIndicator::createCoordEdit()
{
    auto edit = new QLineEdit(this);
    edit->setReadOnly(true);
    edit->setAlignment(Qt::AlignRight);
    return edit;
}

GPSCoordIndicator::GPSCoordIndicator(QWidget *parent) : QWidget(parent)
{
    _coordFormat = DegreeMinutesSecondsF; //todo move to settings

    _uavPos.setIncorrect();
    _screenCenterPos.setIncorrect();
    _targetPos.setIncorrect();

    _sourceSelector = new QComboBox(this);
    _sourceSelector->addItem(tr("UAV"));
    _sourceSelector->addItem(tr("Camera"));
    _sourceSelector->addItem(tr("Target"));
    _sourceSelector->setFocusPolicy(Qt::NoFocus);
    connect(_sourceSelector, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &GPSCoordIndicator::onSourceChanged);

    _edtUAVCoordLat = createCoordEdit();
    _edtUAVCoordLon = createCoordEdit();

    auto coordLayout = new QGridLayout(this);

    coordLayout->setMargin(0);
    coordLayout->setSpacing(1);

    coordLayout->addWidget(_sourceSelector, 1, 1, 1, 1);
    coordLayout->addWidget(_edtUAVCoordLat, 1, 2, 1, 1);
    coordLayout->addWidget(_edtUAVCoordLon, 1, 3, 1, 1);

    _label = new QLabel(tr("Bearing"), this);

    _edtDistance = createCoordEdit();
    _edtAzimut = createCoordEdit();

    coordLayout->addWidget(_label, 2, 1, 1, 1);
    coordLayout->addWidget(_edtDistance, 2, 2, 1, 1);
    coordLayout->addWidget(_edtAzimut, 2, 3, 1, 1);
}

void GPSCoordIndicator::processTelemetry(const TelemetryDataFrame &telemetryDataFrame)
{
    _uavPos = getUavCoordsFromTelemetry(telemetryDataFrame);
    _screenCenterPos = getRangefinderCoordsFromTelemetry(telemetryDataFrame);
    _targetPos = getTrackedTargetCoordsFromTelemetry(telemetryDataFrame);
    if (!_uavPos.isIncorrect() && !_targetPos.isIncorrect() )
    {
        _uavPos.getDistanceAzimuthTo(_targetPos, _distance, _azimuth);
    }

    showData();
}

void GPSCoordIndicator::onSourceChanged(int index)
{
    Q_UNUSED(index)
    showData();
}

void GPSCoordIndicator::showData()
{
    WorldGPSCoord coord;

    int currIndex = _sourceSelector->currentIndex();
    switch (currIndex)
    {
    case 0:
        coord = _uavPos;
        break;
    case 1:
        coord = _screenCenterPos;
        break;
    case 2:
        coord = _targetPos;
        break;
    default:
        coord = _uavPos;
    }

    if (!coord.isIncorrect())
    {
        Q_ASSERT(coord.CoordSystem == WGS84);

        ApplicationSettings& applicationSettings = ApplicationSettings::Instance();
        if (applicationSettings.UIPresentationCoordSystem == SK42)
            coord = coord.convertWGS84toSK42();

        _edtUAVCoordLat->setText(coord.EncodeLatitude(_coordFormat));
        _edtUAVCoordLon->setText(coord.EncodeLongitude(_coordFormat));
    }
    else
    {
        _edtUAVCoordLat->setText("");
        _edtUAVCoordLon->setText("");
    }

    if (!_targetPos.isIncorrect())
    {
        _edtAzimut->setText(tr("%1°").arg(QString::number(_azimuth, 'f', 0)));
        _edtDistance->setText(tr("%1 m").arg(QString::number(_distance, 'f', 0)));
    }
    else
    {
        _edtAzimut->setText("");
        _edtDistance->setText("");
    }
}
