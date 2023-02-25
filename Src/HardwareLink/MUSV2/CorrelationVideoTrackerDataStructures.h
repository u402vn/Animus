#ifndef CORRELATION_VIDEO_TRACKER_DATA_STRUCTURES_H
#define CORRELATION_VIDEO_TRACKER_DATA_STRUCTURES_H
#include <cstdint>

namespace cr
{
namespace vtracker
{

/*
Constants for correlation video tracker. DO NOT MODIFY THEM!!!
*/
#define CVT_MAXIMUM_TRACKING_RECTANGLE_WIDTH 128	/// Maximum horizontal size of tracking rectangle.
#define CVT_MAXIMUM_TRACKING_RECTANGLE_HEIGHT 128	/// Maximum vertical size of tracking rectangle.
#define CVT_MINIMUM_TRACKING_RECTANGLE_WIDTH 16		/// Minimum horizontal size of tracking rectangle.
#define CVT_MINIMUM_TRACKING_RECTANGLE_HEIGHT 16	/// Minimum vertical size of tracking rectangle.
#define CVT_MAXIMUM_CORRELATION_SURFACE_WIDTH 225	/// Maximum correlation surface width.
#define CVT_MAXIMUM_CORRELATION_SURFACE_HEIGHT 225	/// Maximum correlation surface height.
#define CVT_MAXIMUM_SEARCH_WINDOW_WIDTH (CVT_MAXIMUM_TRACKING_RECTANGLE_WIDTH + CVT_MAXIMUM_CORRELATION_SURFACE_WIDTH - 1)		/// Maximum width of search window.
#define CVT_MAXIMUM_SEARCH_WINDOW_HEIGHT (CVT_MAXIMUM_TRACKING_RECTANGLE_HEIGHT + CVT_MAXIMUM_CORRELATION_SURFACE_HEIGHT - 1)	/// Maximum height of search window.
#define CVT_MINIMUM_CORRELATION_SURFACE_WIDTH 27	/// Minimum correlation surface width.
#define CVT_MINIMUM_CORRELATION_SURFACE_HEIGHT 27	/// Minimum correlation surface width.
#define CVT_MINIMUM_FRAME_WIDTH 240					/// Minimum width of video frames.
#define CVT_MINIMUM_FRAME_HEIGHT 240				/// Minimum height of video frames.
#define CVT_MAXIMUM_FRAME_WIDTH 8192				/// Maximum width of video frames.
#define CVT_MAXIMUM_FRAME_HEIGHT 8192				/// Maximum height of video frames.
#define CVT_MAXIMUM_FRAME_BUFFER_SIZE 1024			/// Maximum frame buffer size.
#define CVT_MINIMUM_FRAME_BUFFER_SIZE 2				/// Minimum frame buffer size.
#define CVT_MAXIMUM_NUM_THREADS 8					/// Maximum number of threads. >= 8 threads - no more performance.
#define CVT_MAXIMUM_CORRELATION_SURFACE_STEP 16		/// Maximum step value for correlation surface.


/*
Default values for video tracker parameters.
*/
#define	CVT_DEFAULT_FRAME_WIDTH 0						/// Default video frame width.
#define	CVT_DEFAULT_FRAME_HEIGHT 0						/// Default video frame height.
#define	CVT_DEFAULT_FRAME_BUFFER_SIZE 2					/// Default size (number of video frames to store) of video frame buffer.
#define	CVT_DEFAULT_TRACKING_RECTANGLE_WIDTH 128		/// Default horizontal size of tracking rectangle.
#define	CVT_DEFAULT_TRACKING_RECTANGLE_HEIGHT 128		/// Default vertical size of tracking rectangle.
#define	CVT_DEFAULT_PIXEL_DEVIATION_THRESHOLD 0			/// Default threshold for pixel deviation to capture object.
#define	CVT_DEFAULT_OBJECT_LOSS_THRESHOLD 0.3f			/// Default threshold for detection object loss.
#define	CVT_DEFAULT_OBJECT_DETECTION_THRESHOLD 0.4f		/// Default threshold for detection object.
#define	CVT_DEFAULT_PATTERN_UPDATE_COEFF 0.95f			/// Default coeff to update object pattern image.
#define	CVT_DEFAULT_PROBABILITY_COEFF 0.95f				/// Default coeff to update probability threshold.
#define	CVT_DEFAULT_VELOCITY_COEFF 0.98f				/// Default coeff to update object velocity.
#define CVT_DEFAULT_CORRELATION_SURFACE_WIDTH 225		/// Default correlation surface width.
#define CVT_DEFAULT_CORRELATION_SURFACE_HEIGHT 225		/// Default correlation surface height.
#define	CVT_DEFAULT_LOST_MODE_OPTION 0					/// Default LOST mode option.
#define CVT_DEFAULT_MAXIMUM_NUM_FRAMES_IN_LOST_MODE 256	/// Default maximum number of frames in LOST mode to auto reset of algorithm.
#define CVT_DEFAULT_CORRELATION_SURFACE_STEP 6			/// Default correlation surface step.
#define CVT_DEFAULT_MINIMUM_CORR_DIFFERENCE 0.2			/// Default value of difference between maximums in correlation surface.


/*
Video tracker modes indexes value.
*/
#define CVT_FREE_MODE_INDEX 0		/// FREE mode index.
#define CVT_TRACKING_MODE_INDEX 1	/// TRACKING mode index.
#define CVT_LOST_MODE_INDEX 2		/// LOST mode index.
#define CVT_INERTIAL_MODE_INDEX 3	/// INERTIAL mode index.
#define CVT_STATIC_MODE_INDEX 4		/// STATIC mode index.


