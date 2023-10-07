#include "TelemetryDataStorage.h"
#include <QPainter>
#include <QStringList>
#include <QDateTime>
#include <QDir>
#include <QSqlError>
#include <QHostInfo>
#include <QStorageInfo>
#include <QDebug>
#include "Common/CommonUtils.h"
#include "EnterProc.h"

const QString SCREENSHOTS_FOLDER_NAME = "Screenshots";
const qint32 FRAME_FLUSH_BATCH_SIZE = 50;

TelemetryDataStorage::TelemetryDataStorage(QObject *parent, const QString &sessionFolder,
                                           const quint32 videoFileFrameCount,
                                           const quint32 videoFileQuality,
                                           const bool displayTelemetryOnVideo,
                                           const quint32 telemetryIndicatorFontSize,
                                           const OSDTelemetryTimeFormat telemetryTimeFormat,
                                           const bool displayTargetRectangleOnVideo,
                                           const OSDGimbalIndicatorType gimbalIndicatorType,
                                           const OSDGimbalIndicatorAngles gimbalIndicatorAngles,
                                           const quint32 gimbalIndicatorSize,
                                           const bool isLaserRangefinderLicensed) : QObject(parent)
{
    EnterProcStart("TelemetryDataStorage::TelemetryDataStorage");

    _sessionFoldersDirectory = sessionFolder;
    _defaultVideoFileFrameCount = videoFileFrameCount;
    _defaultVideoFileQuality = videoFileQuality;
    _displayTelemetryOnVideo = displayTelemetryOnVideo;
    _telemetryIndicatorFontSize = telemetryIndicatorFontSize;
    _telemetryTimeFormat = telemetryTimeFormat;
    _gimbalIndicatorType = gimbalIndicatorType;
    _gimbalIndicatorAngles = gimbalIndicatorAngles;
    _gimbalIndicatorSize = gimbalIndicatorSize;
    _displayTargetRectangleOnVideo = displayTargetRectangleOnVideo;
    _isLaserRangefinderLicensed = isLaserRangefinderLicensed;

    _workMode = WorkMode::DisplayOnly;
    _destroing = false;

    _videoRecordingStubImage = nullptr;

    _videoRecorder = new PartitionedVideoRecorder(this);

    _mediaPlayerFrameGraber = new CameraFrameGrabber(this, false);
    connect(_mediaPlayerFrameGraber, &CameraFrameGrabber::frameAvailable, this, &TelemetryDataStorage::videoFrameReceivedInternal, Qt::QueuedConnection);
    _mediaPlayer = new QMediaPlayer(this);
    _mediaPlayer->setVideoOutput(_mediaPlayerFrameGraber);
}

TelemetryDataStorage::~TelemetryDataStorage()
{
    _destroing = true;
    stopSession();
    delete _videoRecorder;
    delete _mediaPlayer;
}

TelemetryDataStorage::WorkMode TelemetryDataStorage::getWorkMode() const
{
    return _workMode;
}

const QStringList TelemetryDataStorage::getStoredSessionsList() const
{
    EnterProcStart("TelemetryDataStorage::getStoredSessionsList");

    QDir sessionsFolders(_sessionFoldersDirectory);
    QStringList result = sessionsFolders.entryList(QDir::AllDirs | QDir::NoDotAndDotDot, QDir::Name | QDir::Reversed);
    result.removeOne(SCREENSHOTS_FOLDER_NAME);
    return result;
}

void TelemetryDataStorage::stopSession()
{
    EnterProcStart("TelemetryDataStorage::stopSession");

    bool isModeChanged = (_workMode != WorkMode::DisplayOnly);

    _mediaPlayer->stop();
    _videoRecorder->stop();
    flushTelemetryDataFrames();
    flushClientCommands();
    flushArtillerySpotterDataPackages();
    flushSessionInfos();

    _sessionDatabase.close();
    _lastSavedTelemetryFrameIndex = -1;
    _lastSavedClientCommandIndex = -1;
    _lastSavedArtillerySpotterDataPackageIndex = -1;
    _lastVideoFrameNumber = -1;
    // _telemetryFramesDatabase.close();

    _sessionDatabase = QSqlDatabase();
    QSqlDatabase::removeDatabase("TelemetryFramesStorageConnection");

    _telemetryFrames.clear();
    _clientCommands.clear();
    _artillerySpotterDataPackages.clear();

    _workMode = WorkMode::DisplayOnly;
    _sessionName = "";
    _lastOpenedVideoFile = "";
    if (!_destroing && isModeChanged)
        emit workModeChanged();
}

