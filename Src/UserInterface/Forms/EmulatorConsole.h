#ifndef EmulatorConsole_H
#define EmulatorConsole_H

#include <QObject>
#include <QWidget>
#include <QGridLayout>
#include "Common/CommonWidgets.h"
#include "TelemetryDataFrame.h"
#include "PreferenceAssociation.h"

class EmulatorConsole : public QWidget
{
    Q_OBJECT

    QGridLayout *_mainLayout;
    QDoubleSpinBoxEx *_sbUavRoll;
    QDoubleSpinBoxEx *_sbUavPitch;
    QDoubleSpinBoxEx *_sbUavYaw;
    QDoubleSpinBoxEx *_sbUavGpsLat;
    QDoubleSpinBoxEx *_sbUavGpsLon;
    QDoubleSpinBoxEx *_sbUavGpsHmsl;
    QDoubleSpinBoxEx *_sbUavGpsCourse;

    PreferenceAssociation _association;

    QDoubleSpinBoxEx *addDoubleSpinBox(int row, const QString &caption, double minValue, double maxValue, double step, int decimals, ApplicationPreferenceDouble *preference);
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
