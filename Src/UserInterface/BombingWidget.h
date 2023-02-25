#ifndef BOMBINGWIDGET_H
#define BOMBINGWIDGET_H

#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QListWidget>
#include "HardwareLink/HardwareLink.h"
#include "PFD.h"
#include "UserInterface/GPSCoordSelector.h"
#include "Map/MarkerStorage.h"
#include "TelemetryDataStorage.h"

class QMarkerListWidgetItem;

class QMarkerListWidget final : public QListWidget
{
    Q_OBJECT
    GPSCoordSelector * _gpsCoordSelector;

    void keyPressEvent(QKeyEvent *event);
    void deleteSelectedMarkers();
    void highlightMarker();

    void mouseDoubleClickEvent(QMouseEvent * event);
    void mousePressEvent(QMouseEvent *event);

    QMarkerListWidgetItem *selectedItem();
public:
    explicit QMarkerListWidget(QWidget * parent);
    QMarkerListWidgetItem *findMarkerItemByGUID(const QString &markerGUID);
    QMarkerListWidgetItem *addMapMarker(TargetMapMarker *marker);
    void removeMarker(const QString &markerGUID);
    void showCoordEditor();
private slots:
    void onCoordSelectorChanged(const WorldGPSCoord &gpsCoord, const QString &description);
};

class QMarkerListWidgetItem final : public QObject, public QListWidgetItem
{
    Q_OBJECT
    friend class QMarkerListWidget;

    TargetMapMarker *_mapMarker;
    explicit QMarkerListWidgetItem(TargetMapMarker *markerItem, QListWidget *parent);

    void updateToolTip();
    void updateImage();
public:
    ~QMarkerListWidgetItem();
    TargetMapMarker *mapMarker();
private slots:
    void onCoodChanged();
    void onDisplayedImageChanged();
    void onDescriptionChanged();
};

class BombingWidget final : public QWidget
{
    Q_OBJECT

    PFD *_PFD;

    QMarkerListWidget *_lwTargetMapMarkers;

    HardwareLink *_hardwareLink;

    TelemetryDataStorage *_telemetryDataStorage;

    TelemetryDataFrame _telemetryFrame;

    QString _highlightedMarkerGUID;

    void initWidgets();

    void loadTargetMapMarkers();
    void addNewMarker(const WorldGPSCoord &coord, bool showEditor);
public:
    explicit BombingWidget(QWidget *parent, HardwareLink *hardwareLink, TelemetryDataStorage *telemetryDataStorage);
    void processTelemetry(const TelemetryDataFrame &telemetryDataFrame);
private slots:
    void onMapMarkerDeleted(const QString &markerGUID);
    void onMapMarkerCreated(const QString &markerGUID);
    void onMapMarkerHighlightedChanged(const QString &markerGUID, bool isHighlighted);
    void onMapMarkerCoordChanged(const QString &markerGUID, const WorldGPSCoord &coord);
public slots:
    void onDropBombClicked();
    void onSendHitCoordinatesClicked();
    void onSendWeatherClicked();
    void onNewMarkerForTargetClicked();
    void onNewMarkerForUAVClicked();
    void onNewMarkerForLaserClicked();
};

#endif // BOMBINGWIDGET_H