void TelemetryDataStorage::openTelemetryFramesDatabase()
{
    EnterProcStart("TelemetryDataStorage::openTelemetryFramesDatabase");

    _sessionDatabase = QSqlDatabase::addDatabase("QSQLITE", "TelemetryFramesStorageConnection");
    _sessionDatabase.setDatabaseName(getSessionFolder() + "/" + _sessionName + ".sqlite");
    _sessionDatabase.open();
    LOG_SQL_ERROR(_sessionDatabase);
    EXEC_SQL(_sessionDatabase, "CREATE TABLE IF NOT EXISTS TelemetryFrames ( "
                               "FrameTime INTEGER, Roll REAL, Pitch REAL, Yaw REAL, GPSLat REAL, GPSLon REAL, GPSHmsl REAL, "
                               "Altitude REAL, VSpeed REAL, CamRoll REAL, CamPitch REAL, CamYaw REAL, CamZoom REAL, "
                               "CamEncoderRoll INTEGER, CamEncoderPitch INTEGER, CamEncoderYaw INTEGER, "
                               "AirSpeed REAL, GroundSpeed_GPS REAL, "
                               "WindDirection REAL, WindSpeed REAL, GroundSpeedNorth_GPS REAL, GroundSpeedEast_GPS REAL, "
                               "BombState INTEGER, RangefinderDistance REAL, "
                               "StabilizedCenterX INTEGER, StabilizedCenterY INTEGER, "
                               "TargetCenterX INTEGER, TargetCenterY INTEGER, TargetRectWidth INTEGER, TargetRectHeight INTEGER, "
                               "CalculatedTargetGPSLat REAL, CalculatedTargetGPSLon REAL, CalculatedTargetGPSHmsl REAL, "
                               "CalculatedTargetSpeed REAL, CalculatedTargetDirection  REAL, "
                               "TelemetryFrameNumber INTEGER, VideoFrameNumber INTEGER, SessionTimeMs INTEGER)");

    EXEC_SQL(_sessionDatabase, "CREATE TABLE IF NOT EXISTS ClientCommands ( "
                               "SessionTimeMs INTEGER, TelemetryFrameNumber INTEGER, VideoFrameNumber INTEGER, "
                               "CommandHEX TEXT, Description VARCHAR(255) )");

    EXEC_SQL(_sessionDatabase, "CREATE TABLE IF NOT EXISTS ArtillerySpotterData ( "
                               "SessionTimeMs INTEGER, TelemetryFrameNumber INTEGER, VideoFrameNumber INTEGER, "
                               "ContentHEX TEXT, Description VARCHAR(255), Direction INTEGER )");

    EXEC_SQL(_sessionDatabase, "PRAGMA journal_mode = MEMORY");

    loadSessionInfos();
}

void TelemetryDataStorage::flushTelemetryDataFrames()
{
    if (!_sessionDatabase.isOpen() || (_workMode != WorkMode::RecordAndDisplay))
        return;
    EnterProcStart("TelemetryDataStorage::flushTelemetryDataFrames");


    _sessionDatabase.transaction();
    LOG_SQL_ERROR(_sessionDatabase);

    QSqlQuery insertQuery(_sessionDatabase);
    insertQuery.prepare("INSERT INTO TelemetryFrames "
                        "(FrameTime, Roll, Pitch, Yaw, GPSLat, GPSLon, GPSHmsl, Altitude, VSpeed, CamRoll, CamPitch, CamYaw, CamZoom, "
                        "CamEncoderRoll, CamEncoderPitch, CamEncoderYaw, "
                        "AirSpeed, GroundSpeed_GPS, "
                        "WindDirection, WindSpeed, GroundSpeedNorth_GPS, GroundSpeedEast_GPS,  BombState, RangefinderDistance, "
                        "StabilizedCenterX, StabilizedCenterY, "
                        "TargetCenterX, TargetCenterY, TargetRectWidth, TargetRectHeight, "
                        "CalculatedTargetGPSLat, CalculatedTargetGPSLon, CalculatedTargetGPSHmsl, "
                        "CalculatedTargetSpeed, CalculatedTargetDirection, "
                        "TelemetryFrameNumber, VideoFrameNumber, SessionTimeMs) "
                        "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)"
                        );
    LOG_SQL_ERROR(insertQuery);

    int telemetryFramesCount = _telemetryFrames.count();
    for (int i = _lastSavedTelemetryFrameIndex + 1; i < telemetryFramesCount; i++)
    {
        const TelemetryDataFrame telemetryFrame = _telemetryFrames[i];

        insertQuery.addBindValue(telemetryFrame.Time);
        insertQuery.addBindValue(telemetryFrame.UavRoll);
        insertQuery.addBindValue(telemetryFrame.UavPitch);
        insertQuery.addBindValue(telemetryFrame.UavYaw);
        insertQuery.addBindValue(telemetryFrame.UavLatitude_GPS);
        insertQuery.addBindValue(telemetryFrame.UavLongitude_GPS);
        insertQuery.addBindValue(telemetryFrame.UavAltitude_GPS);
        insertQuery.addBindValue(telemetryFrame.UavAltitude_Barometric);
        insertQuery.addBindValue(telemetryFrame.VerticalSpeed);
        insertQuery.addBindValue(telemetryFrame.CamRoll);
        insertQuery.addBindValue(telemetryFrame.CamPitch);
        insertQuery.addBindValue(telemetryFrame.CamYaw);
        insertQuery.addBindValue(telemetryFrame.CamZoom);
        insertQuery.addBindValue(telemetryFrame.CamEncoderRoll);
        insertQuery.addBindValue(telemetryFrame.CamEncoderPitch);
        insertQuery.addBindValue(telemetryFrame.CamEncoderYaw);
        insertQuery.addBindValue(telemetryFrame.AirSpeed);
        insertQuery.addBindValue(telemetryFrame.GroundSpeed_GPS);
        insertQuery.addBindValue(telemetryFrame.WindDirection);
        insertQuery.addBindValue(telemetryFrame.WindSpeed);
        insertQuery.addBindValue(telemetryFrame.GroundSpeedNorth_GPS);
        insertQuery.addBindValue(telemetryFrame.GroundSpeedEast_GPS);
        insertQuery.addBindValue(telemetryFrame.BombState);
        insertQuery.addBindValue(telemetryFrame.RangefinderDistance);
        insertQuery.addBindValue(telemetryFrame.StabilizedCenterX);
        insertQuery.addBindValue(telemetryFrame.StabilizedCenterY);
        insertQuery.addBindValue(telemetryFrame.TrackedTargetCenterX);
        insertQuery.addBindValue(telemetryFrame.TrackedTargetCenterY);
        insertQuery.addBindValue(telemetryFrame.TrackedTargetRectWidth);
        insertQuery.addBindValue(telemetryFrame.TrackedTargetRectHeight);
        insertQuery.addBindValue(telemetryFrame.CalculatedTrackedTargetGPSLat);
        insertQuery.addBindValue(telemetryFrame.CalculatedTrackedTargetGPSLon);
        insertQuery.addBindValue(telemetryFrame.CalculatedTrackedTargetGPSHmsl);        
        insertQuery.addBindValue(telemetryFrame.CalculatedTrackedTargetSpeed);
        insertQuery.addBindValue(telemetryFrame.CalculatedTrackedTargetDirection);
        insertQuery.addBindValue(telemetryFrame.TelemetryFrameNumber);
        insertQuery.addBindValue(telemetryFrame.VideoFrameNumber);
        insertQuery.addBindValue(telemetryFrame.SessionTimeMs);
        insertQuery.exec();
        LOG_SQL_ERROR(insertQuery);
    }

    _sessionDatabase.commit();
    LOG_SQL_ERROR(_sessionDatabase);
    _lastSavedTelemetryFrameIndex = telemetryFramesCount - 1;
}

