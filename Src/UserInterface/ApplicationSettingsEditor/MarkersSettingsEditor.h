#ifndef MARKERSSETTINGSEDITOR_H
#define MARKERSSETTINGSEDITOR_H

#include <QWidget>
#include <QCheckBox>
#include <QLineEdit>
#include <QGridLayout>
#include <QScrollArea>
#include <PreferenceAssociation.h>

class MarkersSettingsEditor final : public QScrollArea
{
    Q_OBJECT

    PreferenceAssociation _association;
public:
    explicit MarkersSettingsEditor(QWidget *parent);
    void loadSettings();
    void saveSettings();
private slots:
    void onImportThesauruasFromXMLCicked();
    void onCleanupMarkerThesaurusCicked();
    void onEditArealObjectsCicked();
};

#endif // MARKERSSETTINGSEDITOR_H
