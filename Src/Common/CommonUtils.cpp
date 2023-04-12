#include "CommonUtils.h"
#include <QDateTime>
#include <QFileInfo>
#include <QSqlError>
#include <QDebug>
#include <QPoint>
#include <QFontMetrics>
#include <QDir>
#include <QtMath>
#include "Common/CommonWidgets.h"

static QDateTime beginDateTime1900 = QDateTime::fromString("1.30.1", "M.d.s"); // dateTime is January 30 in 1900 at 00:00:01.
double GetCurrentDateTimeForDB()
{
    QDateTime now = QDateTime::currentDateTime();
    qint64 dateInMsec = beginDateTime1900.msecsTo(now);
    double dateTimeResult = (double)dateInMsec / (24 * 60 * 60 * 1000);
    return dateTimeResult;
}

static QDateTime beginDateTime1970 = QDateTime::fromString("1970.01.01", "yyyy.MM.dd"); // dateTime is January 1 in 1970.
double GetCurrentDateTimeFrom1970Secs()
{
    QDateTime now = QDateTime::currentDateTime();
    qint64 dateInSecs = beginDateTime1970.secsTo(now);
    double dateTimeResult = (double)dateInSecs;
    return dateTimeResult;
}

const QString getTimeAsString(quint32 timeMs)
{
    quint32 msec = timeMs % 1000;
    quint32 totalSec = timeMs / 1000;
    quint32 sec = totalSec % 60;
    quint32 min = totalSec / 60 % 60;
    quint32 hour = totalSec / 3600;

    QChar zeroChar = QChar('0');
    QString result = QString("%1:%2:%3.%4").arg(hour, 2, 10, zeroChar).arg(min, 2, 10, zeroChar).arg(sec, 2, 10, zeroChar).arg(msec, 3, 10, zeroChar);
    return result;
}

bool fileExists(const QString &fileName)
{
    QFileInfo checkFile(fileName);
    bool result = checkFile.exists() && checkFile.isFile();
    return result;
}

bool dirExists(const QString &dirName)
{
    QFileInfo checkFile(dirName);
    bool result =  checkFile.exists() && checkFile.isDir();
    return result;
}

const QString getFixedFilePath(const QString &fileName)
{
    QFileInfo fileInfo(fileName);
    QString result = fileInfo.filePath();
    return result;
}

int randomBetween(int low, int high)
{
    return (qrand() % ((high + 1) - low) + low);
}


inline bool isFixedPitch(const QFont & font)
{
    const QFontInfo fi(font);
    //    qDebug() << fi.family() << fi.fixedPitch();
    return fi.fixedPitch();
}

const QFont getMonospaceFont()
{
    QFont font("monospace");
    if (isFixedPitch(font))
        return font;
    font.setStyleHint(QFont::Monospace);
    if (isFixedPitch(font))
        return font;
    font.setStyleHint(QFont::TypeWriter);
    if (isFixedPitch(font))
        return font;
    font.setFamily("courier");
    if (isFixedPitch(font))
        return font;
    return font;
}

void OutSqlDatabaseErrorInObjectMethod(const QSqlQuery *sqlQuery, QObject *executor, const QString functionName)
{
    QSqlError error = sqlQuery->lastError();
    if (error.isValid())
    {
        QString errorInfo = QString("SQL Query Error in %1.%2: '%3', Code: %4")
                .arg(executor->metaObject()->className()).arg(functionName).arg(error.text()).arg(error.nativeErrorCode());
        qDebug() << errorInfo;
    }
}

void OutSqlDatabaseErrorInObjectMethod(QSqlQuery **sqlQuery, QObject *executor, const QString functionName)
{
    OutSqlDatabaseErrorInObjectMethod(*sqlQuery, executor, functionName);
}

void OutSqlDatabaseErrorInObjectMethod(const QSqlDatabase *sqlDatabase, QObject *executor, const QString functionName)
{
    QSqlError error = sqlDatabase->lastError();
    if (error.isValid())
    {
        QString errorInfo = QString("SQL Database Error in %1.%2: '%3'")
                .arg(executor->metaObject()->className()).arg(functionName).arg(error.text());
        qDebug() << errorInfo;
    }
}

QSqlQuery ExecuteSQLInObjectMethod(const QSqlDatabase &database, const QString &sql, QObject *executor, const QString functionName)
{
    QSqlQuery result =  database.exec(sql);
    OutSqlDatabaseErrorInObjectMethod(&result, executor, functionName);
    return result;
}