void TelemetryDataStorage::flushClientCommands()
{
    if (!_sessionDatabase.isOpen() || (_workMode != WorkMode::RecordAndDisplay))
        return;
    EnterProcStart("TelemetryDataStorage::flushClientCommands");

    _sessionDatabase.transaction();
    LOG_SQL_ERROR(_sessionDatabase);

    QSqlQuery insertQuery(_sessionDatabase);
    insertQuery.prepare("INSERT INTO ClientCommands "
                        "(SessionTimeMs, TelemetryFrameNumber, VideoFrameNumber, CommandHEX, Description) "
                        "VALUES (?, ?, ?, ?, ?)"
                        );
    LOG_SQL_ERROR(insertQuery);

    int clientCommandsCount = _clientCommands.count();
    for (int i = _lastSavedClientCommandIndex + 1; i < clientCommandsCount; i++)
    {
        const DataExchangePackage clientCommand = _clientCommands[i];
        insertQuery.addBindValue(clientCommand.SessionTimeMs);
        insertQuery.addBindValue(clientCommand.TelemetryFrameNumber);
        insertQuery.addBindValue(clientCommand.VideoFrameNumber);
        insertQuery.addBindValue(clientCommand.ContentHEX);
        insertQuery.addBindValue(clientCommand.Description);

        insertQuery.exec();
        LOG_SQL_ERROR(insertQuery);
    }

    _sessionDatabase.commit();
    LOG_SQL_ERROR(_sessionDatabase);
    _lastSavedClientCommandIndex = clientCommandsCount - 1;
}

void TelemetryDataStorage::flushArtillerySpotterDataPackages()
{
    if (!_sessionDatabase.isOpen() || (_workMode != WorkMode::RecordAndDisplay))
        return;
    EnterProcStart("TelemetryDataStorage::flushArtillerySpotterDataPackages");

    _sessionDatabase.transaction();
    LOG_SQL_ERROR(_sessionDatabase);

    QSqlQuery insertQuery(_sessionDatabase);
    insertQuery.prepare("INSERT INTO ArtillerySpotterData "
                        "(SessionTimeMs, TelemetryFrameNumber, VideoFrameNumber, ContentHEX, Description, Direction) "
                        "VALUES (?, ?, ?, ?, ?, ?)"
                        );
    LOG_SQL_ERROR(insertQuery);

    int artillerySpotterDataPackagesCount = _artillerySpotterDataPackages.count();
    for (int i = _lastSavedArtillerySpotterDataPackageIndex + 1; i < artillerySpotterDataPackagesCount; i++)
    {
        const DataExchangePackage dataPackage = _artillerySpotterDataPackages[i];
        insertQuery.addBindValue(dataPackage.SessionTimeMs);
        insertQuery.addBindValue(dataPackage.TelemetryFrameNumber);
        insertQuery.addBindValue(dataPackage.VideoFrameNumber);
        insertQuery.addBindValue(dataPackage.ContentHEX);
        insertQuery.addBindValue(dataPackage.Description);
        insertQuery.addBindValue(dataPackage.Direction);

        insertQuery.exec();
        LOG_SQL_ERROR(insertQuery);
    }

    _sessionDatabase.commit();
    LOG_SQL_ERROR(_sessionDatabase);
    _lastSavedArtillerySpotterDataPackageIndex = artillerySpotterDataPackagesCount - 1;
}

