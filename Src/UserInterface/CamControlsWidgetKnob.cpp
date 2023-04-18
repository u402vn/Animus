#include "CamControlsWidgetKnob.h"
#include <QPainter>
#include <QParallelAnimationGroup>
#include <QPropertyAnimation>
#include <QMouseEvent>
#include <math.h>
#include "ApplicationSettings.h"
#include "Common/CommonWidgets.h"

template<typename T>
T constrain(T Value, T Min, T Max)
{
    return (Value < Min) ? Min : (Value > Max) ? Max : Value;
}


CamControlsWidgetKnob::CamControlsWidgetKnob(QWidget *parent) : QWidget(parent),
    _joystickX(0), _joystickY(0),
    _returnAnimation(new QParallelAnimationGroup(this)),
    _alignment(Qt::AlignTop | Qt::AlignLeft)
{
    setMinimumSize(0.5 * DEFAULT_BUTTON_WIDTH, 0.5 * DEFAULT_BUTTON_WIDTH);
    ApplicationSettings& applicationSettings = ApplicationSettings::Instance();
    _joystickAxisMultiplier = applicationSettings.JoystickCameraAxisMultiplier;

    int animationDurationMsec = 0; //todo move to settings

    _xAnimation = new QPropertyAnimation(this, "x");
    _xAnimation->setEndValue(0.f);
    _xAnimation->setDuration(animationDurationMsec);
    _xAnimation->setEasingCurve(QEasingCurve::OutSine);

    _yAnimation = new QPropertyAnimation(this, "y");
    _yAnimation->setEndValue(0.f);
    _yAnimation->setDuration(animationDurationMsec);
    _yAnimation->setEasingCurve(QEasingCurve::OutSine);

    _returnAnimation->addAnimation(_xAnimation);
    _returnAnimation->addAnimation(_yAnimation);
}

float CamControlsWidgetKnob::x() const
{
    return _joystickX;
}

float CamControlsWidgetKnob::y() const
{
    return _joystickY;
}

void CamControlsWidgetKnob::setX(float value)
{
    _joystickX = constrain(value, -1.f, 1.f);

    qreal radius = ( _bounds.width() - _knopBounds.width() ) / 2;
    _knopBounds.moveCenter(QPointF(_bounds.center().x() + _joystickX * radius, _knopBounds.center().y()));

    update();
    doSetCamMovingSpeed();
}

void CamControlsWidgetKnob::setY(float value)
{
    _joystickY = constrain(value, -1.f, 1.f);

    qreal radius = ( _bounds.width() - _knopBounds.width() ) / 2;
    _knopBounds.moveCenter(QPointF(_knopBounds.center().x(), _bounds.center().y() - _joystickY * radius));

    update();
    doSetCamMovingSpeed();
}

void CamControlsWidgetKnob::setAlignment(Qt::Alignment f)
{
    _alignment = f;
}

void CamControlsWidgetKnob::resizeEvent(QResizeEvent *event)
{
    Q_UNUSED(event)

    float a = qMin(width(), height());

    QPointF topleft;

    if (_alignment.testFlag(Qt::AlignTop))
    {
        topleft.setY(0);
    }
    else if (_alignment.testFlag(Qt::AlignVCenter))
    {
        topleft.setY( ((height()-a)/2) );
    }
    else if(_alignment.testFlag(Qt::AlignBottom))
    {
        topleft.setY( height()-a );
    }

    if (_alignment.testFlag(Qt::AlignLeft))
    {
        topleft.setX(0);
    }
    else if(_alignment.testFlag(Qt::AlignHCenter))
    {
        topleft.setX( (width()-a)/2 );
    }
    else if(_alignment.testFlag(Qt::AlignRight))
    {
        topleft.setX( width()-a );
    }

    _bounds = QRectF(topleft, QSize(a, a));

    _knopBounds.setWidth(a * 0.3);
    _knopBounds.setHeight(a*0.3);

    // adjust knob position
    qreal radius = ( _bounds.width() - _knopBounds.width() ) / 2;
    _knopBounds.moveCenter(QPointF(_bounds.center().x() + _joystickX * radius, _bounds.center().y() - _joystickY * radius));
}

