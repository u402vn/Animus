#include "CameraSettingsEditor.h"
#include <QGridLayout>
#include <QRadioButton>
#include <QCameraInfo>
#include <QLineEdit>
#include <QComboBox>
#include <QDialogButtonBox>
#include "EnterProc.h"
#include "UserInterface/ApplicationSettingsEditor/CameraZoomSettingsEditor.h"
#include "ConstantNames.h"

QComboBoxExt *CameraSettingsEditor::createCamListCombo(QWidget *parent)
{
    ApplicationSettings& applicationSettings = ApplicationSettings::Instance();
    auto cbCameras = new QComboBoxExt(parent);
    for (int i = 0; i < 10; i++)
    {
        auto cameraSettings = applicationSettings.cameraPreferences(i);
        QString description = cameraSettings->UserDescription;
        description = description.isEmpty() ? description : " - " + description;
        cbCameras->addItem(tr("Camera %1 %2").arg(i).arg(description), i);
    }
    cbCameras->setCurrentData(applicationSettings.InstalledCameraIndex.value());
    return cbCameras;
}

CameraSettingsEditor::CameraSettingsEditor(QWidget *parent, const qint32 camIdx) :
    QDialog(parent),
    _association(this),
    _camIdx(camIdx)
{
    EnterProcStart("CameraSettingsEditor::CameraSettingsEditor");

    ApplicationSettings& applicationSettings = ApplicationSettings::Instance();
    _isBombingTabLicensed = applicationSettings.isBombingTabLicensed();
    _isPhotographyLicensed = applicationSettings.isPhotographyLicensed();

    initWidgets();
    initBindings();
    loadSettings();
}

void CameraSettingsEditor::accept()
{
    saveSettings();
    emit onCamInfoUpdated();
    done(QDialog::Accepted);
}

void CameraSettingsEditor::initWidgets()
{
    this->setWindowTitle(tr("Video Source # %1").arg(_camIdx));
    this->setModal(true);
    CommonWidgetUtils::updateWidgetGeometry(this, 900);
    this->setAttribute(Qt::WA_DeleteOnClose, true);

    auto scrolledWidget = CommonWidgetUtils::createScrolledWidget(this);

    auto buttonBox = CommonWidgetUtils::makeDialogButtonBox(this, QDialogButtonBox::Ok | QDialogButtonBox::Cancel);

    auto mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(qobject_cast<QWidget*>(scrolledWidget->parent()->parent()));
    mainLayout->addWidget(buttonBox);
    mainLayout->setStretch(0, 1);
    mainLayout->setStretch(1, 0);

    auto lblCamDescription = new QLabel(tr("Description"), this);
    _edtCamDescription = new QLineEdit(this);

    auto btnEditOpticalDeviceASettings = new QPushButton(tr("Optical Device A"), this);
    connect(btnEditOpticalDeviceASettings, &QPushButton::clicked, this, &CameraSettingsEditor::onEditOpticalDeviceASettingsClicked);

    auto btnEditOpticalDeviceBSettings = new QPushButton(tr("Optical Device B"), this);
    connect(btnEditOpticalDeviceBSettings, &QPushButton::clicked, this, &CameraSettingsEditor::onEditOpticalDeviceBSettingsClicked);

    auto btnEditOpticalDeviceCSettings = new QPushButton(tr("Optical Device C"), this);
    connect(btnEditOpticalDeviceCSettings, &QPushButton::clicked, this, &CameraSettingsEditor::onEditOpticalDeviceCSettingsClicked);

    _cbOpticalDevicesCount = new QComboBoxExt(this, ConstantNames::OpticalDevicesCountCaptions());
    connect(_cbOpticalDevicesCount, static_cast<void(QComboBoxExt::*)(int)>(&QComboBoxExt::activated), this,  [=](int index)
    {
        Q_UNUSED(index)

        auto numbers = _cbOpticalDevicesCount->currentData().toInt();

        btnEditOpticalDeviceBSettings->setVisible(numbers > 1);
        btnEditOpticalDeviceCSettings->setVisible(numbers > 2);
    });


    auto gimbalWidgets = createGimbalWidgets();
    auto connectionWidgets1 = createConnectionWidgets(1);
    auto connectionWidgets2 = createConnectionWidgets(2);
    auto functionsWidgets = createFunctionsWidgets();

    auto camControlsGrid = new QGridLayout(scrolledWidget);

    int row = 0;

    camControlsGrid->addWidget(lblCamDescription,                row, 0, 1, 1);
    camControlsGrid->addWidget(_edtCamDescription,               row, 1, 1, 3);
    row++;

    camControlsGrid->addWidget(_cbOpticalDevicesCount,                 row, 0, 1, 1);
    camControlsGrid->addWidget(btnEditOpticalDeviceASettings,           row, 1, 1, 1);
    camControlsGrid->addWidget(btnEditOpticalDeviceBSettings,           row, 2, 1, 1);
    camControlsGrid->addWidget(btnEditOpticalDeviceCSettings,           row, 3, 1, 1);
    row++;

    camControlsGrid->addWidget(gimbalWidgets,                    row++, 0, 1, 4);
    camControlsGrid->addWidget(connectionWidgets1,               row++, 0, 1, 4);
    camControlsGrid->addWidget(connectionWidgets2,               row++, 0, 1, 4);
    if (functionsWidgets != nullptr)
        camControlsGrid->addWidget(functionsWidgets,             row++, 0, 1, 4);
    camControlsGrid->setRowStretch(row++, 1);
}

