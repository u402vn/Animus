#ifndef SESSIONSSETTINGSEDITOR_H
#define SESSIONSSETTINGSEDITOR_H

#include <QWidget>
#include <QScrollArea>
#include <QLabel>
#include "PreferenceAssociation.h"
#include "Common/CommonWidgets.h"

class SessionsSettingsEditor final : public QScrollArea
{
    Q_OBJECT

    PreferenceAssociation _association;
    QComboBoxExt *_cbCurrentCamera;
    QLabel *_lblCurrentCameraInfo;
public:
    explicit SessionsSettingsEditor(QWidget *parent);
    void loadSettings();
    void saveSettings();
private slots:
    void onCurrentCameraChanged(int index);
};

#endif // SESSIONSSETTINGSEDITOR_H
