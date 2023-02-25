#ifndef MAPSETTINGSEDITOR_H
#define MAPSETTINGSEDITOR_H

#include <QWidget>
#include <QCheckBox>
#include <QLineEdit>
#include <QGridLayout>
#include <QListWidget>
#include <PreferenceAssociation.h>
#include "UserInterface/FilePathSelector.h"

class MapSettingsEditor final : public QWidget
{
    Q_OBJECT

    PreferenceAssociation _association;

    QListWidget *_mapDatabaseFiles;
    FilePathSelector *_fpsMap;

    FilePathSelector *addDBPathEditor(QVBoxLayout * layout, const QString &caption);
public:
    explicit MapSettingsEditor(QWidget *parent);

    void loadSettings();
    void saveSettings();
private slots:
    void onMapUpdateCicked();
    void onMapNewCicked();
    void onMapRemoveCicked();
    void onMapDatabaseFilesSelectionChanged();
};

#endif // MAPSETTINGSEDITOR_H
