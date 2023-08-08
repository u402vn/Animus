#include "BombingWidget.h"
#include <QGridLayout>
#include <QListWidgetItem>
#include <QMenu>
#include <QSplitter>
#include <QActionGroup>
#include "ApplicationSettings.h"
#include "EnterProc.h"
#include "Map/MarkerStorage.h"
#include "Common/CommonWidgets.h"
#include "Common/CommonUtils.h"
#include "ConstantNames.h"


void BombingWidget::initWidgets()
{
    ApplicationSettings& applicationSettings = ApplicationSettings::Instance();

    _PFD = new PFD(this);

    _lwTargetMapMarkers = new QMarkerListWidget(this);

    const QString NO_ICON = "";
    const QString NO_CAPTION = "";

    auto btnNewMarkerForTarget = CommonWidgetUtils::createButton(this, NO_CAPTION, applicationSettings.hidUIHint(hidbtnNewMarkerForTarget),
                                                                 false, QUARTER_BUTTON_WIDTH, DEFAULT_BUTTON_HEIGHT, ":/newtargetmarker_T.png");
    connect(btnNewMarkerForTarget, &QPushButton::clicked, this, &BombingWidget::onNewMarkerForTargetClicked);

    QPushButton *btnNewMarkerForLaser = nullptr;
    if (applicationSettings.isLaserRangefinderLicensed())
    {
        btnNewMarkerForLaser = CommonWidgetUtils::createButton(this, NO_CAPTION, applicationSettings.hidUIHint(hidbtnNewMarkerForLaser),
                                                               false, QUARTER_BUTTON_WIDTH, DEFAULT_BUTTON_HEIGHT, ":/newtargetmarker_L.png");
        connect(btnNewMarkerForLaser, &QPushButton::clicked, this, &BombingWidget::onNewMarkerForLaserClicked);
    }

    auto btnNewMarkerForUAV = CommonWidgetUtils::createButton(this, NO_CAPTION, applicationSettings.hidUIHint(hidbtnNewMarkerForUAV),
                                                              false, QUARTER_BUTTON_WIDTH, DEFAULT_BUTTON_HEIGHT, ":/newtargetmarker_B.png");
    connect(btnNewMarkerForUAV, &QPushButton::clicked, this, &BombingWidget::onNewMarkerForUAVClicked);

    QPushButton *btnDropBomb = nullptr;
    if (applicationSettings.isBombingTabLicensed())
    {
        btnDropBomb = CommonWidgetUtils::createButton(this, applicationSettings.hidCaption(hidbtnDropBomb), applicationSettings.hidUIHint(hidbtnDropBomb),
                                                      false,  QWIDGETSIZE_MAX, DEFAULT_BUTTON_HEIGHT, NO_ICON);
        connect(btnDropBomb, &QPushButton::clicked, this, &BombingWidget::onDropBombClicked);
    }

    auto btnSendHitCoordinates = CommonWidgetUtils::createButton(this, applicationSettings.hidCaption(hidbtnSendHitCoordinates), applicationSettings.hidUIHint(hidbtnSendHitCoordinates),
                                                                 false,  QWIDGETSIZE_MAX, DEFAULT_BUTTON_HEIGHT, NO_ICON);
    connect(btnSendHitCoordinates, &QPushButton::clicked, this, &BombingWidget::onSendHitCoordinatesClicked);

    auto btnSendWeather = CommonWidgetUtils::createButton(this, applicationSettings.hidCaption(hidbtnSendWeather), applicationSettings.hidUIHint(hidbtnSendWeather),
                                                          false,  QWIDGETSIZE_MAX, DEFAULT_BUTTON_HEIGHT, NO_ICON);
    connect(btnSendWeather, &QPushButton::clicked, this, &BombingWidget::onSendWeatherClicked);

    auto mainLayout = new QGridLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);

    auto mainSplitter = new QSplitter(Qt::Vertical, this);

    auto markerWidget = new QWidget(this);
    auto markerLayout = new QGridLayout();
    markerLayout->setContentsMargins(0, 0, 0, 0);
    markerWidget->setLayout(markerLayout);

    int row = 0;
    int col = 1;
    if (btnNewMarkerForLaser != nullptr)
        markerLayout->addWidget(btnNewMarkerForLaser,                                 row, col++, 1, 1);
    markerLayout->addWidget(btnNewMarkerForTarget,                                    row, col++, 1, 1);
    markerLayout->addWidget(btnNewMarkerForUAV,                                       row, col++, 1, 1);
    row++;
    markerLayout->addWidget(_lwTargetMapMarkers,                                       row, 1, 1, 4);

    mainSplitter->addWidget(_PFD);
    mainSplitter->addWidget(markerWidget);

    row = 0;
    mainLayout->addWidget(mainSplitter,                                                row, 1, 1, 4);
    row++;

    if (btnDropBomb != nullptr)
    {
        mainLayout->addWidget(CommonWidgetUtils::createSeparator(this),                row, 1, 1, 3);
        mainLayout->setRowMinimumHeight(row, 0.5 * DEFAULT_BUTTON_HEIGHT);
        row++;

        mainLayout->addWidget(btnDropBomb,                                             row, 1, 1, 4);
        row++;
    }

    mainLayout->addWidget(btnSendHitCoordinates,                                       row, 1, 1, 2);
    mainLayout->addWidget(btnSendWeather,                                           row, 3, 1, 2);
    row++;
}