double constrainAngle360(double x)
{
    x = fmod(x, 360.0);
    if (x < 0.0)
        x += 360.0;
    return x;
}

double constrainAngle180(double x)
{
    x = fmod(x + 180.0, 360.0);
    if (x <= 0.0)
        x += 360.0;
    return x - 180.0;
}

const QString objectCoordinateAsStr(const WorldGPSCoord &coord)
{
    if (coord.isIncorrect())
        return "-";
    else
        return QString("%1 %2")
                .arg(coord.EncodeLatitude(GeographicalCoordinatesFormat::DegreeMinutesSeconds))
                .arg(coord.EncodeLongitude(GeographicalCoordinatesFormat::DegreeMinutesSeconds));
}

void drawTelemetryOnVideo(QPainter &painter, const TelemetryDataFrame &telemetryFrame,
                          bool showRangefinderDistance, OSDTelemetryTimeFormat timeFormat)
{
    WorldGPSCoord uavCoord(telemetryFrame.UavLatitude_GPS, telemetryFrame.UavLongitude_GPS, telemetryFrame.UavAltitude_GPS);
    WorldGPSCoord trackedCoord(telemetryFrame.CalculatedTrackedTargetGPSLat, telemetryFrame.CalculatedTrackedTargetGPSLon, telemetryFrame.CalculatedTrackedTargetGPSHmsl);
    WorldGPSCoord laserCoord(telemetryFrame.CalculatedRangefinderGPSLat, telemetryFrame.CalculatedRangefinderGPSLon, telemetryFrame.CalculatedRangefinderGPSHmsl);
    if (telemetryFrame.RangefinderDistance <= 0)
        laserCoord.setIncorrect();

    QString info;
    if (showRangefinderDistance)
    {
        QString rangefinderDistanceStr = telemetryFrame.RangefinderDistance > 0 ?
                    QString::number(telemetryFrame.RangefinderDistance, 'f', 1) : "-";

        info = QString("S: %1 Y: %2 H: %3 \n" \
                       "LT: %4 LR: %5 \n" \
                       "UAV: %6 \nTGT: %7 \nLSR: %8")
                .arg(telemetryFrame.GroundSpeed_GPS, 0, 'f', 1)
                .arg(constrainAngle360(telemetryFrame.UavYaw), 0, 'f', 1)
                .arg(telemetryFrame.UavAltitude_Barometric, 0, 'f', 1)
                .arg(telemetryFrame.RangefinderTemperature, 0, 'f', 1)
                .arg(rangefinderDistanceStr)
                .arg(objectCoordinateAsStr(uavCoord))
                .arg(objectCoordinateAsStr(trackedCoord))
                .arg(objectCoordinateAsStr(laserCoord));
    }
    else
        info = QString("%1 \n S: %2 Y: %3 \n H: %4")
                .arg(objectCoordinateAsStr(uavCoord))
                .arg(telemetryFrame.GroundSpeed_GPS, 0, 'f', 1)
                .arg(constrainAngle360(telemetryFrame.UavYaw), 0, 'f', 1)
                .arg(telemetryFrame.UavAltitude_Barometric, 0, 'f', 1);

    QPoint drawTelemetryPoint(5, painter.device()->height());
    CommonWidgetUtils::drawText(painter, drawTelemetryPoint, Qt::AlignBottom | Qt::AlignLeft, info, true);


    if (timeFormat == OSDTelemetryTimeFormat::SessionTime)
    {
        int hours = telemetryFrame.SessionTimeMs / 3600000;
        int hoursMs = hours * 3600000;
        int minutes = (telemetryFrame.SessionTimeMs - hoursMs) / 60000;
        int minutesMs = minutes * 60000;
        int seconds = (telemetryFrame.SessionTimeMs - hoursMs - minutesMs) / 1000;
        int secondsMs = seconds * 1000;
        int mseconds = telemetryFrame.SessionTimeMs - hoursMs - minutesMs - secondsMs;
        auto zeroChar = QLatin1Char('0');
        QString sessinTimeStr = QString("%1:%2:%3:%4")
                .arg(hours, 2, 10,  zeroChar)
                .arg(minutes, 2, 10,  zeroChar)
                .arg(seconds, 2, 10,  zeroChar)
                .arg(mseconds / 10, 2, 10, zeroChar);
        QPoint drawSessionTimePoint(painter.device()->width() - 5, painter.device()->height());
        CommonWidgetUtils::drawText(painter, drawSessionTimePoint, Qt::AlignBottom | Qt::AlignRight, sessinTimeStr, true);
    }
    else if (timeFormat == OSDTelemetryTimeFormat::CurrentDateTime)
    {
        QDateTime date = QDateTime::currentDateTime();
        QString sessinTimeStr = QString("%1 FPS: %2").arg(date.toString("dd.MM.yy hh:mm:ss")).arg(telemetryFrame.VideoFPS);
        QPoint drawSessionTimePoint(painter.device()->width() - 5, painter.device()->height());
        CommonWidgetUtils::drawText(painter, drawSessionTimePoint, Qt::AlignBottom | Qt::AlignRight, sessinTimeStr, true);
    }
}

