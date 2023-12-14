#ifndef CAMERASETTINGSEDITOR_H
#define CAMERASETTINGSEDITOR_H

#include <QWidget>
#include <QDialog>
#include <QSpinBox>
#include <QList>
#include <QCheckBox>
#include <QScrollArea>
#include "ApplicationSettings.h"
#include "Common/CommonWidgets.h"
#include "UserInterface/ApplicationSettingsEditor/NetworkAddressEditor.h"
#include "PreferenceAssociation.h"

class CameraSettingsEditor final : public QDialog
{
    Q_OBJECT

    struct SightNumberControls
    {
        QSpinBox * Height;
        QLineEdit * SightNumbers;
    };
    QList<SightNumberControls> _sightControls;

    PreferenceAssociation _association;
    ApplicationPreferenceString *_bombingSightNumbers;

    qint32 _camIdx;

    bool _isBombingTabLicensed;
    bool _isPhotographyLicensed;


    QComboBoxExt *_cbOpticalDevicesCount;
    QLineEdit *_edtCamDescription;

    QCheckBox *_chkOnboardRecording;
    QCheckBox *_chkSnapshot;

    QButtonGroupExt *_gbCameraType;
    QSpinBoxEx *_sbFixedCamPitch;
    QSpinBoxEx *_sbFixedCamRoll;
    QComboBoxExt *_cbCameraControlMode;
    QSpinBoxEx *_sbCamPitchMin;
    QSpinBoxEx *_sbCamRollMin;
    QSpinBoxEx *_sbCamPitchMax;
    QSpinBoxEx *_sbCamRollMax;
    QCheckBox *_chkCamAxisXInverse;
    QCheckBox *_chkCamAxisYInverse;
    QLabel *_lblEncoderAutomaticTracerMultiplie;
    QDoubleSpinBoxEx *_sbEncoderAutomaticTracerMultiplier;
    QSpinBoxEx *_sbFixedPosLandingYaw;
    QSpinBoxEx *_sbFixedPosLandingPitch;
    QSpinBoxEx *_sbFixedPosLandingRoll;
    QCheckBox *_chkFixedPosLandingCommand;
    QSpinBoxEx *_sbFixedPosBeginingYaw;
    QSpinBoxEx *_sbFixedPosBeginingPitch;
    QSpinBoxEx *_sbFixedPosBeginingRoll;
    QSpinBoxEx *_sbFixedPosVerticalYaw;
    QSpinBoxEx *_sbFixedPosVerticalPitch;
    QSpinBoxEx *_sbFixedPosVerticalRoll;

    void initBindings();
    void initWidgets();

    QWidget *createGimbalWidgets();
    QWidget *createConnectionWidgets(int connectionId);


    QWidget *createFunctionsWidgets();
    void addSeparatorRow(QGridLayout *camControlsGrid, int &row);

    QRadioButton *addVideoSourceRadioButton(QButtonGroupExt *gbVideoSource, VideoFrameTrafficSources source);
public:
    explicit CameraSettingsEditor(QWidget *parent, const qint32 camIdx);
    virtual void accept();

    void loadSettings();
    void saveSettings();

    static QComboBoxExt *createCamListCombo(QWidget *parent);

    static const QString getCameraInfo(qint32 camIdx);
private slots:
    void onEditOpticalDeviceASettingsClicked();
    void onEditOpticalDeviceBSettingsClicked();
    void onEditOpticalDeviceCSettingsClicked();
    void onVideoSourceSelected(int id);
signals:
    void onCamInfoUpdated();
};

#endif // CAMERASETTINGSEDITOR_H
