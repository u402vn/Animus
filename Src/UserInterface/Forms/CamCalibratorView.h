#ifndef CAMCALIBRATORVIEW_H
#define CAMCALIBRATORVIEW_H

#include <QObject>
#include <QWidget>
#include <QDialog>
#include <QImage>
#include <QSpinBox>
#include <QProgressBar>
#include <QPushButton>
#include "Common/CommonWidgets.h"
#include "HardwareLink/VideoLink.h"
#include "ImageProcessor/CamCalibrator.h"

class CamCalibratorView : public QDialog
{
    Q_OBJECT
private:
    QImage _frame;

    SimpleVideoLink * _videoLink;
    CamCalibrator * _camCalibrator;

    QComboBoxExt * _cmbPatternSize;
    QDoubleSpinBoxEx * _sbSquareLength;
    QComboBoxExt * _cmbRequiredFrameCount;
    QComboBoxExt * _cmbScanInterval;

    QProgressBar *_prgressBar;

    QPushButton * _btnStart;
    QPushButton * _btnStop;

    void updateControlsForState(bool started);
protected:
    void paintEvent(QPaintEvent *event);
public:
    explicit CamCalibratorView(QWidget *parent);
    ~CamCalibratorView();
private slots:
    void onVideoFrameReceived(const QImage &frame);
    void onProgressChanged(int processedPercent);
    void onCalibrationCompleted();
    void startCalibration();
    void stopCalibration();
};

#endif // CAMCALIBRATORVIEW_H
