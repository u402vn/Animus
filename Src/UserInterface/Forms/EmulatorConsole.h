#ifndef EmulatorConsole_H
#define EmulatorConsole_H

#include <QObject>
#include <QWidget>
#include <QGridLayout>
#include <QDoubleSpinBox>
#include "TelemetryDataFrame.h"
#include "PreferenceAssociation.h"

class EmulatorConsole : public QWidget
{
    Q_OBJECT

    QGridLayout *_mainLayout;
    QDoubleSpinBox *_sbUavRoll;
    QDoubleSpinBox *_sbUavPitch;
    QDoubleSpinBox *_sbUavYaw;
    QDoubleSpinBox *_sbUavGpsLat;
    QDoubleSpinBox *_sbUavGpsLon;
    QDoubleSpinBox *_sbUavGpsHmsl;
    QDoubleSpinBox *_sbUavGpsCourse;

    PreferenceAssociation _association;

    QDoubleSpinBox *addDoubleSpinBox(int row, const QString &caption, double minValue, double maxValue, double step, int decimals, ApplicationPreferenceDouble *preference);
public:    
    explicit EmulatorConsole(QWidget *parent);
    ~EmulatorConsole();
protected:
    void virtual closeEvent(QCloseEvent *event);
private slots:
    void onValueChanged(double value);
signals:
    void onEmulatorTelemetryDataFrame(const EmulatorTelemetryDataFrame &emulatorTelemetryFrame);
};

#endif // EmulatorConsole_H
