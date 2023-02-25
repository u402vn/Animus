#ifndef GPSCOORDSELECTOR_H
#define GPSCOORDSELECTOR_H

#include <QWidget>
#include <QFrame>
#include <QSpinBox>
#include <QTextEdit>
#include "Common/CommonData.h"

class GPSCoordSelector final : public QFrame
{
    Q_OBJECT

    bool _initInProgress;

    QLineEdit * _edDescription;
    QDoubleSpinBox * _sbTargetLat;
    QDoubleSpinBox * _sbTargetLon;
    QDoubleSpinBox * createGeoCoordSpinBox(double min, double max);

    void processChanges();
protected:
    void focusOutEvent(QFocusEvent * event);
public:
    explicit GPSCoordSelector(QWidget *parent);
    ~GPSCoordSelector();
    void show(const QPoint &screenPos, const WorldGPSCoord &gpsCoord, const QString &description);
    void setDescriptionVisible(bool visible);
signals:
    void onCoordSelectorChanged(const WorldGPSCoord &gpsCoord, const QString &description);
private slots:
    void onCoordEditorChanged(double value);
    void onTextEditorChanged();
};

#endif // GPSCOORDSELECTOR_H
