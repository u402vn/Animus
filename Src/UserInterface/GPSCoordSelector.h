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

    quint32 _initInProgress;

    WorldGPSCoord _intialCoord;
    QString _intialDescription;

    QLineEdit * _edDescription;
    QDoubleSpinBox *_sbCoordLat, *_sbCoordLon;
    QLineEdit *_edtCoordLat, *_edtCoordLon;
    QDoubleSpinBox *createGeoCoordSpinBox(double min, double max);

    void setupCoord(const WorldGPSCoord &gpsCoord, const QString &description);
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
    void onCoordAsNumberChanged(double value);
    void onCoordAsTextChanged(const QString &text);
    void onTextEditorChanged();

};

#endif // GPSCOORDSELECTOR_H
