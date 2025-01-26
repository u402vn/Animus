#include "ArtillerySpotter.h"
#include <QDebug>
#include "Common/BinaryContent.h"
#include "Common/CommonUtils.h"
#include "EnterProc.h"

void ArtillerySpotter::processDataExchange(const QString &contentHEX, const QString &description, DataExchangePackageDirection direction)
{
    EnterProcStart("ArtillerySpotter::processDataExchange");

    DataExchangePackage dataPackage;
    dataPackage.SessionTimeMs = _telemetryDataFrame.SessionTimeMs;
    dataPackage.VideoFrameNumber = _telemetryDataFrame.VideoFrameNumber;
    dataPackage.TelemetryFrameNumber = _telemetryDataFrame.TelemetryFrameNumber;
    dataPackage.ContentHEX = contentHEX;
    dataPackage.Description = description;
    emit onArtillerySpotterDataExchange(dataPackage, direction);

    qInfo() << description << contentHEX;
}

void ArtillerySpotter::timerEvent(QTimerEvent *event)
{
    Q_UNUSED(event)

    EnterProcStart("ArtillerySpotter::timerEvent");

    if (_reconnectTimerId != event->timerId())
        return;
    if (!_enabled)
        return;
    if (_tcpSocket.state() == QAbstractSocket::UnconnectedState)
    {
        _tcpSocket.connectToHost(_address, _port);
    }
}

ArtillerySpotter::ArtillerySpotter(QObject *parent) : QObject(parent)
{
    EnterProcStart("ArtillerySpotter::ArtillerySpotter");

    _enabled = false;
    _messageId = 0;

    connect(&_tcpSocket, &QTcpSocket::readyRead, this, &ArtillerySpotter::readSocketData, Qt::ANIMUS_CONNECTION_TYPE);

    _reconnectTimerId = startTimer(2000); // reconnect to socket
}

ArtillerySpotter::~ArtillerySpotter()
{
    killTimer(_reconnectTimerId);
}

void ArtillerySpotter::openSocket(const QHostAddress address, const quint16 port)
{
    EnterProcStart("ArtillerySpotter::openSocket");

    _enabled = true;
    _address = address;
    _port = port;
    _tcpSocket.close();
}

void ArtillerySpotter::processTelemetry(const TelemetryDataFrame &telemetryDataFrame)
{
    _telemetryDataFrame = telemetryDataFrame;
}


#pragma pack(push, 1)
struct HeaderData
{
    uint8_t codeMessage;
    uint8_t protocolVersion;
    uint32_t messageId;
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

struct ReceiptData
{
    uint8_t codeMessage;
    uint8_t protocolVersion;
    uint32_t messageId;
    uint8_t errorCode;
};
#pragma pack(pop)

void ArtillerySpotter::sendMarkers(const QList<MapMarker *> *markers)
{
    EnterProcStart("ArtillerySpotter::sendMarkers");

    if (_tcpSocket.state() != QAbstractSocket::ConnectedState)
    {
        processDataExchange("", "Send Markers. Unable to send message. No connection.", DataExchangePackageDirection::Outgoing);
        emit onMessageExchangeInformation(tr("Unable to send message. No connection."), true);
        return;
    }

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
    header.protocolVersion = 2;
    header.messageId = _messageId++;
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

    _tcpSocket.write(messageContent.toByteArray());

    _sentMessages.insert(header.messageId, header.codeMessage);

    processDataExchange(messageContent.toHex(), "Send Markers", DataExchangePackageDirection::Outgoing);
    emit onMessageExchangeInformation(tr("Targets information sent successfully (# %1)").arg(header.messageId), false);
}

void ArtillerySpotter::sendWeather(const QVector<WeatherDataItem> *weatherDataCollection)
{
    EnterProcStart("ArtillerySpotter::sendMarkers");

    if (_tcpSocket.state() != QAbstractSocket::ConnectedState)
    {
        processDataExchange("", "Send Weather. Unable to send message. No connection.", DataExchangePackageDirection::Outgoing);
        emit onMessageExchangeInformation(tr("Unable to send message. No connection."), true);
        return;
    }

    int weatherDataCount = weatherDataCollection->count();

    if (weatherDataCount == 0)
        return;

    qDebug() << weatherDataCount;

    HeaderData header;
    header.codeMessage = 2;
    header.protocolVersion = 2;
    header.messageId = _messageId++;

    WeatherCommonData commonData;
    commonData.zeroPointAltitude = 0; //???
    commonData.dateTime = GetCurrentDateTimeFrom1970Secs();

    header.lenData = sizeof(HeaderData) + sizeof(WeatherCommonData) + sizeof(PackedWeatherDataItem) * weatherDataCount;

    BinaryContent messageContent;
    messageContent.appendData((const char *)&header, sizeof(header));
    messageContent.appendData((const char *)&commonData, sizeof(commonData));

    for (int i = 0; i < weatherDataCount; i++)
    {
        auto weatherData = weatherDataCollection->at(i);

        PackedWeatherDataItem packedWeatherData;
        packedWeatherData.altitude = weatherData.Altitude;
        packedWeatherData.windDirection = weatherData.WindDirection;
        packedWeatherData.windSpeed = weatherData.WindSpeed;
        packedWeatherData.atmospherePressure = weatherData.AtmospherePressure;
        packedWeatherData.atmosphereTemperature = weatherData.AtmosphereTemperature;

        messageContent.appendData((const char *)&packedWeatherData, sizeof(PackedWeatherDataItem));
    }

    _tcpSocket.write(messageContent.toByteArray());

    _sentMessages.insert(header.messageId, header.codeMessage);

    processDataExchange(messageContent.toHex(), "Send Weather", DataExchangePackageDirection::Outgoing);
    emit onMessageExchangeInformation(tr("Weather information sent successfully (# %1)").arg(header.messageId), false);
}

void ArtillerySpotter::readSocketData()
{
    EnterProcStart("ArtillerySpotter::readSocketData");

    ReceiptData* receiptData;

    do {
        _tcpBuffer.append(_tcpSocket.readAll());

        while (static_cast<quint32>(_tcpBuffer.size()) >= sizeof(ReceiptData))
        {
            receiptData = reinterpret_cast<ReceiptData*>(_tcpBuffer.data());

            BinaryContent messageContent;
            messageContent.clear();
            messageContent.appendData((const char *)receiptData, sizeof(ReceiptData));
            processDataExchange(messageContent.toHex(), "Receipt Data", DataExchangePackageDirection::Outgoing);

            if (receiptData->errorCode == 0)
                emit onMessageExchangeInformation(tr("Information received successfully (# %1)").arg(receiptData->messageId), false);
            else
                emit onMessageExchangeInformation(tr("Information received unsuccessfully (# %1)").arg(receiptData->messageId), true);
            _tcpBuffer.remove(0, sizeof(ReceiptData));
        }

    } while (_tcpSocket.bytesAvailable() > 0);
}