void CameraSettingsEditor::addSeparatorRow(QGridLayout *camControlsGrid, int &row)
{
    camControlsGrid->addWidget(CommonWidgetUtils::createSeparator(this),   row, 0, 1, 5);
    camControlsGrid->setRowStretch(row, 0);
    row++;
}

QWidget *CameraSettingsEditor::createGimbalWidgets()
{
    auto spoilerGimbal = new SpoilerGrid(tr("Gimbal"), this);
    auto gimbalLayout = spoilerGimbal->gridLayout();

    _gbCameraType = new QButtonGroupExt(this);
    auto rbFixedCamera    = _gbCameraType->appendButton(tr("Fixed Camera"),    CameraSuspensionTypes::FixedCamera);
    auto rbRotatingCamera = _gbCameraType->appendButton(tr("Rotating Camera"), CameraSuspensionTypes::RotatingCamera);

    //Fixed Camera
    auto lblFixedCamYaw = new QLabel(tr("Yaw"), this);
    auto lblFixedCamPitch = new QLabel(tr("Pitch"), this);
    auto lblFixedCamRoll =  new QLabel(tr("Roll"), this);

    _sbFixedCamPitch = CommonWidgetUtils::createRangeSpinbox(this, -180, +180);
    _sbFixedCamRoll  = CommonWidgetUtils::createRangeSpinbox(this, -180, +180);

    // Main Controls
    _cbCameraControlMode = new QComboBoxExt(this, ConstantNames::CameraControlModeCaptions());

    auto lblCamHeaderMin = new QLabel(tr("Minimum Value"), this);
    _sbCamPitchMin = CommonWidgetUtils::createRangeSpinbox(this, -720, +720);
    _sbCamRollMin = CommonWidgetUtils::createRangeSpinbox(this, -720, +720);

    auto lblCamHeaderMax = new QLabel(tr("Maximum Value"), this);
    _sbCamPitchMax = CommonWidgetUtils::createRangeSpinbox(this, -720, +720);
    _sbCamRollMax = CommonWidgetUtils::createRangeSpinbox(this, -720, +720);

    _chkCamAxisXInverse = new QCheckBox(tr("Inverse Axis X"), this);
    _chkCamAxisYInverse = new QCheckBox(tr("Inverse Axis Y"), this);
    _lblEncoderAutomaticTracerMultiplie = new QLabel(tr("Tracker Speed Multiplier"), this);
    _sbEncoderAutomaticTracerMultiplier = CommonWidgetUtils::createDoubleRangeSpinbox(this, 0.01, 1000, 0.01, 2);

    // Fast positioning controls
    auto lblFixedPosLanding = new QLabel(tr("Landing Position"), this);
    _sbFixedPosLandingYaw = CommonWidgetUtils::createRangeSpinbox(this, -720, +720);
    _sbFixedPosLandingPitch = CommonWidgetUtils::createRangeSpinbox(this, -720, +720);
    _sbFixedPosLandingRoll  = CommonWidgetUtils::createRangeSpinbox(this, -720, +720);
    _chkFixedPosLandingCommand = new QCheckBox(tr("Special Command for Landing Position"), this);

    auto lblFixedPosBegining = new QLabel(tr("Begining Position"), this);
    _sbFixedPosBeginingYaw = CommonWidgetUtils::createRangeSpinbox(this, -720, +720);
    _sbFixedPosBeginingPitch = CommonWidgetUtils::createRangeSpinbox(this, -720, +720);
    _sbFixedPosBeginingRoll  = CommonWidgetUtils::createRangeSpinbox(this, -720, +720);

    auto lblFixedPosVertical = new QLabel(tr("Vertical Position"), this);
    _sbFixedPosVerticalYaw = CommonWidgetUtils::createRangeSpinbox(this, -720, +720);
    _sbFixedPosVerticalPitch = CommonWidgetUtils::createRangeSpinbox(this, -720, +720);
    _sbFixedPosVerticalRoll  = CommonWidgetUtils::createRangeSpinbox(this, -720, +720);

    int row = 0;

    gimbalLayout->addWidget(rbFixedCamera,                   row, 0, 1, 4);
    gimbalLayout->addWidget(lblFixedCamYaw,                  row, 2, 1, 1);
    gimbalLayout->addWidget(lblFixedCamPitch,                row, 3, 1, 1);
    gimbalLayout->addWidget(lblFixedCamRoll,                 row, 4, 1, 1);
    gimbalLayout->setRowStretch(row, 0);
    row++;

    gimbalLayout->addWidget(_sbFixedCamPitch,                row, 3, 1, 1);
    gimbalLayout->addWidget(_sbFixedCamRoll,                 row, 4, 1, 1);
    gimbalLayout->setRowStretch(row, 0);
    row++;

    gimbalLayout->addWidget(rbRotatingCamera,                row, 0, 1, 2);
    gimbalLayout->addWidget(_cbCameraControlMode,            row, 2, 1, 3);
    gimbalLayout->setRowStretch(row, 0);
    row++;

    gimbalLayout->addWidget(lblCamHeaderMin,                 row, 1, 1, 1);
    gimbalLayout->addWidget(_sbCamPitchMin,                  row, 3, 1, 1);
    gimbalLayout->addWidget(_sbCamRollMin,                   row, 4, 1, 1);
    gimbalLayout->setRowStretch(row, 0);
    row++;

    gimbalLayout->addWidget(lblCamHeaderMax,                 row, 1, 1, 1);
    gimbalLayout->addWidget(_sbCamPitchMax,                  row, 3, 1, 1);
    gimbalLayout->addWidget(_sbCamRollMax,                   row, 4, 1, 1);
    gimbalLayout->setRowStretch(row, 0);
    row++;

    gimbalLayout->addWidget(lblFixedPosLanding,              row, 1, 1, 1);
    gimbalLayout->addWidget(_sbFixedPosLandingYaw,           row, 2, 1, 1);
    gimbalLayout->addWidget(_sbFixedPosLandingPitch,         row, 3, 1, 1);
    gimbalLayout->addWidget(_sbFixedPosLandingRoll,          row, 4, 1, 1);
    row++;

    gimbalLayout->addWidget(_chkFixedPosLandingCommand,      row, 2, 1, 4);
    row++;

    gimbalLayout->addWidget(lblFixedPosBegining,             row, 1, 1, 1);
    gimbalLayout->addWidget(_sbFixedPosBeginingYaw,          row, 2, 1, 1);
    gimbalLayout->addWidget(_sbFixedPosBeginingPitch,        row, 3, 1, 1);
    gimbalLayout->addWidget(_sbFixedPosBeginingRoll,         row, 4, 1, 1);
    gimbalLayout->setRowStretch(row, 0);
    row++;

    gimbalLayout->addWidget(lblFixedPosVertical,             row, 1, 1, 1);
    gimbalLayout->addWidget(_sbFixedPosVerticalYaw,          row, 2, 1, 1);
    gimbalLayout->addWidget(_sbFixedPosVerticalPitch,        row, 3, 1, 1);
    gimbalLayout->addWidget(_sbFixedPosVerticalRoll,         row, 4, 1, 1);
    gimbalLayout->setRowStretch(row, 0);
    row++;

    gimbalLayout->addWidget(_chkCamAxisYInverse,             row, 2, 1, 2);
    gimbalLayout->addWidget(_chkCamAxisXInverse,             row, 4, 1, 2);
    gimbalLayout->setRowStretch(row, 0);
    row++;

    gimbalLayout->addWidget(_lblEncoderAutomaticTracerMultiplie,       row, 1, 1, 1);
    gimbalLayout->addWidget(_sbEncoderAutomaticTracerMultiplier,       row, 2, 1, 4);
    gimbalLayout->setRowStretch(row, 0);
    row++;

    return spoilerGimbal;
}

