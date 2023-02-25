#include "MarkersSettingsEditor.h"
#include "ApplicationSettings.h"
#include "EnterProc.h"
#include "UserInterface/Forms/ArealObjectEditor.h"
#include "Map/MarkerThesaurus.h"
#include "Map/MapTilesImporter.h"

MarkersSettingsEditor::MarkersSettingsEditor(QWidget *parent) :
    QScrollArea(parent),
    _association(this)
{
    EnterProcStart("MarkersSettingsEditor::MarkersSettingsEditor");
    ApplicationSettings& applicationSettings = ApplicationSettings::Instance();

    auto markersLayout = CommonWidgetUtils::createGridLayoutForScrollArea(this);

    int rowIndex = 0;

    if (applicationSettings.isMarkersTabLicensed())
    {
        auto fpsMarkerThesaurus = new FilePathSelector(this, tr("Marker Thesaurus"), tr("Select Marker Thesaurus File"), tr("Database Files (*.db)"));

        auto btnImportThesauruasFromXML = new QPushButton(tr("Import"), this);
        btnImportThesauruasFromXML->setToolTip(tr("Import Thesaurus From XML File"));
        connect(btnImportThesauruasFromXML, &QPushButton::clicked, this, &MarkersSettingsEditor::onImportThesauruasFromXMLCicked);

        auto fpsMarkerStorage = new FilePathSelector(this, tr("Marker Storage"), tr("Select Marker Storage File"), tr("Database Files (*.db)"));

        markersLayout->addWidget(fpsMarkerThesaurus,            rowIndex, 1, 1, 1);
        markersLayout->addWidget(btnImportThesauruasFromXML,    rowIndex, 2, 1, 1);
        rowIndex++;

        markersLayout->addWidget(fpsMarkerStorage,              rowIndex, 1, 1, 1);
        rowIndex++;

        _association.addBinding(&applicationSettings.MarkerStorageDatabase,                     fpsMarkerStorage);
        _association.addBinding(&applicationSettings.MarkerThesaurusDatabase,                   fpsMarkerThesaurus);
    }

    auto fpsArealObjectDatabase = new FilePathSelector(this, tr("Areal Object Database"), tr("Select Areal Object Database File"), tr("Database Files (*.db)"));
    auto btnEditArealObjects = new QPushButton(tr("Edit"), this);
    connect(btnEditArealObjects, &QPushButton::clicked, this, &MarkersSettingsEditor::onEditArealObjectsCicked);

    markersLayout->addWidget(fpsArealObjectDatabase,        rowIndex, 1, 1, 1);
    markersLayout->addWidget(btnEditArealObjects,           rowIndex, 2, 1, 1);
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
    markerThesaurus.ImportAndReplaceFromXML(fileName);
}

void MarkersSettingsEditor::onEditArealObjectsCicked()
{
    auto editor = new ArealObjectEditor(this);
    editor->showNormal();
}
