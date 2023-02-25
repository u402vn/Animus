#ifndef HIDSETTINGSEDITOR_H
#define HIDSETTINGSEDITOR_H

#include <QWidget>
#include <QCheckBox>
#include <QLineEdit>
#include <QGridLayout>
#include <QScrollArea>
#include <QLabel>
#include <PreferenceAssociation.h>
#include "ApplicationSettings.h"
#include "Common/CommonWidgets.h"

class HIDSettingsEditor final : public QScrollArea
{
    Q_OBJECT
    PreferenceAssociation _association;
    UniquePreferenceGroup *_uniqueJButtonsGroup;
    UniquePreferenceGroup *_uniqueKeyboardGroup;
    QGridLayout *_keyMappingLayout;

    QLabel *_lblJoystickStateText;

    void addHIDButtonBinding(HIDButton prefIndex, int &gridRow);
    QLabel *createCommentLabel(const QString &text);
public:
    explicit HIDSettingsEditor(QWidget *parent);
    void loadSettings();
    void saveSettings();
public slots:
    void onJoystickStateTextChanged(const QString stateText);
};

#endif // HIDSETTINGSEDITOR_H
