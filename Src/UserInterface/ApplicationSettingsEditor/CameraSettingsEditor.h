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

    //QComboBoxExt *_cbCameraSelector;

    QComboBoxExt *_cbOpticalDevicesCount;
    QLineEdit *_edtCamDescription;

    //QButtonGroupExt *_gbVideoSource;
    //QComboBoxExt *_cbUSBCamera;

    //NetworkAddressEditor *_naeXPlane;

    //QLineEdit *_edYurionUDPPort;
    //QLabel *_lblYurionUDPPort;
    //QComboBoxExt *_cbCalibrationImagePath;
    //QLineEdit *_edRTSPUrl;

    //QLineEdit *_edMUSV2UDPPort;
    //QLabel *_lblMUSV2UDPPort;

    //FilePathSelector *_fpsVideoFile;

    QCheckBox *_chkOnboardRecording;
    QCheckBox *_chkSnapshot;

    QButtonGroupExt *_gbCameraType;
    QSpinBoxEx *_sbFixedCamPitch;
    QSpinBoxEx *_sbFixedCamRoll;
    QSpinBoxEx *_sbFixedCamZoom;

    QComboBoxExt *_cbCameraControlMode;
    QSpinBoxEx *_sbCamPitchMin;
    QSpinBoxEx *_sbCamRollMin;
    QSpinBoxEx *_sbCamZoomMin;
    QSpinBoxEx *_sbCamPitchMax;
    QSpinBoxEx *_sbCamRollMax;
    QSpinBoxEx *_sbCamZoomMax;
    QCheckBox *_chkCamAxisXInverse;
    QCheckBox *_chkCamAxisYInverse;
    QLabel *_lblEncoderAutomaticTracerMultiplie;
    QDoubleSpinBoxEx *_sbEncoderAutomaticTracerMultiplier;
    QSpinBoxEx *_sbFixedPosLandingYaw;
    QSpinBoxEx *_sbFixedPosLandingPitch;
    QSpinBoxEx *_sbFixedPosLandingRoll;
    QSpinBoxEx *_sbFixedPosLandingZoom;
    QCheckBox *_chkFixedPosLandingCommand;

    QSpinBoxEx *_sbFixedPosBeginingYaw;
    QSpinBoxEx *_sbFixedPosBeginingPitch;
    QSpinBoxEx *_sbFixedPosBeginingRoll;
    QSpinBoxEx *_sbFixedPosBeginingZoom;
    QSpinBoxEx *_sbFixedPosVerticalYaw;
    QSpinBoxEx *_sbFixedPosVerticalPitch;
    QSpinBoxEx *_sbFixedPosVerticalRoll;
    QSpinBoxEx *_sbFixedPosVerticalZoom;

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
