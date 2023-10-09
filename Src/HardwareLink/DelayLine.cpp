#include "DelayLine.h"
#include <QTimer>

TelemetryDelayLine::TelemetryDelayLine(QObject *parent, quint32 delayMs) : QObject(parent)
{
    _delayMs = delayMs;
}

TelemetryDelayLine::~TelemetryDelayLine()
{

}

void TelemetryDelayLine::enqueue(const TelemetryDataFrame &value)
{
    if (_delayMs <= 0)
    {
        emit dequeue(value);
    }
    else
    {
        _frames.enqueue(value);
        QTimer::singleShot(_delayMs, Qt::PreciseTimer, this, &TelemetryDelayLine::onDelayTimer);
    }
}

void TelemetryDelayLine::clear()
{
    _frames.clear();
}

bool TelemetryDelayLine::isEmpty()
{
    return _frames.isEmpty();
}

const TelemetryDataFrame TelemetryDelayLine::head()
{
    return _frames.head();
}

void TelemetryDelayLine::onDelayTimer()
{
    if (!_frames.isEmpty())
    {
        auto value = _frames.dequeue();
        emit dequeue(value);
    }
}

//----------------------------------------------

CameraTelemetryDelayLine::CameraTelemetryDelayLine(QObject *parent, quint32 delayMs): QObject(parent)
{
    _delayMs = delayMs;
}

CameraTelemetryDelayLine::~CameraTelemetryDelayLine()
{

}

void CameraTelemetryDelayLine::enqueue(const CameraTelemetryDataFrame &value)
{
    if (_delayMs <= 0)
    {
        emit dequeue(value);
    }
    else
    {
        _frames.enqueue(value);
        QTimer::singleShot(_delayMs, Qt::PreciseTimer, this, &CameraTelemetryDelayLine::onDelayTimer);
    }
}

void CameraTelemetryDelayLine::clear()
{
    _frames.clear();
}

void CameraTelemetryDelayLine::onDelayTimer()
{
    if (!_frames.isEmpty())
    {
        auto value = _frames.dequeue();
        emit dequeue(value);
    }
}
