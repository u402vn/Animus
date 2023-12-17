#ifndef VIDEOLINK_H
#define VIDEOLINK_H

#include <QObject>
#include <QCamera>
#include <QUrl>
#include <QMap>

class VideoLink : public QObject
{
    Q_OBJECT
private:

protected:
    QMap<int, QObject*> _videoSources;
    quint32 _opticalSystemId;

    QObject *openVideoConnection(int connectionId);

    QObject *openVideoSourceWithURL(const QUrl &url, bool useVerticalFrameMirrororing);
public:
    explicit VideoLink(QObject *parent);
    ~VideoLink();


    void openVideoSource();
    void closeVideoSource();

    virtual void selectActiveCam(int camId);
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
