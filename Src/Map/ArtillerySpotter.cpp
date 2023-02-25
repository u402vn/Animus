#include "ArtillerySpotter.h"
#include <QDebug>
#include "Common/BinaryContent.h"
#include "Common/CommonUtils.h"

void ArtillerySpotter::timerEvent(QTimerEvent *event)
{
    Q_UNUSED(event)
    if (!_enabled)
        return;
    if (_socket.state() == QAbstractSocket::UnconnectedState)
    {
        _socket.connectToHost(_address, _port);
    }
    //else if ( (_socket.state() == QAbstractSocket::ConnectedState) && _needResendMarkerList )
    //    emit onNeedResendMarkerList();
}

ArtillerySpotter::ArtillerySpotter(QObject *parent) : QObject(parent)
{
    _enabled = false;
    _reconnectTimerId = startTimer(2000); // reconnect to socket
}

ArtillerySpotter::~ArtillerySpotter()
{
    killTimer(_reconnectTimerId);
}

void ArtillerySpotter::openSocket(const QHostAddress address, const quint16 port)
{
    _enabled = true;
    _address = address;
    _port = port;
    _socket.close();
}


#pragma pack(push, 1)
struct HeaderData
{
    uint8_t codeMessage;
    uint8_t protocolVersion;
    uint32_t lenData;
};

struct PointItemData
{
    uint16_t pointNo;
    double latitude;
    double longitude;
    double altitude;
    double azimuth;
    double speed;
    uint8_t targetKind;
    double dateTime;
};


struct WeatherCommonData
{
    double dateTime;
    int16_t zeroPointAltitude;
};

struct WeatherItemData
{    
    int16_t altitude;
    double windDirection;
    double windSpeed;
    double atmospherePressure;
    double atmosphereTemperature;
};

#pragma pack(pop)

void ArtillerySpotter::sendMarkers(const QList<MapMarker *> *markers)
{
    if (_socket.state() != QAbstractSocket::ConnectedState)
        return;

    uint32_t markersCount = markers->count();

    HeaderData header;
    header.codeMessage = 1;
    header.protocolVersion = 1;
    header.lenData = markersCount * sizeof(PointItemData) + sizeof(HeaderData);

    BinaryContent messageContent;
    messageContent.appendData((const char *)&header, sizeof(header));

    for (uint32_t i = 0; i < markersCount; i++)
    {
        auto marker = markers->at(i);
        auto gpsCoord = marker->gpsCoord();

        PointItemData pointData;
        pointData.pointNo = marker->tag();
        pointData.latitude = gpsCoord.lat;
        pointData.longitude = gpsCoord.lon;
        pointData.altitude = gpsCoord.hmsl;
        pointData.azimuth = 0;
        pointData.speed = 0;
        pointData.targetKind = marker->artillerySpotterState();
        pointData.dateTime = GetCurrentDateTimeFrom1970Secs();

        messageContent.appendData((const char *)&pointData, sizeof(pointData));
    }

    _socket.write(messageContent.toByteArray());
}

void ArtillerySpotter::sendWeather(const QVector<TelemetryDataFrame> &telemetryDataFrames)
{
    if (_socket.state() != QAbstractSocket::ConnectedState)
        return;

    int frameCount = telemetryDataFrames.count();
    if (frameCount == 0)
        return;

    const int WEATHER_ITEMS_ALTITUDE_STEP = 200; //m
    const int WEATHER_ITEMS_MAX_COUNT = 100; //range 0..20000 m

    int measureCount[WEATHER_ITEMS_MAX_COUNT] = { 0 };
    double windDirectionSum[WEATHER_ITEMS_MAX_COUNT] = { 0 };
    double windSpeedSum[WEATHER_ITEMS_MAX_COUNT] = { 0 };
    double atmospherePressureSum[WEATHER_ITEMS_MAX_COUNT] = { 0 };
    double atmosphereTemperatureSum[WEATHER_ITEMS_MAX_COUNT] = { 0 };

    quint32 endTime = telemetryDataFrames.at(frameCount - 1).SessionTimeMs;
    quint32 prevTelemetryFrameNumber = 0;

    TelemetryDataFrame telemetryFrame;
    for (int i = frameCount - 1; i > 0; i--)
    {
        telemetryFrame = telemetryDataFrames.at(i);

        if (prevTelemetryFrameNumber == telemetryFrame.TelemetryFrameNumber)
            continue;
        if (endTime - telemetryFrame.SessionTimeMs > 1800000) // 30 minutes
            break;

        prevTelemetryFrameNumber = telemetryFrame.TelemetryFrameNumber;
        double altitide = telemetryFrame.UavAltitude_GPS; //??? UavAltitude_Barometric;
        quint32 index = altitide < 0 ? 0 : round(altitide / WEATHER_ITEMS_ALTITUDE_STEP);

        measureCount[index] += 1;
        windDirectionSum[index] += telemetryFrame.WindDirection;
        windSpeedSum[index] += telemetryFrame.WindSpeed;
        atmospherePressureSum[index] += telemetryFrame.AtmospherePressure;
        atmosphereTemperatureSum[index] += telemetryFrame.AtmosphereTemperature;
    }

    QVector<WeatherItemData> weatherDataColl;
    WeatherItemData weatherData;
    for (int index = 0; index < WEATHER_ITEMS_MAX_COUNT; index++)
    {
        int count = measureCount[index];
        if (count > 0)
        {
            weatherData.altitude = index * WEATHER_ITEMS_ALTITUDE_STEP;
            weatherData.windDirection = windDirectionSum[index] / count;
            weatherData.windSpeed = windSpeedSum[index] / count;
            weatherData.atmospherePressure = atmospherePressureSum[index] / count;
            weatherData.atmosphereTemperature = atmosphereTemperatureSum[index] / count;
            weatherDataColl.append(weatherData);
        }
    }

    int weatherDataCount = weatherDataColl.count();

    qDebug() << weatherDataCount;

    HeaderData header;
    header.codeMessage = 2;
    header.protocolVersion = 1;

    WeatherCommonData commonData;
    commonData.zeroPointAltitude = 0; //???
    commonData.dateTime = GetCurrentDateTimeFrom1970Secs();

    header.lenData = sizeof(HeaderData) + sizeof(WeatherCommonData) + sizeof(WeatherItemData) * weatherDataCount;

    BinaryContent messageContent;
    messageContent.appendData((const char *)&header, sizeof(header));
    messageContent.appendData((const char *)&commonData, sizeof(commonData));

    for (int i = 0; i < weatherDataCount; i++)
    {
        weatherData = weatherDataColl.at(i);
        messageContent.appendData((const char *)&weatherData, sizeof(WeatherItemData));
    }

    _socket.write(messageContent.toByteArray());
}
