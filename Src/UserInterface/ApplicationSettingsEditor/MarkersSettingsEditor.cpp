#include "MarkersSettingsEditor.h"
#include "ApplicationSettings.h"
#include "EnterProc.h"
#include "UserInterface/Forms/ArealObjectEditor.h"
#include "Map/MarkerThesaurus.h"
#include "Map/MapTilesImporter.h"

constexpr int MARKERS_TAB_FIRST_COLUMN_WIDTH = 150;

MarkersSettingsEditor::MarkersSettingsEditor(QWidget *parent) :
    QScrollArea(parent),
    _association(this)
{
    EnterProcStart("MarkersSettingsEditor::MarkersSettingsEditor");
    ApplicationSettings& applicationSettings = ApplicationSettings::Instance();

    auto markersLayout = CommonWidgetUtils::createGridLayoutForScrollArea(this);
    markersLayout->setColumnMinimumWidth(0, MARKERS_TAB_FIRST_COLUMN_WIDTH);

    int rowIndex = 0;

    if (applicationSettings.isMarkersTabLicensed())
    {
        auto fpsMarkerThesaurus = new FilePathSelector(this, tr("Marker Thesaurus"), tr("Select Marker Thesaurus File"), tr("Database Files (*.db)"));
        fpsMarkerThesaurus->setLabelWidth(MARKERS_TAB_FIRST_COLUMN_WIDTH);
        auto btnImportThesauruasFromXML = new QPushButton(tr("Import"), this);
        btnImportThesauruasFromXML->setToolTip(tr("Import Thesaurus From XML File"));
        connect(btnImportThesauruasFromXML, &QPushButton::clicked, this, &MarkersSettingsEditor::onImportThesauruasFromXMLCicked);
        auto btnCleanupMarkerThesaurus = new QPushButton(tr("Clean up"), this);
        connect(btnCleanupMarkerThesaurus, &QPushButton::clicked, this, &MarkersSettingsEditor::onCleanupMarkerThesaurusCicked);

        auto fpsMarkerStorage = new FilePathSelector(this, tr("Marker Storage"), tr("Select Marker Storage File"), tr("Database Files (*.db)"));
        fpsMarkerStorage->setLabelWidth(MARKERS_TAB_FIRST_COLUMN_WIDTH);

        markersLayout->addWidget(fpsMarkerThesaurus,            rowIndex, 0, 1, 5);
        rowIndex++;
        markersLayout->addWidget(btnImportThesauruasFromXML,    rowIndex, 1, 1, 1);
        markersLayout->addWidget(btnCleanupMarkerThesaurus,     rowIndex, 2, 1, 1);
        rowIndex++;

        markersLayout->addWidget(fpsMarkerStorage,              rowIndex, 0, 1, 5);
        rowIndex++;

        _association.addBinding(&applicationSettings.MarkerStorageDatabase,                     fpsMarkerStorage);
        _association.addBinding(&applicationSettings.MarkerThesaurusDatabase,                   fpsMarkerThesaurus);
    }

    auto fpsArealObjectDatabase = new FilePathSelector(this, tr("Areal Object Database"), tr("Select Areal Object Database File"), tr("Database Files (*.db)"));
    fpsArealObjectDatabase->setLabelWidth(MARKERS_TAB_FIRST_COLUMN_WIDTH);
    auto btnEditArealObjects = new QPushButton(tr("Edit"), this);
    connect(btnEditArealObjects, &QPushButton::clicked, this, &MarkersSettingsEditor::onEditArealObjectsCicked);

    markersLayout->addWidget(fpsArealObjectDatabase,        rowIndex, 0, 1, 5);
    rowIndex++;
    markersLayout->addWidget(btnEditArealObjects,           rowIndex, 1, 1, 1);
    rowIndex++;

    markersLayout->setRowStretch(rowIndex++, 1);

    _association.addBinding(&applicationSettings.ArealObjectDatabase,                       fpsArealObjectDatabase);
}

void MarkersSettingsEditor::loadSettings()
{
    EnterProcStart("MarkersSettingsEditor::loadSettings");
    _association.toEditor();
}

void MarkersSettingsEditor::saveSettings()
{
    EnterProcStart("MarkersSettingsEditor::saveSettings");
    _association.fromEditor();
}

void MarkersSettingsEditor::onImportThesauruasFromXMLCicked()
{
    EnterProcStart("MarkersSettingsEditor::onImportThesauruasFromXMLCicked");
    QString fileName = CommonWidgetUtils::showOpenFileDialog(tr("Open Thesaurus XML File for import"), QString(), tr("XML Files (*.xml)"));
    if (fileName.isEmpty())
        return;
    MarkerThesaurus& markerThesaurus = MarkerThesaurus::Instance();
    markerThesaurus.importAndReplaceFromXML(fileName);
}

void MarkersSettingsEditor::onCleanupMarkerThesaurusCicked()
{
    bool needCleanup = CommonWidgetUtils::showConfirmDialog(tr("Do you want to clean up the thesaurus and remove obsolete marker templates?"), false);
    if (!needCleanup)
        return;
    MarkerThesaurus& markerThesaurus = MarkerThesaurus::Instance();
    markerThesaurus.cleanUp();
}

void MarkersSettingsEditor::onEditArealObjectsCicked()
{
    auto editor = new ArealObjectEditor(this);
    editor->showNormal();
}
