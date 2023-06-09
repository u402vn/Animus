#include "CameraSettingsEditor.h"
#include <QGridLayout>
#include <QRadioButton>
#include <QCameraInfo>
#include <QLineEdit>
#include <QDialogButtonBox>
#include "EnterProc.h"
#include "UserInterface/ApplicationSettingsEditor/CameraZoomSettingsEditor.h"
#include "UserInterface/ConstantNames.h"

QComboBoxExt *CameraSettingsEditor::createCamListCombo(QWidget *parent)
{
    ApplicationSettings& applicationSettings = ApplicationSettings::Instance();
    auto cbCameras = new QComboBoxExt(parent);
    for (int i = 0; i < 10; i++)
    {
        auto cameraSettings = applicationSettings.cameraSettings(i);
        QString description = cameraSettings->CameraDescription;
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

    auto btnEditPrimaryCamSettings = new QPushButton(tr("Primary camera"), this);
    connect(btnEditPrimaryCamSettings, &QPushButton::clicked, this, &CameraSettingsEditor::onEditPrimaryCamSettingsClicked);

    auto btnEditSecondaryCamSettings = new QPushButton(tr("Secondary camera"), this);
    connect(btnEditSecondaryCamSettings, &QPushButton::clicked, this, &CameraSettingsEditor::onEditSecondaryCamSettingsClicked);

    auto gimbalWidgets = createGimbalWidgets();
    auto connectionWidgets = createConnectionWidgets();
    auto functionsWidgets = createFunctionsWidgets();

    auto camControlsGrid = new QGridLayout(scrolledWidget);

    int row = 0;

    camControlsGrid->addWidget(lblCamDescription,                row, 0, 1, 1);
    camControlsGrid->addWidget(_edtCamDescription,               row, 1, 1, 2);
    row++;

    camControlsGrid->addWidget(btnEditPrimaryCamSettings,        row, 1, 1, 1);
    camControlsGrid->addWidget(btnEditSecondaryCamSettings,      row, 2, 1, 1);
    row++;

    camControlsGrid->addWidget(gimbalWidgets,                    row++, 0, 1, 3);
    camControlsGrid->addWidget(connectionWidgets,                row++, 0, 1, 3);
    if (functionsWidgets != nullptr)
        camControlsGrid->addWidget(functionsWidgets,             row++, 0, 1, 3);
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
    auto lblFixedCamZoom =  new QLabel(tr("Zoom"), this);

    _sbFixedCamPitch = CommonWidgetUtils::createRangeSpinbox(this, -180, +180);
    _sbFixedCamRoll  = CommonWidgetUtils::createRangeSpinbox(this, -180, +180);
    _sbFixedCamZoom  = CommonWidgetUtils::createRangeSpinbox(this, 1, MAXIMAL_CAMERA_ZOOM);

    // Main Controls
    _cbCameraControlMode = new QComboBoxExt(this, ConstantNames::CameraControlModeCaptions());

    auto lblCamHeaderMin = new QLabel(tr("Minimum Value"), this);
    _sbCamPitchMin = CommonWidgetUtils::createRangeSpinbox(this, -720, +720);
    _sbCamRollMin = CommonWidgetUtils::createRangeSpinbox(this, -720, +720);
    _sbCamZoomMin = CommonWidgetUtils::createRangeSpinbox(this, 1, MAXIMAL_CAMERA_ZOOM);

    auto lblCamHeaderMax = new QLabel(tr("Maximum Value"), this);
    _sbCamPitchMax = CommonWidgetUtils::createRangeSpinbox(this, -720, +720);
    _sbCamRollMax = CommonWidgetUtils::createRangeSpinbox(this, -720, +720);
    _sbCamZoomMax = CommonWidgetUtils::createRangeSpinbox(this, 1, MAXIMAL_CAMERA_ZOOM);

    _chkCamAxisXInverse = new QCheckBox(tr("Inverse Axis X"), this);
    _chkCamAxisYInverse = new QCheckBox(tr("Inverse Axis Y"), this);
    _lblEncoderAutomaticTracerMultiplie = new QLabel(tr("Tracker Speed Multiplier"), this);
    _sbEncoderAutomaticTracerMultiplier = CommonWidgetUtils::createDoubleRangeSpinbox(this, 0.01, 1000, 0.01, 2);

    // Fast positioning controls
    auto lblFixedPosLanding = new QLabel(tr("Landing Position"), this);
    _sbFixedPosLandingYaw = CommonWidgetUtils::createRangeSpinbox(this, -720, +720);
    _sbFixedPosLandingPitch = CommonWidgetUtils::createRangeSpinbox(this, -720, +720);
    _sbFixedPosLandingRoll  = CommonWidgetUtils::createRangeSpinbox(this, -720, +720);
    _sbFixedPosLandingZoom  = CommonWidgetUtils::createRangeSpinbox(this, 1, MAXIMAL_CAMERA_ZOOM);
    _chkFixedPosLandingCommand = new QCheckBox(tr("Special Command for Landing Position"), this);

    auto lblFixedPosBegining = new QLabel(tr("Begining Position"), this);
    _sbFixedPosBeginingYaw = CommonWidgetUtils::createRangeSpinbox(this, -720, +720);
    _sbFixedPosBeginingPitch = CommonWidgetUtils::createRangeSpinbox(this, -720, +720);
    _sbFixedPosBeginingRoll  = CommonWidgetUtils::createRangeSpinbox(this, -720, +720);
    _sbFixedPosBeginingZoom  = CommonWidgetUtils::createRangeSpinbox(this, 1, MAXIMAL_CAMERA_ZOOM);

    auto lblFixedPosVertical = new QLabel(tr("Vertical Position"), this);
    _sbFixedPosVerticalYaw = CommonWidgetUtils::createRangeSpinbox(this, -720, +720);
    _sbFixedPosVerticalPitch = CommonWidgetUtils::createRangeSpinbox(this, -720, +720);
    _sbFixedPosVerticalRoll  = CommonWidgetUtils::createRangeSpinbox(this, -720, +720);
    _sbFixedPosVerticalZoom  = CommonWidgetUtils::createRangeSpinbox(this, 1, MAXIMAL_CAMERA_ZOOM);

    int row = 0;

    gimbalLayout->addWidget(rbFixedCamera,                   row, 0, 1, 4);
    gimbalLayout->addWidget(lblFixedCamYaw,                  row, 2, 1, 1);
    gimbalLayout->addWidget(lblFixedCamPitch,                row, 3, 1, 1);
    gimbalLayout->addWidget(lblFixedCamRoll,                 row, 4, 1, 1);
    gimbalLayout->addWidget(lblFixedCamZoom,                 row, 5, 1, 1);
    gimbalLayout->setRowStretch(row, 0);
    row++;

    gimbalLayout->addWidget(_sbFixedCamPitch,                row, 3, 1, 1);
    gimbalLayout->addWidget(_sbFixedCamRoll,                 row, 4, 1, 1);
    gimbalLayout->addWidget(_sbFixedCamZoom,                 row, 5, 1, 1);
    gimbalLayout->setRowStretch(row, 0);
    row++;

    gimbalLayout->addWidget(rbRotatingCamera,                row, 0, 1, 2);
    gimbalLayout->addWidget(_cbCameraControlMode,            row, 2, 1, 4);
    gimbalLayout->setRowStretch(row, 0);
    row++;

    gimbalLayout->addWidget(lblCamHeaderMin,                 row, 1, 1, 1);
    gimbalLayout->addWidget(_sbCamPitchMin,                  row, 3, 1, 1);
    gimbalLayout->addWidget(_sbCamRollMin,                   row, 4, 1, 1);
    gimbalLayout->addWidget(_sbCamZoomMin,                   row, 5, 1, 1);
    gimbalLayout->setRowStretch(row, 0);
    row++;

    gimbalLayout->addWidget(lblCamHeaderMax,                 row, 1, 1, 1);
    gimbalLayout->addWidget(_sbCamPitchMax,                  row, 3, 1, 1);
    gimbalLayout->addWidget(_sbCamRollMax,                   row, 4, 1, 1);
    gimbalLayout->addWidget(_sbCamZoomMax,                   row, 5, 1, 1);
    gimbalLayout->setRowStretch(row, 0);
    row++;

    gimbalLayout->addWidget(lblFixedPosLanding,              row, 1, 1, 1);
    gimbalLayout->addWidget(_sbFixedPosLandingYaw,           row, 2, 1, 1);
    gimbalLayout->addWidget(_sbFixedPosLandingPitch,         row, 3, 1, 1);
    gimbalLayout->addWidget(_sbFixedPosLandingRoll,          row, 4, 1, 1);
    gimbalLayout->addWidget(_sbFixedPosLandingZoom,          row, 5, 1, 1);
    row++;

    gimbalLayout->addWidget(_chkFixedPosLandingCommand,      row, 2, 1, 4);
    row++;

    gimbalLayout->addWidget(lblFixedPosBegining,             row, 1, 1, 1);
    gimbalLayout->addWidget(_sbFixedPosBeginingYaw,          row, 2, 1, 1);
    gimbalLayout->addWidget(_sbFixedPosBeginingPitch,        row, 3, 1, 1);
    gimbalLayout->addWidget(_sbFixedPosBeginingRoll,         row, 4, 1, 1);
    gimbalLayout->addWidget(_sbFixedPosBeginingZoom,         row, 5, 1, 1);
    gimbalLayout->setRowStretch(row, 0);
    row++;

    gimbalLayout->addWidget(lblFixedPosVertical,             row, 1, 1, 1);
    gimbalLayout->addWidget(_sbFixedPosVerticalYaw,          row, 2, 1, 1);
    gimbalLayout->addWidget(_sbFixedPosVerticalPitch,        row, 3, 1, 1);
    gimbalLayout->addWidget(_sbFixedPosVerticalRoll,         row, 4, 1, 1);
    gimbalLayout->addWidget(_sbFixedPosVerticalZoom,         row, 5, 1, 1);
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

const QString CameraSettingsEditor::getSourceCaption(VideoFrameTrafficSources source)
{
    const QString captions[VideoFrameTrafficSources::LastEelemet + 1] =
    {CameraSettingsEditor::tr("USB Camera"),
     CameraSettingsEditor::tr("X-Plane"),
     CameraSettingsEditor::tr("Yurion"),
     CameraSettingsEditor::tr("Image File"),
     CameraSettingsEditor::tr("Video File"),
     CameraSettingsEditor::tr("RTSP"),
     CameraSettingsEditor::tr("MUSV-2")
    };
    return captions[source];
}

const QString CameraSettingsEditor::getCameraInfo(qint32 camIdx)
{
    ApplicationSettings& applicationSettings = ApplicationSettings::Instance();
    auto cameraSettings = applicationSettings.cameraSettings(camIdx);

    VideoFrameTrafficSources source = cameraSettings->VideoTrafficSource;

    QString detail;

    switch(source)
    {
    case VideoFrameTrafficSources::USBCamera:
    {
        auto cameraName = cameraSettings->VideoFrameSourceCameraName.value().toLocal8Bit();
        QCameraInfo cameraInfo = QCameraInfo(cameraName);
        if (cameraInfo.isNull())
            detail = tr("Unknown camera");
        else
            detail = cameraInfo.description();
        break;
    }
    case VideoFrameTrafficSources::XPlane:
    {
        detail = tr("URL: %1:%2").arg(cameraSettings->VideoFrameSourceXPlaneAddress).arg(cameraSettings->VideoFrameSourceXPlanePort);
        break;
    }
    case VideoFrameTrafficSources::Yurion:
    {
        detail = tr("Port: %1").arg(cameraSettings->VideoFrameSourceYurionUDPPort);
        break;
    }
    case VideoFrameTrafficSources::CalibrationImage:
    {
        detail = tr("%1").arg(cameraSettings->CalibrationImagePath);
        break;
    }
    case VideoFrameTrafficSources::VideoFile:
    {
        detail = tr("%1").arg(cameraSettings->VideoFilePath);
        break;
    }
    case VideoFrameTrafficSources::RTSP:
    {
        detail = tr("URL: %1").arg(cameraSettings->RTSPUrl);
        break;
    }
    case VideoFrameTrafficSources::MUSV2:
    {
        detail = tr("UDP Port: %1").arg(cameraSettings->VideoFrameSourceMUSV2UDPPort);
        break;
    }
    }

    QString description = cameraSettings->CameraDescription;
    if (description.isEmpty())
        description = tr("Unnamed video source");

    QString info = tr(" %1\n %2 \t %3x%4\n %5")
            .arg(description)
            .arg(getSourceCaption(source))
            .arg(cameraSettings->CamViewSizeHorizontal).arg(cameraSettings->CamViewSizeVertical)
            .arg(detail);

    return info;
}

void CameraSettingsEditor::onEditPrimaryCamSettingsClicked()
{
    auto cameraZoomSettingsEditor = new CameraZoomSettingsEditor(this, _camIdx, 1);
    //??? connect(cameraSettingsEditor, &CameraSettingsEditor::onCamInfoUpdated, this, &CameraListSettingsEditor::onCamInfoUpdated);
    cameraZoomSettingsEditor->showNormal();
}

void CameraSettingsEditor::onEditSecondaryCamSettingsClicked()
{
    auto cameraZoomSettingsEditor = new CameraZoomSettingsEditor(this, _camIdx, 2);
    //??? connect(cameraSettingsEditor, &CameraSettingsEditor::onCamInfoUpdated, this, &CameraListSettingsEditor::onCamInfoUpdated);
    cameraZoomSettingsEditor->showNormal();
}

QRadioButton *CameraSettingsEditor::addVideoSourceRadioButton(VideoFrameTrafficSources source)
{
    return _gbVideoSource->appendButton(getSourceCaption(source), source);
}

QWidget *CameraSettingsEditor::createConnectionWidgets()
{
    ApplicationSettings& applicationSettings = ApplicationSettings::Instance();
    auto cameraSettings = applicationSettings.cameraSettings(_camIdx);

    auto spoilerConnection = new SpoilerGrid(tr("Connection"), this);
    auto connectionLayout = spoilerConnection->gridLayout();

    _gbVideoSource = new QButtonGroupExt(this);
    auto rbVideoSourceUSBCamera        = addVideoSourceRadioButton(VideoFrameTrafficSources::USBCamera);
    auto rbVideoSourceXPlane           = addVideoSourceRadioButton(VideoFrameTrafficSources::XPlane);
    auto rbVideoSourceYurion           = addVideoSourceRadioButton(VideoFrameTrafficSources::Yurion);
    auto rbVideoSourceImageFile        = addVideoSourceRadioButton(VideoFrameTrafficSources::CalibrationImage);
    auto rbVideoSourceVideoFile        = addVideoSourceRadioButton(VideoFrameTrafficSources::VideoFile);
    auto rbVideoSourceRTSP             = addVideoSourceRadioButton(VideoFrameTrafficSources::RTSP);
    auto rbVideoSourceNetworkCam       = addVideoSourceRadioButton(VideoFrameTrafficSources::MUSV2);

    connect(_gbVideoSource, static_cast<void(QButtonGroup::*)(int)>(&QButtonGroup::buttonClicked), this, &CameraSettingsEditor::onVideoSourceSelected);

    _cbUSBCamera = new QComboBoxExt(this);
    foreach (const QCameraInfo &cameraInfo, QCameraInfo::availableCameras())
        _cbUSBCamera->addItem(cameraInfo.description(), cameraInfo.deviceName());

    _naeXPlane = new NetworkAddressEditor(this, &_association, &cameraSettings->VideoFrameSourceXPlaneAddress, &cameraSettings->VideoFrameSourceXPlanePort);
    _lblYurionUDPPort = new QLabel(tr("UDP Port:"), this);
    _edYurionUDPPort = CommonWidgetUtils::createPortEditor(this);

    _cbCalibrationImagePath = new QComboBoxExt(this);
    _cbCalibrationImagePath->setEditable(true);
    _cbCalibrationImagePath->addItem(DefaultCalibrationImagePath);
    _cbCalibrationImagePath->addItem(":/CalibrationImages/Mira1.png");

    _fpsVideoFile = new FilePathSelector(this, tr("Video File"), tr("Select Video File"), tr("Video Files (*.avi)"));
    _fpsVideoFile->setLabelWidth(0);

    _edRTSPUrl = new QLineEdit(this);

    _lblMUSV2UDPPort = new QLabel(tr("UDP Port:"), this);
    _edMUSV2UDPPort = CommonWidgetUtils::createPortEditor(this);

    int row = 0;
    connectionLayout->addWidget(rbVideoSourceUSBCamera,          row, 0, 1, 1);
    connectionLayout->addWidget(_cbUSBCamera,                    row, 1, 1, 5);
    row++;

    connectionLayout->addWidget(rbVideoSourceXPlane,             row, 0, 1, 1);
    connectionLayout->addWidget(_naeXPlane,                      row, 1, 1, 2);
    row++;

    connectionLayout->addWidget(rbVideoSourceYurion,             row, 0, 1, 1);
    connectionLayout->addWidget(_lblYurionUDPPort,               row, 1, 1, 1);
    connectionLayout->addWidget(_edYurionUDPPort,                row, 2, 1, 1);
    row++;

    connectionLayout->addWidget(rbVideoSourceImageFile,          row, 0, 1, 1);
    connectionLayout->addWidget(_cbCalibrationImagePath,         row, 1, 1, 5);
    row++;

    connectionLayout->addWidget(rbVideoSourceVideoFile,          row, 0, 1, 1);
    connectionLayout->addWidget(_fpsVideoFile,                   row, 1, 1, 5);
    row++;

    connectionLayout->addWidget(rbVideoSourceRTSP,               row, 0, 1, 1);
    connectionLayout->addWidget(_edRTSPUrl,                      row, 1, 1, 5);
    row++;

    connectionLayout->addWidget(rbVideoSourceNetworkCam,         row, 0, 1, 1);
    connectionLayout->addWidget(_lblMUSV2UDPPort,                row, 1, 1, 1);
    connectionLayout->addWidget(_edMUSV2UDPPort,                 row, 2, 1, 1);
    row++;

    return spoilerConnection;
}

QWidget *CameraSettingsEditor::createFunctionsWidgets()
{
    if (!_isPhotographyLicensed && !_isBombingTabLicensed)
        return nullptr;

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
    auto cameraSettings = applicationSettings.cameraSettings(_camIdx);

    _association.addBinding(&(cameraSettings->CameraDescription),                   _edtCamDescription);

    if (_isPhotographyLicensed)
    {
        _association.addBinding(&(cameraSettings->IsOnboardRecording),                  _chkOnboardRecording);
        _association.addBinding(&(cameraSettings->IsSnapshot),                          _chkSnapshot);
    }

    _association.addBinding(&(cameraSettings->CameraSuspensionType),                _gbCameraType);

    _association.addBinding(&(cameraSettings->FixedCamPitch),                       _sbFixedCamPitch);
    _association.addBinding(&(cameraSettings->FixedCamRoll),                        _sbFixedCamRoll);
    _association.addBinding(&(cameraSettings->FixedCamZoom),                        _sbFixedCamZoom);

    _association.addBinding(&(cameraSettings->CamPitchMin),                         _sbCamPitchMin);
    _association.addBinding(&(cameraSettings->CamPitchMax),                         _sbCamPitchMax);
    _association.addBinding(&(cameraSettings->CamAxisYInverse),                     _chkCamAxisYInverse);
    _association.addBinding(&(cameraSettings->EncoderAutomaticTracerMultiplier),     _sbEncoderAutomaticTracerMultiplier);
    _association.addBinding(&(cameraSettings->CamRollMin),                          _sbCamRollMin);
    _association.addBinding(&(cameraSettings->CamRollMax),                          _sbCamRollMax);
    _association.addBinding(&(cameraSettings->CamAxisXInverse),                      _chkCamAxisXInverse);
    _association.addBinding(&(cameraSettings->CamZoomMin),                          _sbCamZoomMin);
    _association.addBinding(&(cameraSettings->CamZoomMax),                          _sbCamZoomMax);

    _association.addBinding(&(cameraSettings->FixedPosLandingYaw),                  _sbFixedPosLandingYaw);
    _association.addBinding(&(cameraSettings->FixedPosLandingPitch),                _sbFixedPosLandingPitch);
    _association.addBinding(&(cameraSettings->FixedPosLandingRoll),                 _sbFixedPosLandingRoll);
    _association.addBinding(&(cameraSettings->FixedPosLandingZoom),                 _sbFixedPosLandingZoom);
    _association.addBinding(&(cameraSettings->FixedPosLandingCommand),              _chkFixedPosLandingCommand);

    _association.addBinding(&(cameraSettings->FixedPosBeginingYaw),                 _sbFixedPosBeginingYaw);
    _association.addBinding(&(cameraSettings->FixedPosBeginingPitch),               _sbFixedPosBeginingPitch);
    _association.addBinding(&(cameraSettings->FixedPosBeginingRoll),                _sbFixedPosBeginingRoll);
    _association.addBinding(&(cameraSettings->FixedPosBeginingZoom),                _sbFixedPosBeginingZoom);

    _association.addBinding(&(cameraSettings->FixedPosVerticalYaw),                 _sbFixedPosVerticalYaw);
    _association.addBinding(&(cameraSettings->FixedPosVerticalPitch),               _sbFixedPosVerticalPitch);
    _association.addBinding(&(cameraSettings->FixedPosVerticalRoll),                _sbFixedPosVerticalRoll);
    _association.addBinding(&(cameraSettings->FixedPosVerticalZoom),                _sbFixedPosVerticalZoom);

    _association.addBinding(&(cameraSettings->CameraControlMode),                   _cbCameraControlMode);

    _association.addBinding(&(cameraSettings->VideoTrafficSource),                  _gbVideoSource);
    _association.addBinding(&(cameraSettings->VideoFrameSourceCameraName),          _cbUSBCamera);
    _association.addBinding(&(cameraSettings->VideoFrameSourceYurionUDPPort),       _edYurionUDPPort);
    _association.addBinding(&(cameraSettings->CalibrationImagePath),                _cbCalibrationImagePath);
    _association.addBinding(&(cameraSettings->VideoFilePath),                       _fpsVideoFile);
    _association.addBinding(&(cameraSettings->RTSPUrl),                             _edRTSPUrl);

    _association.addBinding(&(cameraSettings->VideoFrameSourceMUSV2UDPPort),        _edMUSV2UDPPort);

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
}
