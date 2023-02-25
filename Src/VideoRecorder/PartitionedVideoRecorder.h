#ifndef PARTITIONEDVIDEORECORDER_H
#define PARTITIONEDVIDEORECORDER_H

#include <QObject>
#include <opencv2/highgui.hpp>

class PartitionedVideoRecorder final : public QObject
{
    Q_OBJECT
    quint32 _frameNumber;
    quint32 _frameWidth, _frameHeight;
    QString _fileDirecory, _recordName;
    quint32 _videoFileFrameCount;
    quint32 _videoFileQuality;

    cv::VideoWriter * _videoWriter;

    void swapVideoFiles();
public:
    explicit PartitionedVideoRecorder(QObject *parent);
    ~PartitionedVideoRecorder();
    void start(const QString &fileDirecory, const QString &recordName, quint32 videoFileFrameCount, quint32 videoFileQuality);
    void stop();
    void saveFrame(const QImage &frame);
    quint32 frameWidth();
    quint32 frameHeight();
};

QString getVideoFileNameForFrame(const QString &fileDirecory, const QString &recordName, const quint32 videoFileFrameCount, const quint32 frameNumber);

#endif // PARTITIONEDVIDEORECORDER_H