const QString TelemetryDataStorage::getSessionFolder() const
{
    QString sessionFolder = _sessionFoldersDirectory + "/" + _sessionName;
    return sessionFolder;
}

void TelemetryDataStorage::newSession()
{
    EnterProcStart("TelemetryDataStorage::newSession");

    stopSession();
    //todo
    _sessionName =  QDateTime::currentDateTime().toString("yyyy_MM_dd_hh_mm_ss_zzz");
    QString sessionFolder = getSessionFolder();
    QDir dir;
    if (!dir.mkpath(sessionFolder))
        return;

    openTelemetryFramesDatabase();

    initSessionInfos();

    _videoRecorder->start(sessionFolder, _sessionName, _defaultVideoFileFrameCount, _defaultVideoFileQuality);

    _workMode = WorkMode::RecordAndDisplay;
    emit workModeChanged();
}

void TelemetryDataStorage::openSession(const QString &sessionName)
{
    EnterProcStart("TelemetryDataStorage::openSession");

    stopSession();

    _sessionName = sessionName;

    openTelemetryFramesDatabase();

    readTelemetryFrames();

    _workMode = WorkMode::PlayStored;
    emit workModeChanged();
}

void TelemetryDataStorage::readTelemetryFrames()
{
    EnterProcStart("TelemetryDataStorage::readTelemetryFrames");

    if (getSessionFormatVersion() < 7)
        return;

    QString  sql = "SELECT "
                   "FrameTime, Roll, Pitch, Yaw, GPSLat, GPSLon, GPSHmsl, "
                   "AirSpeed, Altitude, VSpeed, CamRoll, CamPitch, CamYaw, CamZoom, "
                   "CamEncoderRoll, CamEncoderPitch, CamEncoderYaw, "
                   "GroundSpeed_GPS, WindDirection, WindSpeed, "
                   "GroundSpeedNorth_GPS, GroundSpeedEast_GPS, "
                   "BombState, RangefinderDistance, "
                   "StabilizedCenterX, StabilizedCenterY, "
                   "TargetCenterX, TargetCenterY, TargetRectWidth, TargetRectHeight, "
                   "CalculatedTargetGPSLat, CalculatedTargetGPSLon, CalculatedTargetGPSHmsl, "
                   "CalculatedTargetSpeed, CalculatedTargetDirection, "
                   "TelemetryFrameNumber, VideoFrameNumber, SessionTimeMs "
                   "FROM TelemetryFrames";

    QSqlQuery selectQuery = EXEC_SQL(_sessionDatabase, sql);

    while (selectQuery.next())
    {
        int pos = 0;

        TelemetryDataFrame frame;
        frame.Time = selectQuery.value(pos++).toInt();
        frame.UavRoll = selectQuery.value(pos++).toDouble();
        frame.UavPitch = selectQuery.value(pos++).toDouble();
        frame.UavYaw = selectQuery.value(pos++).toDouble();
        frame.UavLatitude_GPS = selectQuery.value(pos++).toDouble();
        frame.UavLongitude_GPS = selectQuery.value(pos++).toDouble();
        frame.UavAltitude_GPS = selectQuery.value(pos++).toDouble();
        frame.AirSpeed = selectQuery.value(pos++).toDouble();
        frame.UavAltitude_Barometric = selectQuery.value(pos++).toDouble();
        frame.VerticalSpeed = selectQuery.value(pos++).toFloat();

        frame.CamRoll = selectQuery.value(pos++).toDouble();
        frame.CamPitch = selectQuery.value(pos++).toDouble();
        frame.CamYaw = selectQuery.value(pos++).toDouble();
        frame.CamZoom = selectQuery.value(pos++).toDouble();
        frame.CamEncoderRoll = selectQuery.value(pos++).toInt();
        frame.CamEncoderPitch = selectQuery.value(pos++).toInt();
        frame.CamEncoderYaw = selectQuery.value(pos++).toInt();

        frame.GroundSpeed_GPS = selectQuery.value(pos++).toDouble();
        frame.WindDirection = selectQuery.value(pos++).toDouble();
        frame.WindSpeed = selectQuery.value(pos++).toDouble();
        frame.GroundSpeedNorth_GPS = selectQuery.value(pos++).toDouble();
        frame.GroundSpeedEast_GPS = selectQuery.value(pos++).toDouble();

        frame.BombState = selectQuery.value(pos++).toLongLong();
        frame.RangefinderDistance = selectQuery.value(pos++).toDouble();

        frame.StabilizedCenterX = selectQuery.value(pos++).toInt();
        frame.StabilizedCenterY = selectQuery.value(pos++).toInt();
        frame.TrackedTargetCenterX = selectQuery.value(pos++).toInt();
        frame.TrackedTargetCenterY = selectQuery.value(pos++).toInt();
        frame.TrackedTargetRectWidth = selectQuery.value(pos++).toInt();
        frame.TrackedTargetRectHeight = selectQuery.value(pos++).toInt();
        frame.CalculatedTrackedTargetGPSLat = selectQuery.value(pos++).toDouble();
        frame.CalculatedTrackedTargetGPSLon = selectQuery.value(pos++).toDouble();
        frame.CalculatedTrackedTargetGPSHmsl = selectQuery.value(pos++).toDouble();
        frame.CalculatedTrackedTargetSpeed = selectQuery.value(pos++).toDouble();
        frame.CalculatedTrackedTargetDirection = selectQuery.value(pos++).toDouble();

        frame.TelemetryFrameNumber = selectQuery.value(pos++).toLongLong();
        frame.VideoFrameNumber = selectQuery.value(pos++).toLongLong();
        frame.SessionTimeMs = selectQuery.value(pos++).toLongLong();
        _telemetryFrames.append(frame);
    }
}

