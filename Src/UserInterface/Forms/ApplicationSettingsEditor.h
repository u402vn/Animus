#ifndef APPLICATIONSETTINGSEDITOR_H
#define APPLICATIONSETTINGSEDITOR_H

#include <QWidget>
#include <QDialog>
#include "UserInterface/ApplicationSettingsEditor/CameraListSettingsEditor.h"
#include "UserInterface/ApplicationSettingsEditor/HIDSettingsEditor.h"
#include "UserInterface/ApplicationSettingsEditor/InterfaceSettingsEditor.h"
#include "UserInterface/ApplicationSettingsEditor/SessionsSettingsEditor.h"
#include "UserInterface/ApplicationSettingsEditor/MarkersSettingsEditor.h"
#include "UserInterface/ApplicationSettingsEditor/MapSettingsEditor.h"
#include "UserInterface/ApplicationSettingsEditor/ApplicationStatisticView.h"
#include "UserInterface/ApplicationSettingsEditor/BallisticSettingsEditor.h"
#include "UserInterface/HIDController.h"
#include "PreferenceAssociation.h"

class ApplicationSettingsEditor final : public QDialog
{
    Q_OBJECT
private:
    PreferenceAssociation _association;

    HIDController *_hidController;

    HIDSettingsEditor *_hidSettingsEditor;
    InterfaceSettingsEditor *_interfaceSettingsEditor;
    MarkersSettingsEditor *_markersSettingsEditor;
    MapSettingsEditor *_mapSettingsEditor;
    BallisticSettingsEditor *_ballisticSettingsEditor;
    SessionsSettingsEditor *_sessionsSettingsEditor;

    void initWidgets();

    void loadSettings();
    void saveSettings();
public:
    virtual void accept();
    explicit ApplicationSettingsEditor(QWidget *parent, HIDController *hidController);
    ~ApplicationSettingsEditor();
};

#endif // APPLICATIONSETTINGSEDITOR_H