void CamControlsWidgetKnob::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setRenderHint(QPainter::HighQualityAntialiasing);

    // draw background
    QRadialGradient gradient(_bounds.center(), _bounds.width() / 2, _bounds.center());
    gradient.setFocalRadius(_bounds.width() * 0.3);
    gradient.setCenterRadius(_bounds.width() * 0.7);
    gradient.setColorAt(0, Qt::white);
    gradient.setColorAt(1, Qt::lightGray);

    painter.setPen(QPen(QBrush(Qt::gray), _bounds.width() * 0.005));
    painter.setBrush(QBrush(gradient));
    painter.drawEllipse(_bounds);

    // draw crosshair
    painter.setPen(QPen(QBrush(Qt::gray), _bounds.width() * 0.005));
    painter.drawLine(QPointF(_bounds.left(), _bounds.center().y()), QPointF(_bounds.center().x() - _bounds.width() * 0.35, _bounds.center().y()));
    painter.drawLine(QPointF(_bounds.center().x() + _bounds.width() * 0.35, _bounds.center().y()), QPointF(_bounds.right(), _bounds.center().y()));
    painter.drawLine(QPointF(_bounds.center().x(), _bounds.top()), QPointF(_bounds.center().x(), _bounds.center().y() - _bounds.width() * 0.35));
    painter.drawLine(QPointF(_bounds.center().x(), _bounds.center().y() + _bounds.width() * 0.35), QPointF(_bounds.center().x(), _bounds.bottom()));

    // draw knob
    if (!this->isEnabled()) return;

    gradient = QRadialGradient(_knopBounds.center(), _knopBounds.width()/2, _knopBounds.center());
    gradient.setColorAt(0, Qt::gray);
    gradient.setColorAt(1, Qt::darkGray);
    gradient.setFocalRadius(_knopBounds.width() * 0.2);
    gradient.setCenterRadius(_knopBounds.width() * 0.5);

    painter.setPen(QPen(QBrush(Qt::darkGray), _bounds.width() * 0.005));
    painter.setBrush(QBrush(gradient));
    painter.drawEllipse(_knopBounds);
}

void CamControlsWidgetKnob::mousePressEvent(QMouseEvent *event)
{
    if (_knopBounds.contains(event->pos()))
    {
        _returnAnimation->stop();
        _lastPos = event->pos();
        _knopPressed = true;
    }
}

void CamControlsWidgetKnob::mouseReleaseEvent(QMouseEvent *event)
{
    Q_UNUSED(event)

    _knopPressed = false;
    _returnAnimation->start();
}

void CamControlsWidgetKnob::mouseMoveEvent(QMouseEvent *event)
{
    if (!_knopPressed) return;

    // moved distance
    QPointF dPos = event->pos() - _lastPos;

    // change the distance sligthly to guarantee overlaping knop and pointer
    dPos += 0.05 * (event->pos() - _knopBounds.center());

    QPointF fromCenterToKnop = _knopBounds.center() + dPos - _bounds.center();

    qreal radius = ( _bounds.width() - _knopBounds.width() ) / 2;

    fromCenterToKnop.setX(constrain(fromCenterToKnop.x(), -radius, radius));
    fromCenterToKnop.setY(constrain(fromCenterToKnop.y(), -radius, radius));

    _knopBounds.moveCenter(fromCenterToKnop + _bounds.center());
    _lastPos = event->pos();

    update();

    if (radius == 0)
        return;
    float x = ( _knopBounds.center().x() - _bounds.center().x() ) / radius;
    float y = (-_knopBounds.center().y() + _bounds.center().y() ) / radius;

    _joystickX = x;
    _joystickY = y;

    doSetCamMovingSpeed();
}

void CamControlsWidgetKnob::doSetCamMovingSpeed()
{
    float speedYaw = _joystickX * _joystickAxisMultiplier;
    float speedPitch = - _joystickY * _joystickAxisMultiplier;
    float speedRoll = 0;

    emit onCamMovingSpeedChange(speedRoll, speedPitch, speedYaw);
}
