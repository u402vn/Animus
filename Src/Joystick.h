#ifndef JOYSTICK_H
#define JOYSTICK_H

#include <QObject>
#ifdef _WIN32
#include <SDL.h>
#else
#include <SDL2/SDL.h>
#endif

class Joystick final : public QObject
{
private:
    Q_OBJECT

    QString _mappingSettings;
    quint64 _workingTime;
    uint _processigIntervalMs;

    QString _joystick_guid;
    QList<int> _povs;
    QList<double> _axes;
    QList<bool> _buttons;

    void cleanupJoystickOut();
    void configureJoystick(const SDL_Event* event);
public:
    explicit Joystick(QObject *parent, const QString &mappingSettings, uint processigIntervalMs);
signals:
    void processJoystick(const QList<int> &povs, const QList<double> &axes, const QList<bool> &buttons);
private slots:
    void update();
};

#endif // JOYSTICK_H
