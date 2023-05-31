#include "MapSettingsEditor.h"
#include "ApplicationSettings.h"
#include "EnterProc.h"
#include "Map/MapTileContainer.h"
#include "UserInterface/ConstantNames.h"

MapSettingsEditor::MapSettingsEditor(QWidget *parent) :
    QWidget(parent),
    _association(this)
{
    EnterProcStart("MarkersSettingsEditor::MarkersSettingsEditor");
    ApplicationSettings& applicationSettings = ApplicationSettings::Instance();

    auto pathLayout = CommonWidgetUtils::createVBoxLayout(this);

    auto cbTileReceivingMode = new QComboBoxExt(this, ConstantNames::TileReceivingModeCaptions());

    pathLayout->addWidget(cbTileReceivingMode);

    _mapDatabaseFiles = new QListWidget(this);
    _mapDatabaseFiles->setDragDropMode(QAbstractItemView::InternalMove);
    connect(_mapDatabaseFiles, &QListWidget::itemSelectionChanged, this, &MapSettingsEditor::onMapDatabaseFilesSelectionChanged);

    auto btnMapUpdate = new QPushButton(tr("Update"), this);
    auto btnMapNew = new QPushButton(tr("Add"), this);
    auto btnMapRemove = new QPushButton(tr("Remove"), this);

    auto buttonsLayout = new QHBoxLayout();
    buttonsLayout->addWidget(btnMapUpdate);
    connect(btnMapUpdate, &QPushButton::clicked, this, &MapSettingsEditor::onMapUpdateCicked);
    buttonsLayout->addWidget(btnMapNew);
    connect(btnMapNew, &QPushButton::clicked, this, &MapSettingsEditor::onMapNewCicked);
    buttonsLayout->addWidget(btnMapRemove);
    connect(btnMapRemove, &QPushButton::clicked, this, &MapSettingsEditor::onMapRemoveCicked);

    pathLayout->addWidget(_mapDatabaseFiles, 1);
    _fpsMap = addDBPathEditor(pathLayout, tr("Tile Map"));
    pathLayout->addLayout(buttonsLayout);

    pathLayout->addWidget(CommonWidgetUtils::createSeparator(this));
    auto fpsMapDownloadCashe = addDBPathEditor(pathLayout, tr("Tile Download Cache"));
    auto fpsHeightMap = addDBPathEditor(pathLayout, tr("Height Map"));
    auto fpsGeocoder = addDBPathEditor(pathLayout, tr("Geocoder"));


    _association.addBinding(&applicationSettings.TileReceivingMode,                    cbTileReceivingMode);
    _association.addBinding(&applicationSettings.DatabaseMapDownloadCashe,             fpsMapDownloadCashe);
    _association.addBinding(&applicationSettings.DatabaseHeightMap,                    fpsHeightMap);
    _association.addBinding(&applicationSettings.DatabaseGeocoder,                     fpsGeocoder);
}

void MapSettingsEditor::loadSettings()
{
    EnterProcStart("MapSettingsEditor::loadSettings");

    ApplicationSettings& applicationSettings = ApplicationSettings::Instance();
    _mapDatabaseFiles->insertItems(0, applicationSettings.getMapDatabaseFiles());

    _association.toEditor();
}

void MapSettingsEditor::saveSettings()
{
    EnterProcStart("MapSettingsEditor::saveSettings");

    ApplicationSettings& applicationSettings = ApplicationSettings::Instance();

    QStringList mapFiles;
    for (int i = 0; i <_mapDatabaseFiles->count(); i++)
        mapFiles.append(_mapDatabaseFiles->item(i)->text());
    applicationSettings.setMapDatabaseFiles(mapFiles);

    _association.fromEditor();
}

FilePathSelector *MapSettingsEditor::addDBPathEditor(QVBoxLayout *layout, const QString &caption)
{
    auto pathSelector = new FilePathSelector(this, caption, tr("Open %1 Database File").arg(caption), tr("Database Files (*.db)"));
    if (layout != nullptr)
        layout->addWidget(pathSelector);

    return pathSelector;
}

void MapSettingsEditor::onMapUpdateCicked()
{
    QString path = _fpsMap->getSelectedPath();
    if (path.isEmpty())
        return;

    auto selectedItems = _mapDatabaseFiles->selectedItems();
    foreach(auto item, selectedItems)
    {
        item->setText(path);
    }
}

void MapSettingsEditor::onMapNewCicked()
{
    QString path = _fpsMap->getSelectedPath();
    if (!path.isEmpty())
        _mapDatabaseFiles->addItem(path);
}

void MapSettingsEditor::onMapRemoveCicked()
{
    qDeleteAll(_mapDatabaseFiles->selectedItems());
}

void MapSettingsEditor::onMapDatabaseFilesSelectionChanged()
{
    auto selectedItems = _mapDatabaseFiles->selectedItems();
    if (!selectedItems.isEmpty())
    {
        auto item = selectedItems.first();
        _fpsMap->setSelectedPath(item->text());
    }
}
