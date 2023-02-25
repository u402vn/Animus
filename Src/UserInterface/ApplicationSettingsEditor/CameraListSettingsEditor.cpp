#include "CameraListSettingsEditor.h"
#include <QGridLayout>
#include <QPushButton>
#include "ApplicationSettings.h"
#include "EnterProc.h"
#include "UserInterface/ApplicationSettingsEditor/CameraSettingsEditor.h"

const char *CAMERA_INDEX = "CAMERA_INDEX";

CameraListSettingsEditor::CameraListSettingsEditor(QWidget *parent):
    QScrollArea(parent)
{
    EnterProcStart("CameraListSettingsEditor::CameraListSettingsEditor");

    QGridLayout *cameraTabGrid = CommonWidgetUtils::createGridLayoutForScrollArea(this);

    for (int camIdx = 0; camIdx < 10; camIdx++)
    {
        auto btnCamera = new QPushButton(tr("Camera %1").arg(camIdx), this);
        btnCamera->setMaximumWidth(100);
        btnCamera->setProperty(CAMERA_INDEX, camIdx);
        connect(btnCamera, &QPushButton::clicked, this, &CameraListSettingsEditor::onCamInfoClicked);

        auto lblCameraInfo = new QLabel(this);
        lblCameraInfo->setProperty(CAMERA_INDEX, camIdx);
        _camInfoLabels.append(lblCameraInfo);

        cameraTabGrid->addWidget(btnCamera,          camIdx, 0, 1, 1, Qt::AlignTop);
        cameraTabGrid->addWidget(lblCameraInfo,      camIdx, 1, 1, 1);
        cameraTabGrid->setRowStretch(camIdx, 0);
    }

    onCamInfoUpdated();

    cameraTabGrid->setRowStretch(11, 1);
}

void CameraListSettingsEditor::onCamInfoClicked()
{
    QPushButton *button = qobject_cast<QPushButton*>(sender());
    int camIdx = button->property(CAMERA_INDEX).toInt();

    auto cameraSettingsEditor = new CameraSettingsEditor(this, camIdx);
    connect(cameraSettingsEditor, &CameraSettingsEditor::onCamInfoUpdated, this, &CameraListSettingsEditor::onCamInfoUpdated);
    cameraSettingsEditor->showNormal();
}

void CameraListSettingsEditor::onCamInfoUpdated()
{
    foreach (auto camInfoLabel, _camInfoLabels)
    {
        int camIdx = camInfoLabel->property(CAMERA_INDEX).toInt();
        camInfoLabel->setText(CameraSettingsEditor::getCameraInfo(camIdx));
    }
}
