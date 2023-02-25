#include "PFD.h"
#include "EnterProc.h"

PFD::PFD(QWidget *parent) : qfi_PFD(parent)
{
    EnterProcStart("PFD::PFD");

    setInteractive(false);
    setFocusPolicy(Qt::NoFocus);
    setDragMode(QGraphicsView::NoDrag);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setContextMenuPolicy(Qt::CustomContextMenu);

    setContentsMargins(0, 0, 0, 0);
    //setMaximumWidth(500);

    QSizePolicy policy(sizePolicy());
    policy.setHeightForWidth(true);
    //policy.setHorizontalPolicy(QSizePolicy::MinimumExpanding);
    //policy.setVerticalPolicy(QSizePolicy::MinimumExpanding);
    setSizePolicy(policy);
}

void PFD::showTelemetryDataFrame(const TelemetryDataFrame &telemetryFrame)
{
    EnterProcStart("PFD::showTelemetryDataFrame");

    setAirspeed(telemetryFrame.AirSpeed);
    setMachNo(telemetryFrame.AirSpeed / 650.0f);
    setHeading(telemetryFrame.UavYaw);
    setRoll(telemetryFrame.UavRoll);
    setPitch(telemetryFrame.UavPitch);
    setAltitude(telemetryFrame.UavAltitude_Barometric);
    setClimbRate(telemetryFrame.VerticalSpeed);

    setFlightPathMarker(0, 0, false);
    setBarH(0, false);
    setBarV(0, false);
    setDotH(0, false);
    setDotV(0, false);

    if (!visibleRegion().isEmpty())
        update();
}