	/**
	 * @brief Enum of video tracker command IDs.
	*/
	enum class CorrelationVideoTrackerCommand
	{
		CAPTURE = 1,							// Object capture command.
		RESET = 2,								// Reset command.
		SET_INERTIAL_MODE = 3,					// Command to set INERTIAL mode.
		SET_LOST_MODE = 4,						// Command to set LOST mode.
		SET_STATIC_MODE = 5,					// Command to set STATIC mode.
		SET_TRACKING_RECTANGLE_AUTO_SIZE = 6,	// Command to set tracking rectangle size and position automatically.
		MOVE_TRACKING_RECTANGLE = 7				// Command to move tracking rectangle (change position).
	};


	/**
	 * @brief Enum of video tracker properties.
	*/
	enum class CorrelationVideoTrackerProperty
	{
		FRAME_BUFFER_SIZE = 1,					// Size (number of video frames to store) of video frame buffer.
		TRACKING_RECTANGLE_WIDTH = 2,			// Horizontal size of tracking rectangle.
		TRACKING_RECTANGLE_HEIGHT = 3,			// Vertical size of tracking rectangle.
		PIXEL_DEVIATION_THRESHOLD = 4,			// Threshold for pixel deviation to capture object.
		OBJECT_LOSS_THRESHOLD = 5,				// Threshold for detection object loss.
		OBJECT_DETECTION_THRESHOLD = 6,			// Threshold for detection object.
		SEARCH_WINDOW_X = 7,					// Horizontal position of search window center.
		SEARCH_WINDOW_Y = 8,					// Vertical position of search window center.
		PATTERN_UPDATE_COEFF = 10,				// Coeff for update pattern.
		PROBABILITY_UPDATE_COEFF = 11,			// Coeff to update probability threshold.
		VELOCITY_UPDATE_COEFF = 12,				// Coeff to update velocity.
		CORRELATION_SURFACE_WIDTH = 13,			// Width of correlation surface. Search window width = TRACKING_RECTANGLE_WIDTH + CORRELATION_SURFACE_WIDTH - 1.
		CORRELATION_SURFACE_HEIGHT = 14,		// Height of correlation surface. Search window height = TRACKING_RECTANGLE_HEIGHT + CORRELATION_SURFACE_HEIGHT - 1.
		LOST_MODE_OPTION = 15,					// LOST mode option.
		MAXIMUM_NUM_FRAMES_IN_LOST_MODE = 16,	// Maximum number of frames in LOST mode to auto reset of algorithm.
		NUM_THREADS = 17,						// Number of threads.
		CORRELATION_SURFACE_STEP = 18,			// Step in pixels for correlation surface when you find object.
		USE_PREDICTED_POSITION = 19,			// Flag of using predicted object position in correlation surface.
		MIN_DIFFERENCE_BETWEEN_MAXIMUMS = 20	// Minimum value difference between maximum in correlation surface to use predicted object position.
	};


	/**
	 * @brief Image types enum.
	*/
	enum class CorrelationVideoTrackerImageType
	{
		PATTERN_IMAGE = 1,				// Object pattern image.
		MASK_IMAGE = 2,					// Mask image.
		CORRELATION_SURFACE_IMAGE = 3	// Correlation surface image.
	};


	/**
	 * @brief Struct for video tracker result data.
	*/
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
	} CorrelationVideoTrackerResultData;


	/**
	 * @brief Struct for video tracker result fileds mask.
	*/
	typedef struct
	{
		bool trackingRectangleCenterX;
		bool trackingRectangleCenterY;
		bool trackingRectangleWidth;
		bool trackingRectangleHeight;	
		bool objectCenterX;
		bool objectCenterY;
		bool objectWidth;
		bool objectHeight;
		bool frameCounterInLostMode;
		bool frameCounter;
		bool frameWidth;
		bool frameHeight;
		bool correlationSurfaceWidth;
		bool correlationSurfaceHeight;
		bool pixelDeviationThreshold;
		bool lostModeOption;
		bool frameBufferSize;
		bool maximumNumberOfFramesInLostMode;
		bool trackerFrameID;
		bool bufferFrameID;
		bool searchWindowCenterX;
		bool searchWindowCenterY;
		bool numThreads;
		bool trackingRectangleCenterFX;
		bool trackingRectangleCenterFY;
		bool horizontalObjectValocity;
		bool verticalObjectVelocity;
		bool objectLossThreshold;
		bool objectDetectionThreshold;
		bool probabilityAdaptiveThreshold;
		bool patternUpdateCoeff;
		bool velocityUpdateCoeff;
		bool probabilityUpdateCoeff;
		bool objectDetectionProbability;
		bool mode;
	} CorrelationVideoTrackerResultFieldsMask;
}
}
#endif // CORRELATION_VIDEO_TRACKER_DATA_STRUCTURES_H


