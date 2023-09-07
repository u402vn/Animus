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


    //auto lblAntennaCoordCaption = new QLabel(tr("Ant."), this);
    _lblAntennaCoord = new QLabelEx(this);
    _antennaCoordSelector = new GPSCoordSelector(this);
    _antennaCoordSelector->setDescriptionVisible(false);
    connect(_lblAntennaCoord, &QLabelEx::clicked, this,  [=]()
    {
        _antennaCoordSelector->show(_lblAntennaCoord, _antennaCoord, "");
    });
    connect(_antennaCoordSelector, &GPSCoordSelector::onCoordSelectorChanged, this, [=](const WorldGPSCoord &gpsCoord, const QString &description)
    {
        _antennaCoord = gpsCoord;
        showCoordValues();
    });

    //auto lblUAVCoordCaption = new QLabel(tr("UAV"), this);
    _lblUAVCoord = new QLabelEx(this);
    _uavCoordelector = new GPSCoordSelector(this);
    _uavCoordelector->setDescriptionVisible(false);
    connect(_lblUAVCoord, &QLabelEx::clicked, this,  [=]()
    {
        _uavCoordelector->show(_lblUAVCoord, _uavCoord, "");
    });
    connect(_uavCoordelector, &GPSCoordSelector::onCoordSelectorChanged, this, [=](const WorldGPSCoord &gpsCoord, const QString &description)
    {
        _uavCoord = gpsCoord;
        showCoordValues();
    });


    auto lblRotation = new QLabel(tr("Rotation"), this);
    _btnRotationAutoMode = CommonWidgetUtils::createButton(this,  tr("Auto"), tr("Rotate Auto"), true, QUARTER_BUTTON_WIDTH, DEFAULT_BUTTON_HEIGHT, "");
    _btnRotationSetManually = CommonWidgetUtils::createButton(this,  tr("Set"), tr("Set Position"), false, QUARTER_BUTTON_WIDTH, DEFAULT_BUTTON_HEIGHT, "");



    auto lblHeating = new QLabel(tr("Heating"), this);
    _btnHeatingAutoMode = CommonWidgetUtils::createButton(this,  tr("Auto"), tr("Heat Auto"), true, QUARTER_BUTTON_WIDTH, DEFAULT_BUTTON_HEIGHT, "");
    _btnHeatingOnOff = CommonWidgetUtils::createButton(this,  tr("On/Off"), tr("Heat On/Off"), true, QUARTER_BUTTON_WIDTH, DEFAULT_BUTTON_HEIGHT, "");
    connect(_btnHeatingAutoMode, &QPushButtonEx::clicked, this, [=](bool checked)
    {
        _btnHeatingOnOff->setEnabled(!checked);
    });
    connect(_btnHeatingOnOff, &QPushButtonEx::clicked, this, [=](bool checked)
    {
        if (! _btnHeatingAutoMode->isChecked())
            _hardwareLink->antenna()->setHeaterEnabled(checked);
    });

    auto lblFan = new QLabel(tr("Fan"), this);
    _btnFanAutoMode = CommonWidgetUtils::createButton(this,  tr("Auto"), tr("Fan Auto"), true, QUARTER_BUTTON_WIDTH, DEFAULT_BUTTON_HEIGHT, "");
    _btnFanOnOff = CommonWidgetUtils::createButton(this,  tr("On/Off"), tr("Fan On/Off"), true, QUARTER_BUTTON_WIDTH, DEFAULT_BUTTON_HEIGHT, "");
    connect(_btnFanAutoMode, &QPushButtonEx::clicked, this, [=](bool checked)
    {
        _btnFanOnOff->setEnabled(!checked);
    });
    connect(_btnFanOnOff, &QPushButtonEx::clicked, this, [=](bool checked)
    {
        if (! _btnHeatingAutoMode->isChecked())
            _hardwareLink->antenna()->setFanEnabled(checked);
    });


    int row = 0;

    //_mainLayout->addWidget(lblUAVCoordCaption,              row, 0, 1, 1, Qt::AlignLeft);
    _mainLayout->addWidget(_lblUAVCoord,                    row, 0, 1, 3);
    row++;

    //_mainLayout->addWidget(lblAntennaCoordCaption,          row, 0, 1, 1, Qt::AlignLeft);
    _mainLayout->addWidget(_lblAntennaCoord,                row, 0, 1, 3);
    row++;

    _mainLayout->addWidget(lblRotation,                     row, 0, 1, 1);
    _mainLayout->addWidget(_btnRotationAutoMode,            row, 1, 1, 1);
    _mainLayout->addWidget(_btnRotationSetManually,         row, 2, 1, 1);
    row++;

    _mainLayout->addWidget(lblHeating,                      row, 0, 1, 1);
    _mainLayout->addWidget(_btnHeatingAutoMode,             row, 1, 1, 1);
    _mainLayout->addWidget(_btnHeatingOnOff,                row, 2, 1, 1);
    row++;

    _mainLayout->addWidget(lblFan,                          row, 0, 1, 1);
    _mainLayout->addWidget(_btnFanAutoMode,                 row, 1, 1, 1);
    _mainLayout->addWidget(_btnFanOnOff,                    row, 2, 1, 1);
    row++;


    _mainLayout->setRowStretch(row, 1);
}

void AntennaControlWidget::showCoordValues()
{
    _lblAntennaCoord->setText(_antennaCoord.EncodeLatLon(DegreeMinutesSecondsF, false));
    _lblUAVCoord->setText(_uavCoord.EncodeLatLon(DegreeMinutesSecondsF, false));
}

void AntennaControlWidget::applyState()
{
    auto antenna = _hardwareLink->antenna();

}

AntennaControlWidget::AntennaControlWidget(QWidget *parent, HardwareLink *hardwareLink) : QWidget(parent)
{
    EnterProc("AntennaControlWidget::AntennaControlWidget");
    _hardwareLink = hardwareLink;
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
