#ifndef CONSTANTNAMES_H
#define CONSTANTNAMES_H

#include <QObject>
#include <QMap>
#include <QString>

class ConstantNames : public QObject
{
    Q_OBJECT
public:
    explicit ConstantNames(QObject *parent);
    ~ConstantNames();

    static const QMap<int, QString> ArtillerySpotterStateCaptions();
    static const QMap<int, QString> TileReceivingModeCaptions();
    static const QMap<int, QString> ColorModeCaptions();
    static const QMap<int, QString> OVRGimbalIndicatorTypeCaptions();
    static const QMap<int, QString> UAVTelemetryFormatCaptions();
    static const QMap<int, QString> CommandProtocolCaptions();
    static const QMap<int, QString> ObjectTrackerTypeCaptions();
    static const QMap<int, QString> ApplicationLanguageCaptions();
    static const QMap<int, QString> ApplicationStyleCaptions();
    static const QMap<int, QString> CameraControlModeCaptions();
    static const QMap<int, QString> StabilizationTypeCaptions();
    static const QMap<int, QString> OSDScreenCenterMarkCaptions();
    static const QMap<int, QString> VideoFrameTrafficSourceCaptions();


};

#endif // CONSTANTNAMES_H
