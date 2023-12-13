#include "CameraZoomSettingsEditor.h"
#include "EnterProc.h"
#include "ConstantNames.h"

const char *BUTTON_ZOOM_VALUE = "BUTTON_ZOOM_VALUE";

CameraZoomSettingsEditor::CameraZoomSettingsEditor(QWidget *parent, const qint32 camIdx, const quint32 opticalSystemNumber):
    QDialog(parent),
    _association(this),
    _camIdx(camIdx),
    _opticalSystemNumber(opticalSystemNumber)
{
    initWidgets();
    loadSettings();
}

void CameraZoomSettingsEditor::accept()
{
    EnterProcStart("CameraZoomSettingsEditor::accept");

    saveSettings();
    //???    emit onCamInfoUpdated();
    done(QDialog::Accepted);
}

QWidget *CameraZoomSettingsEditor::createZoomGrid()
{
    EnterProcStart("CameraZoomSettingsEditor::createZoomGrid");

    auto scrolledWidget = CommonWidgetUtils::createScrolledWidget(this);

    auto lblZoom = new QLabel(tr("Zoom"), this);
    auto lblScale = new QLabel(tr("Scale"), this);
    auto lblVerticalAngle = new QLabel(tr("Vertical\nAngle"), this);
    auto lblHorizontalAngle = new QLabel(tr("Horizontal\nAngle"), this);
    auto lblAutomaticTracerSpeedMultipliers = new QLabel(tr("Automatic Tracer\nSpeed Multiplier"), this);
    auto lblbManualSpeedMultipliers = new QLabel(tr("Manual Moving\nSpeed Multiplier"), this);


    auto camControlsGrid = new QGridLayout(scrolledWidget);

    quint32 row = 0;
    camControlsGrid->addWidget(lblZoom,                                      row, 0, 1, 1, Qt::AlignHCenter);
    camControlsGrid->addWidget(lblScale,                                     row, 1, 1, 1, Qt::AlignHCenter);
    camControlsGrid->addWidget(lblVerticalAngle,                             row, 3, 1, 1, Qt::AlignHCenter);
    camControlsGrid->addWidget(lblHorizontalAngle,                           row, 4, 1, 1, Qt::AlignHCenter);
    camControlsGrid->addWidget(lblAutomaticTracerSpeedMultipliers,           row, 5, 1, 1, Qt::AlignHCenter);
    camControlsGrid->addWidget(lblbManualSpeedMultipliers,                   row, 6, 1, 1, Qt::AlignHCenter);

    for (row = 1; row <= MAXIMAL_CAMERA_ZOOM; row++)
    {
        auto lblZoomValue = new QLabel(QString::number(row), this);

        QPushButton *btnAutoFill = nullptr;
        if (row == 1)
        {
            btnAutoFill = new QPushButton("...", this);
            btnAutoFill->setProperty(BUTTON_ZOOM_VALUE, row);
            connect(btnAutoFill, &QPushButton::clicked, this, &CameraZoomSettingsEditor::onAutoFillTableClicked);
        }
        else
        {
            btnAutoFill = new QPushButton(">>>", this);
            btnAutoFill->setProperty(BUTTON_ZOOM_VALUE, row);
            connect(btnAutoFill, &QPushButton::clicked, this, &CameraZoomSettingsEditor::onAutoFillRowClicked);
        }

        auto sbScale = CommonWidgetUtils::createDoubleRangeSpinbox(this,           0.0,   1.0, 0.001, 3);
        auto sbVerticalAngle = CommonWidgetUtils::createDoubleRangeSpinbox(this,   0.0, 180.0, 0.001, 3);
        auto sbHorizontalAngle = CommonWidgetUtils::createDoubleRangeSpinbox(this, 0.0, 180.0, 0.001, 3);
        auto sbAutomaticTracerSpeedMultiplier = CommonWidgetUtils::createDoubleRangeSpinbox(this,      0.0,   5.0, 0.001, 3);
        auto sbManualSpeedMultiplier = CommonWidgetUtils::createDoubleRangeSpinbox(this,               0.0,   5.0, 0.001, 3);

        _scaleEditors.append(sbScale);
        _verticalAngleEditors.append(sbVerticalAngle);
        _horizontalAngleEditors.append(sbHorizontalAngle);
        _automaticTracerSpeedMultipliersEditors.append(sbAutomaticTracerSpeedMultiplier);
        _manualSpeedMultipliersEditors.append(sbManualSpeedMultiplier);

        camControlsGrid->addWidget(lblZoomValue,                               row, 0, 1, 1);
        camControlsGrid->addWidget(sbScale,                                    row, 1, 1, 1);
        camControlsGrid->addWidget(btnAutoFill,                                row, 2, 1, 1);
        camControlsGrid->addWidget(sbVerticalAngle,                            row, 3, 1, 1);
        camControlsGrid->addWidget(sbHorizontalAngle,                          row, 4, 1, 1);
        camControlsGrid->addWidget(sbAutomaticTracerSpeedMultiplier,           row, 5, 1, 1);
        camControlsGrid->addWidget(sbManualSpeedMultiplier,                    row, 6, 1, 1);
    }

    return scrolledWidget;
}

