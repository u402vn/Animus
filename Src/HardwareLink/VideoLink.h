#ifndef VIDEOLINK_H
#define VIDEOLINK_H

#include <QObject>
#include <QCamera>
#include <QUrl>

class VideoLink : public QObject
{
    Q_OBJECT
private:
    QObject * _videoSource;
public:
    explicit VideoLink(QObject *parent);
    ~VideoLink();

    void openVideoSourceWithURL(const QUrl &url, bool useVerticalFrameMirrororing);
    void openVideoSource();
    void closeVideoSource();
private slots:
    virtual void videoFrameReceivedInternal(const QImage &frame) = 0;
    void usbCameraError(QCamera::Error value);
};

class SimpleVideoLink : public VideoLink
{
    Q_OBJECT
public:
    explicit SimpleVideoLink(QObject *parent);
    ~SimpleVideoLink();
private slots:
    void videoFrameReceivedInternal(const QImage &frame);
signals:
    void videoFrameReceived(const QImage &frame);
};

#endif // VIDEOLINK_H
