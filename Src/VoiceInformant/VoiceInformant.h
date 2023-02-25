#ifndef VOICEINFORMANT_H
#define VOICEINFORMANT_H

#include <QObject>
#include <QSoundEffect>
#include <QVector>
#include <QUrl>
#include <QMap>
#include "Constants.h"

class VoiceInformant : public QObject
{
    Q_OBJECT
    QVector<VoiceMessage> _messageQueue;
    QSoundEffect _sound;
    bool _enabled;

    QMap<VoiceMessage, QUrl> _messagesUrlMap;

    void tryToSay();
public:
    explicit VoiceInformant(QObject *parent);
    void sayMessage(const VoiceMessage message);
    void setVolume(qreal volume);
    void setEnabled(bool enabled);
private slots:
    void onSoundStatusChanged();
};

#endif // VOICEINFORMANT_H
