#ifndef TELEMETRYDATASTORAGE_H
#define TELEMETRYDATASTORAGE_H

#include <QObject>
#include <QVector>
#include <QMediaPlayer>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QImage>
#include "VideoRecorder/CameraFrameGrabber.h"
#include "TelemetryDataFrame.h"
#include "VideoRecorder/PartitionedVideoRecorder.h"
#include "Common/CommonData.h"
#include "Constants.h"

class TelemetryDataStorage final : public QObject
{
    Q_OBJECT
public:
    enum WorkMode
    {
        DisplayOnly,
        PlayStored,
        RecordAndDisplay
    };

    explicit TelemetryDataStorage(QObject *parent, const QString &sessionFolder,
                                  const quint32 videoFileFrameCount, const quint32 videoFileQuality,
                                  const bool displayTelemetryOnVideo,
                                  const OSDTelemetryTimeFormat telemetryTimeFormat,
                                  const bool displayTargetRectangleOnVideo,
                                  const OSDGimbalIndicatorType gimbalIndicatorType, const OSDGimbalIndicatorAngles gimbalIndicatorAngles,
                                  const quint32 gimbalIndicatorSize, const bool isLaserRangefinderLicensed);
    ~TelemetryDataStorage();

    WorkMode getWorkMode() const;

    const QStringList getStoredSessionsList() const;
    void stopSession();
    void newSession();
    void openSession(const QString &sessionName);
    bool deleteSession(const QString &sessionName);
    bool exportSessionToCSV(const QString &fileName);
    const QString &getCurrentSessionName() const;
    const QString getScreenshotFolder() const;

    int getTelemetryDataFrameCount() const;

    const QVector<TelemetryDataFrame> &getTelemetryDataFrames();

    const TelemetryDataFrame getTelemetryDataFrameByIndex(int frameIndex);

    const QVector<WeatherDataItem> getWeatherData(quint32 lastMSeconds = 1800000);

    const QString getTelemetryFrameTimeAsString(const TelemetryDataFrame &telemetryFrame) const;
    const QString getLastTelemetryFrameTimeAsString() const;

    void showStoredDataAsync(const TelemetryDataFrame &telemetryDataFrame);

public slots:
    void onDataReceived(const TelemetryDataFrame &telemetryFrame, const QImage &videoFrame);
    void onClientCommandSent(const DataExchangePackage &clientCommand);
    void onArtillerySpotterDataExchange(const DataExchangePackage &dataPackage);
private:
    const QString SessionInfo_FormatVersion             = "FormatVersion";
    const QString SessionInfo_LocalHostName             = "LocalHostName";
    const QString SessionInfo_UserName                  = "UserName";
    const QString SessionInfo_BeginDateTime             = "BeginDateTime";
    const QString SessionInfo_SessionFoldersDirectory   = "SessionFoldersDirectory";
    const QString SessionInfo_TotalDiskSpace            = "TotalDiskSpace";
    const QString SessionInfo_AvailableDiskSpace        = "AvailableDiskSpace";
    const QString SessionInfo_FreeDiskSpace             = "FreeDiskSpace";
    const QString SessionInfo_FileSystemType            = "FileSystemType";
    const QString SessionInfo_VideoFileFrameCount       = "SingleVideoFile";
    const QString SessionInfo_VideoFileQuality          = "VideoFileQuality";

    QString _sessionFoldersDirectory;
    quint32 _defaultVideoFileFrameCount;
    quint32 _defaultVideoFileQuality;
    QString _sessionName;
    quint32 _sessionVideoFileFrameCount;
    bool _displayTelemetryOnVideo, _displayTargetRectangleOnVideo;
    OSDTelemetryTimeFormat _telemetryTimeFormat;
    OSDGimbalIndicatorType _gimbalIndicatorType;
    OSDGimbalIndicatorAngles _gimbalIndicatorAngles;
    quint32 _gimbalIndicatorSize;
    bool _isLaserRangefinderLicensed;

    QVector<TelemetryDataFrame> _telemetryFrames;
    QVector<DataExchangePackage> _clientCommands;
    QVector<DataExchangePackage> _artillerySpotterDataPackages;
    PartitionedVideoRecorder * _videoRecorder;
    QMediaPlayer * _mediaPlayer;
    CameraFrameGrabber * _mediaPlayerFrameGraber;
    QSqlDatabase _sessionDatabase;
    WorkMode _workMode;
    bool _destroing;

    TelemetryDataFrame _telemetryDataFrameForAsyncShow;

    qint32 _lastSavedTelemetryFrameIndex;
    qint32 _lastSavedClientCommandIndex;
    qint32 _lastSavedArtillerySpotterDataPackageIndex;
    qint32 _lastVideoFrameNumber;
    QString _lastOpenedVideoFile;

    QImage * _videoRecordingStubImage;
    QImage *getVideoRecordingStubImage();

    QMap<QString, QString> _sessionInfos;
    void loadSessionInfos();
    void initSessionInfos();
    void flushSessionInfos();    
    void setSessionInfo(const QString &key, const QString &value);
    const QString getSessionInfo(const QString &key) const;
    int getSessionFormatVersion();

    void readTelemetryFrames();

    const QString getSessionFolder() const;
    void openTelemetryFramesDatabase();
    void flushTelemetryDataFrames();
    void flushClientCommands();
    void flushArtillerySpotterDataPackages();
signals:
    void storedDataReceived(const TelemetryDataFrame &telemetryFrame, const QImage &videoFrame);
    void workModeChanged();
private slots:
    void videoFrameReceivedInternal(const QImage &videoFrame);
};

#endif // TELEMETRYDATASTORAGE_H
