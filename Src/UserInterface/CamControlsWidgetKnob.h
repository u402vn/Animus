#ifndef CAMCONTROLSWIDGETKNOB_H
#define CAMCONTROLSWIDGETKNOB_H

#include <QWidget>

class QPropertyAnimation;
class QParallelAnimationGroup;

class CamControlsWidgetKnob : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(float x READ x WRITE setX)
    Q_PROPERTY(float y READ y WRITE setY)
public:
    explicit CamControlsWidgetKnob(QWidget *parent);

    float x() const;
    float y() const;

signals:
    void onCamMovingSpeedChange(float speedRoll, float speedPitch, float speedYaw);

public slots:
    void setX(float value);
    void setY(float value);

    void setAlignment(Qt::Alignment f);
private:
    void resizeEvent(QResizeEvent *event) override;
    virtual void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;

    void doSetCamMovingSpeed();

    float _joystickX, _joystickY;
    qreal _joystickAxisMultiplier;

    QParallelAnimationGroup *_returnAnimation;
    QPropertyAnimation *_xAnimation, *_yAnimation;

    QRectF _bounds, _knopBounds;

    QPoint _lastPos;
    bool _knopPressed;

    Qt::Alignment _alignment;
};


#endif // CAMCONTROLSWIDGETKNOB_H
