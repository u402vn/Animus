#include "HIDSettingsEditor.h"
#include "EnterProc.h"

void HIDSettingsEditor::addHIDButtonBinding(HIDButton prefIndex, int &gridRow)
{
    ApplicationSettings& applicationSettings = ApplicationSettings::Instance();

    auto joystickPref = applicationSettings.hidJoystickPref(prefIndex);
    auto keyboardPref = applicationSettings.hidKeyboardPref(prefIndex);
    auto caption = applicationSettings.hidCaption(prefIndex);

    auto lblTitlePref = new QLabel(caption, this);
    _keyMappingLayout->addWidget(lblTitlePref, gridRow, 0, 1, 1);

    if (joystickPref != nullptr)
    {
        auto cbJButtonPref = CommonWidgetUtils::createJButtonComboBox(this);
        _keyMappingLayout->addWidget(cbJButtonPref, gridRow, 1, 1, 1);
        _association.addBinding(joystickPref, cbJButtonPref, _uniqueJButtonsGroup);
    }

    if (keyboardPref != nullptr)
    {
        auto edtKeyboardPref = new KeySequenceEditExt(this);
        _keyMappingLayout->addWidget(edtKeyboardPref, gridRow, 2, 1, 1);
        _association.addBinding(keyboardPref, edtKeyboardPref, _uniqueKeyboardGroup);
    }

    gridRow++;
}

QLabel *HIDSettingsEditor::createCommentLabel(const QString &text)
{
    auto label = new QLabel(text, this);
    label->setObjectName("HIDSettingsEditorComment"); //used for stylesheet
    return label;
}