const QString CameraSettingsEditor::getCameraInfo(qint32 camIdx)
{
    ApplicationSettings& applicationSettings = ApplicationSettings::Instance();
    auto info = applicationSettings.cameraPreferences(camIdx)->description();
    return info;
}

void CameraSettingsEditor::onEditOpticalDeviceASettingsClicked()
{
    auto cameraZoomSettingsEditor = new CameraZoomSettingsEditor(this, _camIdx, 1);
    //??? connect(cameraSettingsEditor, &CameraSettingsEditor::onCamInfoUpdated, this, &CameraListSettingsEditor::onCamInfoUpdated);
    cameraZoomSettingsEditor->showNormal();
}

void CameraSettingsEditor::onEditOpticalDeviceBSettingsClicked()
{
    auto cameraZoomSettingsEditor = new CameraZoomSettingsEditor(this, _camIdx, 2);
    //??? connect(cameraSettingsEditor, &CameraSettingsEditor::onCamInfoUpdated, this, &CameraListSettingsEditor::onCamInfoUpdated);
    cameraZoomSettingsEditor->showNormal();
}

void CameraSettingsEditor::onEditOpticalDeviceCSettingsClicked()
{
    auto cameraZoomSettingsEditor = new CameraZoomSettingsEditor(this, _camIdx, 3);
    //??? connect(cameraSettingsEditor, &CameraSettingsEditor::onCamInfoUpdated, this, &CameraListSettingsEditor::onCamInfoUpdated);
    cameraZoomSettingsEditor->showNormal();
}

