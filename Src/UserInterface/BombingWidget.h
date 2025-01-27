#ifndef BOMBINGWIDGET_H
#define BOMBINGWIDGET_H

#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QListWidget>
#include <QStyledItemDelegate>
#include "HardwareLink/HardwareLink.h"
#include "HardwareLink/BombingController.h"
#include "PFD.h"
#include "UserInterface/GPSCoordSelector.h"
#include "TelemetryDataFrame.h"
#include "TelemetryDataStorage.h"
#include "Forms/WeatherView.h"
#include "Map/ArtillerySpotter.h"

class QMarkerListWidgetItem;

class QMarkerListWidget final : public QListWidget
{
    friend class MarkerStyledItemDelegate;

    Q_OBJECT
    TelemetryDataFrame _telemetryFrame;
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
    QMarkerListWidgetItem *addMapMarker(MapMarker *marker);
    void removeMarker(const QString &markerGUID);
    void showCoordEditor();
    void processTelemetry(const TelemetryDataFrame &telemetryDataFrame);
private slots:
    void onCoordSelectorChanged(const WorldGPSCoord &gpsCoord, const QString &description);
};

class QMarkerListWidgetItem final : public QObject, public QListWidgetItem
{
    friend class QMarkerListWidget;

    Q_OBJECT

    MapMarker *_mapMarker;
    explicit QMarkerListWidgetItem(MapMarker *markerItem, QListWidget *parent);

    void updateToolTip();
    void updateImage();
public:
    ~QMarkerListWidgetItem();
    MapMarker *mapMarker();
private slots:
    void onCoodChanged();
    void onDisplayedImageChanged();
    void onDescriptionChanged();
};

class MarkerStyledItemDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    using QStyledItemDelegate::QStyledItemDelegate;
    void paint(QPainter *painter, const QStyleOptionViewItem &option,
               const QModelIndex &index) const override;
};


class BombingWidget final : public QWidget
{
    Q_OBJECT

    PFD *_PFD;

    QMarkerListWidget *_lwTargetMapMarkers;
    WeatherView *_weatherView;

    HardwareLink *_hardwareLink;
    ArtillerySpotter *_artillerySpotter;
    BombingController *_bombingController;

    TelemetryDataStorage *_telemetryDataStorage;
    TelemetryDataFrame _telemetryFrame;

    QString _highlightedMarkerGUID;

    void initWidgets();

    void loadTargetMapMarkers();
    void addNewMarker(const QList<WorldGPSCoord> &coords, bool showEditor);
    void addNewMarker(const WorldGPSCoord &coord, bool showEditor);
public:
    explicit BombingWidget(QWidget *parent, HardwareLink *hardwareLink, ArtillerySpotter *artillerySpotter, BombingController *bombingController,
                           TelemetryDataStorage *telemetryDataStorage);
    ~BombingWidget();
    void processTelemetry(const TelemetryDataFrame &telemetryDataFrame);
private slots:
    void onMapMarkerDeleted(const QString &markerGUID);
    void onMapMarkerCreated(const QString &markerGUID);
    void onMapMarkerHighlightedChanged(const QString &markerGUID, bool isHighlighted);
    void onMapMarkerCoordChanged(const QString &markerGUID, const WorldGPSCoord &coord);
    void onMessageExchangeInformation(const QString &information, bool isEroor);
    void onNewMarkerButtonRightClicked();
public slots:
    void onDropBombClicked05();
    void onDropBombClicked10();
    void onSendHitCoordinatesClicked();
    void onSendWeatherClicked();
    void onNewMarkerForTargetClicked();
    void onNewMarkerForUAVClicked();
    void onNewMarkerForLaserClicked();
};

#endif // BOMBINGWIDGET_H