bool TelemetryDataStorage::deleteSession(const QString &sessionName)
{
    EnterProcStart("TelemetryDataStorage::deleteSession");

    if (sessionName.isEmpty() || sessionName == _sessionName)
        return false;

    QString sessionFolderName = _sessionFoldersDirectory + "/" + sessionName;
    QDir sessionFolder = QDir(sessionFolderName);
    return sessionFolder.removeRecursively();
}

bool TelemetryDataStorage::exportSessionToCSV(const QString &fileName)
{
    EnterProcStart("TelemetryDataStorage::exportSessionToCSV");

    QSqlQuery selectQuery = EXEC_SQL(_sessionDatabase,
                                     "SELECT DISTINCT "
                                     "TelemetryFrameNumber, VideoFrameNumber, FrameTime, SessionTimeMs, "
                                     "Roll, Pitch, Yaw, "
                                     "GPSLat, GPSLon, GPSHmsl, "
                                     "CamRoll, CamPitch, CamZoom, "
                                     "Altitude, VSpeed, AirSpeed, GroundSpeed_GPS, GroundSpeedNorth_GPS, GroundSpeedEast_GPS, "
                                     "WindDirection, WindSpeed, "
                                     "StabilizedCenterX, StabilizedCenterY, "
                                     "TargetCenterX, TargetCenterY, TargetRectWidth, TargetRectHeight, "
                                     "CalculatedTargetGPSLat, CalculatedTargetGPSLon, CalculatedTargetGPSHmsl, "
                                     "CalculatedTargetSpeed, CalculatedTargetDirection "
                                     "FROM TelemetryFrames "
                                     "ORDER BY TelemetryFrameNumber, VideoFrameNumber");

    if (selectQuery.lastError().isValid())
        return false;

    QFile file(fileName);

    if (!file.open(QFile::WriteOnly|QFile::Truncate))
        return false;

    QTextStream stream(&file);

    const char * separator = ";";

    QStringList columnNames = {
        "TelemetryFrameNumber", "VideoFrameNumber", "FrameTime", "SessionTimeMs", "Roll", "Pitch", "Yaw",
        "GPSLat", "GPSLon", "GPSHmsl", "CamRoll", "CamPitch", "CamZoom",
        "Altitude", "VSpeed", "AirSpeed", "GroundSpeed_GPS", "GroundSpeedNorth_GPS", "GroundSpeedEast_GPS",
        "WindDirection", "WindSpeed",
        "StabilizedCenterX", "StabilizedCenterY", "TargetCenterX", "TargetCenterY", "TargetRectWidth", "TargetRectHeight",
        "CalculatedTargetGPSLat", "CalculatedTargetGPSLon", "CalculatedTargetGPSHmsl",
        "CalculatedTargetSpeed", "CalculatedTargetDirection"
    };

    QString header = columnNames.join(separator) + "\n";
    stream << header;

    int pos = 0;

    while (selectQuery.next())
    {
        pos = 0;
        stream << selectQuery.value(pos++).toInt() << separator;       // TelemetryFrameNumber
        stream << selectQuery.value(pos++).toInt() << separator;       // VideoFrameNumber
        stream << selectQuery.value(pos++).toInt() << separator;       // FrameTime
        stream << selectQuery.value(pos++).toInt() << separator;       // SessionTimeMs
        stream << selectQuery.value(pos++).toDouble() << separator;    // Roll
        stream << selectQuery.value(pos++).toDouble() << separator;    // Pitch
        stream << selectQuery.value(pos++).toDouble() << separator;    // Yaw

        stream << selectQuery.value(pos++).toDouble() << separator;    // GPSLat
        stream << selectQuery.value(pos++).toDouble() << separator;    // GPSLon
        stream << selectQuery.value(pos++).toDouble() << separator;    // GPSHmsl
        stream << selectQuery.value(pos++).toDouble() << separator;    // CamRoll
        stream << selectQuery.value(pos++).toDouble() << separator;    // CamPitch
        stream << selectQuery.value(pos++).toDouble() << separator;    // CamZoom

        stream << selectQuery.value(pos++).toDouble() << separator;    // Altitude
        stream << selectQuery.value(pos++).toDouble() << separator;    // VSpeed
        stream << selectQuery.value(pos++).toDouble() << separator;    // AirSpeed
        stream << selectQuery.value(pos++).toDouble() << separator;    // GroundSpeed_GPS
        stream << selectQuery.value(pos++).toDouble() << separator;    // GroundSpeedNorth_GPS
        stream << selectQuery.value(pos++).toDouble() << separator;    // GroundSpeedEast_GPS

        stream << selectQuery.value(pos++).toDouble() << separator;    // WindDirection
        stream << selectQuery.value(pos++).toDouble() << separator;    // WindSpeed

        stream << selectQuery.value(pos++).toDouble() << separator;    // StabilizedCenterX
        stream << selectQuery.value(pos++).toDouble() << separator;    // StabilizedCenterY
        stream << selectQuery.value(pos++).toDouble() << separator;    // TargetCenterX
        stream << selectQuery.value(pos++).toDouble() << separator;    // TargetCenterY
        stream << selectQuery.value(pos++).toDouble() << separator;    // TargetRectWidth
        stream << selectQuery.value(pos++).toDouble() << separator;    // TargetRectHeight

        stream << selectQuery.value(pos++).toDouble() << separator;    // CalculatedTargetGPSLat
        stream << selectQuery.value(pos++).toDouble() << separator;    // CalculatedTargetGPSLon
        stream << selectQuery.value(pos++).toDouble() << separator;    // CalculatedTargetGPSHmsl

        stream << selectQuery.value(pos++).toDouble() << separator;    // CalculatedTargetSpeed
        stream << selectQuery.value(pos++).toDouble() << "\n";    // CalculatedTargetDirection
    }

    file.close();
    return true;
}

