#ifndef DELAYLINE_H
#define DELAYLINE_H

#include <QObject>
#include <QQueue>
#include "TelemetryDataFrame.h"


//??? todo clear from hardwarelink !!!!

class TelemetryDelayLine : public QObject
{
    Q_OBJECT
    quint32 _delayMs;
    QQueue<TelemetryDataFrame> _frames;

public:
    explicit TelemetryDelayLine(QObject *parent, quint32 delayMs);
    ~TelemetryDelayLine();

    void enqueue(const TelemetryDataFrame &value);
    void clear();
signals:
    void dequeue(const TelemetryDataFrame &value);
private slots:
    void onDelayTimer();
};



class CameraTelemetryDelayLine : public QObject
{
    Q_OBJECT
    quint32 _delayMs;
    QQueue<CameraTelemetryDataFrame> _frames;

public:
    explicit CameraTelemetryDelayLine(QObject *parent, quint32 delayMs);
    ~CameraTelemetryDelayLine();

    void enqueue(const CameraTelemetryDataFrame &value);
    void clear();
signals:
    void dequeue(const CameraTelemetryDataFrame &value);
private slots:
    void onDelayTimer();
};










#endif // DELAYLINE_H
