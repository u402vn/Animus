#include "VoiceInformant.h"
#include <QtGlobal>
#include <QTimer>
#include <QDebug>

void VoiceInformant::tryToSay()
{
    if (_sound.isPlaying())
    {
        QTimer::singleShot(200, Qt::PreciseTimer, this, &VoiceInformant::tryToSay);
        return;
    }

    auto status = _sound.status();

    if ((status == QSoundEffect::Status::Null || status == QSoundEffect::Status::Ready) &&
            (_messageQueue.count() > 0))
    {
        VoiceMessage message = _messageQueue.takeFirst();
        QUrl url = _messagesUrlMap.value(message);
        _sound.setSource(url);
        _sound.play();
    }

    if (status == QSoundEffect::Status::Error)
        qDebug() << "Error in VoiceInformant" << _sound.source().url();
}

VoiceInformant::VoiceInformant(QObject *parent) : QObject(parent)
{    
    setVolume(1.0);
    setEnabled(false);

    connect(&_sound, &QSoundEffect::statusChanged, this, &VoiceInformant::onSoundStatusChanged);

    _messagesUrlMap.insert(AnumusActivated, QUrl::fromLocalFile(":/VoiceInformant/AnumusActivated.wav"));
    _messagesUrlMap.insert(DropBomb, QUrl::fromLocalFile(":/VoiceInformant/DropBomb.wav"));
    _messagesUrlMap.insert(TurnLeft, QUrl::fromLocalFile(":/VoiceInformant/TurnLeft.wav"));
    _messagesUrlMap.insert(TurnRight, QUrl::fromLocalFile(":/VoiceInformant/TurnRight.wav"));
    _messagesUrlMap.insert(Lapel, QUrl::fromLocalFile(":/VoiceInformant/Lapel.wav"));
    _messagesUrlMap.insert(TargetLocked, QUrl::fromLocalFile(":/VoiceInformant/TargetLocked.wav"));
    _messagesUrlMap.insert(TargetDropped, QUrl::fromLocalFile(":/VoiceInformant/TargetDropped.wav"));
    _messagesUrlMap.insert(ZoomChange, QUrl::fromLocalFile(":/VoiceInformant/ZoomChange.wav"));
}

void VoiceInformant::sayMessage(const VoiceMessage message)
{
    if (!_enabled)
        return;

    //remove other messages from logical group
    switch (message)
    {
    case TargetLocked:
        _messageQueue.removeAll(TargetDropped);
        _messageQueue.removeAll(TargetLocked);
        break;
    case TargetDropped:
        _messageQueue.removeAll(TargetDropped);
        _messageQueue.removeAll(TargetLocked);
        break;
    default:
        break;
    }

    if (!_messageQueue.contains(message))
        _messageQueue.append(message);

    tryToSay();
}

void VoiceInformant::setVolume(qreal volume)
{
    volume = qBound(0.0, volume, 1.0);

    _sound.setVolume(volume);
}

void VoiceInformant::setEnabled(bool enabled)
{
    _enabled = enabled;
}

void VoiceInformant::onSoundStatusChanged()
{
    tryToSay();
}
