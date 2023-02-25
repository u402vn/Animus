#ifndef UAVSIMMAINWINDOW_H
#define UAVSIMMAINWINDOW_H

#include <QMainWindow>
#include <QUdpSocket>
#include <QTimerEvent>
#include <QPushButton>
#include <QDoubleSpinBox>
#include <QCheckBox>
#include "UAVSimDataSender.h"

class UAVSimMainWindow : public QMainWindow
{
    Q_OBJECT

    QUdpSocket _udpCommandsSocket;
    UAVSimDataSender * _dataSender;

    QPushButton * _btnPauseFlight;

    QDoubleSpinBox * _sbUavRoll;
    QDoubleSpinBox * _sbUavPitch;
    QDoubleSpinBox * _sbUavYaw;
    QDoubleSpinBox * _sbCamRoll;
    QDoubleSpinBox * _sbCamPitch;
    QDoubleSpinBox * _sbCamYaw;
    QDoubleSpinBox * _sbUavGpsLat;
    QDoubleSpinBox * _sbUavGpsLon;
    QDoubleSpinBox * _sbUavGpsHmsl;
    QDoubleSpinBox * _sbUavGpsCourse;
    QDoubleSpinBox * _sbWindDirection;
    QDoubleSpinBox * _sbWindSpeed;
    QDoubleSpinBox * _sbRangefinderDistance;
    QCheckBox * _chkBombState;

    QCheckBox * _chkUavRoll;
    QCheckBox * _chkUavPitch;
    QCheckBox * _chkUavYaw;
    QCheckBox * _chkCamRoll;
    QCheckBox * _chkCamPitch;
    QCheckBox * _chkCamYaw;
    QCheckBox * _chkUavGpsLat;
    QCheckBox * _chkUavGpsLon;
    QCheckBox * _chkUavGpsHmsl;
    QCheckBox * _chkUavGpsCourse;
    QCheckBox * _chkWindDirection;
    QCheckBox * _chkWindSpeed;
    QCheckBox * _chkRangefinderDistance;

    QCheckBox * createCheckBox(const QString &caption);
    QDoubleSpinBox * createSpinBox(double min = -180, double max = 180, double step = 0.1);

    void initWidgets();
    void setupFreeze();
public:
    UAVSimMainWindow(QWidget *parent);
    ~UAVSimMainWindow();
private slots:
    void processCommandsDatagrams();
    void onPauseFlight();


    void onFreezeChecked(bool value);
    void onBombChecked(bool value);
    void onFreezeValueChanged(double value);
};

#endif // UAVSIMMAINWINDOW_H