HIDSettingsEditor::HIDSettingsEditor(QWidget *parent) :
    QScrollArea(parent),
    _association(this)
{
    EnterProcStart("HIDSettingsEditor::HIDSettingsEditor");

    ApplicationSettings& applicationSettings = ApplicationSettings::Instance();

    _uniqueJButtonsGroup = new UniquePreferenceGroup(this);
    _uniqueKeyboardGroup = new UniquePreferenceGroup(this);

    auto lblControl = new QLabel(tr("Control"), this);
    auto chkKeyboardUsing  = new QCheckBox(tr("Use Keyboard"), this);
    auto chkJoystickUsing = new QCheckBox(tr("Use Joystick"), this);

    // Init Joysick State
    auto spoilerJoystickState = new SpoilerGrid(tr("Joystick State"), this);
    auto joystickStateLayout = spoilerJoystickState->gridLayout();

    int rowIndex = 0;

    _lblJoystickStateText = new QLabel(this);

    joystickStateLayout->addWidget(_lblJoystickStateText,                    rowIndex, 0, 1, 4);
    rowIndex++;

    // Init Joysick Section
    auto spoilerJoystickConfig = new SpoilerGrid(tr("Joystick Configuration"), this);
    auto joystickConfigLayout = spoilerJoystickConfig->gridLayout();

    auto lblJoystickMapping = new QLabel(tr("Joystick Configuration"), this);
    auto edtJoystickMapping = new QLineEdit(this);

    auto lblJAxisCameraX = new QLabel(tr("Joystick Axis for Camera (X)"), this);
    auto cbJAxisCameraX = CommonWidgetUtils::createJAxisComboBox(this);

    auto lblJAxisCameraY = new QLabel(tr("Joystick Axis for Camera (Y)"), this);
    auto cbJAxisCameraY = CommonWidgetUtils::createJAxisComboBox(this);

    auto lblJAxisZoom = new QLabel(tr("Joystick Axis for Zoom"), this);
    auto cbJAxisZoom = CommonWidgetUtils::createJAxisComboBox(this);

    auto lblJAxisCursorX = new QLabel(tr("Joystick Axis for Cursor (X)"), this);
    auto cbJAxisCursorX = CommonWidgetUtils::createJAxisComboBox(this);

    auto lblJAxisCursorY = new QLabel(tr("Joystick Axis for Cursor (Y)"), this);
    auto cbJAxisCursorY = CommonWidgetUtils::createJAxisComboBox(this);

    auto lblJoystickEmulationFromKeyboard = new QLabel(tr("Joystick Emulation from Keyboard"), this);
    auto sbJoystickEmulationFromKeyboard = CommonWidgetUtils::createDoubleRangeSpinbox(this, 0.010, 1.000, 0.001, 3);
    auto lblJoystickEmulationFromKeyboardInfo = createCommentLabel(tr("(equivalent to the joystick position for the pressed key [0..1])"));

    auto lblJoystickAxisMultiplier = new QLabel(tr("Joystick Axis Multiplier"), this);
    auto sbJoystickAxisMultiplier = CommonWidgetUtils::createDoubleRangeSpinbox(this, 1, 360, 0.1, 1);
    auto lblJoystickAxisMultiplierInfo = createCommentLabel(tr("(conversion coefficient of joystick deflection to angular velocity)"));

    auto lblJoystickAxisInsensitivity = new QLabel(tr("Joystick Axis Insensitivity"), this);
    auto sbJoystickAxisInsensitivity = CommonWidgetUtils::createDoubleRangeSpinbox(this, 0.000, 1.000, 0.001, 3);
    auto lblJoystickAxisInsensitivityInfo = createCommentLabel(tr("(joystick insensitivity in zero position [0..1])"));

    auto chkUseZoomScaleForManualMoving  = new QCheckBox(tr("Use Zoom Scale for Manual Moving"), this);

    rowIndex = 0;

    joystickConfigLayout->addWidget(lblJoystickMapping,                       rowIndex, 0, 1, 1);
    joystickConfigLayout->addWidget(edtJoystickMapping,                       rowIndex, 1, 1, 3);
    rowIndex++;

    joystickConfigLayout->addWidget(CommonWidgetUtils::createSeparator(this), rowIndex, 0, 1, 4);
    rowIndex++;

    joystickConfigLayout->addWidget(lblJAxisCameraX,                          rowIndex, 0, 1, 1);
    joystickConfigLayout->addWidget(cbJAxisCameraX,                           rowIndex, 1, 1, 1);
    rowIndex++;

    joystickConfigLayout->addWidget(lblJAxisCameraY,                          rowIndex, 0, 1, 1);
    joystickConfigLayout->addWidget(cbJAxisCameraY,                           rowIndex, 1, 1, 1);
    rowIndex++;

    joystickConfigLayout->addWidget(lblJAxisZoom,                             rowIndex, 0, 1, 1);
    joystickConfigLayout->addWidget(cbJAxisZoom,                              rowIndex, 1, 1, 1);
    rowIndex++;

    joystickConfigLayout->addWidget(lblJoystickEmulationFromKeyboard,         rowIndex, 0, 1, 1);
    joystickConfigLayout->addWidget(sbJoystickEmulationFromKeyboard,          rowIndex, 1, 1, 1);
    rowIndex++;

    joystickConfigLayout->addWidget(lblJoystickEmulationFromKeyboardInfo,     rowIndex, 1, 1, 3);
    rowIndex++;

    joystickConfigLayout->addWidget(lblJoystickAxisMultiplier,                rowIndex, 0, 1, 1);
    joystickConfigLayout->addWidget(sbJoystickAxisMultiplier,                 rowIndex, 1, 1, 1);
    rowIndex++;

    joystickConfigLayout->addWidget(lblJoystickAxisMultiplierInfo,            rowIndex, 1, 1, 3);
    rowIndex++;

    joystickConfigLayout->addWidget(chkUseZoomScaleForManualMoving,           rowIndex, 1, 1, 3);
    rowIndex++;

    joystickConfigLayout->addWidget(lblJoystickAxisInsensitivity,             rowIndex, 0, 1, 1);
    joystickConfigLayout->addWidget(sbJoystickAxisInsensitivity,              rowIndex, 1, 1, 1);
    rowIndex++;

    joystickConfigLayout->addWidget(lblJoystickAxisInsensitivityInfo,         rowIndex, 1, 1, 3);
    rowIndex++;

    joystickConfigLayout->addWidget(CommonWidgetUtils::createSeparator(this), rowIndex, 0, 1, 4);
    rowIndex++;

    joystickConfigLayout->addWidget(lblJAxisCursorX,                         rowIndex, 0, 1, 1);
    joystickConfigLayout->addWidget(cbJAxisCursorX,                          rowIndex, 1, 1, 1);
    rowIndex++;

    joystickConfigLayout->addWidget(lblJAxisCursorY,                         rowIndex, 0, 1, 1);
    joystickConfigLayout->addWidget(cbJAxisCursorY,                          rowIndex, 1, 1, 1);
    rowIndex++;

    // Init Key Mapping section


    auto spoilerKeyMapping = new SpoilerGrid(tr("Key Mapping"), this);
    _keyMappingLayout = spoilerKeyMapping->gridLayout();

    _keyMappingLayout->setColumnStretch(0, 1);
    _keyMappingLayout->setColumnStretch(1, 0);
    _keyMappingLayout->setColumnStretch(2, 0);

    rowIndex = 0;

    addHIDButtonBinding(hidbtnNewSession,                               rowIndex);
    addHIDButtonBinding(hidbtnDisplayOnly,                              rowIndex);
    addHIDButtonBinding(hidbtnSelectSessions,                           rowIndex);
    addHIDButtonBinding(hidbtnChangeVideo2Map,                          rowIndex);

    _keyMappingLayout->addWidget(CommonWidgetUtils::createSeparator(this), rowIndex, 0, 1, 4);
    rowIndex++;

    addHIDButtonBinding(hidbtnMapZoomOut,                               rowIndex);
    addHIDButtonBinding(hidbtnMapZoomIn,                                rowIndex);
    addHIDButtonBinding(hidbtnFollowThePlane,                           rowIndex);

    _keyMappingLayout->addWidget(CommonWidgetUtils::createSeparator(this), rowIndex, 0, 1, 4);
    rowIndex++;

    addHIDButtonBinding(hidbtnChangeActiveCam,                          rowIndex);
    addHIDButtonBinding(hidbtnEnableSoftwareStab,              rowIndex);
    addHIDButtonBinding(hidbtnDriversOff,                               rowIndex);
    addHIDButtonBinding(hidbtnFixedPosLanding,                          rowIndex);
    addHIDButtonBinding(hidbtnFixedPosBegining,                         rowIndex);
    addHIDButtonBinding(hidbtnFixedPosVertical,                         rowIndex);
    addHIDButtonBinding(hidbtnCamZoomIn,                                rowIndex);
    addHIDButtonBinding(hidbtnCamZoomOut,                               rowIndex);
    addHIDButtonBinding(hidbtnColorModeUp,                              rowIndex);
    addHIDButtonBinding(hidbtnColorModeDown,                            rowIndex);
    addHIDButtonBinding(hidbtnLaserActivation,                          rowIndex);

    _keyMappingLayout->addWidget(CommonWidgetUtils::createSeparator(this), rowIndex, 0, 1, 4);
    rowIndex++;

    if (applicationSettings.isBombingTabLicensed())
        addHIDButtonBinding(hidbtnBombingSight,                         rowIndex);
    if (applicationSettings.isPhotographyLicensed())
    {
        addHIDButtonBinding(hidbtnScreenshot,                           rowIndex);
        addHIDButtonBinding(hidbtnSnapshot,                             rowIndex);
        addHIDButtonBinding(hidbtnSnapshotSeries,                       rowIndex);
    }
    if (applicationSettings.isBombingTabLicensed() || applicationSettings.isPhotographyLicensed())
    {
        _keyMappingLayout->addWidget(CommonWidgetUtils::createSeparator(this), rowIndex, 0, 1, 4);
        rowIndex++;
    }

    addHIDButtonBinding(hidbtnTargetUnlock,                             rowIndex);
    addHIDButtonBinding(hidbtnTargetLockInCursor,                       rowIndex);
    addHIDButtonBinding(hidbtnMagnifier,                                rowIndex);

    if (applicationSettings.isPhotographyLicensed())
        addHIDButtonBinding(hidbtnCamRecording,                         rowIndex);
    addHIDButtonBinding(hidbtnAutomaticTracer,                          rowIndex);
    if (applicationSettings.isBombingTabLicensed())
    {
        addHIDButtonBinding(hidbtnDropBomb05,                           rowIndex);
        addHIDButtonBinding(hidbtnDropBomb10,                           rowIndex);
    }

    addHIDButtonBinding(hidbtnSendHitCoordinates,                       rowIndex);
    addHIDButtonBinding(hidbtnSendWeather,                              rowIndex);

    _keyMappingLayout->addWidget(CommonWidgetUtils::createSeparator(this), rowIndex, 0, 1, 4);
    rowIndex++;

    addHIDButtonBinding(hidbtnNewMarkerForTarget,                       rowIndex);
    if (applicationSettings.isLaserRangefinderLicensed())
        addHIDButtonBinding(hidbtnNewMarkerForLaser,                    rowIndex);
    addHIDButtonBinding(hidbtnNewMarkerForUAV,                          rowIndex);

    _keyMappingLayout->addWidget(CommonWidgetUtils::createSeparator(this), rowIndex, 0, 1, 4);
    rowIndex++;

    addHIDButtonBinding(hidbtnNormalFlight,                             rowIndex);
    addHIDButtonBinding(hidbtnPatrolMovingTargetMode,                   rowIndex);
    addHIDButtonBinding(hidbtnPatrolStaticTargetMode,                   rowIndex);
    addHIDButtonBinding(hidbtnManualFlightMode,                         rowIndex);

    // Fill main layout

    auto hidGridLayout = CommonWidgetUtils::createGridLayoutForScrollArea(this);

    hidGridLayout->addWidget(lblControl,                           rowIndex, 0, 1, 1);
    hidGridLayout->addWidget(chkJoystickUsing,                     rowIndex, 1, 1, 1);
    hidGridLayout->addWidget(chkKeyboardUsing,                     rowIndex, 2, 1, 1);
    rowIndex++;

    hidGridLayout->addWidget(CommonWidgetUtils::createSeparator(this), rowIndex, 0, 1, 4);
    rowIndex++;

    hidGridLayout->addWidget(spoilerJoystickState,                rowIndex, 0, 1, 4);
    rowIndex++;

    hidGridLayout->addWidget(spoilerJoystickConfig,                rowIndex, 0, 1, 4);
    rowIndex++;

    hidGridLayout->addWidget(spoilerKeyMapping,                    rowIndex, 0, 1, 4);
    rowIndex++;

    hidGridLayout->setRowStretch(rowIndex++, 1);

    // Associations
    _association.addBinding(&applicationSettings.KeyboardUsing,                     chkKeyboardUsing);
    _association.addBinding(&applicationSettings.JoystickUsing,                     chkJoystickUsing);
    _association.addBinding(&applicationSettings.JoystickMapping,                   edtJoystickMapping);

    _association.addBinding(&applicationSettings.JoystickAxisCameraXIndex,          cbJAxisCameraX);
    _association.addBinding(&applicationSettings.JoystickAxisCameraYIndex,          cbJAxisCameraY);
    _association.addBinding(&applicationSettings.JoystickAxisZoomIndex,             cbJAxisZoom);
    _association.addBinding(&applicationSettings.JoystickAxisCursorXIndex,         cbJAxisCursorX);
    _association.addBinding(&applicationSettings.JoystickAxisCursorYIndex,         cbJAxisCursorY);

    _association.addBinding(&applicationSettings.JoystickCameraEmulationFromKeyboard,     sbJoystickEmulationFromKeyboard);
    _association.addBinding(&applicationSettings.JoystickCameraAxisMultiplier,            sbJoystickAxisMultiplier);
    _association.addBinding(&applicationSettings.JoystickCameraAxisInsensitivity,         sbJoystickAxisInsensitivity);
    _association.addBinding(&applicationSettings.UseZoomScaleForManualMoving,       chkUseZoomScaleForManualMoving);
}

void HIDSettingsEditor::loadSettings()
{
    EnterProcStart("HIDSettingsEditor::loadSettings");
    _association.toEditor();
}

void HIDSettingsEditor::saveSettings()
{
    EnterProcStart("HIDSettingsEditor::saveSettings");
    _association.fromEditor();
}

void HIDSettingsEditor::onJoystickStateTextChanged(const QString stateText)
{
    _lblJoystickStateText->setText(stateText);
}
