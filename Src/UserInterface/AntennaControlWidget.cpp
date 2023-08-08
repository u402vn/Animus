#include "AntennaControlWidget.h"

void AntennaControlWidget::initWidgets()
{
    EnterProc("AntennaControlWidget::initWidgets");

    _mainLayout = new QGridLayout(this);
    _mainLayout->setContentsMargins(0, 0, 0, 0);
    _mainLayout->setColumnStretch(0, 1);
    _mainLayout->setColumnStretch(1, 0);
    _mainLayout->setColumnStretch(2, 0);
    _mainLayout->setColumnStretch(3, 0);
    _mainLayout->setColumnStretch(4, 0);
    _mainLayout->setColumnStretch(5, 1);


    auto lblAntennaCoordCaption = new QLabel(tr("Ant."), this);
    _lblAntennaCoord = new QLabelEx(this);
    _antennaCoordSelector = new GPSCoordSelector(this);
    _antennaCoordSelector->setDescriptionVisible(false);
    connect(_lblAntennaCoord, &QLabelEx::clicked, this,  [=]()
    {
        _antennaCoordSelector->show(this->mapToGlobal(lblAntennaCoordCaption->pos()), _antennaCoord, "");
    });
    connect(_antennaCoordSelector, &GPSCoordSelector::onCoordSelectorChanged, this, [=](const WorldGPSCoord &gpsCoord, const QString &description)
    {
        _antennaCoord = gpsCoord;
        showCoordValues();
    });

    auto lblUAVCoordCaption = new QLabel(tr("UAV"), this);
    _lblUAVCoord = new QLabelEx(this);
    _uavCoordelector = new GPSCoordSelector(this);
    _uavCoordelector->setDescriptionVisible(false);
    connect(_lblUAVCoord, &QLabelEx::clicked, this,  [=]()
    {
        _uavCoordelector->show(this->mapToGlobal(lblAntennaCoordCaption->pos()), _uavCoord, "");
    });
    connect(_uavCoordelector, &GPSCoordSelector::onCoordSelectorChanged, this, [=](const WorldGPSCoord &gpsCoord, const QString &description)
    {
        _uavCoord = gpsCoord;
        showCoordValues();
    });

    _btnRotationAutoMode = CommonWidgetUtils::createButton(this,  tr("Rotate Auto"), tr("Rotate Auto"), true, 0, 0, "");
    _btnRotationSetManually = CommonWidgetUtils::createButton(this,  tr("Set Manually"), tr("Set Manually"), false, 0, 0, "");

    _btnHeatingAutoMode = CommonWidgetUtils::createButton(this,  tr("Heat Auto"), tr("Heat Auto"), true, 0, 0, "");
    _btnHeatingOnOff = CommonWidgetUtils::createButton(this,  tr("Heat On/Off"), tr("Heat On/Off"), true, 0, 0, "");

    _btnFanAutoMode = CommonWidgetUtils::createButton(this,  tr("Fan Auto"), tr("Fan Auto"), true, 0, 0, "");
    _btnFanOnOff = CommonWidgetUtils::createButton(this,  tr("Fan On/Off"), tr("Fan On/Off"), true, 0, 0, "");



    int row = 0;
    _mainLayout->addWidget(lblAntennaCoordCaption,          row, 0, 1, 1, Qt::AlignLeft);
    _mainLayout->addWidget(_lblAntennaCoord,                row, 1, 1, 4);
    row++;

    _mainLayout->addWidget(lblUAVCoordCaption,              row, 0, 1, 1, Qt::AlignLeft);
    _mainLayout->addWidget(_lblUAVCoord,                    row, 1, 1, 4);
    row++;

    _mainLayout->addWidget(_btnRotationAutoMode,            row, 1, 1, 2);
    _mainLayout->addWidget(_btnRotationSetManually,         row, 3, 1, 2);
    row++;

    _mainLayout->addWidget(_btnHeatingAutoMode,             row, 1, 1, 2);
    _mainLayout->addWidget(_btnHeatingOnOff,                row, 3, 1, 2);
    row++;

    _mainLayout->addWidget(_btnFanAutoMode,                 row, 1, 1, 2);
    _mainLayout->addWidget(_btnFanOnOff,                    row, 3, 1, 2);
    row++;


    _mainLayout->setRowStretch(row, 1);
}

void AntennaControlWidget::showCoordValues()
{
    _lblAntennaCoord->setText(_antennaCoord.EncodeLatLon(DegreeMinutesSecondsF, false));
    _lblUAVCoord->setText(_uavCoord.EncodeLatLon(DegreeMinutesSecondsF, false));
}

AntennaControlWidget::AntennaControlWidget(QWidget *parent) : QWidget(parent)
{
    EnterProc("AntennaControlWidget::AntennaControlWidget");
    initWidgets();


    showCoordValues();
}

AntennaControlWidget::~AntennaControlWidget()
{

}

void AntennaControlWidget::processTelemetry(const TelemetryDataFrame &telemetryFrame)
{
    _uavCoord = getUavCoordsFromTelemetry(telemetryFrame);
    showCoordValues();
}