void drawTextInShadowRect(QPainter &painter, int centerX, int topY, const QString &text)
{
    if (!text.isEmpty())
    {
        QFontMetrics fm = painter.fontMetrics();
        QRect boundingRect = fm.boundingRect(text);
        QColor shadowColor(0, 0, 0, 128);
        boundingRect.moveCenter(QPoint(centerX, topY + boundingRect.height()));
        painter.fillRect(boundingRect, shadowColor);
        painter.drawText(boundingRect, Qt::AlignTop | Qt::AlignHCenter, text);
    }
}

void drawGimbalOnVideoInternalPart(const QPixmap &planePix, QPainter &painter, int x, int y, int planeR, int sectorR,
                                   double camAngle, double fowSize, double planeAngle, int sectorAngle1, int sectorAngle2,
                                   const QString &text, double presentationAngle)
{
    painter.save();

    QColor sectorColor(0, 0, 0, 128);
    QColor fowColor(0, 255, 0, 128);
    QColor axisColor(0, 255, 0);

    QPen zeroPen = QPen(painter.pen());
    zeroPen.setStyle(Qt::DotLine);
    zeroPen.setColor(axisColor);

    QPen axisPen = QPen(painter.pen());
    axisPen.setStyle(Qt::DashLine);
    axisPen.setColor(axisColor);

    QPen fowPen = QPen(painter.pen());

    drawTextInShadowRect(painter, x, y + sectorR, text);

    const double LineBugDrawingFix = 1;

    QRect planeRect(-planeR, -planeR, 2 * planeR, 2 * planeR);
    QRect sectorRect(-sectorR, -sectorR, 2 * sectorR, 2 * sectorR);

    painter.translate(QPointF(x, y));

    if (presentationAngle != 0)
        painter.rotate(presentationAngle);

    painter.setPen(zeroPen);
    painter.drawLine(-sectorR, 0, +sectorR, LineBugDrawingFix);

    //Draw FOV
    painter.setPen(fowPen);
    double startAngle = - camAngle - fowSize / 2;
    painter.setBrush(fowColor);
    painter.drawPie(sectorRect, 16 * startAngle, 16 * fowSize);

    painter.setPen(axisPen);
    double axisAngleRad = -qDegreesToRadians(startAngle + fowSize / 2);
    painter.drawLine(0, 0, sectorR * qCos(axisAngleRad) + LineBugDrawingFix, sectorR * qSin(axisAngleRad) + LineBugDrawingFix);

    painter.rotate(planeAngle);

    //Draw moving range
    painter.setBrush(sectorColor);
    painter.drawPie(sectorRect, 16 * sectorAngle1, 16 * (sectorAngle2 - sectorAngle1));

    painter.drawPixmap(planeRect, planePix);

    painter.restore();
}

const QString gimbalAngleAsString(double angle)
{
    angle = constrainAngle180(angle);
    static const QString angleText = "%1°";
    QString text = angleText.arg(angle, 7, 'f', 1, ' ');
    return text;
}

