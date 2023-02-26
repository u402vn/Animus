#include "WeatherView.h"
#include <QPushButton>
#include <QStringList>
#include <QDialogButtonBox>
#include <QVBoxLayout>
#include "Common/CommonWidgets.h"
#include "Common/CommonUtils.h"
#include "Map/MarkerStorage.h"

WeatherView::WeatherView(QWidget *parent, TelemetryDataStorage *telemetryDataStorage) : QDialog(parent),
    _telemetryDataStorage(telemetryDataStorage)
{
    const int WEATHER_VIEW_WIDTH = 800;

    this->setWindowTitle(tr("Weather"));
    this->setModal(true);
    //this->setWindowFlags(this->windowFlags() | Qt::WindowStaysOnTopHint | Qt::WindowMinMaxButtonsHint);

    CommonWidgetUtils::updateWidgetGeometry(this, WEATHER_VIEW_WIDTH, WEATHER_VIEW_WIDTH * 0.5);

    QStringList horizontalHeaderLabels = {tr("Altitude"), tr("Wind (Direction)"), tr("Wind (Speed)"), tr("Pressure"), tr("Temperature")};
    int columnCount = horizontalHeaderLabels.count();

    _weatherTable = new QTableWidget(this);
    _weatherTable->setSelectionMode(QAbstractItemView::NoSelection);
    _weatherTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    _weatherTable->horizontalHeader()->sortIndicatorOrder();
    _weatherTable->setColumnCount(columnCount);
    for (int i = 0; i < columnCount; i++)
        _weatherTable->setColumnWidth(i, WEATHER_VIEW_WIDTH / columnCount - 7);

    _weatherTable->setHorizontalHeaderLabels(horizontalHeaderLabels);
    _weatherTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    _weatherTable->setContextMenuPolicy(Qt::CustomContextMenu);
    _weatherTable->setSortingEnabled(false);
    _weatherTable->horizontalHeader()->setStretchLastSection(true);
    _weatherTable->verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    _weatherTable->verticalHeader()->setDefaultSectionSize(18);
    _weatherTable->verticalHeader()->setVisible(false);

    auto buttonBox = CommonWidgetUtils::makeDialogButtonBox(this, QDialogButtonBox::Cancel);
    auto btnSendWeather = buttonBox->addButton(tr("Send"), QDialogButtonBox::AcceptRole);
    connect(btnSendWeather, &QPushButton::clicked, this, &WeatherView::onSendWeatherClicked);

    auto mainLayout = new QVBoxLayout(this);

    mainLayout->addWidget(_weatherTable);
    mainLayout->addWidget(buttonBox);
    mainLayout->setStretch(0, 0);
    mainLayout->setStretch(1, 1);
}

WeatherView::~WeatherView()
{

}

void WeatherView::setCell(int row, int col, double value)
{
    auto item = new QTableWidgetItem(QString::number(value, 'f', 2));
    item->setTextAlignment(Qt::AlignRight);
    _weatherTable->setItem(row, col, item);
}

void WeatherView::reinit()
{
    _weatherDataCollection = _telemetryDataStorage->getWeatherData();
    auto weatherDataCount = _weatherDataCollection.count();
    _weatherTable->setRowCount(weatherDataCount);

    for (int i = 0; i < weatherDataCount; i++)
    {
        auto weatherData = _weatherDataCollection.at(i);
        setCell(i, 0, weatherData.Altitude);
        setCell(i, 1, weatherData.WindDirection);
        setCell(i, 2, weatherData.WindSpeed);
        setCell(i, 3, weatherData.AtmospherePressure);
        setCell(i, 4, weatherData.AtmosphereTemperature);
    }
}

void WeatherView::onSendWeatherClicked()
{
    MarkerStorage& markerStorage = MarkerStorage::Instance();
    markerStorage.sendWeatherToArtillerySpotter(_weatherDataCollection);
}
