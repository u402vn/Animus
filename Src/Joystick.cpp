#include "Joystick.h"
#include <QTimer>
#include <QDebug>

// http://blog.5pmcasual.com/game-controller-api-in-sdl2.html

Joystick::Joystick(QObject *parent, const QString &mappingSettings, uint processigIntervalMs) : QObject(parent)
{
    _mappingSettings = mappingSettings;
    _workingTime = 0;
    _processigIntervalMs = processigIntervalMs;

    if (SDL_Init(SDL_INIT_GAMECONTROLLER))
    {
        qDebug() << "Cannot initialize SDL: " << SDL_GetError();
    }

    QTimer::singleShot(100, Qt::PreciseTimer, this, &Joystick::update);
}

void Joystick::cleanupJoystickOut()
{
    for (int i = 0; i < _povs.count(); i++)
        _povs[i] = 0;

    for (int i = 0; i < _axes.count(); i++)
        _axes[i] = 0;

    for (int i = 0; i < _buttons.count(); i++)
        _buttons[i] = false;
}

QString getJoystickGUID(int device_index)
{
    QString joystick_guid_str;
    SDL_Joystick * sdl_joystick = SDL_JoystickOpen(device_index);

    if (sdl_joystick)
    {
        char joystick_guid[1024];
        SDL_JoystickGetGUIDString (SDL_JoystickGetGUID(sdl_joystick), joystick_guid, sizeof(joystick_guid));
        joystick_guid_str = joystick_guid;
        SDL_JoystickClose(sdl_joystick);
    }
    return joystick_guid_str;
}

void Joystick::configureJoystick(const SDL_Event *event)
{
    _joystick_guid = getJoystickGUID(event->jdevice.which);

    if (!SDL_IsGameController (event->cdevice.which))
    {
        SDL_Joystick * sdl_joystick = SDL_JoystickOpen (event->jdevice.which);

        if (sdl_joystick)
        {
            const char * joystick_name = SDL_JoystickName (sdl_joystick);
            int povs = SDL_JoystickNumHats(sdl_joystick);
            int axes = SDL_JoystickNumAxes(sdl_joystick);
            int buttons = SDL_JoystickNumButtons(sdl_joystick);

            _povs.clear();
            for (int i = 0; i < povs; ++i)
                _povs.append(0);

            _axes.clear();
            for (int i = 0; i < axes; ++i)
                _axes.append(0);

            _buttons.clear();
            for (int i = 0; i < buttons; ++i)
                _buttons.append (false);

            // https://github.com/gabomdq/SDL_GameControllerDB
			// https://github.com/gabomdq/SDL_GameControllerDB/blob/master/gamecontrollerdb.txt
            QString mapping = QString ("%1,%2,%3")
                    .arg (_joystick_guid)
                    .arg (joystick_name)
                    .arg (_mappingSettings);

            SDL_GameControllerAddMapping(mapping.toStdString().c_str());
            SDL_JoystickClose(sdl_joystick);
        }
    }

    SDL_GameControllerOpen(event->cdevice.which);
}

int getPOVangle(quint8 anglePOV)
{
    switch (anglePOV)
    {
    case SDL_HAT_RIGHTUP:
        return 45;
    case SDL_HAT_RIGHTDOWN:
        return 135;
    case SDL_HAT_LEFTDOWN:
        return 225;
    case SDL_HAT_LEFTUP:
        return 315;
    case SDL_HAT_UP:
        return 0;
    case SDL_HAT_RIGHT:
        return 90;
    case SDL_HAT_DOWN:
        return 180;
    case SDL_HAT_LEFT:
        return 270;
    default:
        return -1;
    }
}

void Joystick::update()
{
    SDL_Event sdl_event;

    while (SDL_PollEvent (&sdl_event))
    {
        switch (sdl_event.type)
        {
        case SDL_JOYDEVICEADDED:
            configureJoystick(&sdl_event);
            break;
        case SDL_JOYDEVICEREMOVED:
            SDL_JoystickClose(SDL_JoystickOpen(sdl_event.jdevice.which));
            SDL_GameControllerClose(SDL_GameControllerOpen(sdl_event.cdevice.which));
            _joystick_guid.clear();
            cleanupJoystickOut();
            break;
        case SDL_CONTROLLERAXISMOTION:
            if (sdl_event.caxis.axis < _axes.size())
                _axes[sdl_event.caxis.axis] = static_cast<qreal> (sdl_event.caxis.value) / 32767;
            break;
        case SDL_JOYBUTTONUP:
        case SDL_JOYBUTTONDOWN:
            _buttons[sdl_event.jbutton.button] = sdl_event.jbutton.state == SDL_PRESSED;
            break;
        case SDL_JOYHATMOTION:
            _povs[sdl_event.jhat.hat] = getPOVangle(sdl_event.jhat.value);
            break;
        }
    }

    if (!_joystick_guid.isEmpty() && (_workingTime % _processigIntervalMs == 0))
        emit processJoystick(_povs, _axes, _buttons);

    _workingTime += 10;
    QTimer::singleShot(10, Qt::PreciseTimer, this, &Joystick::update);
}
