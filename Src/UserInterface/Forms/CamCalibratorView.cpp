#include "CamCalibratorView.h"
#include <QGridLayout>
#include <QPainter>
#include "ApplicationSettings.h"
#include "EnterProc.h"

const int FIXED_CONTROLS_COLUMN_WIDTH = 120;

void CamCalibratorView::startCalibration()
{
    EnterProcStart("CalibrationView::startCalibration");

    _prgressBar->setValue(0);
    updateControlsForState(true);

    CamCalibratorParams params;
    params.scanInterval = _cmbScanInterval->currentData().toInt();
    params.patternSize = _cmbPatternSize->currentData().toInt();
    params.requiredFrameCount = _cmbRequiredFrameCount->currentData().toInt();
    params.squareLength = _sbSquareLength->value();

    _camCalibrator->start(params);
}

void CamCalibratorView::stopCalibration()
{
    _camCalibrator->stop();

    updateControlsForState(false);
}

void CamCalibratorView::updateControlsForState(bool started)
{
    _cmbPatternSize->setEnabled(!started);
    _sbSquareLength->setEnabled(!started);
    _cmbRequiredFrameCount->setEnabled(!started);
    _cmbScanInterval->setEnabled(!started);

    _prgressBar->setVisible(started);

    _btnStart->setEnabled(!started);
    _btnStop->setEnabled(started);
}

void CamCalibratorView::paintEvent(QPaintEvent *event)
{
    QSize frameSize = _frame.size();
    QSize viewSize = this->size();
    viewSize.setWidth(viewSize.width() - FIXED_CONTROLS_COLUMN_WIDTH);

    frameSize.scale(viewSize, Qt::KeepAspectRatio);
    QRect screenViewRect = QRect(QPoint(), frameSize);

    QPoint viewCenter = this->rect().center();
    viewCenter.setX(viewCenter.x() - FIXED_CONTROLS_COLUMN_WIDTH);
    screenViewRect.moveCenter(viewCenter);
    QRect sourceFrameRect = _frame.rect();

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.drawImage(screenViewRect, _frame, sourceFrameRect);
}

