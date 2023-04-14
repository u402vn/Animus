#ifndef COMMONUTILS_H
#define COMMONUTILS_H

#include <QObject>
#include <QString>
#include <QSqlQuery>
#include <QSqlDatabase>
#include <QPainter>
#include <QFont>
#include <QPixmap>
#include <QColor>
#include "Common/CommonData.h"
#include "TelemetryDataFrame.h"
#include "Constants.h"

enum AnimusLicenseState {Uncalculated, Licended, Trial, Expired};

bool fileExists(const QString &fileName);
bool dirExists(const QString &dirName);
bool makeDir(const QString &dirName);
const QString getFixedFilePath(const QString &fileName);
void cleanupLogFolder(const QString logFolderPath, const QString currentLogFile, quint32 maximalSizeMb);
double GetCurrentDateTimeForDB();
double GetCurrentDateTimeFrom1970Secs();
const QString getTimeAsString(quint32 timeMs);
int randomBetween(int low, int high);
const QFont getMonospaceFont();

// Range (0, 360)
double constrainAngle360(double x);
// Range (-180, +180)
double constrainAngle180(double x);

const QString getAnimusExpiringTerm();
AnimusLicenseState getAnimusLicenseState();

#define LOG_SQL_ERROR(databaseObject) OutSqlDatabaseErrorInObjectMethod(&(databaseObject), this, __FUNCTION__);
void OutSqlDatabaseErrorInObjectMethod(const QSqlQuery *sqlQuery, QObject *executor, const QString functionName);
void OutSqlDatabaseErrorInObjectMethod(QSqlQuery **sqlQuery, QObject *executor, const QString functionName);
void OutSqlDatabaseErrorInObjectMethod(const QSqlDatabase *sqlDatabase, QObject *executor, const QString functionName);

#define EXEC_SQL(database, sql) ExecuteSQLInObjectMethod(database, sql, this, __FUNCTION__);
QSqlQuery ExecuteSQLInObjectMethod(const QSqlDatabase &database, const QString &sql, QObject *executor, const QString functionName);

void drawTelemetryOnVideo(QPainter &painter, const TelemetryDataFrame &telemetryFrame, quint32 fontSize,
                          bool showRangefinderDistance, OSDTelemetryTimeFormat timeFormat);
void drawGimbalOnVideo(QPainter &painter, const OSDGimbalIndicatorType gimbalIndicatorType, const OSDGimbalIndicatorAngles gimbalIndicatorAngles,
                       const quint32 size, const TelemetryDataFrame &telemetryFrame);
void drawTargetRectangleOnVideo(QPainter &painter, const QRect &targetRect);

const QPixmap changeImageColor(const QPixmap &srcImage, const QColor &newColor);

const WorldGPSCoord getUavCoordsFromTelemetry(const TelemetryDataFrame &telemetryFrame);
const WorldGPSCoord getRangefinderCoordsFromTelemetry(const TelemetryDataFrame &telemetryFrame);
const WorldGPSCoord getTrackedTargetCoordsFromTelemetry(const TelemetryDataFrame &telemetryFrame);
const WorldGPSCoord getBombingPlaceCoordsFromTelemetry(const TelemetryDataFrame &telemetryFrame);

#endif // COMMONUTILS_H
