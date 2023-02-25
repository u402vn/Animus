#include "EnterProc.h"
#include <QList>
#include <QDebug>

class StatisticsKeeper final
{
    QHash<QString, EnterProcMeasure *> _measures;
    QTime _globalTime;
public:
    StatisticsKeeper();
    ~StatisticsKeeper();

    EnterProcMeasure *getMeasureByName(const QString &procName);

    int getGlobalTime();
    void clear();
    QList<EnterProcMeasure *> getMeasures();
    void outStatisticsToDebug(StatisticsSortMode sortMode);
};

static StatisticsKeeper gStatisticsKeeper;
static bool gEnableComputingStatistics = false;



EnterProc::EnterProc(const QString procName)
{
    if (!gEnableComputingStatistics)
    {
        _localProcMeasure = nullptr;
        return;
    }
    _localProcMeasure = gStatisticsKeeper.getMeasureByName(procName);
    _localStartTime = gStatisticsKeeper.getGlobalTime();
}

EnterProc::~EnterProc()
{
    if (_localProcMeasure == nullptr)
        return;
    int currentTime = gStatisticsKeeper.getGlobalTime();
    _localProcMeasure->appendCallTime(currentTime - _localStartTime);
}

void EnterProc::begin()
{
}

void EnterProc::outStatisticsToDebug(StatisticsSortMode sortMode)
{
    gStatisticsKeeper.outStatisticsToDebug(sortMode);
}

void EnterProc::setEnableComputingStatistics(bool enable)
{
    gEnableComputingStatistics = enable;
}

void EnterProc::clearStatistics()
{
    gStatisticsKeeper.clear();
}

QList<EnterProcMeasure *> EnterProc::getMeasures()
{
    QList<EnterProcMeasure *> measures = gStatisticsKeeper.getMeasures();
    return measures;
}




EnterProcMeasure::EnterProcMeasure()
{
    TotalTime = 0;
    CallCount = 0;
    MinTime = 0;
    MaxTime = 0;
}

EnterProcMeasure::~EnterProcMeasure()
{
}

int EnterProcMeasure::getAvgTime()
{
    if (CallCount > 0)
        return TotalTime / CallCount;
    else
        return 0;
}

void EnterProcMeasure::appendCallTime(int timeMs)
{
    CallCount++;
    TotalTime += timeMs;
    if (MinTime > timeMs || CallCount == 1)
        MinTime = timeMs;
    if (MaxTime < timeMs)
        MaxTime = timeMs;
}

StatisticsKeeper::StatisticsKeeper()
{
    _globalTime.start();
}

StatisticsKeeper::~StatisticsKeeper()
{
    clear();
}

void StatisticsKeeper::clear()
{
    auto i = _measures.begin();
    while (i != _measures.end())
    {
        auto measure = i.value();
        i = _measures.erase(i);
        delete measure;
    }
}

QList<EnterProcMeasure *> StatisticsKeeper::getMeasures()
{
    QList<EnterProcMeasure *> measureList = _measures.values();
    return measureList;
}

bool procNameLetssThan(const EnterProcMeasure * epm1, const EnterProcMeasure * epm2)
{
    return (*epm1).ProcName < (*epm2).ProcName;
}

bool maxTimeLetssThan(const EnterProcMeasure * epm1, const EnterProcMeasure * epm2)
{
    return (*epm1).MaxTime < (*epm2).MaxTime;
}

bool avgTimeLetssThan(const EnterProcMeasure * epm1, const EnterProcMeasure * epm2)
{
    int time1 = (EnterProcMeasure(*epm1)).getAvgTime();
    int time2 = (EnterProcMeasure(*epm2)).getAvgTime();
    return (time1 < time2);
}

void StatisticsKeeper::outStatisticsToDebug(StatisticsSortMode sortMode)
{
    QList<EnterProcMeasure *> measuresList = _measures.values();
    switch (sortMode)
    {
    case StatisticsSortMode::SortByProcName:
        qSort(measuresList.begin(), measuresList.end(), procNameLetssThan);
        break;
    case StatisticsSortMode::SortByMaxTime:
        qSort(measuresList.begin(), measuresList.end(), maxTimeLetssThan);
        break;
    case StatisticsSortMode::SortByAvgTime:
        qSort(measuresList.begin(), measuresList.end(), avgTimeLetssThan);
        break;
    }

    for (QList<EnterProcMeasure *>::iterator i = measuresList.begin(); i != measuresList.end(); i++)
    {
        EnterProcMeasure * measure = *i;
        QString logRecord = QString("%1 Calls: %2   Total(ms): %3   AVG(ms): %4   Min(ms): %5    Max(ms): %6")
                .arg(measure->ProcName.leftJustified(60, ' '))
                .arg(measure->CallCount, 5)
                .arg(measure->TotalTime, 5)
                .arg(measure->getAvgTime(), 5)
                .arg(measure->MinTime, 5)
                .arg(measure->MaxTime, 5);

        qDebug() << logRecord;
    }
}

EnterProcMeasure *StatisticsKeeper::getMeasureByName(const QString &procName)
{
    EnterProcMeasure * measure = _measures.value(procName, nullptr);
    if (measure == nullptr)
    {
        QHash<QString, EnterProcMeasure *>::iterator i =
                _measures.insert(procName, new EnterProcMeasure());
        measure = i.value();
        measure->ProcName = procName;
    }
    return measure;
}

int StatisticsKeeper::getGlobalTime()
{
    int elapsedTime = _globalTime.elapsed();
    return elapsedTime;
}
