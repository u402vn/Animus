#include "InterfaceSettingsEditor.h"
#include <QApplication>
#include <QCheckBox>
#include <QLineEdit>
#include <QGridLayout>
#include <QDir>
#include "ApplicationSettings.h"
#include "Common/CommonWidgets.h"
#include "EnterProc.h"


const int INTERFACE_TAB_FIRST_COLUMN_WIDTH = 200;

QGridLayout *makeInterfaceSpoilerGridLayout()
{
    auto layout = new QGridLayout();
    layout->setColumnMinimumWidth(0, INTERFACE_TAB_FIRST_COLUMN_WIDTH);
    layout->setHorizontalSpacing(4);
    return layout;
}

InterfaceSettingsEditor::InterfaceSettingsEditor(QWidget *parent):
    QScrollArea(parent),
    _association(this)
{
    EnterProcStart("InterfaceSettingsEditor::InterfaceSettingsEditor");
    ApplicationSettings& applicationSettings = ApplicationSettings::Instance();

    auto interfaceLayout = CommonWidgetUtils::createGridLayoutForScrollArea(this);

    // Init Common Section

    auto lblApplicationFile = new QLabel(tr("Application File"), this);
    auto edtApplicationFile = new QLineEdit(QDir::toNativeSeparators(qApp->applicationFilePath()), this);
    edtApplicationFile->setReadOnly(true);

    auto lblSettingsFile = new QLabel(tr("Settings File"), this);
    auto edtSettingsFile = new QLineEdit(QDir::toNativeSeparators(applicationSettings.getSettingsFileName()), this);
    edtSettingsFile->setReadOnly(true);

    auto lblHelpFile = new QLabel(tr("Help File"), this);
    auto fpsHelpFile = new FilePathSelector(this, "", tr("Select Help File"), tr("HTML Files (*.html)"));

    auto lblAppLanguage = new QLabel(tr("Interface Language"), this);

    const QMap <int, QString> mapAppLanguage {
        { ApplicationLanguages::English,    tr("English") },
        { ApplicationLanguages::Russian,    tr("Russian") },
        { ApplicationLanguages::Belarusian, tr("Belarusian") },
        { ApplicationLanguages::Arabic,     tr("Arabic") }
    };
    auto cbAppLanguage = new QComboBoxExt(this, mapAppLanguage);

    auto lblAppStyle = new QLabel(tr("Application Style"), this);
    const QMap <int, QString> mapAppStyle {
        { DisplayModes::SingleDisplay, tr("Single Display") },
        { DisplayModes::Camera1Map2,   tr("Camera (1) & Map (2)") },
        { DisplayModes::Map1Camera2,   tr("Map (1) & Camera (2)") }
    };
    auto cbDisplayMode = new QComboBoxExt(this, mapAppStyle);

    auto chkMinimalisticDesign = new QCheckBox(tr("Minimalistic Design"), this);

    auto lblAskAboutQuit = new QLabel(tr("Ask About Quit"), this);
    auto chkAskAboutQuitOnSaveSettings = new QCheckBox(tr("On Save Settings"), this);
    auto chkAskAboutQuitOnCloseApp = new QCheckBox(tr("On Close Application"), this);

    auto chkUseLaserRangefinderForGroundLevelCalculation = applicationSettings.isLaserRangefinderLicensed() ?
                new QCheckBox(tr("Use Laser Rangefinder For Ground Level Calculation"), this) : nullptr;

    auto commonLayout = makeInterfaceSpoilerGridLayout();
    int rowIndex = 0;

    commonLayout->addWidget(lblApplicationFile,                      rowIndex, 0, 1, 1);
    commonLayout->addWidget(edtApplicationFile,                      rowIndex, 1, 1, 3);
    rowIndex++;

    commonLayout->addWidget(lblSettingsFile,                         rowIndex, 0, 1, 1);
    commonLayout->addWidget(edtSettingsFile,                         rowIndex, 1, 1, 3);
    rowIndex++;

    commonLayout->addWidget(lblHelpFile,                             rowIndex, 0, 1, 1);
    commonLayout->addWidget(fpsHelpFile,                             rowIndex, 1, 1, 3);
    rowIndex++;

    commonLayout->addWidget(lblAppLanguage,                          rowIndex, 0, 1, 1);
    commonLayout->addWidget(cbAppLanguage,                           rowIndex, 1, 1, 1);
    rowIndex++;

    commonLayout->addWidget(lblAppStyle,                             rowIndex, 0, 1, 1);
    commonLayout->addWidget(cbDisplayMode,                           rowIndex, 1, 1, 1);
    commonLayout->addWidget(chkMinimalisticDesign,                   rowIndex, 2, 1, 1);
    rowIndex++;

    commonLayout->addWidget(lblAskAboutQuit,                         rowIndex, 0, 1, 1);
    commonLayout->addWidget(chkAskAboutQuitOnCloseApp,               rowIndex, 1, 1, 1);
    commonLayout->addWidget(chkAskAboutQuitOnSaveSettings,           rowIndex, 2, 1, 1);
    rowIndex++;

    commonLayout->addWidget(CommonWidgetUtils::createSeparator(this), rowIndex, 0, 1, 4);
    rowIndex++;

    if (chkUseLaserRangefinderForGroundLevelCalculation != nullptr)
    {
        commonLayout->addWidget(chkUseLaserRangefinderForGroundLevelCalculation,  rowIndex, 0, 1, 4);
        rowIndex++;
    }

    auto spoilerCommon = new Spoiler(tr("Common"), this);
    spoilerCommon->setContentLayout(commonLayout);

    // Init Allowed Functions Section

    auto chkCamControlsTabAllowed   = new QCheckBox(tr("Camera"), this);
    chkCamControlsTabAllowed->setChecked(true);
    chkCamControlsTabAllowed->setEnabled(false);
    auto chkBombingTabAllowed       = applicationSettings.isBombingTabLicensed() ? new QCheckBox(tr("Bombing"), this) : nullptr;
    auto chkMarkersTabAllowed       = applicationSettings.isMarkersTabLicensed() ? new QCheckBox(tr("Markers"), this) : nullptr;
    auto chkToolsTabAllowed         = new QCheckBox(tr("Tools"), this);
    auto chkPatrolTabAllowed        = applicationSettings.isPatrolTabLicensed() ? new QCheckBox(tr("Patrol"), this) : nullptr;

    auto chkCamControlsTabCoordIndicatorAllowed = new QCheckBox(tr("Coord Indicator"), this);

    auto functionsLayout = makeInterfaceSpoilerGridLayout();
    rowIndex = 0;

    functionsLayout->addWidget(chkCamControlsTabAllowed,                rowIndex, 0, 1, 1);
    functionsLayout->addWidget(chkCamControlsTabCoordIndicatorAllowed,  rowIndex, 1, 1, 1);
    rowIndex++;

    if (chkMarkersTabAllowed != nullptr)
    {
        functionsLayout->addWidget(chkMarkersTabAllowed,                    rowIndex, 0, 1, 1);
        rowIndex++;
    }

    if (chkBombingTabAllowed != nullptr)
    {
        functionsLayout->addWidget(chkBombingTabAllowed,                    rowIndex, 0, 1, 1);
        rowIndex++;
    }

    functionsLayout->addWidget(chkToolsTabAllowed,                      rowIndex, 0, 1, 1);
    rowIndex++;

    if (chkPatrolTabAllowed != nullptr)
    {
        functionsLayout->addWidget(chkPatrolTabAllowed,                     rowIndex, 0, 1, 1);
        rowIndex++;
    }

    auto spoilerFunctions = new Spoiler(tr("Allowed Functions"), this);
    spoilerFunctions->setContentLayout(functionsLayout);

    // Init OSD Section

    auto lblOSDLineWidth = new QLabel(tr("OSD Line Width"), this);
    auto sbOSDLineWidth = CommonWidgetUtils::createRangeSpinbox(this, 1, 10);
    auto chkUseFixedOSDLineWidth = new QCheckBox(tr("Fixed Width"), this);

    auto lblOSDScreen = new QLabel(tr("OSD Information"), this);
    auto cbOSDScreenCenterMark = new QComboBoxExt(this);
    cbOSDScreenCenterMark->addItem(tr("Cross in Center"),  OSDScreenCenterMarks::SimpleCross);
    cbOSDScreenCenterMark->addItem(tr("Circle in Center"), OSDScreenCenterMarks::CircleCross);
    cbOSDScreenCenterMark->addItem(tr("Cross and Rulers"), OSDScreenCenterMarks::CrossAndRulers);

    auto scbOSDColor = new SelectColorButton(tr("Lines Color"), this);
    auto csbOSDCenterMarkColor = new SelectColorButton(tr("Mark Color"), this);

    auto lblOSDGimbalIndicatorSize = new QLabel(tr("Gimbal Indicator Size"), this);
    auto sbOSDGimbalIndicatorSize = CommonWidgetUtils::createRangeSpinbox(this, 20, 200);

    auto lblOSDTelemetryIndicatorFontSize = new QLabel(tr("Telemetry Indicator Font Size"), this);
    auto sbOSDTelemetryIndicatorFontSize = CommonWidgetUtils::createRangeSpinbox(this, 2, 40);

    auto osdLayout = makeInterfaceSpoilerGridLayout();
    rowIndex = 0;

    osdLayout->addWidget(lblOSDLineWidth,                         rowIndex, 0, 1, 1);
    osdLayout->addWidget(sbOSDLineWidth,                          rowIndex, 1, 1, 1);
    osdLayout->addWidget(chkUseFixedOSDLineWidth,                 rowIndex, 2, 1, 1);
    rowIndex++;

    osdLayout->addWidget(lblOSDScreen,                            rowIndex, 0, 1, 1);
    osdLayout->addWidget(cbOSDScreenCenterMark,                   rowIndex, 1, 1, 1);
    osdLayout->addWidget(scbOSDColor,                             rowIndex, 2, 1, 1);
    osdLayout->addWidget(csbOSDCenterMarkColor,                   rowIndex, 3, 1, 1);
    rowIndex++;

    osdLayout->addWidget(lblOSDGimbalIndicatorSize,               rowIndex, 0, 1, 1);
    osdLayout->addWidget(sbOSDGimbalIndicatorSize,                rowIndex, 1, 1, 1);
    rowIndex++;

    osdLayout->addWidget(lblOSDTelemetryIndicatorFontSize,        rowIndex, 0, 1, 1);
    osdLayout->addWidget(sbOSDTelemetryIndicatorFontSize,         rowIndex, 1, 1, 1);
    rowIndex++;

    auto spoilerOSD = new Spoiler(tr("OSD"), this);
    spoilerOSD->setContentLayout(osdLayout);

    // Init Map Section

    auto lblViewFieldLineWidth = new QLabel(tr("View Field Border"), this);
    auto sbViewFieldLineWidth = CommonWidgetUtils::createRangeSpinbox(this, 1, 10);
    auto scbViewFieldLineColor = new SelectColorButton(tr("Color"), this);

    auto lblTrajectoryPathLineWidth = new QLabel(tr("Trajectory Line"), this);
    auto sbTrajectoryPathLineWidth = CommonWidgetUtils::createRangeSpinbox(this, 1, 10);
    auto scbTrajectoryPathLineColor = new SelectColorButton(tr("Color"), this);

    auto lblVisiblePathPointsPixelDistance = new QLabel(tr("Path Points Distance (pixel)"), this);
    auto sbVisiblePathPointsPixelDistance = CommonWidgetUtils::createRangeSpinbox(this, 2, 300);

    auto mapLayout = makeInterfaceSpoilerGridLayout();
    rowIndex = 0;

    mapLayout->addWidget(lblViewFieldLineWidth,                    rowIndex, 0, 1, 1);
    mapLayout->addWidget(sbViewFieldLineWidth,                     rowIndex, 1, 1, 1);
    mapLayout->addWidget(scbViewFieldLineColor,                    rowIndex, 2, 1, 1);
    rowIndex++;

    mapLayout->addWidget(lblTrajectoryPathLineWidth,               rowIndex, 0, 1, 1);
    mapLayout->addWidget(sbTrajectoryPathLineWidth,                rowIndex, 1, 1, 1);
    mapLayout->addWidget(scbTrajectoryPathLineColor,               rowIndex, 2, 1, 1);
    rowIndex++;

    mapLayout->addWidget(lblVisiblePathPointsPixelDistance,        rowIndex, 0, 1, 1);
    mapLayout->addWidget(sbVisiblePathPointsPixelDistance,         rowIndex, 1, 1, 1);
    rowIndex++;

    auto spoilerMap = new Spoiler(tr("Map"), this);
    spoilerMap->setContentLayout(mapLayout);

    // Init Sound Section

    auto chkSoundEffectsAllowed         = new QCheckBox(tr("Sound Effects Allowed"), this);
    auto lblSoundLevel                  = new QLabel(tr("Sound Level"), this);
    auto sbSoundLevel                   = CommonWidgetUtils::createDoubleRangeSpinbox(this, 0.00, 1.00, 0.1, 2);
    sbSoundLevel->setMinimumWidth(100);

    auto soundLayout = makeInterfaceSpoilerGridLayout();
    rowIndex = 0;

    soundLayout->addWidget(chkSoundEffectsAllowed,                 rowIndex, 0, 1, 3);
    rowIndex++;

    soundLayout->addWidget(lblSoundLevel,                          rowIndex, 0, 1, 1);
    soundLayout->addWidget(sbSoundLevel,                           rowIndex, 1, 1, 1);
    rowIndex++;

    soundLayout->setColumnStretch(3, 1);

    auto spoilerSound = new Spoiler(tr("Sound"), this);
    spoilerSound->setContentLayout(soundLayout);

    // Fill main layout

    rowIndex = 0;

    interfaceLayout->addWidget(spoilerCommon, rowIndex, 0, 1, 4);
    rowIndex++;

    interfaceLayout->addWidget(spoilerFunctions, rowIndex, 0, 1, 4);
    rowIndex++;

    interfaceLayout->addWidget(spoilerOSD, rowIndex, 0, 1, 4);
    rowIndex++;

    interfaceLayout->addWidget(spoilerMap, rowIndex, 0, 1, 4);
    rowIndex++;

    interfaceLayout->addWidget(spoilerSound, rowIndex, 0, 1, 4);
    rowIndex++;

    interfaceLayout->setRowStretch(rowIndex++, 1);

    _association.addBinding(&applicationSettings.InterfaceLanguage,                              cbAppLanguage);
    _association.addBinding(&applicationSettings.DisplayMode,                                    cbDisplayMode);
    _association.addBinding(&applicationSettings.MinimalisticDesign,                             chkMinimalisticDesign);
    _association.addBinding(&applicationSettings.AskAboutQuitOnCloseApp,                         chkAskAboutQuitOnCloseApp);
    _association.addBinding(&applicationSettings.AskAboutQuitOnSaveSettings,                     chkAskAboutQuitOnSaveSettings);
    if (chkBombingTabAllowed != nullptr)
        _association.addBinding(&applicationSettings.BombingTabAllowed,                          chkBombingTabAllowed);
    if (chkMarkersTabAllowed != nullptr)
        _association.addBinding(&applicationSettings.MarkersTabAllowed,                          chkMarkersTabAllowed);
    _association.addBinding(&applicationSettings.ToolsTabAllowed,                                chkToolsTabAllowed);
    if (chkPatrolTabAllowed != nullptr)
        _association.addBinding(&applicationSettings.PatrolTabAllowed,                           chkPatrolTabAllowed);
    _association.addBinding(&applicationSettings.CamControlsTabCoordIndicatorAllowed,            chkCamControlsTabCoordIndicatorAllowed);
    _association.addBinding(&applicationSettings.UseFixedOSDLineWidth,                           chkUseFixedOSDLineWidth);
    _association.addBinding(&applicationSettings.HelpFilePath,                                   fpsHelpFile);
    _association.addBinding(&applicationSettings.OSDLineWidth,                                   sbOSDLineWidth);
    _association.addBinding(&applicationSettings.OSDScreenCenterMark,                            cbOSDScreenCenterMark);
    _association.addBinding(&applicationSettings.OSDScreenLinesColor,                            scbOSDColor);
    _association.addBinding(&applicationSettings.OSDScreenCenterMarkColor,                       csbOSDCenterMarkColor);
    _association.addBinding(&applicationSettings.OSDGimbalIndicatorSize,                         sbOSDGimbalIndicatorSize);
    _association.addBinding(&applicationSettings.OSDTelemetryIndicatorFontSize,                  sbOSDTelemetryIndicatorFontSize);
    _association.addBinding(&applicationSettings.ViewFieldLineWidth,                             sbViewFieldLineWidth);
    _association.addBinding(&applicationSettings.ViewFieldLineColor,                             scbViewFieldLineColor);
    _association.addBinding(&applicationSettings.TrajectoryPathLineWidth,                        sbTrajectoryPathLineWidth);
    _association.addBinding(&applicationSettings.TrajectoryPathLineColor,                        scbTrajectoryPathLineColor);
    _association.addBinding(&applicationSettings.VisiblePathPointsPixelDistance,                 sbVisiblePathPointsPixelDistance);
    if (chkUseLaserRangefinderForGroundLevelCalculation != nullptr)
        _association.addBinding(&applicationSettings.UseLaserRangefinderForGroundLevelCalculation,   chkUseLaserRangefinderForGroundLevelCalculation);
    _association.addBinding(&applicationSettings.SoundEffectsAllowed,                            chkSoundEffectsAllowed);
    _association.addBinding(&applicationSettings.SoundLevel,                                     sbSoundLevel);
}

void InterfaceSettingsEditor::loadSettings()
{
    EnterProcStart("InterfaceSettingsEditor::loadSettings");
    _association.toEditor();
}

void InterfaceSettingsEditor::saveSettings()
{
    EnterProcStart("InterfaceSettingsEditor::saveSettings");
    _association.fromEditor();
}
