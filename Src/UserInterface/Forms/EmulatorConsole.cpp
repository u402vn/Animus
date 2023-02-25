#include "EmulatorConsole.h"
#include <QLabel>
#include "Common/CommonWidgets.h"
#include "Common/CommonUtils.h"
#include "ApplicationSettings.h"

QDoubleSpinBox *EmulatorConsole::addDoubleSpinBox(int row, const QString &caption, double minValue, double maxValue, double step, int decimals, ApplicationPreferenceDouble *preference)
{
    auto label = new QLabel(caption, this);
    auto spinBox = CommonWidgetUtils::createDoubleRangeSpinbox(this, minValue, maxValue, step, decimals);
    _association.addBinding(preference, spinBox);
    connect(spinBox, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), this, &EmulatorConsole::onValueChanged);

    _mainLayout->addWidget(label,              row, 0, 1, 1);
    _mainLayout->addWidget(spinBox,            row, 1, 1, 1);
    return spinBox;
}

EmulatorConsole::EmulatorConsole(QWidget *parent) : QWidget(parent),
    _association(this)
{
    this->setWindowTitle(tr("Emulator Console"));
    this->setWindowFlags(this->windowFlags() | Qt::WindowStaysOnTopHint | Qt::WindowMinMaxButtonsHint);

    CommonWidgetUtils::updateWidgetGeometry(this, 300, 200);
    this->setMaximumSize(this->minimumSize());

    ApplicationSettings& applicationSettings = ApplicationSettings::Instance();

    _mainLayout = new QGridLayout();
    this->setLayout(_mainLayout);

    int row = 0;
    _sbUavRoll      = addDoubleSpinBox(row++, tr("UAV Roll"),       -360,   +360,       0.01,   2, &(applicationSettings.EmulatorConsoleUavRoll));
    _sbUavPitch     = addDoubleSpinBox(row++, tr("UAV Pitch"),      -360,   +360,       0.01,   2, &(applicationSettings.EmulatorConsoleUavPitch));
    _sbUavYaw       = addDoubleSpinBox(row++, tr("UAV Yaw"),        -360,   +360,       0.01,   2, &(applicationSettings.EmulatorConsoleUavYaw));
    _sbUavGpsLat    = addDoubleSpinBox(row++, tr("Latitude"),       -360,   +360,   0.000001,   6, &(applicationSettings.EmulatorConsoleGpsLat));
    _sbUavGpsLon    = addDoubleSpinBox(row++, tr("Longitude"),      -360,   +360,   0.000001,   6, &(applicationSettings.EmulatorConsoleGpsLon));
    _sbUavGpsHmsl   = addDoubleSpinBox(row++, tr("Altitude (GPS)"), -100,  +1000,        0.1,   1, &(applicationSettings.EmulatorConsoleGpsHmsl));
    _sbUavGpsCourse = addDoubleSpinBox(row++, tr("Course"),         -360,   +360,       0.01,   2, &(applicationSettings.EmulatorConsoleGpsCourse));

    _mainLayout->setRowStretch(row++, 1);
    row++;

    _association.toEditor();
}

EmulatorConsole::~EmulatorConsole()
{

}

void EmulatorConsole::closeEvent(QCloseEvent *event)
{
    Q_UNUSED(event)

    ApplicationSettings& applicationSettings = ApplicationSettings::Instance();
    _association.fromEditor();
    applicationSettings.savePreferences();

    QWidget::closeEvent(event);
}

void EmulatorConsole::onValueChanged(double value)
{
    Q_UNUSED(value)

    EmulatorTelemetryDataFrame emulatorTelemetryFrame;
    emulatorTelemetryFrame.UavRoll = _sbUavRoll->value();
    emulatorTelemetryFrame.UavPitch = _sbUavPitch->value();
    emulatorTelemetryFrame.UavYaw = _sbUavYaw->value();
    emulatorTelemetryFrame.UavLatitude_GPS = _sbUavGpsLat->value();
    emulatorTelemetryFrame.UavLongitude_GPS = _sbUavGpsLon->value();
    emulatorTelemetryFrame.UavAltitude_GPS = _sbUavGpsHmsl->value();
    emulatorTelemetryFrame.Course_GPS = _sbUavGpsCourse->value();

    emit onEmulatorTelemetryDataFrame(emulatorTelemetryFrame);
}
