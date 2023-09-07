#include "GPSCoordSelector.h"
#include <QGridLayout>
#include "Common/CommonWidgets.h"

QDoubleSpinBox *GPSCoordSelector::createGeoCoordSpinBox(double min, double max)
{
    auto spinBox = new QDoubleSpinBox(this);
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

    _initInProgress = 0;

    _sbCoordLat = createGeoCoordSpinBox(-90, +90);
    connect(_sbCoordLat, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),
            this, &GPSCoordSelector::onCoordAsNumberChanged);
    _sbCoordLon = createGeoCoordSpinBox(-180, +180);
    connect(_sbCoordLon, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),
            this, &GPSCoordSelector::onCoordAsNumberChanged);

    _edDescription = new QLineEdit (this);
    _edDescription->setMinimumHeight(DEFAULT_BUTTON_HEIGHT);
    connect(_edDescription, &QLineEdit::textChanged, this, &GPSCoordSelector::onTextEditorChanged);


    _edtCoordLat = CommonWidgetUtils::createCoordEdit(this);
    connect(_edtCoordLat, &QLineEdit::textEdited, this, &GPSCoordSelector::onCoordAsTextChanged);
    _edtCoordLon = CommonWidgetUtils::createCoordEdit(this);
    connect(_edtCoordLon, &QLineEdit::textEdited, this, &GPSCoordSelector::onCoordAsTextChanged);


    auto _btnCancel = CommonWidgetUtils::createButton(this,  tr("Cancel"), tr("Cancel"), false, 0, 0, "");
    connect(_btnCancel, &QPushButtonEx::clicked, this, [=]()
    {
        setupCoord(_intialCoord, _intialDescription);
        WorldGPSCoord gpsCoord(_sbCoordLat->value(), _sbCoordLon->value());
        emit onCoordSelectorChanged(gpsCoord, _edDescription->text());
        close();
    });

    auto mainGridLayout = new QGridLayout(this);
    mainGridLayout->setContentsMargins(0, 0, 0, 0);
    int row = 0;

    mainGridLayout->addWidget(_edDescription,                       row, 1, 1, 2);
    row++;

    mainGridLayout->addWidget(_edtCoordLat,                         row, 1, 1, 1);
    mainGridLayout->addWidget(_edtCoordLon,                         row, 2, 1, 1);
    row++;


    mainGridLayout->addWidget(_sbCoordLat,                          row, 1, 1, 1);
    mainGridLayout->addWidget(_sbCoordLon,                          row, 2, 1, 1);
    row++;

    mainGridLayout->addWidget(_btnCancel,                           row, 2, 1, 1);
    row++;
}

GPSCoordSelector::~GPSCoordSelector()
{

}

void GPSCoordSelector::setupCoord(const WorldGPSCoord &gpsCoord, const QString &description)
{
    _initInProgress++;

    _intialCoord = gpsCoord;
    _intialDescription = description;

    _edDescription->setText(description);

    _sbCoordLat->setValue(gpsCoord.lat);
    _sbCoordLon->setValue(gpsCoord.lon);

    _edtCoordLat->setText(gpsCoord.EncodeLatitude(GeographicalCoordinatesFormat::DegreeMinutesSeconds));
    _edtCoordLon->setText(gpsCoord.EncodeLongitude(GeographicalCoordinatesFormat::DegreeMinutesSeconds));

    _initInProgress--;
}

void GPSCoordSelector::show(const QPoint &screenPos, const WorldGPSCoord &gpsCoord, const QString &description)
{
    setupCoord(gpsCoord, description);

    int selectorHeight = height();
    resize(DEFAULT_BUTTON_WIDTH, selectorHeight);
    move(screenPos);
    QFrame::show();
}

void GPSCoordSelector::show(const QLabel *label, const WorldGPSCoord &gpsCoord, const QString &description)
{
    auto pos = label->pos() + QPoint(0, label->height());
    auto screenPos = static_cast<QWidget*>(label->parent())->mapToGlobal(pos);
    show(screenPos, gpsCoord, description);
}


void GPSCoordSelector::setDescriptionVisible(bool visible)
{
    _edDescription->setVisible(visible);
}

void GPSCoordSelector::onCoordAsNumberChanged(double value)
{
    Q_UNUSED(value)

    if (_initInProgress > 0)
        return;

    _initInProgress++;

    WorldGPSCoord gpsCoord(_sbCoordLat->value(), _sbCoordLon->value());

    _edtCoordLat->setText(gpsCoord.EncodeLatitude(GeographicalCoordinatesFormat::DegreeMinutesSeconds));
    _edtCoordLon->setText(gpsCoord.EncodeLongitude(GeographicalCoordinatesFormat::DegreeMinutesSeconds));

    emit onCoordSelectorChanged(gpsCoord, _edDescription->text());

    _initInProgress--;
}

void GPSCoordSelector::onCoordAsTextChanged(const QString &text)
{
    Q_UNUSED(text)

    if (_initInProgress > 0)
        return;

    _initInProgress++;

    WorldGPSCoord gpsCoord;
    gpsCoord.DecodeLatLon(QString("%1 %2").arg(_edtCoordLat->displayText()).arg(_edtCoordLon->displayText()));

    _sbCoordLat->setValue(gpsCoord.lat);
    _sbCoordLon->setValue(gpsCoord.lon);

    emit onCoordSelectorChanged(gpsCoord, _edDescription->text());

    _initInProgress--;
}

void GPSCoordSelector::onTextEditorChanged()
{
    if (_initInProgress > 0)
        return;

    _initInProgress++;

    WorldGPSCoord gpsCoord(_sbCoordLat->value(), _sbCoordLon->value());
    emit onCoordSelectorChanged(gpsCoord, _edDescription->text());

    _initInProgress--;
}