void drawGimbalOnVideo(QPainter &painter, const OSDGimbalIndicatorType gimbalIndicatorType,
                       const OSDGimbalIndicatorAngles gimbalIndicatorAngles,
                       const quint32 size,
                       const TelemetryDataFrame &telemetryFrame)
{
    double presentationAngleYaw = -90;
    double presentationAnglePitch = 0;
    double camYaw =     telemetryFrame.recalculatedCamYaw();
    double camPitch =   telemetryFrame.CamPitch;
    double planeYaw =   telemetryFrame.UavYaw;
    double planePitch = telemetryFrame.UavPitch;

    switch (gimbalIndicatorType)
    {
    case OSDGimbalIndicatorType::StaticPlane:
        presentationAngleYaw -= telemetryFrame.UavYaw;
        presentationAnglePitch -= telemetryFrame.UavPitch;
        break;
    case OSDGimbalIndicatorType::RotatingPlane:
        break;
    case OSDGimbalIndicatorType::CombinedPresentation:
        presentationAngleYaw -= telemetryFrame.UavYaw;
        planePitch = 0;
        break;
    default:
        return;
    };

    QString yawText, pitchText;
    if (gimbalIndicatorAngles == OSDGimbalIndicatorAngles::RelativeAngles)
    {
        yawText = gimbalAngleAsString(telemetryFrame.UavYaw - camYaw);
        pitchText = gimbalAngleAsString(telemetryFrame.UavPitch - camPitch);
    }
    else
    {
        yawText = gimbalAngleAsString(camYaw);
        pitchText = gimbalAngleAsString(camPitch);
    }

    static QPixmap planeTopPix(":/plane_top.png");
    static QPixmap planeLeftPix(":/plane_left.png");

    int screenHeight = painter.device()->height();

    int sectorR = size;
    int planeR = sectorR / 3;
    int pointSize = planeR < 2 ? 1 : planeR * .7;

    painter.save(); // A

    QFont font = painter.font();
    font.setPointSize(pointSize);
    painter.setFont(font);

    // Yaw
    {
        int x =  sectorR + 5;
        int y =  screenHeight / 2 + 20 + sectorR;
        drawGimbalOnVideoInternalPart(planeTopPix, painter, x, y, planeR, sectorR,
                                      camYaw, telemetryFrame.FOVHorizontalAngle, planeYaw,
                                      -90, +90, yawText, presentationAngleYaw);
    }
    // Pitch
    {
        int x =  sectorR + 5;
        int y =  screenHeight / 2 - 20 - sectorR;
        drawGimbalOnVideoInternalPart(planeLeftPix, painter, x, y, planeR, sectorR,
                                      camPitch, telemetryFrame.FOVVerticalAngle, planePitch,
                                      +30, -120, pitchText, presentationAnglePitch);
    }

    //View
    if (gimbalIndicatorAngles != OSDGimbalIndicatorAngles::NoAngles)
    {
        int x =  sectorR + 5;
        int y =  screenHeight / 2 - 20 - 4 * sectorR;

        int digits = (telemetryFrame.FOVHorizontalAngle > 10) || (telemetryFrame.FOVVerticalAngle > 10) ? 1 : 2;
        QString text = QString("  %1x: %2° x %3°")
                .arg(telemetryFrame.CamZoom)
                .arg(telemetryFrame.FOVHorizontalAngle, 0, 'f', digits)
                .arg(telemetryFrame.FOVVerticalAngle, 0, 'f', digits);

        drawTextInShadowRect(painter, x, y + sectorR, text);
    }

    painter.restore(); // A
}

const qint32 UnlimitedLicenseRemainingDays = 1000000;

qint32 getRemainingDays()
{
    //qint32 result = qRound(44510 - GetCurrentDateTimeForDB());
    qint32 result = UnlimitedLicenseRemainingDays;
    return result;
}

const QString getAnimusExpiringTerm()
{
    qint32 remainingDays = getRemainingDays();
    QString info;
    if (remainingDays < UnlimitedLicenseRemainingDays)
    {
        info = remainingDays > 0 ? QObject::tr("Animus license expires in %1 day(s).").arg(remainingDays) :
                                   QObject::tr("Animus license expired %1 day(s) ago.").arg(-remainingDays);
    }
    else
        info = QObject::tr("Animus has unlimited license.");
    return info;
}

static AnimusLicenseState computedAnimusLicenseState = AnimusLicenseState::Uncalculated;

AnimusLicenseState getAnimusLicenseState()
{
    if (computedAnimusLicenseState == AnimusLicenseState::Uncalculated)
    {
        qInfo() << getAnimusExpiringTerm();
        qint32 remainingDays = getRemainingDays();
        if (remainingDays == UnlimitedLicenseRemainingDays)
            computedAnimusLicenseState = AnimusLicenseState::Licended;
        else if (remainingDays >= 0)
            computedAnimusLicenseState = AnimusLicenseState::Trial;
        else
            computedAnimusLicenseState = AnimusLicenseState::Expired;
    }
    return computedAnimusLicenseState;
}

