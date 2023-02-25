#ifndef MUSV2VIDEORECEIVER_H
#define MUSV2VIDEORECEIVER_H

#include <QObject>
#include <QByteArray>
#include <QImage>
#include <QLocalSocket>

typedef struct
{
    uint32_t ProtocolVersion;   // Версия протокола

    uint32_t FrameSourceId;     // идентификатор камеры-источника видео (число)
    uint32_t FrameNumber;       // инкреиментальный номер кадра с камеры-источника видео (число)
    uint32_t FrameWidth;        // ширина кадра (пиксели)
    uint32_t FrametHeight;      // высота кадра (пиксели)

    float CamRoll;              // Угол камеры по крену (градусы)
    float CamPitch;             // Угол камеры по тангажа (градусы)
    float CamYaw;               // Угол камеры по рысканью (градусы)
    float CamZoom;              // Зум камеры

    int32_t CamEncoderRoll;     // Значения энкодера камеры для крена
    int32_t CamEncoderPitch;    // Значения энкодера камеры для тангажа
    int32_t CamEncoderYaw;      // Значения энкодера камеры для рысканья

    uint32_t ColorMode;         // цветовой режим камеры

    float RangefinderDistance;  // Значение дистанции от дальномера (метры)

    uint32_t TargetCenterX;     // экранная координата центра сопровождаемой цели по оси X (пиксели)
    uint32_t TargetCenterY;     // экранная координата центра сопровождаемой цели по оси Y (пиксели)

    uint32_t TargetWidth;       // экранная ширина сопровождаемой цели (пиксели)
    uint32_t TargetHeight;      // экранная высота сопровождаемой цели (пиксели)

    uint32_t TargetRectWidth;   // экранная ширина прямоугольника сопровождаемой цели (пиксели)
    uint32_t TargetRectHeight;  // экранная высота прямоугольника сопровождаемой цели (пиксели)
    uint32_t TargetState;       // статус сопровождаемой цели (0 - выкл. 1 - цель найдена, 2 - цель потеряна)

    float StabilizedCenterX;    // вычисленная смещенная координата центра для стабилизированного кадра по оси X (пиксели)
    float StabilizedCenterY;    // вычисленная смещенная координата центра для стабилизированного кадра по оси Y (пиксели)
    float StabilizedRotationAngle; // вычисленный поворот стабилизированного кадра (градусы)

    uint32_t TracerState;       // статус системы подслеживания за целью (0 - выключена, 1 - включена)

} MUSV2CamTelemetry;

/*
typedef struct
{
    int32_t trackingRectangleCenterX;			// Tracking rectangle horizontal center position.
    int32_t trackingRectangleCenterY;			// Tracking rectangle vertical center position.
    int32_t trackingRectangleWidth;				// Tracking rectangle width.
    int32_t trackingRectangleHeight;			// Tracking rectangle height.
    int32_t objectCenterX;						// Estimated horizontal position of oject center.
    int32_t objectCenterY;						// Estimated vertical position of oject center.
    int32_t objectWidth;						// Estimated object width.
    int32_t objectHeight;						// Estimated object height.
    int32_t frameCounterInLostMode;				// Frame counter in LOST mode.
    int32_t frameCounter;						// Counter for processed frames after captue object.
    int32_t frameWidth;							// Width of processed video frame.
    int32_t frameHeight;						// Height of processed video frame.
    int32_t correlationSurfaceWidth;			// Width of correlation surface.
    int32_t correlationSurfaceHeight;			// Height of correlation surface.
    int32_t pixelDeviationThreshold;			// Pixel deviation threshold to capture object.
    int32_t lostModeOption;						// Option for LOST mode.
    int32_t frameBufferSize;					// Size of frame buffer (number of frames to store).
    int32_t maximumNumberOfFramesInLostMode;	// Maximum number of frames in LOST mode to auto reset of algorithm.
    int32_t trackerFrameID;						// ID of last processed frame in frame buffer.
    int32_t bufferFrameID;						// ID of last added frame to frame buffer.
    int32_t searchWindowCenterX;				// Horizontal position of search window center.
    int32_t searchWindowCenterY;				// Vertical position of search window center.
    int32_t numThreads;							// Number of threads to calculate.
    float trackingRectangleCenterFX;			// Subpixel horizontal position of tracking rectangle center.
    float trackingRectangleCenterFY;			// Subpixel vertical position of tracking rectangle center.
    float horizontalObjectValocity;				// Horizontal velocity of object on video frames ( pixel/frame ).
    float verticalObjectVelocity;				// Vertical velocity of object on video frames ( pixel/frame ).
    float objectLossThreshold;					// Threshold to detect object loss.
    float objectDetectionThreshold;				// Threshold to detect object.
    float probabilityAdaptiveThreshold;			// Adaptive threshold to detect object loss and to detect object.
    float patternUpdateCoeff;					// Coeff to update pattern.
    float velocityUpdateCoeff;					// Coeff to update valocity.
    float probabilityUpdateCoeff;				// Coeff to update probability threshold.
    float objectDetectionProbability;			// Estimated probability of object detection.
    uint8_t mode;								// Tracker mode index: 0 - FREE, 1 - TRACKING, 2 - INERTIAL, 3 - STATIC.
} MUSV2Telemetry; // CorrelationVideoTrackerResultData
*/

typedef uint8_t (*DecodeFunction)(
        const void *const inputDataBuffer,
        const uint32_t inputDataSize,
        void *const outDataBuffer,
        void *const outpTelemetryBuffer);


class MUSV2VideoReceiver final : public QObject
{
    Q_OBJECT
    QLocalSocket *_socket;
public:
    explicit MUSV2VideoReceiver(QObject *parent, bool verticalMirror, int udpReceivePort);
    ~MUSV2VideoReceiver();
private slots:
    void onReadyRead();
    void onError(QLocalSocket::LocalSocketError socketError);
signals:
    void frameAvailable(const QImage &frame);
};

#endif // MUSV2VIDEORECEIVER_H