QWidget *CameraZoomSettingsEditor::createImageParams()
{
    EnterProcStart("CameraZoomSettingsEditor::createImageParams");

    auto imageParamsWidget = new QWidget(this);
    auto imageParamsGrid = new QGridLayout(imageParamsWidget);

    auto lblDeviceLink = new QLabel(tr("Connection"), this);
    _cbDeviceLink = new QComboBoxExt(this, ConstantNames::OpticalDeviceLinkCaptions());

    auto lblCamViewSize = new QLabel(tr("Resolution"), this);
    _sbCamViewSizeHorizontal = CommonWidgetUtils::createRangeSpinbox(this, 320, 1920);
    _sbCamViewSizeVertical = CommonWidgetUtils::createRangeSpinbox(this, 240, 1080);

    _chkCamViewSizeForceSet = new QCheckBox(tr("Force Set"), this);

    _chkVerticalMirror = new QCheckBox(tr("Vertical Mirroring"), this);

    auto lblMagnifier = new QLabel(tr("Magnifier"), this);
    _sbMagnifierSourceSize = CommonWidgetUtils::createRangeSpinbox(this, 50, 250);

    int row = 0;

    imageParamsGrid->addWidget(lblDeviceLink,                   row, 0, 1, 1);
    imageParamsGrid->addWidget(_cbDeviceLink,                   row, 2, 1, 1);
    row++;

    imageParamsGrid->addWidget(lblCamViewSize,                  row, 0, 1, 1);
    imageParamsGrid->addWidget(_sbCamViewSizeHorizontal,        row, 2, 1, 1);
    imageParamsGrid->addWidget(_sbCamViewSizeVertical,          row, 3, 1, 1);
    imageParamsGrid->addWidget(_chkCamViewSizeForceSet,         row, 4, 1, 2);
    imageParamsGrid->setRowStretch(row, 0);
    row++;

    imageParamsGrid->addWidget(_chkVerticalMirror,              row, 2, 1, 3);
    imageParamsGrid->setRowStretch(row, 0);
    row++;

    imageParamsGrid->addWidget(lblMagnifier,                  row, 0, 1, 1);
    imageParamsGrid->addWidget(_sbMagnifierSourceSize,        row, 2, 1, 1);
    imageParamsGrid->setRowStretch(row, 0);
    row++;

    return imageParamsWidget;
}