QRadioButton *CameraSettingsEditor::addVideoSourceRadioButton(QButtonGroupExt *gbVideoSource, VideoFrameTrafficSources source)
{
    return gbVideoSource->appendButton(ConstantNames::VideoFrameTrafficSourceCaptions()[source], source);
}

QWidget *CameraSettingsEditor::createConnectionWidgets(int connectionId)
{
    ApplicationSettings& applicationSettings = ApplicationSettings::Instance();
    auto cameraPrefenences = applicationSettings.cameraPreferences(_camIdx);
    auto connectionSetting = cameraPrefenences->videoConnectionSetting(connectionId);

    auto spoilerConnection = new SpoilerGrid(tr("Connection %1").arg(connectionId), this);
    auto connectionLayout = spoilerConnection->gridLayout();

    auto gbVideoSource = new QButtonGroupExt(this);
    auto rbVideoSourceUSBCamera        = addVideoSourceRadioButton(gbVideoSource, VideoFrameTrafficSources::USBCamera);
    auto rbVideoSourceXPlane           = addVideoSourceRadioButton(gbVideoSource, VideoFrameTrafficSources::XPlane);
    auto rbVideoSourceYurion           = addVideoSourceRadioButton(gbVideoSource, VideoFrameTrafficSources::Yurion);
    auto rbVideoSourceImageFile        = addVideoSourceRadioButton(gbVideoSource, VideoFrameTrafficSources::CalibrationImage);
    auto rbVideoSourceVideoFile        = addVideoSourceRadioButton(gbVideoSource, VideoFrameTrafficSources::VideoFile);
    auto rbVideoSourceRTSP             = addVideoSourceRadioButton(gbVideoSource, VideoFrameTrafficSources::RTSP);
    auto rbVideoSourceNetworkCam       = addVideoSourceRadioButton(gbVideoSource, VideoFrameTrafficSources::MUSV2);

    connect(gbVideoSource, static_cast<void(QButtonGroup::*)(int)>(&QButtonGroup::buttonClicked), this, &CameraSettingsEditor::onVideoSourceSelected);

    auto cbUSBCamera = new QComboBoxExt(this);
    foreach (const QCameraInfo &cameraInfo, QCameraInfo::availableCameras())
        cbUSBCamera->addItem(cameraInfo.description(), cameraInfo.deviceName());

    auto naeXPlane = new NetworkAddressEditor(this, &_association,
                                              connectionSetting->VideoFrameSourceXPlaneAddress, connectionSetting->VideoFrameSourceXPlanePort);
    auto lblYurionUDPPort = new QLabel(tr("UDP Port:"), this);
    auto edYurionUDPPort = CommonWidgetUtils::createPortEditor(this);

    auto cbCalibrationImagePath = new QComboBoxExt(this);
    cbCalibrationImagePath->setEditable(true);
    cbCalibrationImagePath->addItem(DefaultCalibrationImagePath);
    cbCalibrationImagePath->addItem(":/CalibrationImages/Mira1.png");

    auto fpsVideoFile = new FilePathSelector(this, tr("Video File"), tr("Select Video File"), tr("Video Files (*.avi)"));
    fpsVideoFile->setLabelWidth(0);

    auto edRTSPUrl = new QLineEdit(this);

    auto lblMUSV2UDPPort = new QLabel(tr("UDP Port:"), this);
    auto edMUSV2UDPPort = CommonWidgetUtils::createPortEditor(this);

    int row = 0;
    connectionLayout->addWidget(rbVideoSourceUSBCamera,         row, 0, 1, 1);
    connectionLayout->addWidget(cbUSBCamera,                    row, 1, 1, 5);
    row++;

    connectionLayout->addWidget(rbVideoSourceXPlane,            row, 0, 1, 1);
    connectionLayout->addWidget(naeXPlane,                      row, 1, 1, 2);
    row++;

    connectionLayout->addWidget(rbVideoSourceYurion,            row, 0, 1, 1);
    connectionLayout->addWidget(lblYurionUDPPort,               row, 1, 1, 1);
    connectionLayout->addWidget(edYurionUDPPort,                row, 2, 1, 1);
    row++;

    connectionLayout->addWidget(rbVideoSourceImageFile,         row, 0, 1, 1);
    connectionLayout->addWidget(cbCalibrationImagePath,         row, 1, 1, 5);
    row++;

    connectionLayout->addWidget(rbVideoSourceVideoFile,         row, 0, 1, 1);
    connectionLayout->addWidget(fpsVideoFile,                   row, 1, 1, 5);
    row++;

    connectionLayout->addWidget(rbVideoSourceRTSP,              row, 0, 1, 1);
    connectionLayout->addWidget(edRTSPUrl,                      row, 1, 1, 5);
    row++;

    connectionLayout->addWidget(rbVideoSourceNetworkCam,        row, 0, 1, 1);
    connectionLayout->addWidget(lblMUSV2UDPPort,                row, 1, 1, 1);
    connectionLayout->addWidget(edMUSV2UDPPort,                 row, 2, 1, 1);
    row++;


    _association.addBinding(connectionSetting->VideoTrafficSource,                  gbVideoSource);
    _association.addBinding(connectionSetting->VideoFrameSourceCameraName,          cbUSBCamera);
    _association.addBinding(connectionSetting->VideoFrameSourceYurionUDPPort,       edYurionUDPPort);
    _association.addBinding(connectionSetting->CalibrationImagePath,                cbCalibrationImagePath);
    _association.addBinding(connectionSetting->VideoFilePath,                       fpsVideoFile);
    _association.addBinding(connectionSetting->RTSPUrl,                             edRTSPUrl);
    _association.addBinding(connectionSetting->VideoFrameSourceMUSV2UDPPort,        edMUSV2UDPPort);

    return spoilerConnection;
}

