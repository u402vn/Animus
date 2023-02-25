#include "WeatherSettings.h"
#include <QLabel>

WeatherSettings::WeatherSettings(QWidget *parent) : QFrame(parent)
{
    setWindowFlags(Qt::Popup);
    setFocusPolicy(Qt::ClickFocus);

    _windDirection =  CommonWidgetUtils::createDialEx(this, 0.45 * DEFAULT_BUTTON_WIDTH, 0, 360);
    _windDirection->setWrapping(true);
    connect(_windDirection, &QDialEx::valueChanged, this, &WeatherSettings::windChanged);

    QLabel * lblWindDirection = new QLabel(tr("Direction"), this);

    _windSpeed = new QSlider(this);
    _windSpeed->setOrientation(Qt::Vertical);
    _windSpeed->setMaximumWidth(20);
    _windSpeed->setRange(0, 20);
    connect(_windSpeed, &QSlider::valueChanged, this, &WeatherSettings::windChanged);

    QLabel * lblWindSpeed = new QLabel(tr("Speed"), this);


    QGridLayout * gridLayout = new QGridLayout(this);
    gridLayout->setHorizontalSpacing(15);
    gridLayout->setVerticalSpacing(15);
    int row = 0;

    gridLayout->addWidget(_windDirection,              row, 1, 1, 2, Qt::AlignCenter);
    gridLayout->addWidget(_windSpeed,                  row, 3, 1, 1, Qt::AlignHCenter);
    row++;

    gridLayout->addWidget(lblWindDirection,            row, 1, 1, 2, Qt::AlignCenter);
    gridLayout->addWidget(lblWindSpeed,                row, 3, 1, 1, Qt::AlignHCenter);
    row++;
}

void WeatherSettings::focusOutEvent(QFocusEvent *event)
{
    close();
}

void WeatherSettings::activate()
{
    QWidget *parentWidget = (QWidget *)parent();
    move(parentWidget->mapToGlobal(QPoint(0, parentWidget->height())));
    show();
}

void WeatherSettings::windChanged(int)
{
    // do nothing
}