void CameraZoomSettingsEditor::initWidgets()
{
    EnterProcStart("CameraZoomSettingsEditor::initWidgets");

    this->setWindowTitle(tr("Optical parameters of camera # %1.%2").arg(_camIdx).arg(_opticalSystemNumber));
    this->setModal(true);
    CommonWidgetUtils::updateWidgetGeometry(this, 850);
    this->setAttribute(Qt::WA_DeleteOnClose, true);

    auto scrolledWidget = createZoomGrid();
    auto imageParamsWidget = createImageParams();
    auto buttonBox = CommonWidgetUtils::makeDialogButtonBox(this, QDialogButtonBox::Ok | QDialogButtonBox::Cancel);

    auto mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(imageParamsWidget, 0);
    mainLayout->addWidget(CommonWidgetUtils::createSeparator(this), 0);
    mainLayout->addWidget(qobject_cast<QWidget*>(scrolledWidget->parent()->parent()), 1);
    mainLayout->addWidget(buttonBox, 0);

    ApplicationSettings& applicationSettings = ApplicationSettings::Instance();
    auto cameraSettings = applicationSettings.cameraPreferences(_camIdx);

    if (_opticalSystemNumber == 1)
    {        
        //_association.addBinding(&(cameraSettings->DeviceLinkIdA),                         &_cbDeviceLink);
        _association.addBinding(&(cameraSettings->CamScaleCoefficientA),                  &_scaleEditors);
        _association.addBinding(&(cameraSettings->CamViewAnglesHorizontalA),              &_horizontalAngleEditors);
        _association.addBinding(&(cameraSettings->CamViewAnglesVerticalA),                &_verticalAngleEditors);
        _association.addBinding(&(cameraSettings->CamAutomaticTracerSpeedMultipliersA),   &_automaticTracerSpeedMultipliersEditors);
        _association.addBinding(&(cameraSettings->CamManualSpeedMultipliersA),            &_manualSpeedMultipliersEditors);

        _association.addBinding(&(cameraSettings->CamViewSizeHorizontalA),               _sbCamViewSizeHorizontal);
        _association.addBinding(&(cameraSettings->CamViewSizeVerticalA),                 _sbCamViewSizeVertical);
        _association.addBinding(&(cameraSettings->CamViewSizeForceSetA),                 _chkCamViewSizeForceSet);
        _association.addBinding(&(cameraSettings->UseVerticalFrameMirrororingA),         _chkVerticalMirror);

        _association.addBinding(&(cameraSettings->MagnifierSourceSizeA),                 _sbMagnifierSourceSize);
    }
    else if (_opticalSystemNumber == 2)
    {
        //_association.addBinding(&(cameraSettings->DeviceLinkIdB),                         &_cbDeviceLink);
        _association.addBinding(&(cameraSettings->CamScaleCoefficientB),                  &_scaleEditors);
        _association.addBinding(&(cameraSettings->CamViewAnglesHorizontalB),              &_horizontalAngleEditors);
        _association.addBinding(&(cameraSettings->CamViewAnglesVerticalB),                &_verticalAngleEditors);
        _association.addBinding(&(cameraSettings->CamAutomaticTracerSpeedMultipliersB),   &_automaticTracerSpeedMultipliersEditors);
        _association.addBinding(&(cameraSettings->CamManualSpeedMultipliersB),            &_manualSpeedMultipliersEditors);

        _association.addBinding(&(cameraSettings->CamViewSizeHorizontalB),               _sbCamViewSizeHorizontal);
        _association.addBinding(&(cameraSettings->CamViewSizeVerticalB),                 _sbCamViewSizeVertical);
        _association.addBinding(&(cameraSettings->CamViewSizeForceSetB),                 _chkCamViewSizeForceSet);
        _association.addBinding(&(cameraSettings->UseVerticalFrameMirrororingB),         _chkVerticalMirror);

        _association.addBinding(&(cameraSettings->MagnifierSourceSizeB),                 _sbMagnifierSourceSize);
    }
    else if (_opticalSystemNumber == 3)
    {
        //_association.addBinding(&(cameraSettings->DeviceLinkIdC),                         &_cbDeviceLink);
        _association.addBinding(&(cameraSettings->CamScaleCoefficientC),                  &_scaleEditors);
        _association.addBinding(&(cameraSettings->CamViewAnglesHorizontalC),              &_horizontalAngleEditors);
        _association.addBinding(&(cameraSettings->CamViewAnglesVerticalC),                &_verticalAngleEditors);
        _association.addBinding(&(cameraSettings->CamAutomaticTracerSpeedMultipliersC),   &_automaticTracerSpeedMultipliersEditors);
        _association.addBinding(&(cameraSettings->CamManualSpeedMultipliersC),            &_manualSpeedMultipliersEditors);

        _association.addBinding(&(cameraSettings->CamViewSizeHorizontalC),               _sbCamViewSizeHorizontal);
        _association.addBinding(&(cameraSettings->CamViewSizeVerticalC),                 _sbCamViewSizeVertical);
        _association.addBinding(&(cameraSettings->CamViewSizeForceSetC),                 _chkCamViewSizeForceSet);
        _association.addBinding(&(cameraSettings->UseVerticalFrameMirrororingC),         _chkVerticalMirror);

        _association.addBinding(&(cameraSettings->MagnifierSourceSizeC),                 _sbMagnifierSourceSize);
    }

    else
        Q_ASSERT(false);
}

