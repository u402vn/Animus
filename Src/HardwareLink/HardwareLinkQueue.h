#ifndef HARDWARELINKQUEUE_H
#define HARDWARELINKQUEUE_H

#include <QQueue>
#include <QTime>


template<typename T>
class HardwareLinkQueue
{
    struct QueueItem
    {
        int ProcessingTime;
        T Data;
    };

    QQueue<QueueItem> _queue;
    QTime _queueTime;
public:
    HardwareLinkQueue()
    {
        _queueTime.start();
    }

    ~HardwareLinkQueue()
    {

    }

    void append(T data)
    {
        QueueItem item;
        item.ProcessingTime = _queueTime.elapsed();
        item.Data = data;
        _queue.enqueue(item);
    }
};

#endif // HARDWARELINKQUEUE_H
