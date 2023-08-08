#include "MapTileExportDialog.h"
#include <QGridLayout>
#include <QStyle>
#include <QRect>
#include <QSize>
#include <QCheckBox>
#include <QPushButton>
#include <QDialogButtonBox>
#include <QSizePolicy>
#include "EnterProc.h"


const char *propertyDataSourceId = "dataSourceId";
const char *propertyTileScale = "tileScale";

void MapTileExportDialog::initWidgets()
{
    _selectMenu = new QMenu(this);
    _acSelectAll = CommonWidgetUtils::createMenuAction(tr("Select All"), _selectMenu);
    _acUnelectAll = CommonWidgetUtils::createMenuAction(tr("Unselect All"), _selectMenu);

    _coordLeftTopSelector = new GPSCoordSelector(this);
    _coordLeftTopSelector->setDescriptionVisible(false);
    connect(_coordLeftTopSelector, &GPSCoordSelector::onCoordSelectorChanged, this, &MapTileExportDialog::onLeftTopCoordSelectorChanged);
    _coordRightBottomSelector = new GPSCoordSelector(this);
    _coordRightBottomSelector->setDescriptionVisible(false);
    connect(_coordRightBottomSelector, &GPSCoordSelector::onCoordSelectorChanged, this, &MapTileExportDialog::onRightBottomCoordSelectorChanged);

    auto mainGrid = new QGridLayout();
    mainGrid->setColumnStretch(1, 1);
    mainGrid->setColumnStretch(2, 0);
    this->setLayout(mainGrid);
    int row = 0;

    _lblCoordLeftTop = new QLabelEx(this);
    connect(_lblCoordLeftTop, &QLabelEx::clicked, this,  [=]()
    {
        _coordLeftTopSelector->show(QCursor::pos(), _coordLeftTop, "");
    });
    _lblCoordRightBottom = new QLabelEx(this);
    connect(_lblCoordRightBottom, &QLabelEx::clicked, this,  [=]()
    {
        _coordRightBottomSelector->show(QCursor::pos(), _coordRightBottom, "");
    });

    mainGrid->addWidget(_lblCoordLeftTop,                row, 1, 1, 10);
    row++;
    mainGrid->addWidget(_lblCoordRightBottom,            row, 1, 1, 10);
    row++;

    for (int i = 1; i <= 20; i++)
    {
        auto btnScale = new QPushButton(QString::number(i), this);
        btnScale->setMinimumWidth(btnScale->height());

        btnScale->setProperty(propertyTileScale, i);
        connect(btnScale, &QPushButton::clicked, this, &MapTileExportDialog::onScaleButtonClicked);
        mainGrid->addWidget(btnScale,            row, i + 1, 1, 1);
    }
    row++;

    auto sourceInfos = _mapTileContainer->getTileSourceInfos();
    foreach (auto sourceInfo, sourceInfos)
    {
        int sourceId = sourceInfo->sourceId();
        if (sourceId < 0)
            continue;

        bool enabled = sourceInfo->dbConnections()->count() > 0;
        QString caption = _mapTileContainer->getTileSourceName(sourceId);

        auto btnSource = new QPushButton(caption, this);
        btnSource->setEnabled(enabled);
        btnSource->setProperty(propertyDataSourceId, sourceId);
        connect(btnSource, &QPushButton::clicked, this, &MapTileExportDialog::onSourceButtonClicked);
        mainGrid->addWidget(btnSource,            row, 1, 1, 1);
        mainGrid->setRowStretch(row, 0);

        for (int i = 1; i <= 20; i++)
        {
            auto chkScale = new QCheckBox("", this); //QString::number(i)
            chkScale->setToolTip(QString("%1 - %2").arg(caption).arg(i));
            chkScale->setEnabled(enabled);
            chkScale->setProperty(propertyDataSourceId, sourceId);
            chkScale->setProperty(propertyTileScale, i);
            mainGrid->addWidget(chkScale,            row, i + 1, 1, 1);
        }
        row++;
    }

    mainGrid->addWidget(CommonWidgetUtils::createSeparator(this),   row, 0, 1, 22);
    mainGrid->setRowStretch(row, 0);
    row++;

    _fileSelector = new FilePathSelector(this, tr("Target Database File"), tr("Select Target Database File"), tr("Database Files (*.db)"));
    _fileSelector->setLabelWidth(200);

    mainGrid->addWidget(_fileSelector,            row, 1, 1, 21);
    mainGrid->setRowStretch(row, 0);
    row++;


    mainGrid->setRowStretch(row, 1);
    row++;

    auto buttonBox = new QDialogButtonBox(this);
    buttonBox->addButton(tr("Export"), QDialogButtonBox::AcceptRole);
    buttonBox->addButton(QDialogButtonBox::Cancel);
    buttonBox->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &MapTileExportDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &MapTileExportDialog::reject);

    mainGrid->addWidget(buttonBox,   row, 0, 1, 22);
    mainGrid->setRowStretch(row, 0);
    row++;

}

