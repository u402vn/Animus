#include "TrackerHardwareLink.h"
#include <QString>
#include <QByteArray>
#include "EnterProc.h"
#include "ApplicationSettings.h"

void TrackerHardwareLink::sendTrackerCommand(const BinaryContent &commandContent, const QString &commandDescription)
{
    EnterProc("TrackerHardwareLink::sendCommand");

    if (commandContent.size() == 0)
        return;

    onTrackerCommandSent(commandContent, commandDescription);

    _udpTrackerCommandSocket.writeDatagram(commandContent.toByteArray(), _trackerCommandUDPAddress, _trackerCommandUDPPort);
}

uint8_t TrackerHardwareLink::trackingState() const
{
    return _mode;
}

int32_t TrackerHardwareLink::trackingRectangleHeight() const
{
    if (_mode == 1 || _mode == 2)
        return _trackingRectangleHeight;
    return 0;
}

int32_t TrackerHardwareLink::trackingRectangleWidth() const
{
    if (_mode == 1 || _mode == 2)
        return _trackingRectangleWidth;
    return 0;
}

int32_t TrackerHardwareLink::trackingRectangleCenterY() const
{
    if (_mode == 1 || _mode == 2)
        return _trackingRectangleCenterY;
    return 0;
}

int32_t TrackerHardwareLink::trackingRectangleCenterX() const
{
    if (_mode == 1 || _mode == 2)
        return _trackingRectangleCenterX;
    return 0;
}

TrackerHardwareLink::TrackerHardwareLink(QObject *parent) : QObject(parent)
{
    _trackingRectangleCenterX = -1;
    _trackingRectangleCenterY = -1;
    _trackingRectangleWidth = -1;
    _trackingRectangleHeight = -1;
    _objectCenterX = -1;
    _objectCenterY = -1;
    _objectWidth = -1;
    _objectHeight = -1;
    _frameCounterInLostMode = -1;
    _frameCounter = -1;
    _frameWidth = -1;
    _frameHeight = -1;
    _correlationSurfaceWidth = -1;
    _correlationSurfaceHeight = -1;
    _pixelDeviationThreshold = -1;
    _lostModeOption = -1;
    _frameBufferSize = -1;
    _maximumNumberOfFramesInLostMode = -1;
    _trackerFrameID = -1;
    _bufferFrameID = -1;
    _searchWindowCenterX = -1;
    _searchWindowCenterY = -1;
    _numThreads = -1;
    _trackingRectangleCenterFX = -1;
    _trackingRectangleCenterFY = -1;
    _horizontalObjectValocity = -1;
    _verticalObjectVelocity = -1;
    _objectLossThreshold = -1;
    _objectDetectionThreshold = -1;
    _probabilityAdaptiveThreshold = -1;
    _patternUpdateCoeff = -1;
    _velocityUpdateCoeff = -1;
    _probabilityUpdateCoeff = -1;
    _objectDetectionProbability = -1;
    _mode = -1;


    ApplicationSettings& applicationSettings = ApplicationSettings::Instance();

    _trackerCommandUDPPort = applicationSettings.TrackerCommandUDPPort;
    _trackerCommandUDPAddress = applicationSettings.TrackerCommandUDPAddress;

    _trackerTelemetryUDPPort = applicationSettings.TrackerTelemetryUDPPort;

    connect(&_udpTrackerTelemetrySocket, &QUdpSocket::readyRead, this, &TrackerHardwareLink::processTelemetryPendingDatagrams, Qt::ANIMUS_CONNECTION_TYPE);
}

void TrackerHardwareLink::open()
{
    _udpTrackerTelemetrySocket.bind(_trackerTelemetryUDPPort, QUdpSocket::DefaultForPlatform);
    _udpTrackerTelemetrySocket.setSocketOption(QAbstractSocket::ReceiveBufferSizeSocketOption, 10000); // -1 == Unlimited ???
}

void TrackerHardwareLink::close()
{
    _udpTrackerCommandSocket.close();
    _udpTrackerTelemetrySocket.close();
}

void TrackerHardwareLink::lockTarget(const QPoint &targetCenter)
{
    BinaryContent command;

    qint32 x = targetCenter.x();
    qint32 y = targetCenter.y();

    //    x = 123;
    //    y = 321;

    command.appendChar(2);  // commandType
    command.appendChar(5);  // trackerMajorVersion
    command.appendChar(0);  // trackerMinorVersion
    command.appendDWord(1); // commandID
    command.appendDWord(x);
    command.appendDWord(y);
    command.appendDWord(0);

    auto description = QString("lockTarget: (%1, %2)").arg(x).arg(y);
    sendTrackerCommand(command, description);
}

void TrackerHardwareLink::unlockTarget()
{
    BinaryContent command;

    command.appendChar(2);  // commandType
    command.appendChar(5);  // trackerMajorVersion
    command.appendChar(0);  // trackerMinorVersion
    command.appendDWord(2); // commandID RESET
    command.appendDWord(0);
    command.appendDWord(0);
    command.appendDWord(0);

    auto description = QString("unlockTarget");
    sendTrackerCommand(command, description);
}