BombingWidget::BombingWidget(QWidget *parent, HardwareLink *hardwareLink, ArtillerySpotter *artillerySpotter, TelemetryDataStorage *telemetryDataStorage) : QWidget(parent),
    _hardwareLink(hardwareLink),
    _artillerySpotter(artillerySpotter),
    _telemetryDataStorage(telemetryDataStorage)
{
    EnterProc("BombingWidget::BombingWidget");

    _weatherView = nullptr;

    MarkerStorage& markerStorage = MarkerStorage::Instance();

    connect(&markerStorage, &MarkerStorage::onMapMarkerDeleted, this, &BombingWidget::onMapMarkerDeleted);
    connect(&markerStorage, &MarkerStorage::onMapMarkerCreated, this, &BombingWidget::onMapMarkerCreated);
    connect(&markerStorage, &MarkerStorage::onTargetMapMarkerHighlightedChanged, this, &BombingWidget::onMapMarkerHighlightedChanged);
    connect(&markerStorage, &MarkerStorage::onMapMarkerCoordChanged, this, &BombingWidget::onMapMarkerCoordChanged);

    connect(_artillerySpotter, &ArtillerySpotter::onMessageExchangeInformation, this, &BombingWidget::onMessageExchangeInformation);

    initWidgets();

    loadTargetMapMarkers();

    CommonWidgetUtils::installEventFilterToApplication(this);
}

BombingWidget::~BombingWidget()
{
    if (_weatherView != nullptr)
        delete _weatherView;
}

void BombingWidget::addNewMarker(const WorldGPSCoord &coord, bool showEditor)
{
    if (coord.isIncorrect())
        return;

    MarkerStorage& markerStorage = MarkerStorage::Instance();
    auto mapMarker = markerStorage.createNewMarker(TargetMarkerTemplateGUID, coord);

    if (showEditor)
    {
        auto markerItem = _lwTargetMapMarkers->findMarkerItemByGUID(mapMarker->GUID());
        _lwTargetMapMarkers->scrollToItem(markerItem);
        markerItem->setSelected(true);
        _lwTargetMapMarkers->showCoordEditor();
    }
}

void BombingWidget::onNewMarkerForUAVClicked()
{
    EnterProc("BombingWidget::onAddNewMarkerForUAVClicked");
    addNewMarker(getUavCoordsFromTelemetry(_telemetryFrame), true);
}

void BombingWidget::onNewMarkerForLaserClicked()
{
    EnterProc("BombingWidget::onAddNewMarkerForLaserClicked");
    addNewMarker(getRangefinderCoordsFromTelemetry(_telemetryFrame), false);
}

void BombingWidget::onNewMarkerForTargetClicked()
{
    EnterProc("BombingWidget::onNewMarkerForTargetClicked");
    addNewMarker(getTrackedTargetCoordsFromTelemetry(_telemetryFrame), false);
}

void BombingWidget::onDropBombClicked()
{
    EnterProc("BombingWidget::onDropBombClicked");
    _hardwareLink->dropBomb(1);
}

void BombingWidget::onSendHitCoordinatesClicked()
{
    MarkerStorage& markerStorage = MarkerStorage::Instance();
    _artillerySpotter->sendMarkers(markerStorage.getMapMarkers());
}