const QString &TelemetryDataStorage::getCurrentSessionName() const
{
    return _sessionName;
}

const QString TelemetryDataStorage::getScreenshotFolder() const
{
    QString screenshotsFolder = getSessionFolder() + "/" + SCREENSHOTS_FOLDER_NAME;
    QDir dir;
    dir.mkpath(screenshotsFolder);
    return screenshotsFolder;
}

int TelemetryDataStorage::getTelemetryDataFrameCount() const
{
    return _telemetryFrames.count();
}

const QVector<TelemetryDataFrame> &TelemetryDataStorage::getTelemetryDataFrames()
{
    return _telemetryFrames;
}

void TelemetryDataStorage::onDataReceived(const TelemetryDataFrame &telemetryFrame, const QImage &videoFrame)
{
    //0.
    _telemetryFrames.append(telemetryFrame);

    if (_workMode != WorkMode::RecordAndDisplay)
        return;
    EnterProcStart("TelemetryDataStorage::onDataReceived");
    //1. Telemetry
    if (_telemetryFrames.count() - _lastSavedTelemetryFrameIndex > FRAME_FLUSH_BATCH_SIZE)
        flushTelemetryDataFrames();

    //2. Video
    if (_lastVideoFrameNumber != (qint32)telemetryFrame.VideoFrameNumber)
    {
        if (!videoFrame.isNull())
        {
            QImage videoFrameForSave = videoFrame.copy();
            QPainter painter(&videoFrameForSave);
            painter.setRenderHint(QPainter::Antialiasing, true);
            QFont font = painter.font();
            font.setPointSize(14);
            painter.setFont(font);
            QPen pen = painter.pen();
            pen.setColor(Qt::green);
            painter.setPen(pen);
            if (_displayTelemetryOnVideo)
                drawTelemetryOnVideo(painter, telemetryFrame, _telemetryIndicatorFontSize, _isLaserRangefinderLicensed,  _telemetryTimeFormat);

            drawGimbalOnVideo(painter, _gimbalIndicatorType, _gimbalIndicatorAngles, _gimbalIndicatorSize, telemetryFrame);

            if (_displayTargetRectangleOnVideo && telemetryFrame.targetIsVisible())
            {
                const QColor targetRectColor = ((AutomaticTracerMode)telemetryFrame.CamTracerMode == AutomaticTracerMode::atmScreenPoint) ?  Qt::blue : Qt::red;
                pen.setColor(targetRectColor);
                painter.setPen(pen);
                drawTargetRectangleOnVideo(painter, telemetryFrame.targetRect());
            }

            _videoRecorder->saveFrame(videoFrameForSave);
        }
        _lastVideoFrameNumber = telemetryFrame.VideoFrameNumber;
    }
}

void TelemetryDataStorage::onClientCommandSent(const DataExchangePackage &clientCommand)
{
    if (_workMode != WorkMode::RecordAndDisplay)
        return;
    EnterProcStart("TelemetryDataStorage::onClientCommandSent");
    _clientCommands.append(clientCommand);
    if (_clientCommands.count() - _lastSavedClientCommandIndex > FRAME_FLUSH_BATCH_SIZE)
        flushClientCommands();
}

void TelemetryDataStorage::onArtillerySpotterDataExchange(const DataExchangePackage &dataPackage)
{
    if (_workMode != WorkMode::RecordAndDisplay)
        return;
    EnterProcStart("TelemetryDataStorage::onArtillerySpotterDataExchange");
    _artillerySpotterDataPackages.append(dataPackage);
    if (_artillerySpotterDataPackages.count() - _lastSavedArtillerySpotterDataPackageIndex > FRAME_FLUSH_BATCH_SIZE)
        flushArtillerySpotterDataPackages();
}

const TelemetryDataFrame TelemetryDataStorage::getTelemetryDataFrameByIndex(int frameIndex)
{
    if (_telemetryFrames.count() <= 0)
        qDebug() << "getTelemetryDataFrameByIndex crash";
    return _telemetryFrames[frameIndex];
}

