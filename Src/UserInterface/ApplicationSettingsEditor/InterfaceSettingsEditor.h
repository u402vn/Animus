#ifndef INTERFACESETTINGSEDITOR_H
#define INTERFACESETTINGSEDITOR_H

#include <QWidget>
#include <QScrollArea>
#include <PreferenceAssociation.h>

class InterfaceSettingsEditor final : public QScrollArea
{
    Q_OBJECT

    PreferenceAssociation _association;
public:
    explicit InterfaceSettingsEditor(QWidget *parent);
    void loadSettings();
    void saveSettings();
};

#endif // INTERFACESETTINGSEDITOR_H