QWidget *CameraSettingsEditor::createFunctionsWidgets()
{
    auto spoilerFunctions = new SpoilerGrid(tr("Functions"), this);
    auto functionsLayout = spoilerFunctions->gridLayout();

    int row = 0;

    if (_isPhotographyLicensed)
    {
        auto lblFunctions = new QLabel(tr("Functions"), this);
        _chkOnboardRecording = new QCheckBox(tr("On Board Recording"), this);
        _chkSnapshot = new QCheckBox(tr("Snapshot"), this);

        functionsLayout->addWidget(lblFunctions,                     row, 0, 1, 1);
        functionsLayout->addWidget(_chkOnboardRecording,             row, 2, 1, 2);
        functionsLayout->addWidget(_chkSnapshot,                     row, 4, 1, 1);
        row++;
    }

    if (_isBombingTabLicensed)
    {
        auto lblSightNumbers = new QLabel(tr("Numbers on Sight"), this);
        functionsLayout->addWidget(lblSightNumbers,                  row, 0, 1, 5);

        static QRegExp sightNumbersRegexp("^(([0-9]+((\\.|\\,){0,1}[0-9]+){0,1}\\;)|\\;)+$");
        static QRegExpValidator sightNumbersValidator{sightNumbersRegexp};

        for (int i = 0; i < 5; i++)
        {
            SightNumberControls controls;
            controls.Height = CommonWidgetUtils::createRangeSpinbox(this, 0, 10000);
            controls.SightNumbers = new QLineEdit(this);
            controls.SightNumbers->setValidator(&sightNumbersValidator);
            controls.SightNumbers->setMaxLength(100);

            _sightControls.append(controls);

            functionsLayout->addWidget(controls.Height,                     row, 2, 1, 1);
            functionsLayout->addWidget(controls.SightNumbers,               row, 3, 1, 3);
            row++;
        }
    }

    return spoilerFunctions;
}