const QVector<WeatherDataItem> TelemetryDataStorage::getWeatherData(quint32 lastMSeconds)
{
    const int WEATHER_ITEMS_ALTITUDE_STEP = 200; //m
    const int WEATHER_ITEMS_MAX_COUNT = 100; //range 0..20000 m

    int measureCount[WEATHER_ITEMS_MAX_COUNT] = { 0 };
    double windDirectionSum[WEATHER_ITEMS_MAX_COUNT] = { 0 };
    double windSpeedSum[WEATHER_ITEMS_MAX_COUNT] = { 0 };
    double atmospherePressureSum[WEATHER_ITEMS_MAX_COUNT] = { 0 };
    double atmosphereTemperatureSum[WEATHER_ITEMS_MAX_COUNT] = { 0 };

    quint32 frameCount = _telemetryFrames.count();
    quint32 endTime = _telemetryFrames.at(frameCount - 1).SessionTimeMs;
    quint32 prevTelemetryFrameNumber = 0;

    TelemetryDataFrame telemetryFrame;
    for (int i = frameCount - 1; i > 0; i--)
    {
        telemetryFrame = _telemetryFrames.at(i);

        if (prevTelemetryFrameNumber == telemetryFrame.TelemetryFrameNumber)
            continue;
        if (endTime - telemetryFrame.SessionTimeMs > lastMSeconds) // 1800000 ms = 30 minutes
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

    QVector<WeatherDataItem> weatherDataColl;
    WeatherDataItem weatherItem;
    for (int index = 0; index < WEATHER_ITEMS_MAX_COUNT; index++)
    {
        int count = measureCount[index];
        if (count > 0)
        {
            weatherItem.Altitude = index * WEATHER_ITEMS_ALTITUDE_STEP;
            weatherItem.WindDirection = windDirectionSum[index] / count;
            weatherItem.WindSpeed = windSpeedSum[index] / count;
            weatherItem.AtmospherePressure = atmospherePressureSum[index] / count;
            weatherItem.AtmosphereTemperature = atmosphereTemperatureSum[index] / count;
            weatherDataColl.append(weatherItem);
        }
    }

    return weatherDataColl;
}

const QString TelemetryDataStorage::getTelemetryFrameTimeAsString(const TelemetryDataFrame &telemetryFrame) const
{
    quint32 timeFromStart = telemetryFrame.SessionTimeMs;
    QString result = getTimeAsString(timeFromStart);
    return result;
}

const QString TelemetryDataStorage::getLastTelemetryFrameTimeAsString() const
{
    unsigned int timeMs = 0;
    int frameCount = _telemetryFrames.count();
    if (frameCount > 0)
        timeMs = _telemetryFrames[frameCount - 1].SessionTimeMs;
    QString result = getTimeAsString(timeMs);
    return result;
}

void TelemetryDataStorage::showStoredDataAsync(const TelemetryDataFrame &telemetryDataFrame)
{
    EnterProcStart("TelemetryDataStorage::showVideoFrameAsync");

    _telemetryDataFrameForAsyncShow = telemetryDataFrame;

    int telemetryFramesCount = _telemetryFrames.count();

    unsigned int frameNumber = -1;
    if (telemetryFramesCount > 0)
        frameNumber = telemetryDataFrame.VideoFrameNumber - _telemetryFrames[0].VideoFrameNumber;
    QString currentVideoFile = getVideoFileNameForFrame(getSessionFolder(), _sessionName, _sessionVideoFileFrameCount, frameNumber);

    if ((telemetryFramesCount > 0) && (_workMode == WorkMode::RecordAndDisplay))
    {
        unsigned int recordingFrameNumber =
                _telemetryFrames[telemetryFramesCount - 1].VideoFrameNumber -
                _telemetryFrames[0].VideoFrameNumber;
        QString recordingVideoFile = getVideoFileNameForFrame(getSessionFolder(), _sessionName, _sessionVideoFileFrameCount, recordingFrameNumber);
        if (recordingVideoFile == currentVideoFile)
        {
            emit storedDataReceived(_telemetryDataFrameForAsyncShow, *getVideoRecordingStubImage());
            return;
        }
    }

    if ((frameNumber == 0) && (_workMode == WorkMode::PlayStored) && !fileExists(currentVideoFile))
        emit videoFrameReceivedInternal(QImage());

    if (currentVideoFile != _lastOpenedVideoFile)
    {
        _lastOpenedVideoFile = currentVideoFile;
        _mediaPlayer->stop();
        QUrl fileUrl = QUrl::fromLocalFile(currentVideoFile);
        _mediaPlayer->setMedia(fileUrl);
        _mediaPlayer->play();
        _mediaPlayer->pause();
    }

    int time = int(1000.0 / VIDEO_FILE_FRAME_FREQUENCY * (frameNumber % _sessionVideoFileFrameCount));

    _mediaPlayer->setPosition(time);
}

QImage *TelemetryDataStorage::getVideoRecordingStubImage()
{
    EnterProcStart("TelemetryDataStorage::getVideoRecordingStubImage");
    if (_videoRecordingStubImage == nullptr)
    {
        unsigned int width = _videoRecorder->frameWidth();
        unsigned int height = _videoRecorder->frameHeight();
        if (width <= 0 || height <= 0)
        {
            width = 640;
            height = 480;
        }
        _videoRecordingStubImage = new QImage(width, height, QImage::Format_RGB32);
        QPainter painter(_videoRecordingStubImage);
        painter.setPen(QPen(Qt::red));
        QString text = tr("Recording video...");

        painter.setFont(QFont(painter.font().family(), 40));
        while (painter.fontMetrics().width(text) < width * 0.8)
        {
            int newSize = painter.font().pointSize() + 1;
            painter.setFont(QFont(painter.font().family(), newSize));
        }

        painter.drawText(QRect(0, 0, width, height), Qt::AlignCenter, text);
    }
    return _videoRecordingStubImage;
}

void TelemetryDataStorage::loadSessionInfos()
{
    EnterProcStart("TelemetryDataStorage::loadSessionInfos");

    EXEC_SQL(_sessionDatabase, "CREATE TABLE IF NOT EXISTS SessionInfo (InfoKey Varchar(255), InfoValue Varchar(255))");

    EXEC_SQL(_sessionDatabase, "CREATE UNIQUE INDEX IF NOT EXISTS Idx_SessionInfo_Key ON SessionInfo(InfoKey)");

    _sessionInfos.clear();
    QSqlQuery selectQuery = EXEC_SQL(_sessionDatabase, "SELECT InfoKey, InfoValue FROM SessionInfo");
    while (selectQuery.next())
        _sessionInfos.insert(selectQuery.value(0).toString(), selectQuery.value(1).toString());

    bool ok;
    _sessionVideoFileFrameCount = getSessionInfo(SessionInfo_VideoFileFrameCount).toInt(&ok);
    if (!ok)
        _sessionVideoFileFrameCount = VIDEO_FRAMES_PER_FILE_DEFAULT;
}

void TelemetryDataStorage::initSessionInfos()
{
    EnterProcStart("TelemetryDataStorage::initSessionInfos");

    //loadSessionInfos();

    QStorageInfo storeageInfo(_sessionFoldersDirectory);
    QString userName = qgetenv("USER");
    if (userName.isEmpty())
        userName = qgetenv("USERNAME");

    setSessionInfo(SessionInfo_FormatVersion,               QString::number(7));
    setSessionInfo(SessionInfo_LocalHostName,               QHostInfo::localHostName());
    setSessionInfo(SessionInfo_UserName,                    userName);
    setSessionInfo(SessionInfo_BeginDateTime,               QDateTime::currentDateTime().toString());
    setSessionInfo(SessionInfo_SessionFoldersDirectory,     _sessionFoldersDirectory);
    setSessionInfo(SessionInfo_TotalDiskSpace,              QString::number(storeageInfo.bytesTotal()));
    setSessionInfo(SessionInfo_AvailableDiskSpace,          QString::number(storeageInfo.bytesAvailable()));
    setSessionInfo(SessionInfo_FreeDiskSpace,               QString::number(storeageInfo.bytesFree()));
    setSessionInfo(SessionInfo_FileSystemType,              storeageInfo.fileSystemType()); // fileSystemType name
    setSessionInfo(SessionInfo_VideoFileFrameCount,         QString::number(_defaultVideoFileFrameCount));
    setSessionInfo(SessionInfo_VideoFileQuality,            QString::number(_defaultVideoFileQuality));
    //more information about network connections http://prog3.com/sbdm/blog/u013007900/article/details/50444459

    flushSessionInfos();
}

void TelemetryDataStorage::flushSessionInfos()
{
    if (!_sessionDatabase.isOpen() || (_workMode != WorkMode::RecordAndDisplay))
        return;
    EnterProcStart("TelemetryDataStorage::flushSessionInfos");


    _sessionDatabase.transaction();
    LOG_SQL_ERROR(_sessionDatabase);

    QSqlQuery insertQuery(_sessionDatabase);
    insertQuery.prepare("INSERT OR REPLACE INTO SessionInfo (InfoKey, InfoValue) VALUES (?, ?)");
    LOG_SQL_ERROR(insertQuery);

    auto keys = _sessionInfos.keys();
    foreach (auto k, keys)
    {
        auto v = _sessionInfos.value(k);
        insertQuery.addBindValue(k);
        insertQuery.addBindValue(v);
        insertQuery.exec();
        LOG_SQL_ERROR(insertQuery);
    }

    _sessionDatabase.commit();
    LOG_SQL_ERROR(_sessionDatabase);
}

void TelemetryDataStorage::setSessionInfo(const QString &key, const QString &value)
{
    _sessionInfos.insert(key, value);
}

const QString TelemetryDataStorage::getSessionInfo(const QString &key) const
{
    return _sessionInfos.value(key);
}

int TelemetryDataStorage::getSessionFormatVersion()
{
    bool ok;
    int version = getSessionInfo(SessionInfo_FormatVersion).toInt(&ok);
    if (!ok)
        version = 1;
    return version;
}

void TelemetryDataStorage::videoFrameReceivedInternal(const QImage &videoFrame)
{
    emit storedDataReceived(_telemetryDataFrameForAsyncShow, videoFrame);
}
