#ifndef MAPTILEEXPORTDIALOG_H
#define MAPTILEEXPORTDIALOG_H

#include <QObject>
#include <QWidget>
#include <QDialog>
#include <QMenu>
#include <QAction>
#include <QProgressDialog>
#include "Map/MapTileContainer.h"
#include "Map/MapTilesExporter.h"
#include "UserInterface/FilePathSelector.h"
#include "UserInterface/GPSCoordSelector.h"
#include "Common/CommonWidgets.h"

class MapTileExportDialog final: public QDialog
{
    Q_OBJECT

    QMenu *_selectMenu;
    QAction *_acSelectAll, *_acUnelectAll;

    WorldGPSCoord _coordLeftTop, _coordRightBottom;
    QLabelEx *_lblCoordLeftTop, *_lblCoordRightBottom;
    GPSCoordSelector  *_coordLeftTopSelector, *_coordRightBottomSelector;

    FilePathSelector * _fileSelector;
    MapTileContainer * _mapTileContainer;
    MapTilesExporter _mapTilesExporter;
    QProgressDialog * _progressDlg;

    void initWidgets();
    void showCoordValues();
    void runExport();
public:
    MapTileExportDialog(QWidget *parent, MapTileContainer *mapTileContainer,
                        const WorldGPSCoord &coordLeftTop, const WorldGPSCoord &coordRightBottom);
    ~MapTileExportDialog();
    virtual void accept();
private slots:
    void onSourceButtonClicked();
    void onScaleButtonClicked();

    void onLeftTopCoordSelectorChanged(const WorldGPSCoord &gpsCoord, const QString &description);
    void onRightBottomCoordSelectorChanged(const WorldGPSCoord &gpsCoord, const QString &description);

    void onExportProcessChanged(double processedPrecent);
    void onExportProcessEnded();
};

#endif // MAPTILEEXPORTDIALOG_H