void CameraZoomSettingsEditor::loadSettings()
{
    EnterProcStart("CameraZoomSettingsEditor::loadSettings");
    _association.toEditor();

    _scaleEditors.at(0)->setValue(1);
    _scaleEditors.at(0)->setEnabled(false);
}

void CameraZoomSettingsEditor::saveSettings()
{
    EnterProcStart("CameraZoomSettingsEditor::saveSettings");
    _association.fromEditor();
}

void CameraZoomSettingsEditor::onAutoFillRowClicked()
{
    auto button = qobject_cast<QPushButton*>(QObject::sender());
    int zoom = button->property(BUTTON_ZOOM_VALUE).toInt();
    recalculateRow(zoom);
}

void CameraZoomSettingsEditor::recalculateRow(int zoom)
{
    int i = zoom - 1;

    auto scaleEditor = _scaleEditors.at(i);
    auto verticalAngleEditor = _verticalAngleEditors.at(i);
    auto horizontalAngleEditor = _horizontalAngleEditors.at(i);
    auto automaticTracerSpeedMultipliersEditor = _automaticTracerSpeedMultipliersEditors.at(i);
    auto manualSpeedMultiplier = _manualSpeedMultipliersEditors.at(i);

    auto verticalAngleEditorSrc = _verticalAngleEditors.at(0);
    auto horizontalAngleEditorSrc = _horizontalAngleEditors.at(0);
    auto automaticTracerSpeedMultipliersEditorSrc = _automaticTracerSpeedMultipliersEditors.at(0);
    auto manualSpeedMultiplierSrc = _manualSpeedMultipliersEditors.at(0);

    double k = scaleEditor->value();
    verticalAngleEditor->setValue(verticalAngleEditorSrc->value() * k);
    horizontalAngleEditor->setValue(horizontalAngleEditorSrc->value() * k);
    automaticTracerSpeedMultipliersEditor->setValue(automaticTracerSpeedMultipliersEditorSrc->value());
    manualSpeedMultiplier->setValue(manualSpeedMultiplierSrc->value() * k);
}

void CameraZoomSettingsEditor::onAutoFillTableClicked()
{
    EnterProcStart("CameraZoomSettingsEditor::onAutoFillTableClicked");

    auto menu = new QMenu(this);
    QAction *acFillNonlinearScale = nullptr;//CommonWidgetUtils::createMenuAction(tr("Fill Nonlinear Zoom (Scale Only)"), menu);
    QAction *acFillLinearScale = CommonWidgetUtils::createMenuAction(tr("Fill Linear Zoom (Scale Only)"), menu);
    QAction *acFillNonlinearAll = nullptr;//CommonWidgetUtils::createMenuAction(tr("Fill Nonlinear Zoom (All Data)"), menu);
    QAction *acFillLinearAll = CommonWidgetUtils::createMenuAction(tr("Fill Linear Zoom (All Data)"), menu);

    auto action = menu->exec(QCursor::pos());

    if (action == nullptr)
        return;

    bool needUpdate = CommonWidgetUtils::showConfirmDialog(tr("The manually entered data will be updated automatically.\nAre you sure you want to update it?"), false);
    if (!needUpdate)
        return;

    if ( (action == acFillNonlinearScale) || (action == acFillNonlinearAll))
    {


    }
    else if ( (action == acFillLinearScale) || (action == acFillLinearAll) )
    {
        for (quint32 zoom = 2; zoom < MAXIMAL_CAMERA_ZOOM; zoom++)
        {
            int i = zoom - 1;
            auto scaleEditor = _scaleEditors.at(i);
            scaleEditor->setValue(1.0 / zoom);
        }
    }

    if ( (action == acFillNonlinearAll) || (action == acFillLinearAll) )
    {
        for (quint32 zoom = 1; zoom < MAXIMAL_CAMERA_ZOOM; zoom++)
            recalculateRow(zoom);
    }
}