void BombingWidget::onSendWeatherClicked()
{
    if (_weatherView == nullptr)
        _weatherView = new WeatherView(nullptr, _telemetryDataStorage, _artillerySpotter);
    _weatherView->reinit();
    _weatherView->showNormal();
}

void BombingWidget::processTelemetry(const TelemetryDataFrame &telemetryDataFrame)
{
    EnterProcStart("BombingWidget::processTelemetry");

    _telemetryFrame = telemetryDataFrame;
    _lwTargetMapMarkers->processTelemetry(_telemetryFrame);
    _PFD->showTelemetryDataFrame(_telemetryFrame);
}

void BombingWidget::loadTargetMapMarkers()
{
    EnterProcStart("BombingWidget::loadTargetMapMarkers");
    MarkerStorage& markerStorage = MarkerStorage::Instance();

    auto salvoCenterMarker = markerStorage.getSalvoCenterMarker();
    if (salvoCenterMarker != nullptr)
        _lwTargetMapMarkers->addMapMarker(salvoCenterMarker);

    auto mapMarkers = markerStorage.getTargetMapMarkers();
    foreach (auto marker, *mapMarkers)
    {
        _lwTargetMapMarkers->addMapMarker(marker);
        if (marker->isHighlighted())
        {
            _highlightedMarkerGUID = marker->GUID();
            onMapMarkerHighlightedChanged(_highlightedMarkerGUID, true);
        }
    }
}

void BombingWidget::onMapMarkerDeleted(const QString &markerGUID)
{
    EnterProcStart("BombingWidget::onMapMarkerDeleted");
    _lwTargetMapMarkers->removeMarker(markerGUID);

    if (markerGUID == _highlightedMarkerGUID)
        onMapMarkerHighlightedChanged(_highlightedMarkerGUID, false);
}

void BombingWidget::onMapMarkerCreated(const QString &markerGUID)
{
    EnterProcStart("BombingWidget::onMapMarkerCreated");
    MarkerStorage& markerStorage = MarkerStorage::Instance();
    auto marker = markerStorage.getMapMarkerByGUID(markerGUID);

    auto targetMarker = dynamic_cast<TargetMapMarker*>(marker);
    if (targetMarker != nullptr)
        _lwTargetMapMarkers->addMapMarker(targetMarker);
    auto salvoCenterMarker = dynamic_cast<ArtillerySalvoCenterMarker*>(marker);
    if (salvoCenterMarker != nullptr)
    {
        auto markerItem = _lwTargetMapMarkers->addMapMarker(salvoCenterMarker);
        _lwTargetMapMarkers->insertItem(0, markerItem);
    }
}

void BombingWidget::onMapMarkerHighlightedChanged(const QString &markerGUID, bool isHighlighted)
{
    EnterProcStart("BombingWidget::onMapMarkerHighlightedChanged");

    MarkerStorage& markerStorage = MarkerStorage::Instance();
    auto marker = markerStorage.getMapMarkerByGUID(markerGUID);

    if (marker != nullptr && isHighlighted)
    {
        _highlightedMarkerGUID = markerGUID;
        WorldGPSCoord coord = marker->gpsCoord();
        _hardwareLink->setBombingPlacePos(coord.lat, coord.lon, coord.hmsl);
    }
    else
    {
        _highlightedMarkerGUID = "";
        WorldGPSCoord coord;
        coord.setIncorrect();
        _hardwareLink->setBombingPlacePos(coord.lat, coord.lon, coord.hmsl);
    }
}

void BombingWidget::onMapMarkerCoordChanged(const QString &markerGUID, const WorldGPSCoord &coord)
{
    if (_highlightedMarkerGUID == markerGUID)
    {
        _hardwareLink->setBombingPlacePos(coord.lat, coord.lon, coord.hmsl);
    }
}

void BombingWidget::onMessageExchangeInformation(const QString &information, bool isEroor)
{\
    CommonWidgetUtils::showInfoDialogAutoclose(isEroor ?  QMessageBox::Critical : QMessageBox::Information,  information, 3000);
}