MapTileExportDialog::MapTileExportDialog(QWidget *parent, MapTileContainer *mapTileContainer,
                                         const WorldGPSCoord &coordLeftTop, const WorldGPSCoord &coordRightBottom
                                         ) : QDialog(parent)
{
    EnterProcStart("MapTileExportDialog::MapTileExportDialog");

    this->setWindowTitle(tr("Map Export"));
    this->setModal(true);

    CommonWidgetUtils::updateWidgetGeometry(this, 1000);

    this->setContentsMargins(0, 0, 0, 0);
    this->setAttribute(Qt::WA_DeleteOnClose, true);

    _mapTileContainer = mapTileContainer;
    _coordLeftTop = coordLeftTop;
    _coordRightBottom = coordRightBottom;

    _progressDlg = new QProgressDialog(tr("Export in progress..."), tr("Cancel"), 0, 100);
    _progressDlg->setMinimumWidth(600);
    _progressDlg->setWindowTitle(tr("Map Export"));
    _progressDlg->setWindowModality(Qt::WindowModal);
    _progressDlg->setAutoClose(true);
    _progressDlg->close();
    connect(_progressDlg, &QProgressDialog::canceled, &_mapTilesExporter, &MapTilesExporter::cancelExport);

    connect(&_mapTilesExporter, &MapTilesExporter::exportProcessChanged, this, &MapTileExportDialog::onExportProcessChanged);
    connect(&_mapTilesExporter, &MapTilesExporter::exportProcessEnded, this, &MapTileExportDialog::onExportProcessEnded);

    initWidgets();

    showCoordValues();
}

MapTileExportDialog::~MapTileExportDialog()
{

}

void MapTileExportDialog::accept()
{
    auto targetDatabaseFileName = _fileSelector->getSelectedPath();

    if (!targetDatabaseFileName.isEmpty())
    {
        runExport();
        //  done(QDialog::Accepted);
    }
}

void MapTileExportDialog::onSourceButtonClicked()
{
    QAction *action = _selectMenu->exec(QCursor::pos());
    bool checked = (action == _acSelectAll);

    QPushButton * btnSource = qobject_cast<QPushButton *>(sender());
    int sourceId = btnSource->property(propertyDataSourceId).toInt();

    QList<QCheckBox *> checkBoxes = this->findChildren<QCheckBox *>();
    foreach (auto checkBox, checkBoxes)
    {
        if (!checkBox->isEnabled())
            continue;
        int checkBoxSourceId = checkBox->property(propertyDataSourceId).toInt();
        if (checkBoxSourceId == sourceId)
            checkBox->setChecked(checked);
    }
}

void MapTileExportDialog::onScaleButtonClicked()
{
    QAction *action = _selectMenu->exec(QCursor::pos());
    bool checked = (action == _acSelectAll);

    QPushButton * btnScale = qobject_cast<QPushButton *>(sender());
    int scale = btnScale->property(propertyTileScale).toInt();

    QList<QCheckBox *> checkBoxes = this->findChildren<QCheckBox *>();
    foreach (auto checkBox, checkBoxes)
    {
        if (!checkBox->isEnabled())
            continue;
        int checkBoxScale = checkBox->property(propertyTileScale).toInt();
        if (checkBoxScale == scale)
            checkBox->setChecked(checked);
    }
}

void MapTileExportDialog::onLeftTopCoordSelectorChanged(const WorldGPSCoord &gpsCoord, const QString &description)
{
    _coordLeftTop = gpsCoord;
    showCoordValues();
}

void MapTileExportDialog::onRightBottomCoordSelectorChanged(const WorldGPSCoord &gpsCoord, const QString &description)
{
    _coordRightBottom = gpsCoord;
    showCoordValues();
}

void MapTileExportDialog::onExportProcessChanged(double processedPrecent)
{
    _progressDlg->setValue(ceil(processedPrecent));
}

void MapTileExportDialog::onExportProcessEnded()
{
    _progressDlg->close();
}

void MapTileExportDialog::showCoordValues()
{
    _lblCoordLeftTop->setText(QString(tr("From\t\t Lat:%1\t\tLon:%2"))
                              .arg(_coordLeftTop.EncodeLatitude(DegreeMinutesSecondsF))
                              .arg(_coordLeftTop.EncodeLongitude(DegreeMinutesSecondsF)));

    _lblCoordRightBottom->setText(QString(tr("To\t\t Lat:%1\t\tLon:%2"))
                                  .arg(_coordRightBottom.EncodeLatitude(DegreeMinutesSecondsF))
                                  .arg(_coordRightBottom.EncodeLongitude(DegreeMinutesSecondsF)));
}

void MapTileExportDialog::runExport()
{
    auto targetDatabaseFileName = _fileSelector->getSelectedPath();
    QList<QCheckBox *> checkBoxes = this->findChildren<QCheckBox *>();
    QMultiMap<int, QString> tileSource2DBFileMap = _mapTileContainer->getTileSource2DBFileMap();

    _mapTilesExporter.clear();

    foreach (auto checkBox, checkBoxes)
    {
        if (!checkBox->isChecked())
            continue;
        int scale = checkBox->property(propertyTileScale).toInt();
        int sourceId = checkBox->property(propertyDataSourceId).toInt();
        QList<QString> sourceDBFiles = tileSource2DBFileMap.values(sourceId);

        _mapTilesExporter.AddExportTask(sourceId, scale, _coordLeftTop, _coordRightBottom, sourceDBFiles);
    }

    _progressDlg->reset();
    _mapTilesExporter.RunExport(targetDatabaseFileName);
}
