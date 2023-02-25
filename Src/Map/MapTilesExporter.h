#ifndef MAPTILESEXPORTER_H
#define MAPTILESEXPORTER_H

#include <QObject>
#include <QProgressDialog>
#include <QList>
#include <QString>
#include "Common/CommonData.h"

// https://sqlite.org/c3ref/progress_handler.html - progress
// http://www.sqlite.org/lang_attach.html - attach database

class MapTilesExporter final : public QObject
{
    Q_OBJECT

    struct ExportTask final
    {
        int sourceId, scale, X1, Y1, X2, Y2;
        QString sourceDB;
    };

    bool _cancelExecution;
    QList<ExportTask> _tasks;

    void processExport(const QList<ExportTask> tasks, const QString destDB);
    void prepareProgressDialog(int tileTotalCount);
public:
    explicit MapTilesExporter(QObject *parent = nullptr);
    ~MapTilesExporter();

    void AddExportTask(int sourceId, int scale, const WorldGPSCoord &coord1, const WorldGPSCoord &coord2, QList<QString> &sourceDBFiles);
    void RunExport(const QString &destDB);
    void clear();
signals:
    void exportProcessChanged(double processedPrecent);
    void exportProcessEnded();
public slots:
    void cancelExport();
};

#endif // MAPTILESEXPORTER_H