QMarkerListWidgetItem::QMarkerListWidgetItem(MapMarker *markerItem, QListWidget *parent) : QListWidgetItem(parent)
{
    _mapMarker = markerItem;
    connect(_mapMarker, &MapMarker::onCoodChanged, this, &QMarkerListWidgetItem::onCoodChanged);
    connect(_mapMarker, &MapMarker::onDisplayedImageChanged, this, &QMarkerListWidgetItem::onDisplayedImageChanged);
    connect(_mapMarker, &MapMarker::onDescriptionChanged, this, &QMarkerListWidgetItem::onDescriptionChanged);

    setText(_mapMarker->description());
    updateImage();
    updateToolTip();
}

void QMarkerListWidgetItem::updateToolTip()
{
    setToolTip(_mapMarker->hint());
}

void QMarkerListWidgetItem::updateImage()
{
    setIcon(_mapMarker->displayedImage());
    setToolTip(_mapMarker->hint());
    setBackgroundColor(MapArtillerySpotterStateColors[_mapMarker->artillerySpotterState()]);
}

QMarkerListWidgetItem::~QMarkerListWidgetItem()
{

}

MapMarker *QMarkerListWidgetItem::mapMarker()
{
    return _mapMarker;
}

void QMarkerListWidgetItem::onCoodChanged()
{
    updateToolTip();
}

void QMarkerListWidgetItem::onDisplayedImageChanged()
{
    updateImage();
}

void QMarkerListWidgetItem::onDescriptionChanged()
{
    setText(_mapMarker->description());
}

void QMarkerListWidget::keyPressEvent(QKeyEvent *event)
{
    switch (event->key())
    {
    case Qt::Key_Delete:
        deleteSelectedMarkers();
        break;
    case Qt::Key_Space:
        highlightMarker();
        break;
    default:
        QListWidget::keyPressEvent(event);
    };
    event->accept();
}

QMarkerListWidgetItem *QMarkerListWidget::selectedItem()
{
    auto selectedItems = this->selectedItems();
    if (selectedItems.isEmpty())
        return nullptr;
    auto firstSelected = selectedItems.first();
    if (firstSelected == nullptr)
        return nullptr;
    auto markerItem = dynamic_cast<QMarkerListWidgetItem*>(firstSelected);
    return markerItem;
}

void QMarkerListWidget::deleteSelectedMarkers()
{
    EnterProcStart("QMarkerListWidget::deleteSelectedMarkers");

    auto markerItem = selectedItem();
    if (markerItem != nullptr)
    {
        auto targetMapMarker = dynamic_cast<TargetMapMarker*> (markerItem->_mapMarker);
        if (targetMapMarker != nullptr)
        {
            MarkerStorage& markerStorage = MarkerStorage::Instance();
            markerStorage.deleteMarker(targetMapMarker);
        }
    }
}

void QMarkerListWidget::highlightMarker()
{
    auto markerItem = selectedItem();
    if (markerItem != nullptr)
    {
        auto targetMapMarker = dynamic_cast<TargetMapMarker*> (markerItem->_mapMarker);
        if (targetMapMarker != nullptr)
            targetMapMarker->setHighlighted(!targetMapMarker->isHighlighted());
    }
}

QMarkerListWidget::QMarkerListWidget(QWidget *parent) : QListWidget(parent)
{    
    _gpsCoordSelector = new GPSCoordSelector(this);
    connect(_gpsCoordSelector, &GPSCoordSelector::onCoordSelectorChanged, this, &QMarkerListWidget::onCoordSelectorChanged);
    this->setItemDelegate(new MarkerStyledItemDelegate(this));
}

QMarkerListWidgetItem *QMarkerListWidget::findMarkerItemByGUID(const QString &markerGUID)
{
    int count = this->count();
    for (int i = 0; i < count; i++)
    {
        auto markerItem = dynamic_cast<QMarkerListWidgetItem*>(item(i));
        if (markerItem != nullptr)
            if (markerItem->_mapMarker->GUID() == markerGUID)
                return markerItem;
    }
    return nullptr;
}

QMarkerListWidgetItem *QMarkerListWidget::addMapMarker(MapMarker *marker)
{
    auto result = new QMarkerListWidgetItem(marker, this);
    return result;
}

void QMarkerListWidget::removeMarker(const QString &markerGUID)
{
    EnterProcStart("QMarkerListWidget::removeMarker");
    auto markerItem = findMarkerItemByGUID(markerGUID);
    removeItemWidget(markerItem);
    delete markerItem;
}