void TrackerHardwareLink::setTargetSize(int targetSize)
{
    BinaryContent commandX;

    commandX.appendChar(1);             // commandType
    commandX.appendChar(5);             // trackerMajorVersion
    commandX.appendChar(0);             // trackerMinorVersion
    commandX.appendDWord(2);            // propertyID TRACKING_RECTANGLE_WIDTH
    commandX.appendDWord(targetSize);   // propertyValue

    auto descriptionX = QString("setTargetSizeX: %1").arg(targetSize);
    sendTrackerCommand(commandX, descriptionX);


    BinaryContent commandY;

    commandY.appendChar(1);             // commandType
    commandY.appendChar(5);             // trackerMajorVersion
    commandY.appendChar(0);             // trackerMinorVersion
    commandY.appendDWord(3);            // propertyID TRACKING_RECTANGLE_HEIGHT
    commandY.appendDWord(targetSize);   // propertyValue

    auto descriptionY = QString("setTargetSizeY: %1").arg(targetSize);
    sendTrackerCommand(commandY, descriptionY);
}

void TrackerHardwareLink::processTelemetryPendingDatagrams()
{
    while (_udpTrackerTelemetrySocket.hasPendingDatagrams())
    {
        quint64 dataSize = _udpTrackerTelemetrySocket.pendingDatagramSize();

        QByteArray telemetryData(dataSize, 0);
        _udpTrackerTelemetrySocket.readDatagram((char*)telemetryData.data(), dataSize);
        decodeTrackerData((const uint8_t*)telemetryData.data(), dataSize);
    }
}

