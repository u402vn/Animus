#ifndef VIDEOLINK_H
#define VIDEOLINK_H

#include <QObject>
#include <QCamera>
#include <QUrl>
#include <QMap>
#include "CamPreferences.h"

class VideoLink : public QObject
{
    Q_OBJECT
private:
    quint32 _opticalSystemId;
    quint32 _activeVideoConnectionId;
    CamAssemblyPreferences *_camAssemblyPreferences;
protected:
    QMap<int, QObject*> _videoSources;
    QObject *openVideoConnection(int connectionId);
    QObject *openVideoSourceWithURL(quint32 videoConnectionId, const QUrl &url, bool useVerticalFrameMirrororing);
public:
    explicit VideoLink(QObject *parent);
    ~VideoLink();


    void openVideoSource();
    void closeVideoSource();

    virtual void setActiveOpticalSystemId(quint32 camId);

    quint32 activeOpticalSystemId();
    quint32 activeVideoConnectionId();
    CamAssemblyPreferences *camAssemblyPreferences();
private slots:
    virtual void videoFrameReceivedInternal(const QImage &frame, quint32 videoConnectionId) = 0;
    void usbCameraError(QCamera::Error value);
};

class SimpleVideoLink : public VideoLink
{
    Q_OBJECT
public:
    explicit SimpleVideoLink(QObject *parent);
    ~SimpleVideoLink();
private slots:
    void videoFrameReceivedInternal(const QImage &frame, quint32 videoConnectionId);
signals:
    void videoFrameReceived(const QImage &frame);
};

#endif // VIDEOLINK_H
