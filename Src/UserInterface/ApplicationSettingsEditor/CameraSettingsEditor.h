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

    QComboBoxExt *_cbCameraSelector;
    QLineEdit *_edtCamDescription;

    QButtonGroupExt *_gbVideoSource;
    QComboBoxExt *_cbUSBCamera;

    NetworkAddressEditor *_naeXPlane;

    QLineEdit *_edYurionUDPPort;
    QLabel *_lblYurionUDPPort;
    QComboBoxExt *_cbCalibrationImagePath;
    QLineEdit *_edRTSPUrl;

    QLineEdit *_edMUSV2UDPPort;
    QLabel *_lblMUSV2UDPPort;

    FilePathSelector *_fpsVideoFile;

    QCheckBox *_chkOnboardRecording;
    QCheckBox *_chkSnapshot;

    QButtonGroupExt *_gbCameraType;
    QSpinBox *_sbFixedCamPitch;
    QSpinBox *_sbFixedCamRoll;
    QSpinBox *_sbFixedCamZoom;

    QComboBoxExt *_cbCameraControlMode;
    QSpinBox *_sbCamPitchMin;
    QSpinBox *_sbCamRollMin;
    QSpinBox *_sbCamZoomMin;
    QSpinBox *_sbCamPitchMax;
    QSpinBox *_sbCamRollMax;
    QSpinBox *_sbCamZoomMax;
    QCheckBox *_chkCamAxisXInverse;
    QCheckBox *_chkCamAxisYInverse;
    QLabel *_lblEncoderAutomaticTracerMultiplie;
    QDoubleSpinBox *_sbEncoderAutomaticTracerMultiplier;
    QSpinBox *_sbFixedPosLandingYaw;
    QSpinBox *_sbFixedPosLandingPitch;
    QSpinBox *_sbFixedPosLandingRoll;
    QSpinBox *_sbFixedPosLandingZoom;
    QCheckBox *_chkFixedPosLandingCommand;

    QSpinBox *_sbFixedPosBeginingYaw;
    QSpinBox *_sbFixedPosBeginingPitch;
    QSpinBox *_sbFixedPosBeginingRoll;
    QSpinBox *_sbFixedPosBeginingZoom;
    QSpinBox *_sbFixedPosVerticalYaw;
    QSpinBox *_sbFixedPosVerticalPitch;
    QSpinBox *_sbFixedPosVerticalRoll;
    QSpinBox *_sbFixedPosVerticalZoom;

    void initBindings();
    void initWidgets();

    QWidget *createGimbalWidgets();
    QWidget *createConnectionWidgets();
    QWidget *createFunctionsWidgets();
    void addSeparatorRow(QGridLayout *camControlsGrid, int &row);

    QRadioButton *addVideoSourceRadioButton(VideoFrameTrafficSources source);
public:
    explicit CameraSettingsEditor(QWidget *parent, const qint32 camIdx);
    virtual void accept();

    void loadSettings();
    void saveSettings();

    static QComboBoxExt *createCamListCombo(QWidget *parent);

    static const QString getSourceCaption(VideoFrameTrafficSources source);
    static const QString getCameraInfo(qint32 camIdx);
private slots:
    void onEditPrimaryCamSettingsClicked();
    void onEditSecondaryCamSettingsClicked();
    void onVideoSourceSelected(int id);
signals:
    void onCamInfoUpdated();
};

#endif // CAMERASETTINGSEDITOR_H
