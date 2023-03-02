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

struct PackedWeatherDataItem
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

    QList<MapMarker *> _messageMarkers;

    foreach(auto marker, *markers)
    {
        auto targetMapMarker = dynamic_cast<TargetMapMarker*>(marker);
        if (targetMapMarker != nullptr)
            _messageMarkers.append(targetMapMarker);

        auto salvoCenterMarke = dynamic_cast<ArtillerySalvoCenterMarker*>(marker);
        if (salvoCenterMarke != nullptr)
            _messageMarkers.append(salvoCenterMarke);
    }

    uint32_t targetCount = _messageMarkers.count();
    auto lenData = sizeof(HeaderData) + targetCount * sizeof(PointItemData);

    HeaderData header;
    header.codeMessage = 1;
    header.protocolVersion = 1;
    header.lenData = lenData;

    BinaryContent messageContent;
    messageContent.appendData((const char *)&header, sizeof(header));

    foreach(auto marker, _messageMarkers)
    {
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

void ArtillerySpotter::sendWeather(const QVector<WeatherDataItem> weatherDataCollection)
{
    if (_socket.state() != QAbstractSocket::ConnectedState)
        return;


    int weatherDataCount = weatherDataCollection.count();

    if (weatherDataCount == 0)
        return;

    qDebug() << weatherDataCount;

    HeaderData header;
    header.codeMessage = 2;
    header.protocolVersion = 1;

    WeatherCommonData commonData;
    commonData.zeroPointAltitude = 0; //???
    commonData.dateTime = GetCurrentDateTimeFrom1970Secs();

    header.lenData = sizeof(HeaderData) + sizeof(WeatherCommonData) + sizeof(PackedWeatherDataItem) * weatherDataCount;

    BinaryContent messageContent;
    messageContent.appendData((const char *)&header, sizeof(header));
    messageContent.appendData((const char *)&commonData, sizeof(commonData));

    for (int i = 0; i < weatherDataCount; i++)
    {
        auto weatherData = weatherDataCollection.at(i);

        PackedWeatherDataItem packedWeatherData;
        packedWeatherData.altitude = weatherData.Altitude;
        packedWeatherData.windDirection = weatherData.WindDirection;
        packedWeatherData.windSpeed = weatherData.WindSpeed;
        packedWeatherData.atmospherePressure = weatherData.AtmospherePressure;
        packedWeatherData.atmosphereTemperature = weatherData.AtmosphereTemperature;

        messageContent.appendData((const char *)&packedWeatherData, sizeof(PackedWeatherDataItem));
    }

    _socket.write(messageContent.toByteArray());
}
