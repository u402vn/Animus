#include "ApplicationSettingsEditor.h"
#include <QApplication>
#include <QDesktopWidget>
#include <QRadioButton>
#include <QStyle>
#include <QDialogButtonBox>
#include <QVBoxLayout>
#include <QTabWidget>
#include "ApplicationSettings.h"
#include "EnterProc.h"

ApplicationSettingsEditor::ApplicationSettingsEditor(QWidget *parent, HIDController *hidController) :
    QDialog(parent),
    _association(this),
    _hidController(hidController)
{
    EnterProcStart("ApplicationSettingsEditor::ApplicationSettingsEditor");

    initWidgets();
    loadSettings();
}

ApplicationSettingsEditor::~ApplicationSettingsEditor()
{
    ApplicationSettings& applicationSettings = ApplicationSettings::Instance();
    applicationSettings.reloadPreferences();
}

void ApplicationSettingsEditor::initWidgets()
{
    //MapTilesImporter importer;
    //importer.importMapFromMapnikFiles("d:\\Projects\\Forest\\ForestMap.db", "d:\\Projects\\Forest\\к передаче Гомельский район\\Tiles\\Mapnik", ForestMap);
    //importer.importMapFromSASFiles("d:\\Data\\Maps\\ghyb_gibraltar2_en.db", "d:\\Projects\\SAS.Planet.Release.140303\\cache\\both", GoogleHybrid);
    //importer.importMapFromSASFiles("d:\\Data\\Maps\\gsat_gibraltar2.db", "d:\\Projects\\SAS.Planet.Release.140303\\cache\\sat", GoogleSatellite);
    //importer.importMapFromSASFiles("d:\\Data\\Maps\\yahyb_gibraltar2.db", "d:\\Projects\\SAS.Planet.Release.140303\\cache\\yahyb", YandexHybrid);

    //importer.importMapFromSASFiles("c:\\Map_KZ\\ghyb_kz.db", "d:\\Projects\\SAS.Planet.Release.181221\\cache\\both", GoogleHybrid);
    //importer.importMapFromSASFiles("c:\\Map_KZ\\gsat_kz.db", "d:\\Projects\\SAS.Planet.Release.181221\\cache\\sat", GoogleSatellite);
    //importer.importMapFromSASFiles("c:\\Map_KZ\\yasat_kz.db", "d:\\Projects\\SAS.Planet.Release.181221\\cache\\yasat", YandexSatellite);
    //importer.importMapFromSASFiles("c:\\Map_KZ\\yahib_kz.db", "d:\\Projects\\SAS.Planet.Release.181221\\cache\\maps.yandex.com.Hybrid", YandexHybrid);
    //importer.importMapFromSASFiles("c:\\Map_KZ\\WikiMap_kz.db", "d:\\Projects\\SAS.Planet.Release.181221\\cache\\WikiMap", WikiMap);
    //importer.importMapFromSASFiles("c:\\Map_KZ\\genstab_kz.db", "d:\\Projects\\SAS.Planet.Release.181221\\cache\\topomapper.com", JointStaffMap);
    EnterProcStart("ApplicationSettingsEditor::InitWidgets");

    //Dialog Form
    this->setWindowTitle(tr("Settings"));

    CommonWidgetUtils::updateWidgetGeometry(this, 850);

    ApplicationSettings& applicationSettings = ApplicationSettings::Instance();

    _mapSettingsEditor = new MapSettingsEditor(this);
    _interfaceSettingsEditor =  new InterfaceSettingsEditor(this);
    _markersSettingsEditor = new MarkersSettingsEditor(this);
    _sessionsSettingsEditor = new SessionsSettingsEditor(this);
    _ballisticSettingsEditor = applicationSettings.isBombingTabLicensed() ? new BallisticSettingsEditor(this) : nullptr;
    auto statisticsTab = applicationSettings.isStatisticViewLicensed() ? new ApplicationStatisticView(this) : nullptr;
    auto cameraTab = new CameraListSettingsEditor(this);
    _hidSettingsEditor = new HIDSettingsEditor(this);
    connect(_hidController, &HIDController::onJoystickStateTextChanged, _hidSettingsEditor, &HIDSettingsEditor::onJoystickStateTextChanged);

    //Dialog controls
    auto tabWidget = new QTabWidget(this);
    tabWidget->tabBar()->setObjectName("ApplicationSettingsTab"); //used in QSS
    tabWidget->addTab(_mapSettingsEditor,           tr("Maps"));
    tabWidget->addTab(_markersSettingsEditor,       tr("Markers"));
    tabWidget->addTab(_interfaceSettingsEditor,     tr("Interface"));
    tabWidget->addTab(_hidSettingsEditor,           tr("HID"));
    tabWidget->addTab(_sessionsSettingsEditor,      tr("Sessions"));
    tabWidget->addTab(cameraTab,                    tr("Camera"));
    if (_ballisticSettingsEditor != nullptr)
        tabWidget->addTab(_ballisticSettingsEditor, tr("Ballistic"));
    if (statisticsTab != nullptr)
        tabWidget->addTab(statisticsTab,            tr("Statistics"));

    auto buttonBox = CommonWidgetUtils::makeDialogButtonBox(this, QDialogButtonBox::Save | QDialogButtonBox::Cancel);

    auto mainLayout = CommonWidgetUtils::createVBoxLayout(this, 10);
    mainLayout->addWidget(tabWidget);
    mainLayout->addWidget(buttonBox);
}

void ApplicationSettingsEditor::loadSettings()
{
    EnterProcStart("ApplicationSettingsEditor::LoadSettings");

    _association.toEditor();

    _hidSettingsEditor->loadSettings();
    _sessionsSettingsEditor->loadSettings();
    _interfaceSettingsEditor->loadSettings();
    _markersSettingsEditor->loadSettings();
    _mapSettingsEditor->loadSettings();
    if (_ballisticSettingsEditor != nullptr)
        _ballisticSettingsEditor->loadSettings();
}

void ApplicationSettingsEditor::saveSettings()
{
    EnterProcStart("ApplicationSettingsEditor::SaveSettings");

    ApplicationSettings& applicationSettings = ApplicationSettings::Instance();

    _association.fromEditor();

    _hidSettingsEditor->saveSettings();
    _sessionsSettingsEditor->saveSettings();
    _interfaceSettingsEditor->saveSettings();
    _markersSettingsEditor->saveSettings();
    _mapSettingsEditor->saveSettings();
    if (_ballisticSettingsEditor != nullptr)
        _ballisticSettingsEditor->saveSettings();

    applicationSettings.savePreferences();
}

void ApplicationSettingsEditor::accept()
{
    EnterProcStart("ApplicationSettingsEditor::accept");

    ApplicationSettings& applicationSettings = ApplicationSettings::Instance();

    done(QDialog::Accepted);
    bool needExit = !(applicationSettings.AskAboutQuitOnSaveSettings);
    if (!needExit)
        needExit = CommonWidgetUtils::showConfirmDialog(tr("The chosen settings will be used after restart. Do you want to close the program?"), false);
    if (needExit)
    {
        saveSettings();
        qApp->exit(ApplicationRestartExitCode);
    }
}
