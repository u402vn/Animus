#ifndef CAMERAFRAMEGRABBER_H
#define CAMERAFRAMEGRABBER_H

#include <QAbstractVideoSurface>
#include <QList>

class CameraFrameGrabber final : public QAbstractVideoSurface
{
    Q_OBJECT
    bool _verticalMirror;
public:
    explicit CameraFrameGrabber(QObject *parent, bool verticalMirror);

    QList<QVideoFrame::PixelFormat> supportedPixelFormats(QAbstractVideoBuffer::HandleType handleType) const;
    bool present(const QVideoFrame &frame);
signals:
    void frameAvailable(const QImage &frame);
};

#endif // CAMERAFRAMEGRABBER_H
