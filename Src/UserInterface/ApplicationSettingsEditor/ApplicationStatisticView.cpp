#include "ApplicationStatisticView.h"
#include <QPushButton>
#include <QVBoxLayout>
#include <QHeaderView>
#include "ApplicationSettings.h"
#include "EnterProc.h"

class QTableWidgetItemWithIntValue : public QTableWidgetItem
{
public:
    QTableWidgetItemWithIntValue(const QString &text, int type = Type) : QTableWidgetItem(text, type)
    {
    }

    bool operator < (const QTableWidgetItem &other) const
    {
        return (this->text().toInt() < other.text().toInt());
    }
};

QTableWidgetItem * createTableWidgetItemWithIntValue(int value)
{
    QTableWidgetItem * item = new QTableWidgetItemWithIntValue(QString::number(value));
    item->setTextAlignment(Qt::AlignRight);
    return item;
}

void ApplicationStatisticView::fillStatisticsList()
{
    EnterProcStart("ApplicationStatisticView::fillStatisticsList");

    _statisticsList->setSortingEnabled(false);
    _statisticsList->clearContents();

    QList<EnterProcMeasure *> measures = EnterProc::getMeasures();

    _statisticsList->setRowCount(measures.count());

    int row = 0;
    int col;
    foreach (EnterProcMeasure * measure, measures)
    {
        col = 0;
        _statisticsList->setItem(row, col++, new QTableWidgetItem(measure->ProcName));
        _statisticsList->setItem(row, col++, createTableWidgetItemWithIntValue(measure->CallCount));
        _statisticsList->setItem(row, col++, createTableWidgetItemWithIntValue(measure->TotalTime));
        _statisticsList->setItem(row, col++, createTableWidgetItemWithIntValue(measure->MinTime));
        _statisticsList->setItem(row, col++, createTableWidgetItemWithIntValue(measure->getAvgTime()));
        _statisticsList->setItem(row, col++, createTableWidgetItemWithIntValue(measure->MaxTime));

        row++;
    }
    _statisticsList->setSortingEnabled(true);
    _statisticsList->resizeColumnsToContents();
}

ApplicationStatisticView::ApplicationStatisticView(QWidget *parent) : QWidget(parent)
{
    const int spacing = 5;
    auto statisticsLayout = new QVBoxLayout();
    this->setLayout(statisticsLayout);
    statisticsLayout->setMargin(spacing);
    statisticsLayout->setSpacing(spacing);

    auto buttonsLayout = new QHBoxLayout();

    auto chkEnableComputingStatistics = new QCheckBox(tr("Enable Computing Statistics"), this);
    connect(chkEnableComputingStatistics, &QCheckBox::toggled, this, &ApplicationStatisticView::onEnableComputingToggled);

    auto btnClearStatistics = new QPushButton(tr("Clear"), this);
    connect(btnClearStatistics, &QPushButton::clicked, this, &ApplicationStatisticView::onClearStatisticsCicked);

    auto btnRefreshStatistics = new QPushButton(tr("Refresh"), this);
    connect(btnRefreshStatistics, &QPushButton::clicked, this, &ApplicationStatisticView::onRefreshStatisticsCicked);

    QStringList horizontalHeaderLabels = QStringList();
    horizontalHeaderLabels << tr("Proc Name") << tr("Call Count") << tr("Total Time") << tr("Min Time") << tr("Avg Time") << tr("Max Time");

    _statisticsList = new QTableWidget(this);
    _statisticsList->horizontalHeader()->sortIndicatorOrder();
    _statisticsList->setColumnCount(horizontalHeaderLabels.count());
    _statisticsList->setHorizontalHeaderLabels(horizontalHeaderLabels);
    _statisticsList->setEditTriggers(QAbstractItemView::NoEditTriggers);

    buttonsLayout->addWidget(chkEnableComputingStatistics, 1);
    buttonsLayout->addWidget(btnClearStatistics, 0);
    buttonsLayout->addWidget(btnRefreshStatistics, 0);
    statisticsLayout->addLayout(buttonsLayout, 0);
    statisticsLayout->addWidget(_statisticsList, 1);
    fillStatisticsList();

    ApplicationSettings& applicationSettings = ApplicationSettings::Instance();
    chkEnableComputingStatistics->setChecked(applicationSettings.EnableComputingStatistics);
}

void ApplicationStatisticView::onRefreshStatisticsCicked()
{
    EnterProcStart("ApplicationStatisticView::onRefreshStatisticsCicked");
    fillStatisticsList();
}

void ApplicationStatisticView::onClearStatisticsCicked()
{
    _statisticsList->clearContents();
    _statisticsList->setRowCount(0);
    EnterProc::clearStatistics();
}

void ApplicationStatisticView::onEnableComputingToggled(bool checked)
{
    ApplicationSettings& applicationSettings = ApplicationSettings::Instance();
    applicationSettings.EnableComputingStatistics = checked;
    EnterProc::setEnableComputingStatistics(checked);
}

