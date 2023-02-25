#ifndef MAPTILESIMPORTER_H
#define MAPTILESIMPORTER_H

#include <QObject>

class MapTilesImporter final : public QObject
{
    Q_OBJECT
public:
    explicit MapTilesImporter(QObject *parent = nullptr);
    void importMapFromMapnikFiles(const QString &destDB, const QString &srcFolderName, int sourceId);
    void importMapFromSASFiles(const QString &destDB, const QString &srcFolderName, int sourceId);
};

#endif // MAPTILESIMPORTER_H
