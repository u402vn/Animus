#include "MapTilesExporter.h"
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <QtConcurrentRun>
#include "MapTileContainer.h"

void MapTilesExporter::processExport(const QList<ExportTask> tasks, const QString destDB)
{
    QSqlDatabase destinationDatabase = QSqlDatabase::addDatabase("QSQLITE", "DestDB");
    destinationDatabase.setDatabaseName(destDB);
    destinationDatabase.open();
    QSqlQuery destDBQuery(destinationDatabase);

    destDBQuery.exec("CREATE TABLE MapTile (x INTEGER, y INTEGER, scale INTEGER, sourceId INTEGER, format INTEGER, autogenerated INTEGER, datetime REAL, signature INTEGER, tile BLOB)");
    //qDebug() << "SqLite error:" << destDBQuery.lastError().text() << ", SqLite error code:" << destDBQuery.lastError().number();
    destDBQuery.exec("PRAGMA journal_mode = MEMORY");

    QSqlQuery copyQuery(destinationDatabase);

    QString prevSourceDatabasePath = "";
    int processedCount = 0;
    int tileTotalCount = 0;
    foreach (auto task, tasks)
        tileTotalCount += (task.X2 - task.X1) * (task.Y2 - task.Y1);

    foreach (auto task, tasks)
    {
        QString sourceDatabasePath = task.sourceDB;

        if (sourceDatabasePath.isEmpty())
            continue;
        if (sourceDatabasePath != prevSourceDatabasePath)
        {
            if (!prevSourceDatabasePath.isEmpty())
                destDBQuery.exec("DETACH DATABASE SrcDB");

            destDBQuery.exec("ATTACH DATABASE '" + sourceDatabasePath + "' AS SrcDB");

            copyQuery.prepare("INSERT INTO MapTile (x, y, scale, sourceId, format, autogenerated, datetime, signature, tile)" \
                              "SELECT x, y, scale, sourceId, format, autogenerated, datetime, signature, tile " \
                              "FROM SrcDB.MapTile WHERE x>=? AND x<=? AND y>=? AND y<=? AND scale=? AND sourceId=?");
            qDebug() << "SqLite error:" << destDBQuery.lastError().text() << ", SqLite error code:" << destDBQuery.lastError().number();

            prevSourceDatabasePath = sourceDatabasePath;
        }

        if (_cancelExecution)
            break;

        copyQuery.addBindValue(task.X1);
        copyQuery.addBindValue(task.X2);
        copyQuery.addBindValue(task.Y1);
        copyQuery.addBindValue(task.Y2);
        copyQuery.addBindValue(task.scale);
        copyQuery.addBindValue(task.sourceId);
        copyQuery.exec();

        processedCount += (task.X2 - task.X1) * (task.Y2 - task.Y1);
        emit exportProcessChanged(100.0 * processedCount / tileTotalCount);
    }

    destDBQuery.exec("DETACH DATABASE SrcDB");

    destDBQuery.exec("CREATE UNIQUE INDEX TILE_SCALE_X_Y_SourceID ON MapTile (x, y, scale, sourceId)");

    destDBQuery.clear();
    destinationDatabase.close();

    QSqlDatabase::removeDatabase("DestDB");

    emit exportProcessEnded();
}

MapTilesExporter::MapTilesExporter(QObject *parent) : QObject(parent)
{

}

MapTilesExporter::~MapTilesExporter()
{

}

void MapTilesExporter::AddExportTask(int sourceId, int scale, const WorldGPSCoord &coord1, const WorldGPSCoord &coord2, QList<QString> &sourceDBFiles)
{
    foreach (QString sourceDB, sourceDBFiles)
    {
        double fx1, fy1, fx2, fy2;
        if (sourceId == YandexSatellite || sourceId == YandexMap || sourceId == YandexHybrid)
        {
            ConvertGPS2YandexXY(coord1, scale, fx1, fy1);
            ConvertGPS2YandexXY(coord2, scale, fx2, fy2);
        }
        else
        {
            ConvertGPS2GoogleXY(coord1, scale, fx1, fy1);
            ConvertGPS2GoogleXY(coord2, scale, fx2, fy2);
        }

        int x1 = floor(fx1);
        int y1 = floor(fy1);
        int x2 = ceil(fx2);
        int y2 = ceil(fy2);

        int batchX = x2 - x1 + 1;
        int batchY = y2 - y1 + 1;
        const int MAX_BATCH_SIZE = 10000;

        while (batchX * batchY > MAX_BATCH_SIZE)
        {
            if (batchX > 1)
                batchX = batchX / 2;
            if (batchY > 1)
                batchY = batchX / 2;
        }

        for (int x = x1; x <= x2; x += batchX)
            for (int y = y1; y <= y2; y += batchY)
            {
                int toX = x + batchX - 1;
                if (toX > x2)
                    toX = x2;
                int toY = y + batchY - 1;
                if (toY > y2)
                    toY = y2;
                ExportTask task = {sourceId, scale, x, y, toX, toY, sourceDB};
                _tasks.append(task);
            }
    } // sourceDB
}

void MapTilesExporter::RunExport(const QString &destDB)
{
    _cancelExecution = false;
    QtConcurrent::run(this, &MapTilesExporter::processExport, _tasks, destDB);
}

void MapTilesExporter::clear()
{
    _cancelExecution = false;
    _tasks.clear();
}

void MapTilesExporter::cancelExport()
{
    _cancelExecution = true;
}