int32_t TrackerHardwareLink::decodeTrackerData(const uint8_t *data, const uint32_t dataSize)
{
    // Check data size.
    if (dataSize > 145 || dataSize < 8)
        return -1;

    // Decode tracker data.
    size_t pos = 8;
    if ((data[3] & (uint8_t)128) == (uint8_t)128)
    {
        memcpy(&_trackingRectangleCenterX, &data[pos], 4);
        pos += 4;
    }
    else
    {
        _trackingRectangleCenterX = -1;
    }
    if ((data[3] & (uint8_t)64) == (uint8_t)64)
    {
        memcpy(&_trackingRectangleCenterY, &data[pos], 4);
        pos += 4;
    }
    else
    {
        _trackingRectangleCenterY = -1;
    }
    if ((data[3] & (uint8_t)32) == (uint8_t)32)
    {
        memcpy(&_trackingRectangleWidth, &data[pos], 4);
        pos += 4;
    }
    else
    {
        _trackingRectangleWidth = -1;
    }
    if ((data[3] & (uint8_t)16) == (uint8_t)16)
    {
        memcpy(&_trackingRectangleHeight, &data[pos], 4);
        pos += 4;
    }
    else
    {
        _trackingRectangleHeight = -1;
    }
    if ((data[3] & (uint8_t)8) == (uint8_t)8)
    {
        memcpy(&_objectCenterX, &data[pos], 4);
        pos += 4;
    }
    else
    {
        _objectCenterX = -1;
    }
    if ((data[3] & (uint8_t)4) == (uint8_t)4)
    {
        memcpy(&_objectCenterY, &data[pos], 4);
        pos += 4;
    }
    else
    {
        _objectCenterY = -1;
    }
    if ((data[3] & (uint8_t)2) == (uint8_t)2)
    {
        memcpy(&_objectWidth, &data[pos], 4);
        pos += 4;
    }
    else
    {
        _objectWidth = -1;
    }
    if ((data[3] & (uint8_t)1) == (uint8_t)1)
    {
        memcpy(&_objectHeight, &data[pos], 4);
        pos += 4;
    }
    else
    {
        _objectHeight = -1;
    }
    if ((data[4] & (uint8_t)128) == (uint8_t)128)
    {
        memcpy(&_frameCounterInLostMode, &data[pos], 4);
        pos += 4;
    }
    else
    {
        _frameCounterInLostMode = -1;
    }
    if ((data[4] & (uint8_t)64) == (uint8_t)64)
    {
        memcpy(&_frameCounter, &data[pos], 4);
        pos += 4;
    }
    else
    {
        _frameCounter = -1;
    }
    if ((data[4] & (uint8_t)32) == (uint8_t)32)
    {
        memcpy(&_frameWidth, &data[pos], 4);
        pos += 4;
    }
    else
    {
        _frameWidth = -1;
    }
    if ((data[4] & (uint8_t)16) == (uint8_t)16)
    {
        memcpy(&_frameHeight, &data[pos], 4);
        pos += 4;
    }
    else
    {
        _frameHeight = -1;
    }
    if ((data[4] & (uint8_t)8) == (uint8_t)8)
    {
        memcpy(&_correlationSurfaceWidth, &data[pos], 4);
        pos += 4;
    }
    else
    {
        _correlationSurfaceWidth = -1;
    }
    if ((data[4] & (uint8_t)4) == (uint8_t)4)
    {
        memcpy(&_correlationSurfaceHeight, &data[pos], 4);
        pos += 4;
    }
    else
    {
        _correlationSurfaceHeight = -1;
    }
    if ((data[4] & (uint8_t)2) == (uint8_t)2)
    {
        memcpy(&_pixelDeviationThreshold, &data[pos], 4);
        pos += 4;
    }
    else
    {
        _pixelDeviationThreshold = -1;
    }
    if ((data[4] & (uint8_t)1) == (uint8_t)1)
    {
        memcpy(&_lostModeOption, &data[pos], 4);
        pos += 4;
    }
    else
    {
        _lostModeOption = -1;
    }
    if ((data[5] & (uint8_t)128) == (uint8_t)128)
    {
        memcpy(&_frameBufferSize, &data[pos], 4);
        pos += 4;
    }
    else
    {
        _frameBufferSize = -1;
    }
    if ((data[5] & (uint8_t)64) == (uint8_t)64)
    {
        memcpy(&_maximumNumberOfFramesInLostMode, &data[pos], 4);
        pos += 4;
    }
    else
    {
        _maximumNumberOfFramesInLostMode = -1;
    }
    if ((data[5] & (uint8_t)32) == (uint8_t)32)
    {
        memcpy(&_trackerFrameID, &data[pos], 4);
        pos += 4;
    }
    else
    {
        _trackerFrameID = -1;
    }
    if ((data[5] & (uint8_t)16) == (uint8_t)16)
    {
        memcpy(&_bufferFrameID, &data[pos], 4);
        pos += 4;
    }
    else
    {
        _bufferFrameID = -1;
    }
    if ((data[5] & (uint8_t)8) == (uint8_t)8)
    {
        memcpy(&_searchWindowCenterX, &data[pos], 4);
        pos += 4;
    }
    else
    {
        _searchWindowCenterX = -1;
    }
    if ((data[5] & (uint8_t)4) == (uint8_t)4)
    {
        memcpy(&_searchWindowCenterY, &data[pos], 4);
        pos += 4;
    }
    else
    {
        _searchWindowCenterY = -1;
    }
    if ((data[5] & (uint8_t)2) == (uint8_t)2)
    {
        memcpy(&_numThreads, &data[pos], 4);
        pos += 4;
    }
    else
    {
        _numThreads = -1;
    }
    if ((data[5] & (uint8_t)1) == (uint8_t)1)
    {
        memcpy(&_trackingRectangleCenterFX, &data[pos], 4);
        pos += 4;
    }
    else
    {
        _trackingRectangleCenterFX = -1;
    }
    if ((data[6] & (uint8_t)128) == (uint8_t)128)
    {
        memcpy(&_trackingRectangleCenterFY, &data[pos], 4);
        pos += 4;
    }
    else
    {
        _trackingRectangleCenterFY = -1;
    }
    if ((data[6] & (uint8_t)64) == (uint8_t)64)
    {
        memcpy(&_horizontalObjectValocity, &data[pos], 4);
        pos += 4;
    }
    else
    {
        _horizontalObjectValocity = -1;
    }
    if ((data[6] & (uint8_t)32) == (uint8_t)32)
    {
        memcpy(&_verticalObjectVelocity, &data[pos], 4);
        pos += 4;
    }
    else
    {
        _verticalObjectVelocity = -1;
    }
    if ((data[6] & (uint8_t)16) == (uint8_t)16)
    {
        memcpy(&_objectLossThreshold, &data[pos], 4);
        pos += 4;
    }
    else
    {
        _objectLossThreshold = -1;
    }
    if ((data[6] & (uint8_t)8) == (uint8_t)8)
    {
        memcpy(&_objectDetectionThreshold, &data[pos], 4);
        pos += 4;
    }
    else
    {
        _objectDetectionThreshold = -1;
    }
    if ((data[6] & (uint8_t)4) == (uint8_t)4)
    {
        memcpy(&_probabilityAdaptiveThreshold, &data[pos], 4);
        pos += 4;
    }
    else
    {
        _probabilityAdaptiveThreshold = -1;
    }
    if ((data[6] & (uint8_t)2) == (uint8_t)2)
    {
        memcpy(&_patternUpdateCoeff, &data[pos], 4);
        pos += 4;
    }
    else
    {
        _patternUpdateCoeff = -1;
    }
    if ((data[6] & (uint8_t)1) == (uint8_t)1)
    {
        memcpy(&_velocityUpdateCoeff, &data[pos], 4);
        pos += 4;
    }
    else
    {
        _velocityUpdateCoeff = -1;
    }
    if ((data[7] & (uint8_t)128) == (uint8_t)128)
    {
        memcpy(&_probabilityUpdateCoeff, &data[pos], 4);
        pos += 4;
    }
    else
    {
        _probabilityUpdateCoeff = -1;
    }
    if ((data[7] & (uint8_t)64) == (uint8_t)64)
    {
        memcpy(&_objectDetectionProbability, &data[pos], 4);
        pos += 4;
    }
    else
    {
        _objectDetectionProbability = -1;
    }
    if ((data[7] & (uint8_t)32) == (uint8_t)32)
    {
        _mode = data[pos];
        pos += 1;
    }
    else
    {
        _mode = 255;
    }

    return 0;
}
