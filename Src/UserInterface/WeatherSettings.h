#ifndef WEATHERSETTINGS_H
#define WEATHERSETTINGS_H

#include <QWidget>
#include <QSlider>
#include "Common/CommonWidgets.h"

class WeatherSettings final : public QFrame
{
    Q_OBJECT

    QDialEx * _windDirection;
    QSlider * _windSpeed;
protected:
    void focusOutEvent(QFocusEvent * event);
public:
    explicit WeatherSettings(QWidget *parent);
    void activate();
public slots:
    void windChanged(int);
};

#endif // WEATHERSETTINGS_H
