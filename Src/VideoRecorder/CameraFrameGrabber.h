#ifndef CAMERAFRAMEGRABBER_H
#define CAMERAFRAMEGRABBER_H

#include <QAbstractVideoSurface>
#include <QList>

class CameraFrameGrabber final : public QAbstractVideoSurface
{
    Q_OBJECT
    bool _verticalMirror;
    quint32 _videoConnectionId;
public:
    explicit CameraFrameGrabber(QObject *parent, quint32 videoConnectionId, bool verticalMirror);

    QList<QVideoFrame::PixelFormat> supportedPixelFormats(QAbstractVideoBuffer::HandleType handleType) const;
    bool present(const QVideoFrame &frame);
signals:
    void frameAvailable(const QImage &frame, quint32 videoConnectionId);
};

#endif // CAMERAFRAMEGRABBER_H
