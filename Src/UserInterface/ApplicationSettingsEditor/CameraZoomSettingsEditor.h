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

    QList<QDoubleSpinBox*> _scaleEditors, _verticalAngleEditors, _horizontalAngleEditors, _automaticTracerSpeedMultipliersEditors, _manualSpeedMultipliersEditors;
    QSpinBox *_sbCamViewSizeHorizontal;
    QSpinBox *_sbCamViewSizeVertical;
    QCheckBox *_chkCamViewSizeForceSet;
    QCheckBox *_chkVerticalMirror;

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
