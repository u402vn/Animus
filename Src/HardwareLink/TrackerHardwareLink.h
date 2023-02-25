#ifndef TRACKERHARDWARELINK_H
#define TRACKERHARDWARELINK_H

#include <QObject>
#include <QUdpSocket>
#include <QHostAddress>
#include "Common/BinaryContent.h"

class TrackerHardwareLink : public QObject
{
    Q_OBJECT
    void sendTrackerCommand(const BinaryContent &commandContent, const QString &commandDescription);
    QUdpSocket _udpTrackerCommandSocket;
    QHostAddress _trackerCommandUDPAddress;
    quint32 _trackerCommandUDPPort;

    QUdpSocket _udpTrackerTelemetrySocket;
    quint32 _trackerTelemetryUDPPort;

    int32_t _trackingRectangleCenterX;			// Tracking rectangle horizontal center position.
    int32_t _trackingRectangleCenterY;			// Tracking rectangle vertical center position.
    int32_t _trackingRectangleWidth;			// Tracking rectangle width.
    int32_t _trackingRectangleHeight;			// Tracking rectangle height.
    int32_t _objectCenterX;						// Estimated horizontal position of oject center.
    int32_t _objectCenterY;						// Estimated vertical position of oject center.
    int32_t _objectWidth;						// Estimated object width.
    int32_t _objectHeight;						// Estimated object height.
    int32_t _frameCounterInLostMode;			// Frame counter in LOST mode.
    int32_t _frameCounter;						// Counter for processed frames after captue object.
    int32_t _frameWidth;						// Width of processed video frame.
    int32_t _frameHeight;						// Height of processed video frame.
    int32_t _correlationSurfaceWidth;			// Width of correlation surface.
    int32_t _correlationSurfaceHeight;			// Height of correlation surface.
    int32_t _pixelDeviationThreshold;			// Pixel deviation threshold to capture object.
    int32_t _lostModeOption;					// Option for LOST mode.
    int32_t _frameBufferSize;					// Size of frame buffer (number of frames to store).
    int32_t _maximumNumberOfFramesInLostMode;	// Maximum number of frames in LOST mode to auto reset of algorithm.
    int32_t _trackerFrameID;					// ID of last processed frame in frame buffer.
    int32_t _bufferFrameID;						// ID of last added frame to frame buffer.
    int32_t _searchWindowCenterX;				// Horizontal position of search window center.
    int32_t _searchWindowCenterY;				// Vertical position of search window center.
    int32_t _numThreads;						// Number of threads to calculate.
    float _trackingRectangleCenterFX;			// Subpixel horizontal position of tracking rectangle center.
    float _trackingRectangleCenterFY;			// Subpixel vertical position of tracking rectangle center.
    float _horizontalObjectValocity;			// Horizontal velocity of object on video frames ( pixel/frame ).
    float _verticalObjectVelocity;				// Vertical velocity of object on video frames ( pixel/frame ).
    float _objectLossThreshold;					// Threshold to detect object loss.
    float _objectDetectionThreshold;			// Threshold to detect object.
    float _probabilityAdaptiveThreshold;		// Adaptive threshold to detect object loss and to detect object.
    float _patternUpdateCoeff;					// Coeff to update pattern.
    float _velocityUpdateCoeff;					// Coeff to update valocity.
    float _probabilityUpdateCoeff;				// Coeff to update probability threshold.
    float _objectDetectionProbability;			// Estimated probability of object detection.
    uint8_t _mode;								// Tracker mode index: 0 - FREE, 1 - TRACKING, 2 - INERTIAL, 3 - STATIC.

    int32_t decodeTrackerData(const uint8_t* data, const uint32_t dataSize);
public:
    explicit TrackerHardwareLink(QObject *parent);

    void open();
    void close();

    void lockTarget(const QPoint &targetCenter);
    void unlockTarget();
    void setTargetSize(int targetSize);

    int32_t trackingRectangleCenterX() const;
    int32_t trackingRectangleCenterY() const;
    int32_t trackingRectangleWidth() const;
    int32_t trackingRectangleHeight() const;
    uint8_t trackingState() const;
private slots:
    void processTelemetryPendingDatagrams();
signals:
    void onTrackerCommandSent(const BinaryContent &commandContent, const QString &commandDescription);
};

#endif // TRACKERHARDWARELINK_H