void drawTargetRectangleOnVideo(QPainter &painter, const QRect &targetRect)
{
    const float FrameCornerSize = 0.2f;
    int x = targetRect.x();
    int y = targetRect.y();
    int w = targetRect.width();
    int h = targetRect.height();

    QVector<QLineF> frameLines;

    frameLines.append(QLine(x, y, x + w * FrameCornerSize, y));
    frameLines.append(QLine(x, y, x, y + h * FrameCornerSize));
    frameLines.append(QLine(x + w, y, x + w * (1 - FrameCornerSize), y));
    frameLines.append(QLine(x + w, y, x + w, y + h * FrameCornerSize));
    frameLines.append(QLine(x, y + h, x + w * FrameCornerSize, y + h));
    frameLines.append(QLine(x, y + h, x, y + h * (1 - FrameCornerSize)));
    frameLines.append(QLine(x + w, y + h, x + w * (1 - FrameCornerSize), y + h));
    frameLines.append(QLine(x + w, y + h, x + w, y + h * (1 - FrameCornerSize)));

    painter.drawLines(frameLines);
}

const QPixmap changeImageColor(const QPixmap &srcImage, const QColor &newColor)
{
    QImage image = srcImage.toImage();

    int width = image.width();
    int height = image.height();

    int r = newColor.red();
    int g = newColor.green();
    int b = newColor.blue();

    for (int i = 0; i < width; i++)
        for (int j = 0; j < height; j++)
        {
            QRgb rgb = image.pixel(j, i);
            float single = (255 - qMax(qMax(qRed(rgb), qGreen(rgb)), qBlue(rgb))) / 255;
            int alpha = qAlpha(rgb);

            if (alpha > 0)
            {
                rgb = qRgba(single * r, single * g, single * b, alpha);
                image.setPixel(j, i, rgb);
            }
        }
    QPixmap result;
    result.convertFromImage(image);
    return result;
}

const WorldGPSCoord getUavCoordsFromTelemetry(const TelemetryDataFrame &telemetryFrame)
{
    WorldGPSCoord uavCoords(telemetryFrame.UavLatitude_GPS, telemetryFrame.UavLongitude_GPS, telemetryFrame.UavAltitude_GPS);
    if (telemetryFrame.TelemetryFrameNumber <= 0)
        uavCoords.setIncorrect();
    return uavCoords;
}

const WorldGPSCoord getRangefinderCoordsFromTelemetry(const TelemetryDataFrame &telemetryFrame)
{
    WorldGPSCoord rangefinderCoords(telemetryFrame.CalculatedRangefinderGPSLat,
                                    telemetryFrame.CalculatedRangefinderGPSLon,
                                    telemetryFrame.CalculatedRangefinderGPSHmsl);
    if (telemetryFrame.TelemetryFrameNumber <= 0)
        rangefinderCoords.setIncorrect();
    return rangefinderCoords;
}

const WorldGPSCoord getTrackedTargetCoordsFromTelemetry(const TelemetryDataFrame &telemetryFrame)
{
    WorldGPSCoord trackedTargetCoords(telemetryFrame.CalculatedTrackedTargetGPSLat,
                                      telemetryFrame.CalculatedTrackedTargetGPSLon,
                                      telemetryFrame.CalculatedTrackedTargetGPSHmsl);
    if (telemetryFrame.TelemetryFrameNumber <= 0)
        trackedTargetCoords.setIncorrect();
    return trackedTargetCoords;
}

const WorldGPSCoord getBombingPlaceCoordsFromTelemetry(const TelemetryDataFrame &telemetryFrame)
{
    WorldGPSCoord trackedTargetCoords(telemetryFrame.BombingPlacePosLat,
                                      telemetryFrame.BombingPlacePosLon,
                                      telemetryFrame.BombingPlacePosHmsl);
    if (telemetryFrame.TelemetryFrameNumber <= 0)
        trackedTargetCoords.setIncorrect();
    return trackedTargetCoords;
}

bool makeDir(const QString &dirName)
{
    if (dirExists(dirName))
        return true;
    return QDir().mkpath(dirName);
}

void cleanupLogFolder(const QString logFolderPath, const QString currentLogFile, quint32 maximalSizeMb)
{
    QFileInfo currentLogFileInfo(currentLogFile);
    QString excludedLogFileName = currentLogFileInfo.fileName();

    QDir dir(logFolderPath);
    QFileInfoList files = dir.entryInfoList(QDir::NoFilter, QDir::Time);
    quint64 filesTotalSizeBytes = 0;
    quint64 maximalSizeBytes = 1024 * 1024 * maximalSizeMb;
    foreach (QFileInfo file, files)
    {
        filesTotalSizeBytes += file.size();

        QString fileName = file.fileName();
        if ((excludedLogFileName != fileName) && (filesTotalSizeBytes > maximalSizeBytes) )
        {
            bool result = dir.remove(file.filePath());
            qInfo() << QString("Remove log file %1 - %2").arg(fileName).arg(result ? "Ok" : "Error");
        }
    }
}
