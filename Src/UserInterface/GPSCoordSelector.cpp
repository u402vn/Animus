#include "GPSCoordSelector.h"
#include <QGridLayout>
#include "Common/CommonWidgets.h"

QDoubleSpinBox *GPSCoordSelector::createGeoCoordSpinBox(double min, double max)
{
    QDoubleSpinBox * spinBox = new QDoubleSpinBox(this);
    spinBox->setMinimum(min);
    spinBox->setMaximum(max);
    spinBox->setDecimals(6);
    spinBox->setSingleStep(0.000001);
    spinBox->setAlignment(Qt::AlignRight);
    spinBox->setMinimumHeight(DEFAULT_BUTTON_HEIGHT);
    return spinBox;
}

void GPSCoordSelector::focusOutEvent(QFocusEvent *event)
{
    close();
}

GPSCoordSelector::GPSCoordSelector(QWidget *parent) : QFrame(parent)
{
    setWindowFlags(Qt::Popup);
    setFocusPolicy(Qt::ClickFocus);

    _initInProgress = true;

    _sbTargetLat = createGeoCoordSpinBox(-90, +90);
    connect(_sbTargetLat, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),
            this, &GPSCoordSelector::onCoordEditorChanged);
    _sbTargetLon = createGeoCoordSpinBox(-180, +180);
    connect(_sbTargetLon, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),
            this, &GPSCoordSelector::onCoordEditorChanged);

    _edDescription = new QLineEdit (this);
    _edDescription->setMinimumHeight(DEFAULT_BUTTON_HEIGHT);
    connect(_edDescription, &QLineEdit::textChanged, this, &GPSCoordSelector::onTextEditorChanged);

    QGridLayout * mainGridLayout = new QGridLayout(this);
    mainGridLayout->setContentsMargins(0, 0, 0, 0);
    int row = 0;

    mainGridLayout->addWidget(_edDescription,                      row, 1, 1, 2);
    row++;

    mainGridLayout->addWidget(_sbTargetLat,                        row, 1, 1, 1);
    mainGridLayout->addWidget(_sbTargetLon,                        row, 2, 1, 1);
    row++;
}

GPSCoordSelector::~GPSCoordSelector()
{

}

void GPSCoordSelector::show(const QPoint &screenPos, const WorldGPSCoord &gpsCoord, const QString &description)
{
    _initInProgress = true;

    _edDescription->setText(description);
    _sbTargetLat->setValue(gpsCoord.lat);
    _sbTargetLon->setValue(gpsCoord.lon);

    int selectorHeight = height();
    resize(DEFAULT_BUTTON_WIDTH, selectorHeight);
    move(screenPos);
    QFrame::show();

    _initInProgress = false;
}

void GPSCoordSelector::setDescriptionVisible(bool visible)
{
    _edDescription->setVisible(visible);
}

void GPSCoordSelector::onCoordEditorChanged(double value)
{
    Q_UNUSED(value)
    processChanges();
}

void GPSCoordSelector::onTextEditorChanged()
{
    processChanges();
}


void GPSCoordSelector::processChanges()
{
    if (_initInProgress)
        return;
    WorldGPSCoord gpsCoord(_sbTargetLat->value(), _sbTargetLon->value());
    emit onCoordSelectorChanged(gpsCoord, _edDescription->text());
}