CamCalibratorView::CamCalibratorView(QWidget *parent) : QDialog(parent)
{
    EnterProcStart("CalibrationView::CalibrationView");

    qRegisterMetaType<CamCalibratorParams>("CamCalibratorParams");

    //Dialog Form
    this->setWindowTitle(tr("Camera Calibration"));
    this->setModal(true);
    CommonWidgetUtils::updateWidgetGeometry(this, 1000);
    this->setMinimumSize(640, 480);
    this->setContentsMargins(0, 0, 0, 0);
    this->setAttribute(Qt::WA_DeleteOnClose, true);

    _videoLink = new SimpleVideoLink(this);
    _videoLink->openVideoSource();
    connect(_videoLink, &SimpleVideoLink::videoFrameReceived, this, &CamCalibratorView::onVideoFrameReceived, Qt::QueuedConnection);

    _camCalibrator = new CamCalibrator(this);
    connect(_videoLink, &SimpleVideoLink::videoFrameReceived, _camCalibrator, &CamCalibrator::processFrame, Qt::QueuedConnection);
    connect(_camCalibrator, &CamCalibrator::progressChanged, this, &CamCalibratorView::onProgressChanged, Qt::QueuedConnection);
    connect(_camCalibrator, &CamCalibrator::calibrationCompleted, this, &CamCalibratorView::onCalibrationCompleted, Qt::QueuedConnection);

    QLabel * lblPatternSize = new QLabel(tr("Pattern Size:"), this);
    _cmbPatternSize = new QComboBoxExt(this);
    for (int i = 7; i < 14; i++)
        _cmbPatternSize->addItem(QString("%1 x %1").arg(i), i - 1);
    _cmbPatternSize->setCurrentData(7);

    QLabel * lblSquareLength = new QLabel(tr("Cell Length (mm):"), this);
    _sbSquareLength = new QDoubleSpinBoxEx(this); // createDoubleRangeSpinbox
    _sbSquareLength->setRange(10, 200);
    _sbSquareLength->setValue(20);

    QLabel * lblRequiredFrameCount = new QLabel(tr("Frame Count:"), this);
    _cmbRequiredFrameCount = new QComboBoxExt(this);
    for (int i = 20; i < 40; i += 5)
        _cmbRequiredFrameCount->addItem(QString("%1").arg(i), i);
    _cmbRequiredFrameCount->setCurrentData(30);

    QLabel * lblScanFreq  = new QLabel(tr("Scan Frequency:"), this);
    _cmbScanInterval = new QComboBoxExt(this);
    _cmbScanInterval->addItem(QString("Low"), 45);
    _cmbScanInterval->addItem(QString("Normal"), 30);
    _cmbScanInterval->addItem(QString("Fast"), 15);
    _cmbScanInterval->setCurrentData(30);

    _prgressBar = new QProgressBar(this);
    _prgressBar->setAlignment(Qt::AlignCenter);
    _prgressBar->setRange(0, 100);

    _btnStart = new QPushButton(tr("Start"), this);
    connect(_btnStart, &QPushButton::clicked, this, &CamCalibratorView::startCalibration);
    _btnStop = new QPushButton(tr("Stop"), this);
    connect(_btnStop, &QPushButton::clicked, this, &CamCalibratorView::stopCalibration);
    QPushButton * _btnSave = new QPushButton(tr("Save"), this);
    connect(_btnSave, &QPushButton::clicked, this, &CamCalibratorView::accept);
    QPushButton * _btnCancel = new QPushButton(tr("Cancel"), this);
    connect(_btnCancel, &QPushButton::clicked, this, &CamCalibratorView::reject);

    updateControlsForState(false);

    QGridLayout * mainGrid = new QGridLayout();
    //mainGrid->setMargin(0);
    //mainGrid->setSpacing(0);
    mainGrid->setColumnStretch(1, 1);
    mainGrid->setColumnStretch(2, 0);
    mainGrid->setColumnMinimumWidth(2, FIXED_CONTROLS_COLUMN_WIDTH);
    this->setLayout(mainGrid);

    int row = 0;

    mainGrid->addWidget(lblPatternSize,            row, 2, 1, 1);
    mainGrid->setRowStretch(row, 0);
    row++;

    mainGrid->addWidget(_cmbPatternSize,           row, 2, 1, 1);
    mainGrid->setRowStretch(row, 0);
    row++;

    mainGrid->addWidget(lblSquareLength,           row, 2, 1, 1);
    mainGrid->setRowStretch(row, 0);
    row++;

    mainGrid->addWidget(_sbSquareLength,           row, 2, 1, 1);
    mainGrid->setRowStretch(row, 0);
    row++;

    mainGrid->addWidget(lblRequiredFrameCount,     row, 2, 1, 1);
    mainGrid->setRowStretch(row, 0);
    row++;

    mainGrid->addWidget(_cmbRequiredFrameCount,    row, 2, 1, 1);
    mainGrid->setRowStretch(row, 0);
    row++;

    mainGrid->addWidget(lblScanFreq,               row, 2, 1, 1);
    mainGrid->setRowStretch(row, 0);
    row++;

    mainGrid->addWidget(_cmbScanInterval,          row, 2, 1, 1);
    mainGrid->setRowStretch(row, 0);
    row++;

    mainGrid->setRowStretch(row, 1);
    row++;


    mainGrid->addWidget(_prgressBar,               row, 2, 1, 1);
    mainGrid->setRowStretch(row, 0);
    row++;

    mainGrid->addWidget(_btnStart,                 row, 2, 1, 1);
    mainGrid->setRowStretch(row, 0);
    row++;

    mainGrid->addWidget(_btnStop,                  row, 2, 1, 1);
    mainGrid->setRowStretch(row, 0);
    row++;

    mainGrid->addWidget(_btnSave,                  row, 2, 1, 1);
    mainGrid->setRowStretch(row, 0);
    row++;

    mainGrid->addWidget(_btnCancel,                row, 2, 1, 1);
    mainGrid->setRowStretch(row, 0);
    row++;
}

CamCalibratorView::~CamCalibratorView()
{
    delete _camCalibrator;
}

void CamCalibratorView::onVideoFrameReceived(const QImage &frame)
{
    _frame = frame.copy();
    this->update();
}

void CamCalibratorView::onProgressChanged(int processedPercent)
{
    _prgressBar->setValue(processedPercent);
}

void CamCalibratorView::onCalibrationCompleted()
{
    stopCalibration();
}

