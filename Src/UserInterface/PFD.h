#ifndef PFD_H
#define PFD_H

#include <QObject>
#include <QWidget>
#include "UserInterface/Instruments/qfi_PFD.h"
#include "TelemetryDataFrame.h"

class PFD final : public qfi_PFD
{
    Q_OBJECT
public:
    PFD(QWidget *parent);

    void showTelemetryDataFrame(const TelemetryDataFrame &telemetryFrame);
};

#endif // PFD_H