void QMarkerListWidget::showCoordEditor()
{
    EnterProcStart("QMarkerListWidget::showCoordEditor");
    auto markerItem = selectedItem();
    if (markerItem != nullptr)
    {
        QRect itemRect = visualItemRect(markerItem);
        QPoint screenPos = mapToGlobal(QPoint(itemRect.left(), itemRect.bottom()));
        _gpsCoordSelector->show(screenPos, markerItem->mapMarker()->gpsCoord(), markerItem->mapMarker()->description());
    }
}

void QMarkerListWidget::processTelemetry(const TelemetryDataFrame &telemetryDataFrame)
{
    _telemetryFrame = telemetryDataFrame;
    if (_telemetryFrame.TelemetryFrameNumber % 20 == 0)
        this->update();
}

void QMarkerListWidget::mouseDoubleClickEvent(QMouseEvent *event)
{
    QListWidget::mouseDoubleClickEvent(event);

    if (event->button() == Qt::LeftButton)
        showCoordEditor();
}

void QMarkerListWidget::mousePressEvent(QMouseEvent *event)
{
    QListWidget::mousePressEvent(event);

    auto markerItem = selectedItem();
    if ((event->button() == Qt::RightButton) && (markerItem != nullptr))
    {
        auto targetMapMarker = dynamic_cast<TargetMapMarker*> (markerItem->_mapMarker);
        if (targetMapMarker == nullptr)
            return;

        QMenu menu;

        auto acActivateStatus = menu.addAction(targetMapMarker->isHighlighted() ? tr("Deactivate") : tr("Activate"));
        auto acChange = menu.addAction(tr("Change"));
        auto acDelete = menu.addAction(tr("Delete"));
        menu.addSeparator();

        QActionGroup artillerySpotterStateGroup(this);

        auto captions = ConstantNames::ArtillerySpotterStateCaptions();

        auto i = captions.begin();
        while (i != captions.end())
        {
            int state = i.key();
            QString description = captions[ArtillerySpotterState(state)];

            bool isChecked = markerItem->_mapMarker->artillerySpotterState() == state;
            CommonWidgetUtils::createCheckableMenuGroupAction(description, isChecked, &artillerySpotterStateGroup, &menu, state);
            ++i;
        }

        auto selectedAction = menu.exec(QCursor::pos());

        if (selectedAction == acDelete)
            deleteSelectedMarkers();
        else if (selectedAction == acActivateStatus)
            highlightMarker();
        else if (selectedAction == acChange)
            showCoordEditor();
        else if (artillerySpotterStateGroup.actions().contains(selectedAction))
        {
            auto state = ArtillerySpotterState(selectedAction->data().toInt());
            markerItem->_mapMarker->setArtillerySpotterState(state);
            ApplicationSettings& applicationSettings = ApplicationSettings::Instance();
            applicationSettings.LastTargetArtillerySpotterState = state;
        }
    }
}

void QMarkerListWidget::onCoordSelectorChanged(const WorldGPSCoord &gpsCoord, const QString &description)
{
    auto markerItem = selectedItem();
    if (markerItem == nullptr)
        return;
    auto mapMarker = markerItem->mapMarker();
    mapMarker->setGPSCoord(gpsCoord);
    mapMarker->setDescription(description);
}

void MarkerStyledItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QStyledItemDelegate::paint(painter, option, index);

    static QPixmap directionImage = QPixmap(":/targetlistdirection.png");

    auto listWidget = dynamic_cast<QMarkerListWidget*>(this->parent());
    auto item = dynamic_cast<QMarkerListWidgetItem*>(listWidget->itemFromIndex(index));

    auto uavCoords = getUavCoordsFromTelemetry(listWidget->_telemetryFrame);
    if (!uavCoords.isIncorrect())
    {
        double distance, azimuth;
        item->mapMarker()->gpsCoord().getDistanceAzimuthTo(uavCoords, distance, azimuth);
        auto itemRect = option.rect; //listWidget->visualItemRect(item);
        auto size = itemRect.height();

        painter->save();
        painter->translate(itemRect.width() - size / 2, itemRect.top() + size / 2);
        painter->rotate(azimuth + 180);
        painter->drawPixmap(QRect(- size / 2, - size / 2, size, size), directionImage);
        painter->restore();

        itemRect.setWidth(itemRect.width() - size);
        painter->drawText(itemRect, Qt::AlignRight, QString::number(distance, 'f', 0));
    }
}
