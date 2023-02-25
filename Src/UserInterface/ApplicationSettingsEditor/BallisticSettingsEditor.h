#ifndef BALLISTICSETTINGSEDITOR_H
#define BALLISTICSETTINGSEDITOR_H

#include <QWidget>
#include "PreferenceAssociation.h"
#include "UserInterface/ApplicationSettingsEditor/jsedit.h"

class BallisticSettingsEditor final : public QWidget
{
    Q_OBJECT

    PreferenceAssociation _association;

    // https://github.com/fffaraz/Etcetera/blob/master/ofi-labs-x2/javascript/jsedit/jsedit.cpp
    JSEdit *_macroTextEditor;
public:
    explicit BallisticSettingsEditor(QWidget *parent);
    void loadSettings();
    void saveSettings();
private slots:
    void onResetMacroCicked();
};

#endif // BALLISTICSETTINGSEDITOR_H
