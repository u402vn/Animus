#ifndef ENTERPROC_H
#define ENTERPROC_H
#include <QHash>
#include <QTime>

#define EnterProcStart(x) EnterProc ep(x); ep.begin();

enum class StatisticsSortMode {SortByProcName, SortByMaxTime, SortByAvgTime};

class EnterProcMeasure final
{
public:
    EnterProcMeasure();
    ~EnterProcMeasure();
    QString ProcName;
    int TotalTime;
    int CallCount;
    int MinTime;
    int MaxTime;
    int getAvgTime();
    void appendCallTime(int timeMs);
};

class EnterProc
{
    int _localStartTime;
    EnterProcMeasure * _localProcMeasure;
public:   
    EnterProc(const QString procName);
    ~EnterProc();
    void begin();
    static void outStatisticsToDebug(StatisticsSortMode sortMode);
    static void setEnableComputingStatistics(bool enable);
    static void clearStatistics();
    static QList<EnterProcMeasure *> getMeasures();
};

#endif // ENTERPROC_H