void CameraSettingsEditor::initBindings()
{
    ApplicationSettings& applicationSettings = ApplicationSettings::Instance();
    auto cameraSettings = applicationSettings.cameraPreferences(_camIdx);

    _association.addBinding(&(cameraSettings->UserDescription),                     _edtCamDescription);
    _association.addBinding(&(cameraSettings->PhisycalLensCount),                   _cbOpticalDevicesCount);


    if (_isPhotographyLicensed)
    {
        _association.addBinding(&(cameraSettings->IsOnboardRecording),              _chkOnboardRecording);
        _association.addBinding(&(cameraSettings->IsSnapshot),                      _chkSnapshot);
    }

    _association.addBinding(&(cameraSettings->CameraSuspensionType),                _gbCameraType);

    _association.addBinding(&(cameraSettings->FixedCamPitch),                       _sbFixedCamPitch);
    _association.addBinding(&(cameraSettings->FixedCamRoll),                        _sbFixedCamRoll);

    _association.addBinding(&(cameraSettings->CamPitchMin),                         _sbCamPitchMin);
    _association.addBinding(&(cameraSettings->CamPitchMax),                         _sbCamPitchMax);
    _association.addBinding(&(cameraSettings->CamAxisYInverse),                     _chkCamAxisYInverse);
    _association.addBinding(&(cameraSettings->EncoderAutomaticTracerMultiplier),     _sbEncoderAutomaticTracerMultiplier);
    _association.addBinding(&(cameraSettings->CamRollMin),                          _sbCamRollMin);
    _association.addBinding(&(cameraSettings->CamRollMax),                          _sbCamRollMax);
    _association.addBinding(&(cameraSettings->CamAxisXInverse),                      _chkCamAxisXInverse);


    _association.addBinding(&(cameraSettings->FixedPosLandingYaw),                  _sbFixedPosLandingYaw);
    _association.addBinding(&(cameraSettings->FixedPosLandingPitch),                _sbFixedPosLandingPitch);
    _association.addBinding(&(cameraSettings->FixedPosLandingRoll),                 _sbFixedPosLandingRoll);
    _association.addBinding(&(cameraSettings->FixedPosLandingCommand),              _chkFixedPosLandingCommand);

    _association.addBinding(&(cameraSettings->FixedPosBeginingYaw),                 _sbFixedPosBeginingYaw);
    _association.addBinding(&(cameraSettings->FixedPosBeginingPitch),               _sbFixedPosBeginingPitch);
    _association.addBinding(&(cameraSettings->FixedPosBeginingRoll),                _sbFixedPosBeginingRoll);

    _association.addBinding(&(cameraSettings->FixedPosVerticalYaw),                 _sbFixedPosVerticalYaw);
    _association.addBinding(&(cameraSettings->FixedPosVerticalPitch),               _sbFixedPosVerticalPitch);
    _association.addBinding(&(cameraSettings->FixedPosVerticalRoll),                _sbFixedPosVerticalRoll);

    _association.addBinding(&(cameraSettings->CameraControlMode),                   _cbCameraControlMode);
    _bombingSightNumbers = &(cameraSettings->BombingSightNumbers);
}

