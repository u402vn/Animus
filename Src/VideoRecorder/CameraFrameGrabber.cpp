#include "CameraFrameGrabber.h"
#include <QImage>

CameraFrameGrabber::CameraFrameGrabber(QObject *parent, quint32 videoConnectionId, bool verticalMirror) :  QAbstractVideoSurface(parent)
{
    _verticalMirror = verticalMirror;
    _videoConnectionId = videoConnectionId;
}

QList<QVideoFrame::PixelFormat> CameraFrameGrabber::supportedPixelFormats(QAbstractVideoBuffer::HandleType handleType) const
{
    Q_UNUSED(handleType);
    qDebug() << "CameraFrameGrabber received supportedPixelFormats request (handleType = " << handleType << """)";

    return QList<QVideoFrame::PixelFormat>()
            << QVideoFrame::Format_ARGB32
            << QVideoFrame::Format_ARGB32_Premultiplied
            << QVideoFrame::Format_RGB32
            << QVideoFrame::Format_RGB24
            << QVideoFrame::Format_RGB565
            << QVideoFrame::Format_RGB555
            << QVideoFrame::Format_ARGB8565_Premultiplied
            << QVideoFrame::Format_BGRA32;
    /*
            << QVideoFrame::Format_BGRA32_Premultiplied
            << QVideoFrame::Format_BGR32
            << QVideoFrame::Format_BGR24
            << QVideoFrame::Format_BGR565
            << QVideoFrame::Format_BGR555
            << QVideoFrame::Format_BGRA5658_Premultiplied
            << QVideoFrame::Format_AYUV444
            << QVideoFrame::Format_AYUV444_Premultiplied
            << QVideoFrame::Format_YUV444
            << QVideoFrame::Format_YUV420P
            << QVideoFrame::Format_YV12
            << QVideoFrame::Format_UYVY
            << QVideoFrame::Format_YUYV
            << QVideoFrame::Format_NV12
            << QVideoFrame::Format_NV21
            << QVideoFrame::Format_IMC1
            << QVideoFrame::Format_IMC2
            << QVideoFrame::Format_IMC3
            << QVideoFrame::Format_IMC4
            << QVideoFrame::Format_Y8
            << QVideoFrame::Format_Y16
            << QVideoFrame::Format_Jpeg
            << QVideoFrame::Format_CameraRaw
            << QVideoFrame::Format_AdobeDng;
            */
}


// qt_imageFromVideoFrame
// https://coderoad.ru/27829830/%D0%BF%D1%80%D0%B5%D0%BE%D0%B1%D1%80%D0%B0%D0%B7%D0%BE%D0%B2%D0%B0%D0%BD%D0%B8%D0%B5-QVideoFrame-%D0%B2-QImage

bool CameraFrameGrabber::present(const QVideoFrame &frame)
{
    if (!frame.isValid())
        return false;

    QVideoFrame cloneFrame(frame);
    if (!cloneFrame.map(QAbstractVideoBuffer::ReadOnly))
    {
        return false;
    }

    QVideoFrame::PixelFormat pixelFormat = cloneFrame.pixelFormat();
    QImage::Format imageFormat = QVideoFrame::imageFormatFromPixelFormat(pixelFormat);

    if (imageFormat == QImage::Format_Invalid)
        qDebug() << "Invalid Image Format";

    const QImage image(cloneFrame.bits(), cloneFrame.width(), cloneFrame.height(), cloneFrame.bytesPerLine(), imageFormat);

    QImage outImage = _verticalMirror ? image.mirrored(false, true) : image.copy();

    cloneFrame.unmap();
    emit frameAvailable(outImage, _videoConnectionId);

    return true;
}
