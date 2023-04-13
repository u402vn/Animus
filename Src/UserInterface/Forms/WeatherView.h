#ifndef WEATHERVIEW_H
#define WEATHERVIEW_H

#include <QWidget>
#include <QTextEdit>
#include <QMenu>
#include <QAction>
#include <QPushButton>
#include <QTableWidget>
#include "TelemetryDataFrame.h"
#include "TelemetryDataStorage.h"
#include "Map/ArtillerySpotter.h"

class WeatherView : public QDialog
{
    Q_OBJECT
    TelemetryDataStorage *_telemetryDataStorage;
    ArtillerySpotter *_artillerySpotter;
    QTableWidget *_weatherTable;
    QVector<WeatherDataItem> _weatherDataCollection;
    void setCell(int row, int col, double value);
public:    
    explicit WeatherView(QWidget *parent, TelemetryDataStorage *telemetryDataStorage, ArtillerySpotter *artillerySpotter);
    ~WeatherView();
    void reinit();
private slots:
    void onSendWeatherClicked();
};

#endif // WEATHERVIEW_H