void CameraSettingsEditor::loadSettings()
{
    EnterProcStart("CameraSettingsEditor::loadSettings");

    _association.toEditor();

    if (_isBombingTabLicensed)
    {
        QString serializedPreference = _bombingSightNumbers->value();
        QStringList settingPairs = serializedPreference.split("&");
        for (int i = 0; i < _sightControls.count(); i++)
        {
            auto controls = _sightControls.at(i);
            if (i < settingPairs.count())
            {
                QStringList values = settingPairs.at(i).split("=");
                if (values.count() != 2)
                {
                    controls.Height->setValue(0);
                    controls.SightNumbers->setText("");
                }
                else
                {
                    controls.Height->setValue(values.at(0).toInt());
                    controls.SightNumbers->setText(values.at(1));
                }
            }
        }
    }

    _cbOpticalDevicesCount->activated(_cbOpticalDevicesCount->currentIndex()); //activate siognal processing
}

void CameraSettingsEditor::saveSettings()
{
    EnterProcStart("CameraSettingsEditor::saveSettings");
    _association.fromEditor();

    if (_isBombingTabLicensed)
    {
        QString serializedPreference;
        foreach (auto controls, _sightControls)
        {
            int height = controls.Height->value();
            QString numbers = controls.SightNumbers->text();
            serializedPreference = QString("%1%2=%3&").arg(serializedPreference).arg(height).arg(numbers);
        }
        _bombingSightNumbers->setValue(serializedPreference);
    }
}

void CameraSettingsEditor::onVideoSourceSelected(int id)
{
    /*
    QList<QWidget*> disabledControls, enabledControls;
    disabledControls << _cbUSBCamera << _naeXPlane \
                     << _lblYurionUDPPort << _edYurionUDPPort << _cbCalibrationImagePath \
                     << _fpsVideoFile << _edRTSPUrl \
                     << _edMUSV2UDPPort << _lblMUSV2UDPPort;

    switch (id)
    {
    case VideoFrameTrafficSources::USBCamera:
    {
        enabledControls << _cbUSBCamera;
        break;
    }
    case VideoFrameTrafficSources::XPlane:
    {
        enabledControls << _naeXPlane;
        break;
    }
    case VideoFrameTrafficSources::Yurion:
    {
        enabledControls << _lblYurionUDPPort << _edYurionUDPPort;
        break;
    }
    case VideoFrameTrafficSources::CalibrationImage:
    {
        enabledControls << _cbCalibrationImagePath;
        break;
    }
    case VideoFrameTrafficSources::VideoFile:
    {
        enabledControls << _fpsVideoFile;
        break;
    }
    case VideoFrameTrafficSources::RTSP:
    {
        enabledControls << _edRTSPUrl;
        break;
    }
    case VideoFrameTrafficSources::MUSV2:
    {
        enabledControls << _edMUSV2UDPPort << _lblMUSV2UDPPort;
        break;
    }
    }

    foreach (auto widget, enabledControls)
    {
        widget->setEnabled(true);
        disabledControls.removeOne(widget);
    }
    foreach (auto widget, disabledControls)
        widget->setEnabled(false);
        */
}
