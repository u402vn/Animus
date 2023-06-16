#ifndef CAMERAZOOMSETTINGSEDITOR_H
#define CAMERAZOOMSETTINGSEDITOR_H

#include <QObject>
#include <QWidget>
#include <QDialog>
#include <QList>
#include "ApplicationSettings.h"
#include "Common/CommonWidgets.h"
#include "PreferenceAssociation.h"

class CameraZoomSettingsEditor final : public QDialog
{
    Q_OBJECT
    PreferenceAssociation _association;

    QList<QDoubleSpinBoxEx*> _scaleEditors, _verticalAngleEditors, _horizontalAngleEditors, _automaticTracerSpeedMultipliersEditors, _manualSpeedMultipliersEditors;
    QSpinBoxEx *_sbCamViewSizeHorizontal;
    QSpinBoxEx *_sbCamViewSizeVertical;
    QCheckBox *_chkCamViewSizeForceSet;
    QCheckBox *_chkVerticalMirror;
    QSpinBoxEx *_sbMagnifierSourceSize;

    qint32 _camIdx, _opticalSystemNumber;

    void recalculateRow(int zoom);
public:
    explicit CameraZoomSettingsEditor(QWidget *parent, const qint32 camIdx, const quint32 opticalSystemNumber);
    virtual void accept();

    QWidget *createZoomGrid();
    QWidget *createImageParams();
    void initWidgets();
    void loadSettings();
    void saveSettings();
private slots:
    void onAutoFillRowClicked();
    void onAutoFillTableClicked();
};

#endif // CAMERAZOOMSETTINGSEDITOR_H
