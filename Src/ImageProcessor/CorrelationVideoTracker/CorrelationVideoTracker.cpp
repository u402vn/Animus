#include <omp.h>
#include <cmath>
#include <chrono>
#include <iostream>
#include <cstring>
#include "CorrelationVideoTracker.h"


#define MAXIMUM_SEARCH_WINDOW_WIDTH (CVT_MAXIMUM_TRACKING_RECTANGLE_WIDTH + CVT_MAXIMUM_CORRELATION_SURFACE_WIDTH - 1)		/// Search window maximum width.
#define MAXIMUM_SEARCH_WINDOW_HEIGHT (CVT_MAXIMUM_TRACKING_RECTANGLE_WIDTH + CVT_MAXIMUM_CORRELATION_SURFACE_HEIGHT - 1)	/// Search window maximum height.


#define CORR_STEP 4					/// Pixels step to find object.
#define MIN_MASK_VALUE 32			/// Minimum value of ssearch full mask.
#define MAX_MASK_VALUE 255			/// Maximum value of mask.
#define PAT_DIVIDER 4				/// Divider to pattern value.
#define ZNAM_DIVIDER 16				/// Divider to zanm value.
#define LOCAL_MAX_DELTA 15			/// Minimum distance between loval maximum in correlation surface.
#define LOCAL_MAX_BORDER 4			/// Boarder for additional serach around local maximum.
#define MASK_THRESHOLD 5			/// Value in reduced mask to indetify objec borders.
#define MIN_CORR_DIFFERENCES 0.1f	/// Minimum differences between maximim values in correlation surface to use trajectory information.
#define PAT_MUL_IN_MASK 4			/// Multiple coeff.
#define SKO_MULTIPLICATOR 1.0f		/// Multiplicator for deviation surface.
#define MASK_ADD_VALUE 4			/// Adding value to mask.
#define MIN_CORR_VALUE 0.4f			/// Minimum correlation value due finding frame_ID.



vtracker::CorrelationVideoTracker::CorrelationVideoTracker() :
	patternImage(nullptr),
	differencePattern(nullptr),
	reducedMask(nullptr),
	fullMask(nullptr),
	trackingRectangleImage(nullptr),
	differenceWindow(nullptr),
	doubledDifferenceWindow(nullptr),
	deviationSurface(nullptr),
	correlationSurface(nullptr),
	frameBuffer(nullptr),
	Xcorrect(0.0f),
	Ycorrect(0.0f),
	XCovariance(0.0f),
	YCovariance(0.0f),
	RX(0.0f),
	RY(0.0f),
	trackerData({ 0 }),
	trackingRectangleIndex(0),
	last_chisl_value(0),
	chislSurface(nullptr)
{
	// Init tracker result data.
	trackerData.frameBufferSize = CVT_DEFAULT_FRAME_BUFFER_SIZE;
	trackerData.frameWidth = CVT_DEFAULT_FRAME_WIDTH;
	trackerData.frameHeight = CVT_DEFAULT_FRAME_HEIGHT;
	trackerData.lostModeOption = CVT_DEFAULT_LOST_MODE_OPTION;
	trackerData.maximumNumberOfFramesInLostMode = CVT_DEFAULT_MAXIMUM_NUM_FRAMES_IN_LOST_MODE;
	trackerData.correlationSurfaceWidth = CVT_DEFAULT_CORRELATION_SURFACE_WIDTH;
	trackerData.correlationSurfaceHeight = CVT_DEFAULT_CORRELATION_SURFACE_HEIGHT;
	trackerData.mode = CVT_FREE_MODE_INDEX;
	trackerData.objectDetectionThreshold = CVT_DEFAULT_OBJECT_DETECTION_THRESHOLD;
	trackerData.objectLossThreshold = CVT_DEFAULT_OBJECT_LOSS_THRESHOLD;
	trackerData.patternUpdateCoeff = CVT_DEFAULT_PATTERN_UPDATE_COEFF;
	trackerData.pixelDeviationThreshold = CVT_DEFAULT_PIXEL_DEVIATION_THRESHOLD;
	trackerData.probabilityUpdateCoeff = CVT_DEFAULT_PROBABILITY_COEFF;
	trackerData.velocityUpdateCoeff = CVT_DEFAULT_VELOCITY_COEFF;
	trackerData.trackingRectangleWidth = CVT_DEFAULT_TRACKING_RECTANGLE_WIDTH;
	trackerData.trackingRectangleHeight = CVT_DEFAULT_TRACKING_RECTANGLE_HEIGHT;
	trackerData.probabilityAdaptiveThreshold = 0.0f;
	trackerData.numThreads = 0;

	// Allocate memory.
	frameBuffer = nullptr;

	patternImage = new uint8_t[CVT_MAXIMUM_TRACKING_RECTANGLE_WIDTH * CVT_MAXIMUM_TRACKING_RECTANGLE_HEIGHT];
	memset(patternImage, 0, CVT_MAXIMUM_TRACKING_RECTANGLE_WIDTH * CVT_MAXIMUM_TRACKING_RECTANGLE_HEIGHT);

	differencePattern = new int32_t[CVT_MAXIMUM_TRACKING_RECTANGLE_WIDTH * CVT_MAXIMUM_TRACKING_RECTANGLE_HEIGHT];
	memset(differencePattern, 0, CVT_MAXIMUM_TRACKING_RECTANGLE_WIDTH * CVT_MAXIMUM_TRACKING_RECTANGLE_HEIGHT * sizeof(int32_t));

	reducedMask = new uint8_t[CVT_MAXIMUM_TRACKING_RECTANGLE_WIDTH * CVT_MAXIMUM_TRACKING_RECTANGLE_HEIGHT];
	memset(reducedMask, 0, CVT_MAXIMUM_TRACKING_RECTANGLE_WIDTH * CVT_MAXIMUM_TRACKING_RECTANGLE_HEIGHT);

	fullMask = new uint8_t[CVT_MAXIMUM_TRACKING_RECTANGLE_WIDTH * CVT_MAXIMUM_TRACKING_RECTANGLE_HEIGHT];
	memset(fullMask, 0, CVT_MAXIMUM_TRACKING_RECTANGLE_WIDTH * CVT_MAXIMUM_TRACKING_RECTANGLE_HEIGHT);

	trackingRectangleImage = new uint8_t *[2];
	trackingRectangleImage[0] = new uint8_t[CVT_MAXIMUM_TRACKING_RECTANGLE_WIDTH * CVT_MAXIMUM_TRACKING_RECTANGLE_HEIGHT];
	memset(trackingRectangleImage[0], 0, CVT_MAXIMUM_TRACKING_RECTANGLE_WIDTH * CVT_MAXIMUM_TRACKING_RECTANGLE_HEIGHT);
	trackingRectangleImage[1] = new uint8_t[CVT_MAXIMUM_TRACKING_RECTANGLE_WIDTH * CVT_MAXIMUM_TRACKING_RECTANGLE_HEIGHT];
	memset(trackingRectangleImage[1], 0, CVT_MAXIMUM_TRACKING_RECTANGLE_WIDTH * CVT_MAXIMUM_TRACKING_RECTANGLE_HEIGHT);

	differenceWindow = new int32_t[MAXIMUM_SEARCH_WINDOW_WIDTH * MAXIMUM_SEARCH_WINDOW_HEIGHT];
	memset(differenceWindow, 0, MAXIMUM_SEARCH_WINDOW_WIDTH * MAXIMUM_SEARCH_WINDOW_HEIGHT * sizeof(int32_t));

	doubledDifferenceWindow = new uint32_t[MAXIMUM_SEARCH_WINDOW_WIDTH * MAXIMUM_SEARCH_WINDOW_HEIGHT];
	memset(doubledDifferenceWindow, 0, MAXIMUM_SEARCH_WINDOW_WIDTH * MAXIMUM_SEARCH_WINDOW_HEIGHT * sizeof(uint32_t));

	deviationSurface = new float[CVT_MAXIMUM_TRACKING_RECTANGLE_WIDTH * CVT_MAXIMUM_TRACKING_RECTANGLE_HEIGHT];
	memset(deviationSurface, 0, CVT_MAXIMUM_TRACKING_RECTANGLE_WIDTH * CVT_MAXIMUM_TRACKING_RECTANGLE_HEIGHT * sizeof(float));

	correlationSurface = new float[CVT_MAXIMUM_CORRELATION_SURFACE_WIDTH * CVT_MAXIMUM_CORRELATION_SURFACE_HEIGHT];
	memset(correlationSurface, 0, CVT_MAXIMUM_CORRELATION_SURFACE_WIDTH * CVT_MAXIMUM_CORRELATION_SURFACE_HEIGHT * sizeof(float));

	chislSurface = new int32_t[CVT_MAXIMUM_CORRELATION_SURFACE_WIDTH * CVT_MAXIMUM_CORRELATION_SURFACE_HEIGHT];
	memset(chislSurface, 0, CVT_MAXIMUM_CORRELATION_SURFACE_WIDTH * CVT_MAXIMUM_CORRELATION_SURFACE_HEIGHT * sizeof(int32_t));
}


vtracker::CorrelationVideoTracker::~CorrelationVideoTracker()
{
	// Release memory.
	delete[] patternImage;
	delete[] differencePattern;
	delete[] reducedMask;
	delete[] fullMask;
	delete[] trackingRectangleImage[0];
	delete[] trackingRectangleImage[1];
	delete[] trackingRectangleImage;
	delete[] differenceWindow;
	delete[] doubledDifferenceWindow;
	delete[] deviationSurface;
	delete[] correlationSurface;
	delete[] chislSurface;
}


void vtracker::CorrelationVideoTracker::Reset()
{
	// Update tracker data.
	trackerData.frameCounter = 0;
	trackerData.frameCounterInLostMode = 0;
	trackerData.horizontalObjectValocity = 0.0f;
	trackerData.verticalObjectVelocity = 0.0f;
	trackerData.mode = CVT_FREE_MODE_INDEX;
	trackerData.objectCenterX = 0;
	trackerData.objectCenterY = 0;
	trackerData.objectDetectionProbability = 0.0f;
	trackerData.objectHeight = 0;
	trackerData.objectWidth = 0;
	trackerData.searchWindowCenterX = 0;
	trackerData.searchWindowCenterY = 0;
	trackerData.trackerFrameID = 0;
	trackerData.trackingRectangleCenterFX = 0.0f;
	trackerData.trackingRectangleCenterFY = 0.0f;
	trackerData.trackingRectangleCenterX = 0;
	trackerData.trackingRectangleCenterY = 0;

	last_chisl_value = 0;

	// Reset buffers.
	memset(patternImage, 0, CVT_MAXIMUM_TRACKING_RECTANGLE_WIDTH * CVT_MAXIMUM_TRACKING_RECTANGLE_HEIGHT);
	memset(differencePattern, 0, CVT_MAXIMUM_TRACKING_RECTANGLE_WIDTH * CVT_MAXIMUM_TRACKING_RECTANGLE_HEIGHT * sizeof(int32_t));
	memset(reducedMask, 0, CVT_MAXIMUM_TRACKING_RECTANGLE_WIDTH * CVT_MAXIMUM_TRACKING_RECTANGLE_HEIGHT);
	memset(fullMask, 0, CVT_MAXIMUM_TRACKING_RECTANGLE_WIDTH * CVT_MAXIMUM_TRACKING_RECTANGLE_HEIGHT);
	memset(trackingRectangleImage[0], 0, CVT_MAXIMUM_TRACKING_RECTANGLE_WIDTH * CVT_MAXIMUM_TRACKING_RECTANGLE_HEIGHT);
	memset(trackingRectangleImage[1], 0, CVT_MAXIMUM_TRACKING_RECTANGLE_WIDTH * CVT_MAXIMUM_TRACKING_RECTANGLE_HEIGHT);
	memset(differenceWindow, 0, MAXIMUM_SEARCH_WINDOW_WIDTH * MAXIMUM_SEARCH_WINDOW_HEIGHT * sizeof(int32_t));
	memset(doubledDifferenceWindow, 0, MAXIMUM_SEARCH_WINDOW_WIDTH * MAXIMUM_SEARCH_WINDOW_HEIGHT * sizeof(uint32_t));
	memset(deviationSurface, 0, CVT_MAXIMUM_TRACKING_RECTANGLE_WIDTH * CVT_MAXIMUM_TRACKING_RECTANGLE_HEIGHT * sizeof(float));
	memset(correlationSurface, 0, CVT_MAXIMUM_CORRELATION_SURFACE_WIDTH * CVT_MAXIMUM_CORRELATION_SURFACE_HEIGHT * sizeof(float));
	memset(chislSurface, 0, CVT_MAXIMUM_CORRELATION_SURFACE_WIDTH * CVT_MAXIMUM_CORRELATION_SURFACE_HEIGHT * sizeof(int32_t));
}


bool vtracker::CorrelationVideoTracker::CheckResetCriteria()
{
	// Check tracker mode. No reset in FREE mode.
	if (trackerData.mode == CVT_FREE_MODE_INDEX)
		return false;

	// Check tracking rectangle position.
	if (trackerData.trackingRectangleCenterX - trackerData.trackingRectangleWidth / 2 <= 0)
		return true;
	if (trackerData.trackingRectangleCenterX + trackerData.trackingRectangleWidth / 2 >= trackerData.frameWidth - 1)
		return true;
	if (trackerData.trackingRectangleCenterY - trackerData.trackingRectangleHeight / 2 <= 0)
		return true;
	if (trackerData.trackingRectangleCenterY + trackerData.trackingRectangleHeight / 2 >= trackerData.frameHeight - 1)
		return true;

	// Check frame counters.
	if (trackerData.frameCounterInLostMode > trackerData.maximumNumberOfFramesInLostMode)
		return true;

	return false;
}


bool vtracker::CorrelationVideoTracker::SetProperty(CorrelationVideoTrackerProperty propertyID, double propertyValue)
{
	// Global lock.
	std::lock_guard<std::mutex> globalLock(accessManageMutex);

	// Check property ID.
	switch (propertyID)
	{
	case CorrelationVideoTrackerProperty::FRAME_BUFFER_SIZE:
	{
		// Check property value. Sould be in range [CVT_MINIMUM_FRAME_BUFFER_SIZE:CVT_MAXIMUM_FRAME_BUFFER_SIZE];
		if ((uint32_t)propertyValue < CVT_MINIMUM_FRAME_BUFFER_SIZE || (uint32_t)propertyValue > CVT_MAXIMUM_FRAME_BUFFER_SIZE)
			return false;

		// Reset tracker.
		Reset();

		// Delete frame buffer.
		if (frameBuffer != nullptr)
			for (int32_t i = 0; i < trackerData.frameBufferSize; ++i)
				delete[] frameBuffer[i];
		delete[] frameBuffer;
		frameBuffer = nullptr;

		// Set new frame buffer size.
		trackerData.frameBufferSize = (uint32_t)propertyValue;
		trackerData.bufferFrameID = 0;

		// Allocate memory.
		if (trackerData.frameWidth > 0 && trackerData.frameHeight > 0)
		{
			frameBuffer = new uint8_t * [trackerData.frameBufferSize];
			for (int32_t i = 0; i < trackerData.frameBufferSize; ++i)
				frameBuffer[i] = new uint8_t[(size_t)trackerData.frameWidth * (size_t)trackerData.frameHeight];
		}		

		return true;	
	}


	case CorrelationVideoTrackerProperty::LOST_MODE_OPTION:
	{
		// Check property value.
		if ((int32_t)propertyValue < 0 || (int32_t)propertyValue > 2)
			return false;

		// Set property value.
		trackerData.lostModeOption = (int32_t)propertyValue;

		return true;
	}


	case CorrelationVideoTrackerProperty::MAXIMUM_NUM_FRAMES_IN_LOST_MODE:
	{
		// Check property value. Should be in range [1:inf].
		if ((int32_t)propertyValue < 1)
			return false;

		// Set property value.
		trackerData.maximumNumberOfFramesInLostMode = (int32_t)propertyValue;

		return true;
	}
		
	
	case CorrelationVideoTrackerProperty::CORRELATION_SURFACE_WIDTH:
	{
		// Check property value.
		if ((int32_t)propertyValue < CVT_MINIMUM_CORRELATION_SURFACE_WIDTH ||
			(int32_t)propertyValue > CVT_MAXIMUM_CORRELATION_SURFACE_WIDTH)
			return false;

		// Set property value.
		trackerData.correlationSurfaceWidth = (int32_t)propertyValue;

		return true;
	}

	case CorrelationVideoTrackerProperty::CORRELATION_SURFACE_HEIGHT:
	{
		// Check property value.
		if ((int32_t)propertyValue < CVT_MINIMUM_CORRELATION_SURFACE_HEIGHT ||
			(int32_t)propertyValue > CVT_MAXIMUM_CORRELATION_SURFACE_HEIGHT)
			return false;

		// Set property value.
		trackerData.correlationSurfaceHeight = (int32_t)propertyValue;

		return true;
	}

	case CorrelationVideoTrackerProperty::OBJECT_DETECTION_THRESHOLD:
	{
		// Check property value. Should be in range [0:1].
		if ((float)propertyValue < 0.0f || (float)propertyValue > 1.0f)
			return false;

		// Set property value.
		trackerData.objectDetectionThreshold = (float)propertyValue;

		return true;
	}


	case CorrelationVideoTrackerProperty::OBJECT_LOSS_THRESHOLD:
	{
		// Check property value. Should be in range [0:1].
		if ((float)propertyValue < 0.0f || (float)propertyValue > 1.0f)
			return false;

		// Set property value.
		trackerData.objectLossThreshold = (float)propertyValue;

		return true;
	}


	case CorrelationVideoTrackerProperty::PATTERN_UPDATE_COEFF:
	{
		// Check property value. Should be in range [0:1].
		if ((float)propertyValue < 0.0f || (float)propertyValue > 1.0f)
			return false;

		// Set property value.
		trackerData.patternUpdateCoeff = (float)propertyValue;

		return true;
	}


	case CorrelationVideoTrackerProperty::PIXEL_DEVIATION_THRESHOLD:
	{
		// Check property value. Should be in range [0:255].
		if ((int32_t)propertyValue < 0 || (int32_t)propertyValue > 255)
			return false;

		// Set property value.
		trackerData.pixelDeviationThreshold = (int32_t)propertyValue;

		return true;
	}


	case CorrelationVideoTrackerProperty::PROBABILITY_UPDATE_COEFF:
	{
		// Check property value. Should be in range [0:1].
		if ((float)propertyValue < 0.0f || (float)propertyValue > 1.0f)
			return false;

		// Set property value.
		trackerData.probabilityUpdateCoeff = (float)propertyValue;

		return true;
	}
	

	case CorrelationVideoTrackerProperty::SEARCH_WINDOW_X:
	{
		// Check ptoperty value. Should lie inside frame area.
		if ((int32_t)propertyValue < 0 || (int32_t)propertyValue >= trackerData.frameWidth)
			return false;

		// Set property value.
		trackerData.searchWindowCenterX = (int32_t)propertyValue;

		return true;
	}


	case CorrelationVideoTrackerProperty::SEARCH_WINDOW_Y:
	{
		// Check ptoperty value. Should lie inside frame area.
		if ((int32_t)propertyValue < 0 || (int32_t)propertyValue >= trackerData.frameHeight)
			return false;

		// Set property value.
		trackerData.searchWindowCenterY = (int32_t)propertyValue;

		return true;
	}
	

	case CorrelationVideoTrackerProperty::TRACKING_RECTANGLE_HEIGHT:
	{
		// Check property value. Should be in range [CVT_MINIMUM_TRACKING_RECTANGLE_HEIGHT:CVT_MAXIMUM_TRACKING_RECTANGLE_HEIGHT].
		if ((int32_t)propertyValue < CVT_MINIMUM_TRACKING_RECTANGLE_HEIGHT || (int32_t)propertyValue > CVT_MAXIMUM_TRACKING_RECTANGLE_HEIGHT)
			return false;

		// Set property value.
		trackerData.trackingRectangleHeight = (int32_t)propertyValue;

		return true;
	}


	case CorrelationVideoTrackerProperty::TRACKING_RECTANGLE_WIDTH:
	{
		// Check property value. Should be in range [CVT_MINIMUM_TRACKING_RECTANGLE_WIDTH:CVT_MAXIMUM_TRACKING_RECTANGLE_WIDTH].
		if ((int32_t)propertyValue < CVT_MINIMUM_TRACKING_RECTANGLE_WIDTH || (int32_t)propertyValue > CVT_MAXIMUM_TRACKING_RECTANGLE_WIDTH)
			return false;

		// Set property value.
		trackerData.trackingRectangleWidth = (int32_t)propertyValue;

		return true;
	}


	case CorrelationVideoTrackerProperty::VELOCITY_UPDATE_COEFF:
	{
		// Check property value. Should be in range [0:1].
		if ((float)propertyValue < 0.0f || (float)propertyValue > 1.0f)
			return false;

		// Set property value.
		trackerData.velocityUpdateCoeff = (float)propertyValue;

		return true;
	}

	case CorrelationVideoTrackerProperty::NUM_THREADS:
	{
		// Check property value. Should be in range [0:CVT_MAXIMUM_NUM_THREADS].
		if ((int32_t)propertyValue < 0 || (int32_t)propertyValue > CVT_MAXIMUM_NUM_THREADS)
			return false;

		// Set prpperty value.
		trackerData.numThreads = (int32_t)propertyValue;

		return true;
	}

	default:
		return false;
	}

	return false;
}


double vtracker::CorrelationVideoTracker::GetProperty(CorrelationVideoTrackerProperty propertyID)
{
	// Global lock.
	std::lock_guard<std::mutex> globalLock(accessManageMutex);

	// Check property ID.
	switch (propertyID)
	{
	case vtracker::CorrelationVideoTrackerProperty::FRAME_BUFFER_SIZE:
		return (double)trackerData.frameBufferSize;

	case vtracker::CorrelationVideoTrackerProperty::TRACKING_RECTANGLE_WIDTH:
		return (double)trackerData.trackingRectangleWidth;

	case vtracker::CorrelationVideoTrackerProperty::TRACKING_RECTANGLE_HEIGHT:
		return (double)trackerData.trackingRectangleHeight;

	case vtracker::CorrelationVideoTrackerProperty::PIXEL_DEVIATION_THRESHOLD:
		return (double)trackerData.pixelDeviationThreshold;

	case vtracker::CorrelationVideoTrackerProperty::OBJECT_LOSS_THRESHOLD:
		return (double)trackerData.objectLossThreshold;

	case vtracker::CorrelationVideoTrackerProperty::OBJECT_DETECTION_THRESHOLD:
		return (double)trackerData.objectDetectionThreshold;

	case vtracker::CorrelationVideoTrackerProperty::SEARCH_WINDOW_X:
		return (double)trackerData.searchWindowCenterX;

	case vtracker::CorrelationVideoTrackerProperty::SEARCH_WINDOW_Y:
		return (double)trackerData.searchWindowCenterY;

	case vtracker::CorrelationVideoTrackerProperty::PATTERN_UPDATE_COEFF:
		return (double)trackerData.patternUpdateCoeff;

	case vtracker::CorrelationVideoTrackerProperty::PROBABILITY_UPDATE_COEFF:
		return (double)trackerData.probabilityUpdateCoeff;

	case vtracker::CorrelationVideoTrackerProperty::VELOCITY_UPDATE_COEFF:
		return (double)trackerData.velocityUpdateCoeff;

	case vtracker::CorrelationVideoTrackerProperty::CORRELATION_SURFACE_WIDTH:
		return (double)trackerData.correlationSurfaceWidth;

	case vtracker::CorrelationVideoTrackerProperty::CORRELATION_SURFACE_HEIGHT:
		return (double)trackerData.correlationSurfaceHeight;

	case vtracker::CorrelationVideoTrackerProperty::LOST_MODE_OPTION:
		return (double)trackerData.lostModeOption;

	case vtracker::CorrelationVideoTrackerProperty::MAXIMUM_NUM_FRAMES_IN_LOST_MODE:
		return (double)trackerData.maximumNumberOfFramesInLostMode;

	case CorrelationVideoTrackerProperty::NUM_THREADS:
		return (double)trackerData.numThreads;

	default:
		return -1.0;
	}

	return -1.0;
}


bool vtracker::CorrelationVideoTracker::ExecuteCommand(
	CorrelationVideoTrackerCommand commandID,
	const int32_t arg1,
	const int32_t arg2,
	const int32_t arg3,
	const uint8_t* arg4,
	const uint8_t* arg5)
{
	// Global lock.
	std::lock_guard<std::mutex> globalLock(accessManageMutex);

	// Check frames initialization.
	if (trackerData.frameWidth == 0 || trackerData.frameHeight == 0)
		return false;

	// Check command ID.
	switch (commandID)
	{
	case vtracker::CorrelationVideoTrackerCommand::CAPTURE:
		return Capture(arg1, arg2, arg3, arg4, arg5);

	case vtracker::CorrelationVideoTrackerCommand::RESET:
		Reset();
		return true;

	case vtracker::CorrelationVideoTrackerCommand::SET_INERTIAL_MODE:
		return SetInertialMode();

	case vtracker::CorrelationVideoTrackerCommand::SET_LOST_MODE:
		return SetLostMode();

	case vtracker::CorrelationVideoTrackerCommand::SET_STATIC_MODE:
		return SetStaticMode();

	case vtracker::CorrelationVideoTrackerCommand::SET_TRACKING_RECTANGLE_AUTO_SIZE:
		return SetTrackingRectangleSizeAutomatically();

	case vtracker::CorrelationVideoTrackerCommand::MOVE_TRACKING_RECTANGLE:
		return MoveTrackingRectangle(arg1, arg2);

	default:
		return false;
	}

	return false;
}


vtracker::CorrelationVideoTrackerResultData vtracker::CorrelationVideoTracker::GetTrackerResultData()
{
	// Global lock.
	std::lock_guard<std::mutex> globalLock(accessManageMutex);

	return trackerData;
}


bool vtracker::CorrelationVideoTracker::GetImage(CorrelationVideoTrackerImageType imageType, uint8_t* image)
{
	// Global lock.
	std::lock_guard<std::mutex> globalLock(accessManageMutex);

	// Check image ID.
	switch (imageType)
	{
	case vtracker::CorrelationVideoTrackerImageType::PATTERN_IMAGE:
	{
		// Copy data.
		memcpy(image, patternImage, CVT_MAXIMUM_TRACKING_RECTANGLE_WIDTH * CVT_MAXIMUM_TRACKING_RECTANGLE_HEIGHT);

		return true;
	}


	case vtracker::CorrelationVideoTrackerImageType::MASK_IMAGE:
	{
		// Copy data.
		memcpy(image, fullMask, CVT_MAXIMUM_TRACKING_RECTANGLE_WIDTH * CVT_MAXIMUM_TRACKING_RECTANGLE_HEIGHT);

		return true;
	}


	case vtracker::CorrelationVideoTrackerImageType::CORRELATION_SURFACE_IMAGE:
	{
		int32_t corrW = trackerData.correlationSurfaceWidth;
		int32_t corrH = trackerData.correlationSurfaceHeight;

		// Find maximum value in correlation surface.
		float maxValue = 0.0f;
		for (int32_t i = 0; i < corrH; ++i)
			for (int32_t j = 0; j < corrW; ++j)
				if (correlationSurface[i * corrW + j] > maxValue)
					maxValue = correlationSurface[i * corrW + j];

		memset(image, 0, CVT_MAXIMUM_TRACKING_RECTANGLE_WIDTH * CVT_MAXIMUM_TRACKING_RECTANGLE_HEIGHT);

		// Check max value.
		if (maxValue == 0)
			return true;

		// Copy data.
		for (int32_t i = 0; i < corrH; ++i)
			for (int32_t j = 0; j < corrW; ++j)
				if (correlationSurface[i * corrW + j] > 0.0f)
					image[i * CVT_MAXIMUM_TRACKING_RECTANGLE_WIDTH + j] = (uint8_t)(correlationSurface[i * corrW + j] * 255.0f / maxValue);
				else
					image[i * CVT_MAXIMUM_TRACKING_RECTANGLE_WIDTH + j] = 0;

		return true;
	}

	default:
		return false;
	}

	return false;
}


bool vtracker::CorrelationVideoTracker::Capture(
	const int32_t objectCenterX,
	const int32_t objectCenterY,
	const int32_t frameID,
	const uint8_t* frameDescriptor,
	const uint8_t* objectMask)
{
	// Reset tracker.
	Reset();

	// Fill tracker result data.
	trackerData.trackingRectangleCenterX = objectCenterX;
	trackerData.trackingRectangleCenterY = objectCenterY;
	trackerData.searchWindowCenterX = objectCenterX;
	trackerData.searchWindowCenterY = objectCenterY;
	trackerData.trackingRectangleCenterFX = (float)objectCenterX;
	trackerData.trackingRectangleCenterFY = (float)objectCenterY;
	trackerData.objectCenterX = objectCenterX;
	trackerData.objectCenterY = objectCenterY;
	trackerData.objectWidth = trackerData.trackingRectangleWidth;
	trackerData.objectHeight = trackerData.trackingRectangleHeight;

	// Init probability adaptive threshold.
	trackerData.probabilityAdaptiveThreshold = 0.8f;

	last_chisl_value = 0;

	// Check reset criteria.
	if (CheckResetCriteria())
	{
		Reset();
		return false;
	}

	// Check frame ID to capture object.
	if (frameID >= 0)
	{
		// Check frame ID.
		if (frameID >= trackerData.frameBufferSize)
		{
			Reset();
			return false;
		}
		
		// Set necessary frame ID.
		trackerData.trackerFrameID = frameID;
	}
	else
	{
		if (frameDescriptor != nullptr)
		{
			int32_t frame_ID = FindFrameID(objectCenterX, objectCenterY, frameDescriptor);
			if (frame_ID >= 0)
				trackerData.trackerFrameID = frame_ID;
			else
				trackerData.trackerFrameID = trackerData.bufferFrameID;
		}
		else
		{
			trackerData.trackerFrameID = trackerData.bufferFrameID;
		}
	}
	
	// Check object precense.
	if (trackerData.pixelDeviationThreshold > 0.0f)
	{
		if (!CheckObjectPresence(frameBuffer[trackerData.trackerFrameID], objectCenterX, objectCenterX))
		{
			Reset();
			return false;
		}
	}	

	// Init trajectory prediction filter.
	float corrW = (float)(trackerData.correlationSurfaceWidth);
	float corrH = (float)(trackerData.correlationSurfaceHeight);
	Xcorrect = corrW / 2.0f;
	Ycorrect = corrH / 2.0f;
	XCovariance = (corrW - (float)(trackerData.trackingRectangleWidth / 2)) * (corrW - (float)(trackerData.trackingRectangleWidth / 2));
	YCovariance = (corrH - (float)(trackerData.trackingRectangleHeight / 2)) * (corrH - (float)(trackerData.trackingRectangleHeight / 2));
	RX = (corrW * corrW) / 4.0f;
	RY = (corrH * corrH) / 4.0f;

	// Fill pattern image and tracking rectangles image.
	trackingRectangleIndex = 0;
	int32_t x0 = trackerData.trackingRectangleCenterX - CVT_MAXIMUM_TRACKING_RECTANGLE_WIDTH / 2;
	int32_t y0 = trackerData.trackingRectangleCenterY - CVT_MAXIMUM_TRACKING_RECTANGLE_HEIGHT / 2;
	for (int32_t i = 0; i < CVT_MAXIMUM_TRACKING_RECTANGLE_HEIGHT; ++i)
	{
		for (int32_t j = 0; j < CVT_MAXIMUM_TRACKING_RECTANGLE_WIDTH; ++j)
		{
			if (i + y0 >= 0 && i + y0 < trackerData.frameHeight && j + x0 >= 0 && j + x0 < trackerData.frameWidth)
			{
				trackingRectangleImage[0][i * CVT_MAXIMUM_TRACKING_RECTANGLE_WIDTH + j] = frameBuffer[trackerData.trackerFrameID][(i + y0) * trackerData.frameWidth + (j + x0)];
				trackingRectangleImage[1][i * CVT_MAXIMUM_TRACKING_RECTANGLE_WIDTH + j] = trackingRectangleImage[0][i * CVT_MAXIMUM_TRACKING_RECTANGLE_WIDTH + j];
				patternImage[i * CVT_MAXIMUM_TRACKING_RECTANGLE_WIDTH + j] = trackingRectangleImage[0][i * CVT_MAXIMUM_TRACKING_RECTANGLE_WIDTH + j];
			}
		}
	}

	// Fill mask surface if we dont have input mask data.
	if (objectMask == nullptr)
	{
		// Fill by Gauss surface.
		int32_t strobe_x0 = (CVT_MAXIMUM_TRACKING_RECTANGLE_WIDTH - trackerData.trackingRectangleWidth) / 2;
		int32_t strobe_y0 = (CVT_MAXIMUM_TRACKING_RECTANGLE_HEIGHT - trackerData.trackingRectangleHeight) / 2;
		float skoX = (float)(trackerData.trackingRectangleWidth) / 4.0f;
		float skoY = (float)(trackerData.trackingRectangleHeight) / 4.0f;
		float kX = 1.0f / (skoX * 2.506628f);
		float kY = 1.0f / (skoY * 2.506628f);
		float kzX = 2.0f * skoX * skoX;
		float kzY = 2.0f * skoY * skoY;
		float maxGauss = kX * kY;
		float tmpVal;
		for (int32_t i = 0; i < trackerData.trackingRectangleHeight; i++)
		{
			for (int32_t j = 0; j < trackerData.trackingRectangleWidth; j++)
			{
				tmpVal = kX * expf(-powf((float)(j - trackerData.trackingRectangleWidth / 2), 2) / kzX) * kY * expf(-powf((float)(i - trackerData.trackingRectangleHeight / 2), 2) / kzY);
				tmpVal = tmpVal * 255.0f / maxGauss;
				fullMask[(i + strobe_y0) * CVT_MAXIMUM_TRACKING_RECTANGLE_WIDTH + (j + strobe_x0)] = (uint8_t)tmpVal;
				reducedMask[(i + strobe_y0) * CVT_MAXIMUM_TRACKING_RECTANGLE_WIDTH + (j + strobe_x0)] = (uint8_t)tmpVal / MIN_MASK_VALUE;
			}
		}
	}
	else
	{
		// Copy input mask data.
		int32_t strobe_x0 = (CVT_MAXIMUM_TRACKING_RECTANGLE_WIDTH - trackerData.trackingRectangleWidth) / 2;
		int32_t strobe_y0 = (CVT_MAXIMUM_TRACKING_RECTANGLE_HEIGHT - trackerData.trackingRectangleHeight) / 2;
		for (int32_t i = 0; i < trackerData.trackingRectangleHeight; i++)
		{
			for (int32_t j = 0; j < trackerData.trackingRectangleWidth; j++)
			{
				fullMask[(i + strobe_y0) * CVT_MAXIMUM_TRACKING_RECTANGLE_WIDTH + (j + strobe_x0)] = objectMask[i * trackerData.trackingRectangleWidth + j];
				reducedMask[(i + strobe_y0) * CVT_MAXIMUM_TRACKING_RECTANGLE_WIDTH + (j + strobe_x0)] = objectMask[i * trackerData.trackingRectangleWidth + j] / MIN_MASK_VALUE;
			}
		}
	}

	// Set TRACKING mode.
	trackerData.mode = CVT_TRACKING_MODE_INDEX;

	return true;
}


bool vtracker::CorrelationVideoTracker::CheckObjectPresence(uint8_t* frameData, const int32_t objectCenterX, const int32_t objectCenterY)
{
	// Check pixel deviation threshold.
	if (trackerData.pixelDeviationThreshold == 0)
		return true;

	// Calculate medium pixels value.
	int32_t medium = 0;
	int32_t startX = objectCenterX - trackerData.trackingRectangleWidth / 2;
	int32_t startY = objectCenterY - trackerData.trackingRectangleHeight / 2;
	for (int32_t i = 0; i < trackerData.trackingRectangleHeight; ++i)
		for (int32_t j = 0; j < trackerData.trackingRectangleWidth; ++j)
			medium += (int32_t)frameData[(i + startY) * trackerData.frameWidth + (j + startX)];
	medium = medium / (trackerData.trackingRectangleWidth * trackerData.trackingRectangleHeight);

	// Calculate standard deviation.
	uint32_t sko = 0;
	for (int32_t i = 0; i < trackerData.trackingRectangleHeight; ++i)
		for (int32_t j = 0; j < trackerData.trackingRectangleWidth; ++j)
			sko += (uint32_t)(((int32_t)frameData[(i + startY) * trackerData.frameWidth + (j + startX)] - medium) *
				((int32_t)frameData[(i + startY) * trackerData.frameWidth + (j + startX)] - medium));
	sko = (uint32_t)sqrtf((float)(sko / (uint32_t)(trackerData.trackingRectangleWidth * trackerData.trackingRectangleHeight)));

	// Compare standard deviation with threshold.
	if (sko > (uint32_t)trackerData.pixelDeviationThreshold)
		return true;

	return false;
}


bool vtracker::CorrelationVideoTracker::SetInertialMode()
{
	// Check tracker mode.
	if (trackerData.mode == CVT_FREE_MODE_INDEX)
		return false;

	// Set INERTIAL mode.
	trackerData.mode = CVT_INERTIAL_MODE_INDEX;

	return true;
}


bool vtracker::CorrelationVideoTracker::SetLostMode()
{
	// Check tracker mode.
	if (trackerData.mode == CVT_FREE_MODE_INDEX)
		return false;

	// Set LOST mode.
	trackerData.mode = CVT_LOST_MODE_INDEX;

	return true;
}


bool vtracker::CorrelationVideoTracker::SetStaticMode()
{
	// Check tracker mode.
	if (trackerData.mode == CVT_FREE_MODE_INDEX)
		return false;

	// Set STATIC mode.
	trackerData.mode = CVT_STATIC_MODE_INDEX;

	return true;
}


void vtracker::CorrelationVideoTracker::UpdateCooordinatesFilter(int32_t newMaxX, int32_t newMaxY)
{
	float Kx = XCovariance / (XCovariance + RX);
	Xcorrect = Xcorrect + Kx * ((float)newMaxX - Xcorrect);
	XCovariance = (1.0f - Kx) * XCovariance + Kx * (float)powf((float)newMaxX - Xcorrect, 2);
	if (XCovariance < (float)(trackerData.trackingRectangleWidth * trackerData.trackingRectangleWidth / 8))
		XCovariance = (float)(trackerData.trackingRectangleWidth * trackerData.trackingRectangleWidth / 8);

	float Ky = YCovariance / (YCovariance + RY);
	Ycorrect = Ycorrect + Ky * ((float)newMaxY - Ycorrect);
	YCovariance = (1.0f - Ky) * YCovariance + Ky * (float)powf((float)newMaxY - Ycorrect, 2);
	if (YCovariance < (float)(trackerData.trackingRectangleWidth * trackerData.trackingRectangleWidth / 8))
		YCovariance = (float)(trackerData.trackingRectangleWidth * trackerData.trackingRectangleWidth / 8);
}


void vtracker::CorrelationVideoTracker::CalculateObjectRectangle()
{
	int32_t strobe_x0 = (CVT_MAXIMUM_TRACKING_RECTANGLE_WIDTH - trackerData.trackingRectangleWidth) / 2;
	int32_t strobe_y0 = (CVT_MAXIMUM_TRACKING_RECTANGLE_HEIGHT - trackerData.trackingRectangleHeight) / 2;
	int32_t strobe_x1 = strobe_x0 + trackerData.trackingRectangleWidth;
	int32_t strobe_y1 = strobe_y0 + trackerData.trackingRectangleHeight;

	int32_t i, j;
	int32_t tx1 = strobe_x1;
	int32_t tx2 = strobe_x0;
	int32_t ty1 = strobe_y1;
	int32_t ty2 = strobe_y0;
	for (i = strobe_y0; i < strobe_y1; ++i)
	{
		for (j = strobe_x0; j < strobe_x1; ++j)
		{
			if (reducedMask[i * CVT_MAXIMUM_TRACKING_RECTANGLE_WIDTH + j] >= MASK_THRESHOLD)
			{
				if (j < tx1)
					tx1 = j;
				if (j > tx2)
					tx2 = j;
				if (i < ty1)
					ty1 = i;
				if (i > ty2)
					ty2 = i;
			}
		}
	}

	trackerData.objectWidth = tx2 - tx1 + 1;
	trackerData.objectHeight = ty2 - ty1 + 1;
	trackerData.objectCenterX = trackerData.trackingRectangleCenterX - (CVT_MAXIMUM_TRACKING_RECTANGLE_WIDTH / 2) + ((tx1 + tx2) / 2);
	trackerData.objectCenterY = trackerData.trackingRectangleCenterY - (CVT_MAXIMUM_TRACKING_RECTANGLE_HEIGHT / 2) + ((ty1 + ty2) / 2);
}


bool vtracker::CorrelationVideoTracker::MoveTrackingRectangle(int32_t offsetX, int32_t offsetY)
{
	// Check tracker mode and input offset value.
	if (trackerData.mode != CVT_TRACKING_MODE_INDEX ||
		abs(offsetX) > trackerData.trackingRectangleWidth / 2 ||
		abs(offsetY) > trackerData.trackingRectangleHeight / 2)
		return false;

	// Horizontal offset.
	if (offsetX > 0)
	{
		for (int32_t i = 0; i < CVT_MAXIMUM_TRACKING_RECTANGLE_HEIGHT; ++i)
		{
			for (int32_t j = CVT_MAXIMUM_TRACKING_RECTANGLE_WIDTH - 1; j >= offsetX; j--)
			{
				patternImage[i * CVT_MAXIMUM_TRACKING_RECTANGLE_WIDTH + j] = patternImage[i * CVT_MAXIMUM_TRACKING_RECTANGLE_WIDTH + j - offsetX];
				trackingRectangleImage[0][i * CVT_MAXIMUM_TRACKING_RECTANGLE_WIDTH + j] = trackingRectangleImage[0][i * CVT_MAXIMUM_TRACKING_RECTANGLE_WIDTH + j - offsetX];
				trackingRectangleImage[1][i * CVT_MAXIMUM_TRACKING_RECTANGLE_WIDTH + j] = trackingRectangleImage[1][i * CVT_MAXIMUM_TRACKING_RECTANGLE_WIDTH + j - offsetX];
				reducedMask[i * CVT_MAXIMUM_TRACKING_RECTANGLE_WIDTH + j] = reducedMask[i * CVT_MAXIMUM_TRACKING_RECTANGLE_WIDTH + j - offsetX];
				fullMask[i * CVT_MAXIMUM_TRACKING_RECTANGLE_WIDTH + j] = fullMask[i * CVT_MAXIMUM_TRACKING_RECTANGLE_WIDTH + j - offsetX];
			}
		}
	}
	else {
		for (int32_t i = 0; i < CVT_MAXIMUM_TRACKING_RECTANGLE_HEIGHT; ++i)
		{
			for (int32_t j = abs(offsetX); j < CVT_MAXIMUM_TRACKING_RECTANGLE_WIDTH; ++j)
			{
				patternImage[i * CVT_MAXIMUM_TRACKING_RECTANGLE_WIDTH + j - abs(offsetX)] = patternImage[i * CVT_MAXIMUM_TRACKING_RECTANGLE_WIDTH + j];
				trackingRectangleImage[0][i * CVT_MAXIMUM_TRACKING_RECTANGLE_WIDTH + j - abs(offsetX)] = trackingRectangleImage[0][i * CVT_MAXIMUM_TRACKING_RECTANGLE_WIDTH + j];
				trackingRectangleImage[1][i * CVT_MAXIMUM_TRACKING_RECTANGLE_WIDTH + j - abs(offsetX)] = trackingRectangleImage[1][i * CVT_MAXIMUM_TRACKING_RECTANGLE_WIDTH + j];
				reducedMask[i * CVT_MAXIMUM_TRACKING_RECTANGLE_WIDTH + j - abs(offsetX)] = reducedMask[i * CVT_MAXIMUM_TRACKING_RECTANGLE_WIDTH + j];
				fullMask[i * CVT_MAXIMUM_TRACKING_RECTANGLE_WIDTH + j - abs(offsetX)] = fullMask[i * CVT_MAXIMUM_TRACKING_RECTANGLE_WIDTH + j];
			}
		}
	}

	// Vertical offset.
	if (offsetY > 0)
	{
		for (int32_t j = 0; j < CVT_MAXIMUM_TRACKING_RECTANGLE_WIDTH; ++j)
		{
			for (int32_t i = CVT_MAXIMUM_TRACKING_RECTANGLE_HEIGHT - 1; i >= offsetY; i--)
			{
				patternImage[i * CVT_MAXIMUM_TRACKING_RECTANGLE_WIDTH + j] = patternImage[(i - offsetY) * CVT_MAXIMUM_TRACKING_RECTANGLE_WIDTH + j];
				trackingRectangleImage[0][i * CVT_MAXIMUM_TRACKING_RECTANGLE_WIDTH + j] = trackingRectangleImage[0][(i - offsetY) * CVT_MAXIMUM_TRACKING_RECTANGLE_WIDTH + j];
				trackingRectangleImage[1][i * CVT_MAXIMUM_TRACKING_RECTANGLE_WIDTH + j] = trackingRectangleImage[1][(i - offsetY) * CVT_MAXIMUM_TRACKING_RECTANGLE_WIDTH + j];
				reducedMask[i * CVT_MAXIMUM_TRACKING_RECTANGLE_WIDTH + j] = reducedMask[(i - offsetY) * CVT_MAXIMUM_TRACKING_RECTANGLE_WIDTH + j];
				fullMask[i * CVT_MAXIMUM_TRACKING_RECTANGLE_WIDTH + j] = fullMask[(i - offsetY) * CVT_MAXIMUM_TRACKING_RECTANGLE_WIDTH + j];
			}
		}
	}
	else {
		for (int32_t j = 0; j < CVT_MAXIMUM_TRACKING_RECTANGLE_WIDTH; ++j)
		{
			for (int32_t i = abs(offsetY); i < CVT_MAXIMUM_TRACKING_RECTANGLE_HEIGHT; ++i)
			{
				patternImage[(i - abs(offsetY)) * CVT_MAXIMUM_TRACKING_RECTANGLE_WIDTH + j] = patternImage[i * CVT_MAXIMUM_TRACKING_RECTANGLE_WIDTH + j];
				trackingRectangleImage[0][(i - abs(offsetY)) * CVT_MAXIMUM_TRACKING_RECTANGLE_WIDTH + j] = trackingRectangleImage[0][i * CVT_MAXIMUM_TRACKING_RECTANGLE_WIDTH + j];
				trackingRectangleImage[1][(i - abs(offsetY)) * CVT_MAXIMUM_TRACKING_RECTANGLE_WIDTH + j] = trackingRectangleImage[1][i * CVT_MAXIMUM_TRACKING_RECTANGLE_WIDTH + j];
				reducedMask[(i - abs(offsetY)) * CVT_MAXIMUM_TRACKING_RECTANGLE_WIDTH + j] = reducedMask[i * CVT_MAXIMUM_TRACKING_RECTANGLE_WIDTH + j];
				fullMask[(i - abs(offsetY)) * CVT_MAXIMUM_TRACKING_RECTANGLE_WIDTH + j] = fullMask[i * CVT_MAXIMUM_TRACKING_RECTANGLE_WIDTH + j];
			}
		}
	}

	return true;
}


bool vtracker::CorrelationVideoTracker::SetTrackingRectangleSizeAutomatically()
{
	// Check tracker mode.
	if (trackerData.mode != CVT_TRACKING_MODE_INDEX)
		return false;

	// Move tracking rectangle.
	int32_t offsetX = trackerData.trackingRectangleCenterX - trackerData.objectCenterX;
	int32_t offsetY = trackerData.trackingRectangleCenterY - trackerData.objectCenterY;
	if (!MoveTrackingRectangle(offsetX, offsetY))
		return false;
	
	// Set size = 150% of object size.
	double newTrackingRectangleWidth = 1.5 * (double)trackerData.objectWidth;
	if (newTrackingRectangleWidth > (double)CVT_MAXIMUM_TRACKING_RECTANGLE_WIDTH)
		newTrackingRectangleWidth = (double)CVT_MAXIMUM_TRACKING_RECTANGLE_WIDTH;
	if (newTrackingRectangleWidth < (double)CVT_MINIMUM_TRACKING_RECTANGLE_WIDTH)
		newTrackingRectangleWidth = (double)CVT_MINIMUM_TRACKING_RECTANGLE_WIDTH;
	double newTrackingRectangleHeight = 1.5 * (double)trackerData.objectWidth;
	if (newTrackingRectangleHeight > (double)CVT_MAXIMUM_TRACKING_RECTANGLE_HEIGHT)
		newTrackingRectangleHeight = (double)CVT_MAXIMUM_TRACKING_RECTANGLE_HEIGHT;
	if (newTrackingRectangleHeight < (double)CVT_MINIMUM_TRACKING_RECTANGLE_HEIGHT)
		newTrackingRectangleHeight = (double)CVT_MINIMUM_TRACKING_RECTANGLE_HEIGHT;

	// Set property value.
	trackerData.trackingRectangleHeight = (int32_t)newTrackingRectangleHeight;
	trackerData.trackingRectangleWidth = (int32_t)newTrackingRectangleWidth;

	return true;
}


bool vtracker::CorrelationVideoTracker::ProcessFrame(uint8_t* frame_mono8, int32_t width, int32_t height, const uint32_t timeoutMs)
{
	// Check input frame data.
	if (width == 0 || height == 0)
		return false;

	// Remember time.
	std::chrono::time_point<std::chrono::system_clock> startTime = std::chrono::system_clock::now();

	// Global lock.
	std::lock_guard<std::mutex> globalLock(accessManageMutex);

	// Check frame buffer initialization.
	if (trackerData.frameWidth != width || trackerData.frameHeight != height)
	{
		// Reset
		Reset();

		trackerData.frameWidth = width;
		trackerData.frameHeight = height;

		// Delete frame buffer.
		if (frameBuffer != nullptr)
			for (int32_t i = 0; i < trackerData.frameBufferSize; ++i)
				delete[] frameBuffer[i];
		delete[] frameBuffer;
		frameBuffer = nullptr;

		// Allocate memory.
		frameBuffer = new uint8_t * [trackerData.frameBufferSize];
		for (int32_t i = 0; i < trackerData.frameBufferSize; ++i)
			frameBuffer[i] = new uint8_t[(size_t)trackerData.frameWidth * (size_t)trackerData.frameHeight];

		// Reset frame ID.
		trackerData.bufferFrameID = trackerData.frameBufferSize - 1;
	}

	// Copy frame data to frame buffer.
	uint32_t size = width * height;
	++trackerData.bufferFrameID;
	if (trackerData.bufferFrameID >= trackerData.frameBufferSize)
		trackerData.bufferFrameID = 0;
	memcpy(frameBuffer[trackerData.bufferFrameID], frame_mono8, size);

	// Check tracker mode.
	if (trackerData.mode == CVT_FREE_MODE_INDEX)
		return true;

	// Calculate tracking.
	while (trackerData.trackerFrameID != trackerData.bufferFrameID && trackerData.mode != CVT_FREE_MODE_INDEX)
	{
		// Increment tracker frame ID
		++trackerData.trackerFrameID;
		if (trackerData.trackerFrameID >= trackerData.frameBufferSize)
			trackerData.trackerFrameID = 0;

		// Calculate correlation surface.
		if (trackerData.numThreads == 1)
			CalculateCorrelationSurface();
		else
			CalculateCorrelationSurface_omp();

		// Analize correlation surface.
		AnalizeCorrelationSurface();

		// Check timeout.
		if (timeoutMs > 0)
			if ((uint32_t)std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - startTime).count() > timeoutMs)
				return true;
	}

	return true;
}


inline void vtracker::CorrelationVideoTracker::AnalizeCorrelationSurface()
{
	// Increase frame counter.
	++trackerData.frameCounter;

	// Find maximums of correlation surface.
	float maxCorr = 0.0f;
	int32_t maxChisl = 0;
	int32_t maxX = 0;
	int32_t maxY = 0;
	int32_t localMaxX[2];
	localMaxX[0] = 0;
	localMaxX[1] = 0;
	int32_t localMaxY[2];
	localMaxY[0] = 0;
	localMaxY[1] = 0;
	float localCorrMax[2];
	localCorrMax[0] = 0.0f;
	localCorrMax[1] = 0.0f;
	int32_t corrW = trackerData.correlationSurfaceWidth;
	int32_t corrH = trackerData.correlationSurfaceHeight;

	// Find first maximum.
	for (int32_t i = 0; i < corrH; ++i)
	{
		for (int32_t j = 0; j < corrW; ++j)
		{
			if (correlationSurface[i * corrW + j] > localCorrMax[0])
			{
				localCorrMax[0] = correlationSurface[i * corrW + j];
				localMaxX[0] = j;
				localMaxY[0] = i;
			}
		}
	}

	// Find second maximum.
	for (int32_t i = 0; i < corrH; ++i)
	{
		for (int32_t j = 0; j < corrW; ++j)
		{
			if (correlationSurface[i * corrW + j] > localCorrMax[1])
			{
				if (abs(localMaxX[0] - j) > LOCAL_MAX_DELTA && abs(localMaxY[0] - i) > LOCAL_MAX_DELTA)
				{
					localCorrMax[1] = correlationSurface[i * corrW + j];
					localMaxX[1] = j;
					localMaxY[1] = i;
				}
			}
		}
	}

	// Copy absolute maximum.
	maxCorr = localCorrMax[0];
	maxX = localMaxX[0];
	maxY = localMaxY[0];

	maxChisl = chislSurface[maxY * corrW + maxX];

	// Check maximum value.
	if (maxCorr == 0.0)
	{
		Reset();
		return;
	}

	// Check differences between maximus.
	if ((localCorrMax[0] - localCorrMax[1]) / localCorrMax[0] <= MIN_CORR_DIFFERENCES)
	{
		if (sqrtf(((float)localMaxX[0] - Xcorrect) * ((float)localMaxX[0] - Xcorrect) + ((float)localMaxY[0] -
			Ycorrect) * ((float)localMaxY[0] - (int32_t)Ycorrect)) >
			sqrtf(((float)localMaxX[1] - Xcorrect) * ((float)localMaxX[1] - Xcorrect) + ((float)localMaxY[1] -
				Ycorrect) * ((float)localMaxY[1] - (int32_t)Ycorrect)))
		{
			maxCorr = localCorrMax[1];
			maxX = localMaxX[1];
			maxY = localMaxX[1];
		}
	}

	// Calculate position with subpixel accuracy.
	float fMaxX = (float)maxX;
	float fMaxY = (float)maxY;
	if (maxX > 0 && maxX < corrW - 1 && maxY > 0 && maxY < corrH - 1)
	{
		float sum =
			correlationSurface[(maxY - 1) * corrW + maxX - 1] +
			correlationSurface[(maxY - 1) * corrW + maxX] +
			correlationSurface[(maxY - 1) * corrW + maxX + 1] +
			correlationSurface[maxY * corrW + maxX - 1] +
			correlationSurface[maxY * corrW + maxX] +
			correlationSurface[maxY * corrW + maxX + 1] +
			correlationSurface[(maxY + 1) * corrW + maxX - 1] +
			correlationSurface[(maxY + 1) * corrW + maxX] +
			correlationSurface[(maxY + 1) * corrW + maxX + 1];
		fMaxX = (((float)(maxX - 1) * correlationSurface[(maxY - 1) * corrW + maxX - 1]) +
			((float)(maxX) * correlationSurface[(maxY - 1) * corrW + maxX]) +
			((float)(maxX + 1) * correlationSurface[(maxY - 1) * corrW + maxX + 1]) +
			((float)(maxX - 1) * correlationSurface[maxY * corrW + maxX - 1]) +
			((float)(maxX) * correlationSurface[maxY * corrW + maxX]) +
			((float)(maxX + 1) * correlationSurface[maxY * corrW + maxX + 1]) +
			((float)(maxX - 1) * correlationSurface[(maxY + 1) * corrW + maxX - 1]) +
			((float)(maxX) *correlationSurface[(maxY + 1) * corrW + maxX]) +
			((float)(maxX + 1) * correlationSurface[(maxY + 1) * corrW + maxX + 1])) / sum;
        fMaxY = (((float)(maxY - 1) * correlationSurface[(maxY - 1) * corrW + maxX - 1]) +
			((float)(maxY - 1) * correlationSurface[(maxY - 1) * corrW + maxX]) +
			((float)(maxY - 1) * correlationSurface[(maxY - 1) * corrW + maxX + 1]) +
			((float)(maxY) * correlationSurface[maxY * corrW + maxX - 1]) +
			((float)(maxY) * correlationSurface[maxY * corrW + maxX]) +
			((float)(maxY) * correlationSurface[maxY * corrW + maxX + 1]) +
			((float)(maxY + 1) * correlationSurface[(maxY + 1) * corrW + maxX - 1]) +
			((float)(maxY + 1) * correlationSurface[(maxY + 1) * corrW + maxX]) +
			((float)(maxY + 1) * correlationSurface[(maxY + 1) * corrW + maxX + 1])) / sum;
	}

	// Follow procession according to tracker mode.
	switch (trackerData.mode)
	{
	case CVT_TRACKING_MODE_INDEX:
	{
		// Check data loss criteria.
		if ((float)maxCorr < (1.0f - trackerData.objectLossThreshold) * trackerData.probabilityAdaptiveThreshold)
		{
			last_chisl_value = 0;

			// If object loss detected set LOST mode.
			trackerData.mode = CVT_LOST_MODE_INDEX;
			trackerData.frameCounterInLostMode = 1;

			// Processing according to opetion of LOST mode.
			switch (trackerData.lostModeOption)
			{
			case 0: // Don't move tracking rectangle.
			{
				// Do nothing.
				trackerData.trackingRectangleCenterFX = trackerData.trackingRectangleCenterFX;
				trackerData.trackingRectangleCenterFY = trackerData.trackingRectangleCenterFY;

				break;
			}
				
			case 1: // Move up to frame border and then stop.
			{
				float tmp_f_strobe_x = trackerData.trackingRectangleCenterFX;
				trackerData.trackingRectangleCenterFX = trackerData.trackingRectangleCenterFX + (Xcorrect - (float)(corrW / 2));
				if (trackerData.trackingRectangleCenterFX - (trackerData.trackingRectangleWidth / 2) < 1 ||
					trackerData.trackingRectangleCenterFX + (trackerData.trackingRectangleWidth / 2) + 1 >= trackerData.frameWidth)
					trackerData.trackingRectangleCenterFX = tmp_f_strobe_x;

				float tmp_f_strobe_y = trackerData.trackingRectangleCenterFY;
				trackerData.trackingRectangleCenterFY = trackerData.trackingRectangleCenterFY + (Ycorrect - (float)(corrH / 2));
				if (trackerData.trackingRectangleCenterFY - (trackerData.trackingRectangleHeight / 2) < 1 ||
					trackerData.trackingRectangleCenterFY + (trackerData.trackingRectangleHeight / 2) + 1 >= trackerData.frameHeight)
					trackerData.trackingRectangleCenterFY = tmp_f_strobe_y;

				break;
			}

			case 2: // Just move.
			{
				trackerData.trackingRectangleCenterFX = trackerData.trackingRectangleCenterFX + (Xcorrect - (float)(corrW / 2));
				trackerData.trackingRectangleCenterFY = trackerData.trackingRectangleCenterFY + (Ycorrect - (float)(corrH / 2));

				break;
			}

			default: // Undefined case.
			{
				trackerData.trackingRectangleCenterFX = trackerData.trackingRectangleCenterFX;
				trackerData.trackingRectangleCenterFY = trackerData.trackingRectangleCenterFY;

				break;
			}
			}

			trackerData.trackingRectangleCenterX = (int32_t)trackerData.trackingRectangleCenterFX;
			trackerData.trackingRectangleCenterY = (int32_t)trackerData.trackingRectangleCenterFY;
			trackerData.objectDetectionProbability = maxCorr;
			// Set search window center coordinates.
			trackerData.searchWindowCenterX = trackerData.trackingRectangleCenterX;
			trackerData.searchWindowCenterY = trackerData.trackingRectangleCenterY;

			// Check reset criterion.
			if (CheckResetCriteria())
			{
				Reset();
				return;
			}

			// Update trajectiory prediction filter.
			XCovariance += XCovariance * 0.25f;
			YCovariance += YCovariance * 0.25f;
			if (YCovariance > (float)(corrH * corrH))
				YCovariance = (float)(corrH * corrH);
			if (XCovariance > (float)(corrW * corrW))
				XCovariance = (float)(corrW * corrW);

		}
		else
		{
			// If no loss. Update params.
			trackerData.horizontalObjectValocity = Xcorrect - (float)(corrW / 2);
			trackerData.verticalObjectVelocity = Ycorrect - (float)(corrH / 2);
			trackerData.trackingRectangleCenterFX = (float)trackerData.searchWindowCenterX - (float)(corrW / 2) + fMaxX;
			trackerData.trackingRectangleCenterFY = (float)trackerData.searchWindowCenterY - (float)(corrH / 2) + fMaxY;
			trackerData.trackingRectangleCenterX = trackerData.searchWindowCenterX - (corrW / 2) + maxX;
			trackerData.trackingRectangleCenterY = trackerData.searchWindowCenterY - (corrH / 2) + maxY;
			trackerData.objectDetectionProbability = maxCorr;
			// Set search window center coordinates.
			trackerData.searchWindowCenterX = trackerData.trackingRectangleCenterX;
			trackerData.searchWindowCenterY = trackerData.trackingRectangleCenterY;

			// Update prabability threshold.
			trackerData.probabilityAdaptiveThreshold = trackerData.probabilityUpdateCoeff * trackerData.probabilityAdaptiveThreshold +
				(1.0f - trackerData.probabilityUpdateCoeff) * maxCorr;

			// Check reset criterion.
			if (CheckResetCriteria())
			{
				Reset();
				return;
			}

			// Add image to tracking rectangle buffer.
			trackingRectangleIndex = 1 - trackingRectangleIndex;
			int32_t x0 = trackerData.trackingRectangleCenterX - CVT_MAXIMUM_TRACKING_RECTANGLE_WIDTH / 2;
			int32_t y0 = trackerData.trackingRectangleCenterY - CVT_MAXIMUM_TRACKING_RECTANGLE_HEIGHT / 2;
			for (int32_t i = 0; i < CVT_MAXIMUM_TRACKING_RECTANGLE_HEIGHT; ++i)
				for (int32_t j = 0; j < CVT_MAXIMUM_TRACKING_RECTANGLE_WIDTH; ++j)
					if (i + y0 >= 0 && i + y0 < trackerData.frameHeight && j + x0 >= 0 && j + x0 < trackerData.frameWidth)
						trackingRectangleImage[trackingRectangleIndex][i * CVT_MAXIMUM_TRACKING_RECTANGLE_WIDTH + j] = frameBuffer[trackerData.trackerFrameID][(i + y0) * trackerData.frameWidth + (j + x0)];

			// Calculate object rectangle.
			CalculateObjectRectangle();

			// Update coordinate filters.
			UpdateCooordinatesFilter(maxX, maxY);

			last_chisl_value = maxChisl;
		}
		break;
	}


	case CVT_LOST_MODE_INDEX:
	{
		// Check object detection criteria.
		if (maxCorr > (1.0f - trackerData.objectDetectionThreshold) * trackerData.probabilityAdaptiveThreshold)
		{
			// If we detect object set TRACKING mode.
			trackerData.mode = CVT_TRACKING_MODE_INDEX;
			trackerData.frameCounterInLostMode = 0;

			trackerData.horizontalObjectValocity = Xcorrect - (float)(corrW / 2);
			trackerData.verticalObjectVelocity = Ycorrect - (float)(corrH / 2);
			trackerData.trackingRectangleCenterFX = (float)trackerData.searchWindowCenterX - (float)(corrW / 2) + fMaxX;
			trackerData.trackingRectangleCenterFY = (float)trackerData.searchWindowCenterY - (float)(corrH / 2) + fMaxY;
			trackerData.trackingRectangleCenterX = trackerData.searchWindowCenterX - (corrW / 2) + maxX;
			trackerData.trackingRectangleCenterY = trackerData.searchWindowCenterY - (corrH / 2) + maxY;
			trackerData.objectDetectionProbability = maxCorr;
			// Set search window center coordinates.
			trackerData.searchWindowCenterX = trackerData.trackingRectangleCenterX;
			trackerData.searchWindowCenterY = trackerData.trackingRectangleCenterY;

			// Update prabability threshold.
			trackerData.probabilityAdaptiveThreshold = trackerData.probabilityUpdateCoeff * trackerData.probabilityAdaptiveThreshold +
				(1.0f - trackerData.probabilityUpdateCoeff) * maxCorr;

			// Check reset criterion.
			if (CheckResetCriteria())
			{
				Reset();
				return;
			}

			// Add image to tracking rectamgle buffer and update pattern.
			trackingRectangleIndex = 1 - trackingRectangleIndex;
			int32_t x0 = trackerData.trackingRectangleCenterX - CVT_MAXIMUM_TRACKING_RECTANGLE_WIDTH / 2;
			int32_t y0 = trackerData.trackingRectangleCenterY - CVT_MAXIMUM_TRACKING_RECTANGLE_HEIGHT / 2;
			for (int32_t i = 0; i < CVT_MAXIMUM_TRACKING_RECTANGLE_HEIGHT; ++i)
			{
				for (int32_t j = 0; j < CVT_MAXIMUM_TRACKING_RECTANGLE_WIDTH; ++j)
				{
					if (i + y0 >= 0 && i + y0 < trackerData.frameHeight && j + x0 >= 0 && j + x0 < trackerData.frameWidth)
					{
						trackingRectangleImage[trackingRectangleIndex][i * CVT_MAXIMUM_TRACKING_RECTANGLE_WIDTH + j] = frameBuffer[trackerData.trackerFrameID][(i + y0) * trackerData.frameWidth + (j + x0)];
						patternImage[i * CVT_MAXIMUM_TRACKING_RECTANGLE_WIDTH + j] = trackingRectangleImage[trackingRectangleIndex][i * CVT_MAXIMUM_TRACKING_RECTANGLE_WIDTH + j];
					}
				}
			}					
						
			// Calculate object rectangle.
			CalculateObjectRectangle();

			// Update coordinate filters.
			UpdateCooordinatesFilter(maxX, maxY);

			last_chisl_value = maxChisl;
		}
		else
		{
			// Increase frame counter in LOST mode.
			++trackerData.frameCounterInLostMode;

			// Processing according to option of LOST mode.
			switch (trackerData.lostModeOption)
			{
			case 0: // Don't move tracking rectangle.
			{
				// Do nothing.
				trackerData.trackingRectangleCenterFX = trackerData.trackingRectangleCenterFX;
				trackerData.trackingRectangleCenterFY = trackerData.trackingRectangleCenterFY;

				break;
			}

			case 1: // Move up to frame border and then stop.
			{
				float tmp_f_strobe_x = trackerData.trackingRectangleCenterFX;
				trackerData.trackingRectangleCenterFX = trackerData.trackingRectangleCenterFX + (Xcorrect - (float)(corrW / 2));
				if (trackerData.trackingRectangleCenterFX - (trackerData.trackingRectangleWidth / 2) < 1 ||
					trackerData.trackingRectangleCenterFX + (trackerData.trackingRectangleWidth / 2) + 1 >= trackerData.frameWidth)
					trackerData.trackingRectangleCenterFX = tmp_f_strobe_x;

				float tmp_f_strobe_y = trackerData.trackingRectangleCenterFY;
				trackerData.trackingRectangleCenterFY = trackerData.trackingRectangleCenterFY + (Ycorrect - (float)(corrH / 2));
				if (trackerData.trackingRectangleCenterFY - (trackerData.trackingRectangleHeight / 2) < 1 ||
					trackerData.trackingRectangleCenterFY + (trackerData.trackingRectangleHeight / 2) + 1 >= trackerData.frameHeight)
					trackerData.trackingRectangleCenterFY = tmp_f_strobe_y;

				break;
			}

			case 2: // Just move.
			{
				trackerData.trackingRectangleCenterFX = trackerData.trackingRectangleCenterFX + (Xcorrect - (float)(corrW / 2));
				trackerData.trackingRectangleCenterFY = trackerData.trackingRectangleCenterFY + (Ycorrect - (float)(corrH / 2));

				break;
			}

			default: // Undefined case.
			{
				trackerData.trackingRectangleCenterFX = trackerData.trackingRectangleCenterFX;
				trackerData.trackingRectangleCenterFY = trackerData.trackingRectangleCenterFY;

				break;
			}
			}

			trackerData.trackingRectangleCenterX = (int32_t)trackerData.trackingRectangleCenterFX;
			trackerData.trackingRectangleCenterY = (int32_t)trackerData.trackingRectangleCenterFY;
			trackerData.objectDetectionProbability = maxCorr;
			// Set search window center coordinates.
			trackerData.searchWindowCenterX = trackerData.trackingRectangleCenterX;
			trackerData.searchWindowCenterY = trackerData.trackingRectangleCenterY;

			// Check reset criterion.
			if (CheckResetCriteria())
			{
				Reset();
				return;
			}

			// Update trajectiory prediction filter.
			XCovariance += XCovariance * 0.25f;
			YCovariance += YCovariance * 0.25f;
			if (YCovariance > (float)(corrH * corrH))
				YCovariance = (float)(corrH * corrH);
			if (XCovariance > (float)(corrW * corrW))
				XCovariance = (float)(corrW * corrW);
		}

		break;
	}


	case CVT_INERTIAL_MODE_INDEX:
	{
		// Just update coordinates.
		trackerData.trackingRectangleCenterFX = trackerData.trackingRectangleCenterFX + (Xcorrect - (float)(corrW / 2));
		trackerData.trackingRectangleCenterFY = trackerData.trackingRectangleCenterFY + (Ycorrect - (float)(corrH / 2));
		trackerData.trackingRectangleCenterX = (int32_t)trackerData.trackingRectangleCenterFX;
		trackerData.trackingRectangleCenterY = (int32_t)trackerData.trackingRectangleCenterFY;
		trackerData.objectDetectionProbability = maxCorr;
		// Set search window center coordinates.
		trackerData.searchWindowCenterX = trackerData.trackingRectangleCenterX;
		trackerData.searchWindowCenterY = trackerData.trackingRectangleCenterY;

		// Check reset criterion.
		if (CheckResetCriteria())
		{
			Reset();
			return;
		}

		break;
	}


	case CVT_STATIC_MODE_INDEX:
	{
		// Do nothing.
		trackerData.objectDetectionProbability = maxCorr;

		// Check reset criterion.
		if (CheckResetCriteria())
		{
			Reset();
			return;
		}

		break;
	}

	default: // Undefined case.
		Reset();
		break;
	}
}


inline void vtracker::CorrelationVideoTracker::CalculateCorrelationSurface()
{
	// Init variables.
//???	register uint8_t mode = trackerData.mode;
	register int32_t max_strobe_w = CVT_MAXIMUM_TRACKING_RECTANGLE_WIDTH;
	register int32_t max_strobe_h = CVT_MAXIMUM_TRACKING_RECTANGLE_HEIGHT;
	register int32_t frame_w = trackerData.frameWidth;
	register int32_t frame_h = trackerData.frameHeight;
	register int32_t corr_w = trackerData.correlationSurfaceWidth;
	register int32_t corr_h = trackerData.correlationSurfaceHeight;
	register int32_t strobe_w = trackerData.trackingRectangleWidth;
	register int32_t strobe_h = trackerData.trackingRectangleHeight;
	register int32_t wind_w = corr_w + strobe_w - 1;
	register int32_t wind_h = corr_h + strobe_h - 1;
	register int32_t strobe_x0 = (max_strobe_w - strobe_w) / 2;
	register int32_t strobe_y0 = (max_strobe_h - strobe_h) / 2;
	register int32_t strobe_x1 = strobe_x0 + strobe_w;
	register int32_t strobe_y1 = strobe_y0 + strobe_h;
	if (strobe_w % 2 != 0)
		strobe_x0 = strobe_x0 + 1;
	if (strobe_h % 2 != 0)
		strobe_y0 = strobe_y0 + 1;
	register int32_t pat_med = 0;
	register int32_t part_med = 0;
	register uint32_t pat_znam = 0;
	register uint32_t part_znam = 0;
	register int32_t A = 0;
	register int32_t B = 0;
	register int32_t C = 0;
	register int32_t D = 0;
//???	register uint32_t E = 0;
//???	register uint32_t a = 0;
//???	register uint32_t b = 0;
//???	register uint32_t c = 0;
//???	register uint32_t d = 0;
	register int32_t frame_x0 = trackerData.searchWindowCenterX - (wind_w / 2);
	register int32_t frame_y0 = trackerData.searchWindowCenterY - (wind_h / 2);
	register int32_t wind_x0 = 0;
	register int32_t wind_x1 = wind_w;
	register int32_t wind_y0 = 0;
	register int32_t wind_y1 = wind_h;
	register int32_t corr_x0 = 0;
	register int32_t corr_y0 = 0;
	register int32_t corr_x1 = corr_w;
	register int32_t corr_y1 = corr_h;
	register int32_t chisl = 0;
	register float corr_k = 0.0f;

	// Pointer to buffers.
	register uint8_t* p_pattern = patternImage;
	register uint8_t* p_curr_strobe = trackingRectangleImage[trackingRectangleIndex];
	register uint8_t* p_prev_strobe = trackingRectangleImage[1 - trackingRectangleIndex];
	register int32_t* p_razn_pattern = differencePattern;
	register int32_t* tmp_p_razn_pattern = nullptr;
	register uint8_t* p_frame = frameBuffer[trackerData.trackerFrameID];
	register uint8_t* p_reduced_mask = reducedMask;
	register uint8_t* tmp_p_reduced_mask = nullptr;
	register uint8_t* p_full_mask = fullMask;
	register int32_t* p_razn_window = differenceWindow;
	register uint32_t* p_double_razn_window = doubledDifferenceWindow;
	register int32_t* tmp_p_razn_window = nullptr;
	register uint32_t* tmp_p_double_razn_window = nullptr;
	register float* p_corr = correlationSurface;
	register float* p_sko = deviationSurface;
	register int32_t* p_chisl = chislSurface;

	// Calculate position restrictions.
	if (frame_x0 < 0)
	{
		wind_x0 = -frame_x0;
		corr_x0 = -frame_x0;
	}
	if (frame_x0 + wind_x1 > frame_w)
	{
		corr_x1 = corr_x1 - ((frame_x0 + wind_x1) - frame_w);
		wind_x1 = wind_x1 - ((frame_x0 + wind_x1) - frame_w);
	}

	if (frame_y0 < 0)
	{
		wind_y0 = -frame_y0;
		corr_y0 = -frame_y0;
	}

	if (frame_y0 + wind_y1 > frame_h)
	{
		corr_y1 = corr_y1 - ((frame_y0 + wind_y1) - frame_h);
		wind_y1 = wind_y1 - ((frame_y0 + wind_y1) - frame_h);
	}


	// Update pattern image.
	if (trackerData.mode == CVT_TRACKING_MODE_INDEX)
	{
		// Update and calculate medium values.
		for (int32_t i = 0; i < max_strobe_h; ++i)
		{
			for (int32_t j = 0; j < max_strobe_w; ++j)
			{
				A = (int32_t)p_curr_strobe[i * max_strobe_w + j];
				B = (int32_t)p_pattern[i * max_strobe_w + j];
				B = (int32_t)((float)B * trackerData.patternUpdateCoeff + (1.0f - trackerData.patternUpdateCoeff) * (float)A);
				p_pattern[i * max_strobe_w + j] = (uint8_t)B;

				if (i >= strobe_y0 && i < strobe_y1 && j >= strobe_x0 && j < strobe_x1)
				{
					pat_med += B;
					part_med += A;
				}
			}
		}
	}
	else {
		// Just calculate parameters.
		for (int32_t i = strobe_y0; i < strobe_y1; ++i)
		{
			for (int32_t j = strobe_x0; j < strobe_x1; ++j)
			{
				pat_med += (int32_t)p_pattern[i * max_strobe_w + j];
				part_med += (int32_t)p_curr_strobe[i * max_strobe_w + j];
			}
		}
	}

	// Calculate medium values.
	pat_med = pat_med / (strobe_w * strobe_h);
	part_med = part_med / (strobe_w * strobe_h);

	// Reset correlation surface.
	for (int32_t i = 0; i < corr_h; ++i)
	{
		for (int32_t j = 0; j < corr_w; ++j)
		{
			p_corr[i * corr_w + j] = 0.0f;
			p_chisl[i * corr_w + j] = 0;
		}
	}
		

	// Update filters and calculate parameters.
	for (int32_t i = 0; i < max_strobe_h; ++i)
	{
		for (int32_t j = 0; j < max_strobe_w; ++j)
		{
			A = MIN_MASK_VALUE;
			if (i > strobe_y0 && i < strobe_y1 - 1 && j > strobe_x0 && j < strobe_x1 - 1)
			{
				B = -(int32_t)p_pattern[(i - 1) * max_strobe_w + j - 1] -
					2 * (int32_t)p_pattern[(i - 1) * max_strobe_w + j] -
					(int32_t)p_pattern[(i - 1) * max_strobe_w + j + 1] +
					(int32_t)p_pattern[(i + 1) * max_strobe_w + j - 1] +
					2 * (int32_t)p_pattern[(i + 1) * max_strobe_w + j] +
					(int32_t)p_pattern[(i + 1) * max_strobe_w + j + 1];

				C = -(int32_t)p_pattern[(i - 1) * max_strobe_w + j - 1] +
					(int32_t)p_pattern[(i - 1) * max_strobe_w + j + 1] -
					2 * (int32_t)p_pattern[i * max_strobe_w + j - 1] +
					2 * (int32_t)p_pattern[i * max_strobe_w + j + 1] -
					(int32_t)p_pattern[(i + 1) * max_strobe_w + j - 1] +
					(int32_t)p_pattern[(i + 1) * max_strobe_w + j + 1];

				D = (int32_t)sqrtf((float)((B * B) + (C * C)));

				A = (int32_t)p_full_mask[(i - 1) * max_strobe_w + j - 1] * 1 +
					(int32_t)p_full_mask[(i - 1) * max_strobe_w + j] * 2 +
					(int32_t)p_full_mask[(i - 1) * max_strobe_w + j + 1] * 1 +
					(int32_t)p_full_mask[i * max_strobe_w + j - 1] * 2 +
					(int32_t)p_full_mask[i * max_strobe_w + j] * 3 +
					(int32_t)p_full_mask[i * max_strobe_w + j + 1] * 2 +
					(int32_t)p_full_mask[(i + 1) * max_strobe_w + j - 1] * 1 +
					(int32_t)p_full_mask[(i + 1) * max_strobe_w + j] * 2 +
					(int32_t)p_full_mask[(i + 1) * max_strobe_w + j + 1] * 1;

				A = (D * PAT_MUL_IN_MASK) + A;
				A = A / 16;

				D = (int32_t)p_curr_strobe[i * max_strobe_w + j] - (int32_t)p_prev_strobe[i * max_strobe_w + j];

				p_sko[i * max_strobe_w + j] =
					p_sko[i * max_strobe_w + j] * trackerData.patternUpdateCoeff + (1.0f - trackerData.patternUpdateCoeff) * sqrtf((float)(D * D));

				A = A - (int32_t)(SKO_MULTIPLICATOR * p_sko[i * max_strobe_w + j]) + MASK_ADD_VALUE;

				if (A < MIN_MASK_VALUE)
					A = MIN_MASK_VALUE;
				if (A > MAX_MASK_VALUE)
					A = MAX_MASK_VALUE;
			}

			p_reduced_mask[i * max_strobe_w + j] = (uint8_t)A;
			B = (int32_t)p_pattern[i * max_strobe_w + j] - pat_med;
			A = A / MIN_MASK_VALUE;
			p_razn_pattern[i * max_strobe_w + j] = (B * A) / PAT_DIVIDER;

			if (i >= strobe_y0 && i < strobe_y1 && j >= strobe_x0 && j < strobe_x1)
				pat_znam += (B * B * A) / ZNAM_DIVIDER;
		}
	}

	// Calculate znam value.
	pat_znam = (int32_t)sqrt(pat_znam);

	// Copy mask data.
	for (int32_t i = 0; i < max_strobe_h; ++i)
	{
		for (int32_t j = 0; j < max_strobe_w; ++j)
		{
			p_full_mask[i * max_strobe_w + j] = p_reduced_mask[i * max_strobe_w + j];
			p_reduced_mask[i * max_strobe_w + j] = p_reduced_mask[i * max_strobe_w + j] / MIN_MASK_VALUE;
		}
	}

	// Create razn wind and doubled razn wind.
	for (int32_t i = wind_y0; i < wind_y1; ++i)
	{
		for (int32_t j = wind_x0; j < wind_x1; ++j)
		{
			A = p_frame[(i + frame_y0) * frame_w + (j + frame_x0)] - part_med;
			p_razn_window[i * wind_w + j] = A;
			p_double_razn_window[i * wind_w + j] = (uint32_t)(A * A);
		}
	}
	

	// Calculate correlation.
	for (int32_t i = corr_y0; i < corr_y1; i = i + CORR_STEP)
	{
		for (int32_t j = corr_x0; j < corr_x1; j = j + CORR_STEP)
		{
			chisl = 0;
			for (int32_t k = strobe_y0; k < strobe_y1; ++k)
			{
				tmp_p_razn_pattern = &p_razn_pattern[k * max_strobe_w + strobe_x0];
				tmp_p_razn_window = &p_razn_window[(i + k - strobe_y0) * wind_w + j];
				for (int32_t t = strobe_x0; t < strobe_x1; ++t)
				{
					chisl += (*tmp_p_razn_window++) * (*tmp_p_razn_pattern++);
				}
			}

			if (chisl < 0)
			{
				p_chisl[i * corr_w + j] = 0;
				continue;
			}			

			part_znam = 0;
			for (int32_t k = strobe_y0; k < strobe_y1; ++k)
			{
				tmp_p_double_razn_window = &p_double_razn_window[(i + k - strobe_y0) * wind_w + j];
				tmp_p_reduced_mask = &p_reduced_mask[k * max_strobe_w + strobe_x0];
				for (int32_t t = strobe_x0; t < strobe_x1; ++t)
				{
					part_znam += (*tmp_p_double_razn_window++) * (uint32_t)(*tmp_p_reduced_mask++);
				}
			}
			part_znam = (uint32_t)sqrt(part_znam);

			if (part_znam != 0 && pat_znam != 0)
				corr_k = (float)chisl / (float)(part_znam * pat_znam);
			else
				corr_k = 0.0f;

			p_chisl[i * corr_w + j] = chisl;

			p_corr[i * corr_w + j] =
				corr_k * expf(-(((float)j - Xcorrect) * ((float)j - Xcorrect)) / (8.0f * XCovariance)) *
				expf(-(((float)i - Ycorrect) * ((float)i - Ycorrect)) / (8.0f * YCovariance));
		}
	}


	// Find first maximum.
//???	float maxCorr = 0.0f;
	int32_t localMaxX[2];
	localMaxX[0] = 0;
	localMaxX[1] = 0;
	int32_t localMaxY[2];
	localMaxY[0] = 0;
	localMaxY[1] = 0;
	float localCorrMax[2];
	localCorrMax[0] = 0.0f;
	localCorrMax[1] = 0.0f;

	// Find first maximum.
	for (int32_t i = corr_y0; i < corr_y1; i = i + CORR_STEP)
	{
		for (int32_t j = corr_x0; j < corr_x1; j = j + CORR_STEP)
		{
			if (correlationSurface[i * corr_w + j] > localCorrMax[0])
			{
				localCorrMax[0] = correlationSurface[i * corr_w + j];
				localMaxX[0] = j;
				localMaxY[0] = i;
			}
		}
	}

	// Find second maximum.
	for (int32_t i = corr_y0; i < corr_y1; i = i + CORR_STEP)
	{
		for (int32_t j = corr_x0; j < corr_x1; j = j + CORR_STEP)
		{
			if (correlationSurface[i * corr_w + j] > localCorrMax[1])
			{
				if (abs(localMaxX[0] - j) > LOCAL_MAX_DELTA && abs(localMaxY[0] - i) > LOCAL_MAX_DELTA)
				{
					localCorrMax[1] = correlationSurface[i * corr_w + j];
					localMaxX[1] = j;
					localMaxY[1] = i;
				}
			}
		}
	}

	int32_t csx0, csx1, csy0, csy1;
	for (int32_t m = 0; m < 2; ++m)
	{
		csx0 = localMaxX[m] - LOCAL_MAX_BORDER;
		if (csx0 < corr_x0)
			csx0 = corr_x0;
		csx1 = localMaxX[m] + LOCAL_MAX_BORDER + 1;
		if (csx1 > corr_x1)
			csx1 = corr_x1;
		csy0 = localMaxY[m] - LOCAL_MAX_BORDER;
		if (csy0 < corr_y0)
			csy0 = corr_y0;
		csy1 = localMaxY[m] + LOCAL_MAX_BORDER + 1;
		if (csy1 > corr_y1)
			csy1 = corr_y1;

		for (int32_t i = csy0; i < csy1; ++i)
		{
			for (int32_t j = csx0; j < csx1; ++j)
			{
				if ((j - corr_x0) % CORR_STEP != 0 ||
					(i - corr_y0) % CORR_STEP != 0)
				{
					chisl = 0;
					for (int32_t k = strobe_y0; k < strobe_y1; ++k)
					{
						tmp_p_razn_pattern = &p_razn_pattern[k * max_strobe_w + strobe_x0];
						tmp_p_razn_window = &p_razn_window[(i + k - strobe_y0) * wind_w + j];
						for (int32_t t = strobe_x0; t < strobe_x1; ++t)
						{
							chisl += (*tmp_p_razn_window++) * (*tmp_p_razn_pattern++);
						}
					}

					if (chisl < 0)
					{
						p_chisl[i * corr_w + j] = 0;
						continue;
					}

					part_znam = 0;
					for (int32_t k = strobe_y0; k < strobe_y1; ++k)
					{
						tmp_p_double_razn_window = &p_double_razn_window[(i + k - strobe_y0) * wind_w + j];
						tmp_p_reduced_mask = &p_reduced_mask[k * max_strobe_w + strobe_x0];
						for (int32_t t = strobe_x0; t < strobe_x1; ++t)
						{
							part_znam += (*tmp_p_double_razn_window++) * (uint32_t)(*tmp_p_reduced_mask++);
						}
					}
					part_znam = (uint32_t)sqrt(part_znam);

					if (part_znam != 0 && pat_znam != 0)
						corr_k = (float)chisl / (float)(part_znam * pat_znam);
					else
						corr_k = 0.0f;

					p_chisl[i * corr_w + j] = chisl;

					p_corr[i * corr_w + j] =
						corr_k * expf(-(((float)j - Xcorrect) * ((float)j - Xcorrect)) / (8.0f * XCovariance)) *
						expf(-(((float)i - Ycorrect) * ((float)i - Ycorrect)) / (8.0f * YCovariance));
				}
			}
		}
	}

}


inline void vtracker::CorrelationVideoTracker::CalculateCorrelationSurface_omp()
{
	// Calculate num threads.
	if (trackerData.numThreads > 0)
	{
		omp_set_num_threads(trackerData.numThreads);
	}

	// Common variables.
	int32_t pat_med_sum[CVT_MAXIMUM_NUM_THREADS];
	int32_t part_med_sum[CVT_MAXIMUM_NUM_THREADS];
	uint32_t pat_znam_sum[CVT_MAXIMUM_NUM_THREADS];
	memset(pat_med_sum, 0, CVT_MAXIMUM_NUM_THREADS * sizeof(int32_t));
	memset(part_med_sum, 0, CVT_MAXIMUM_NUM_THREADS * sizeof(int32_t));
	memset(pat_znam_sum, 0, CVT_MAXIMUM_NUM_THREADS * sizeof(uint32_t));

	register int32_t pat_med = 0;
	register int32_t part_med = 0;
	register uint32_t pat_znam = 0;
//???	float maxCorr = 0.0f;
	int32_t localMaxX[2];
	localMaxX[0] = 0;
	localMaxX[1] = 0;
	int32_t localMaxY[2];
	localMaxY[0] = 0;
	localMaxY[1] = 0;
	float localCorrMax[2];
	localCorrMax[0] = 0.0f;
	localCorrMax[1] = 0.0f;

#pragma omp parallel
	{
		// Get num threads and thread num.
		register int32_t thread_num = omp_get_thread_num();
		register int32_t num_threads = omp_get_num_threads();

		// Init variables.
		register uint8_t mode = trackerData.mode;
		register int32_t max_strobe_w = CVT_MAXIMUM_TRACKING_RECTANGLE_WIDTH;
		register int32_t max_strobe_h = CVT_MAXIMUM_TRACKING_RECTANGLE_HEIGHT;
		register int32_t frame_w = trackerData.frameWidth;
		register int32_t frame_h = trackerData.frameHeight;
		register int32_t corr_w = trackerData.correlationSurfaceWidth;
		register int32_t corr_h = trackerData.correlationSurfaceHeight;
		register int32_t strobe_w = trackerData.trackingRectangleWidth;
		register int32_t strobe_h = trackerData.trackingRectangleHeight;
		register int32_t wind_w = corr_w + strobe_w - 1;
		register int32_t wind_h = corr_h + strobe_h - 1;
		register int32_t strobe_x0 = (max_strobe_w - strobe_w) / 2;
		register int32_t strobe_y0 = (max_strobe_h - strobe_h) / 2;
		if (strobe_w % 2 != 0)
			strobe_x0 = strobe_x0 + 1;
		if (strobe_h % 2 != 0)
			strobe_y0 = strobe_y0 + 1;
		register int32_t strobe_x1 = strobe_x0 + strobe_w;
		register int32_t strobe_y1 = strobe_y0 + strobe_h;
		register uint32_t part_znam = 0;
		register int32_t A = 0;
		register int32_t B = 0;
		register int32_t C = 0;
		register int32_t D = 0;
//???		register uint32_t E = 0;
//???		register uint32_t a = 0;
//???		register uint32_t b = 0;
//???		register uint32_t c = 0;
//???		register uint32_t d = 0;
		register int32_t frame_x0 = trackerData.searchWindowCenterX - (wind_w / 2);
		register int32_t frame_y0 = trackerData.searchWindowCenterY - (wind_h / 2);
		register int32_t wind_x0 = 0;
		register int32_t wind_x1 = wind_w;
		register int32_t wind_y0 = 0;
		register int32_t wind_y1 = wind_h;
		register int32_t corr_x0 = 0;
		register int32_t corr_y0 = 0;
		register int32_t corr_x1 = corr_w;
		register int32_t corr_y1 = corr_h;
		register int32_t chisl = 0;
		register float corr_k = 0.0f;

		// Pointer to buffers.
		register uint8_t* p_pattern = patternImage;
		register uint8_t* p_curr_strobe = trackingRectangleImage[trackingRectangleIndex];
		register uint8_t* p_prev_strobe = trackingRectangleImage[1 - trackingRectangleIndex];
		register int32_t* p_razn_pattern = differencePattern;
		register int32_t* tmp_p_razn_pattern = nullptr;
		register uint8_t* p_frame = frameBuffer[trackerData.trackerFrameID];
		register uint8_t* p_reduced_mask = reducedMask;
		register uint8_t* tmp_p_reduced_mask = nullptr;
		register uint8_t* p_full_mask = fullMask;
		register int32_t* p_razn_window = differenceWindow;
		register uint32_t* p_double_razn_window = doubledDifferenceWindow;
		register int32_t* tmp_p_razn_window = nullptr;
		register uint32_t* tmp_p_double_razn_window = nullptr;
		register float* p_corr = correlationSurface;
		register float* p_sko = deviationSurface;
		register int32_t* p_chisl = chislSurface;

		// Calculate position restrictions.
		if (frame_x0 < 0)
		{
			wind_x0 = -frame_x0;
			corr_x0 = -frame_x0;
		}
		if (frame_x0 + wind_x1 > frame_w)
		{
			corr_x1 = corr_x1 - ((frame_x0 + wind_x1) - frame_w);
			wind_x1 = wind_x1 - ((frame_x0 + wind_x1) - frame_w);
		}

		if (frame_y0 < 0)
		{
			wind_y0 = -frame_y0;
			corr_y0 = -frame_y0;
		}

		if (frame_y0 + wind_y1 > frame_h)
		{
			corr_y1 = corr_y1 - ((frame_y0 + wind_y1) - frame_h);
			wind_y1 = wind_y1 - ((frame_y0 + wind_y1) - frame_h);
		}

		// Update pattern image.
		register int32_t strobe_h0 = (max_strobe_h * thread_num) / num_threads;
		register int32_t strobe_h1 = (max_strobe_h * (thread_num + 1)) / num_threads;
		if (mode == CVT_TRACKING_MODE_INDEX)
		{
			// Update and calculate medium values.
			for (int32_t i = strobe_h0; i < strobe_h1; ++i)
			{
				for (int32_t j = 0; j < max_strobe_w; ++j)
				{
					A = (int32_t)p_curr_strobe[i * max_strobe_w + j];
					B = (int32_t)p_pattern[i * max_strobe_w + j];
					B = (int32_t)((float)B * trackerData.patternUpdateCoeff + (1.0f - trackerData.patternUpdateCoeff) * (float)A);
					p_pattern[i * max_strobe_w + j] = (uint8_t)B;

					if (i >= strobe_y0 && i < strobe_y1 && j >= strobe_x0 && j < strobe_x1)
					{
						pat_med_sum[thread_num] += B;
						part_med_sum[thread_num] += A;
					}
				}
			}
		}
		else {
			// Just calculate parameters.
			for (int32_t i = strobe_h0; i < strobe_h1; ++i)
			{
				for (int32_t j = 0; j < max_strobe_w; ++j)
				{
					if (i >= strobe_y0 && i < strobe_y1 && j >= strobe_x0 && j < strobe_x1)
					{
						pat_med_sum[thread_num] += (int32_t)p_pattern[i * max_strobe_w + j];
						part_med_sum[thread_num] += (int32_t)p_curr_strobe[i * max_strobe_w + j];
					}
				}
			}
		}

#pragma omp barrier

		// Calculate medium values by first thread.
		if (thread_num == 0)
		{
			for (int32_t i = 0; i < num_threads; ++i)
			{
				pat_med += pat_med_sum[i];
				part_med += part_med_sum[i];
			}
			pat_med = pat_med / (strobe_w * strobe_h);
			part_med = part_med / (strobe_w * strobe_h);
		}
		
		// Reset correlation surface.
		register int32_t corr_h0 = (corr_h * thread_num) / num_threads;
		register int32_t corr_h1 = (corr_h * (thread_num + 1)) / num_threads;
		for (int32_t i = corr_h0; i < corr_h1; ++i)
		{
			for (int32_t j = 0; j < corr_w; ++j)
			{
				p_corr[i * corr_w + j] = 0.0f;
				p_chisl[i * corr_w + j] = 0;
			}
		}

#pragma omp barrier

		// Update filters and calculate parameters.
		for (int32_t i = strobe_h0; i < strobe_h1; ++i)
		{
			for (int32_t j = 0; j < max_strobe_w; ++j)
			{
				A = MIN_MASK_VALUE;
				if (i > strobe_y0 && i < strobe_y1 - 1 && j > strobe_x0 && j < strobe_x1 - 1)
				{
					B = -(int32_t)p_pattern[(i - 1) * max_strobe_w + j - 1] -
						2 * (int32_t)p_pattern[(i - 1) * max_strobe_w + j] -
						(int32_t)p_pattern[(i - 1) * max_strobe_w + j + 1] +
						(int32_t)p_pattern[(i + 1) * max_strobe_w + j - 1] +
						2 * (int32_t)p_pattern[(i + 1) * max_strobe_w + j] +
						(int32_t)p_pattern[(i + 1) * max_strobe_w + j + 1];

					C = -(int32_t)p_pattern[(i - 1) * max_strobe_w + j - 1] +
						(int32_t)p_pattern[(i - 1) * max_strobe_w + j + 1] -
						2 * (int32_t)p_pattern[i * max_strobe_w + j - 1] +
						2 * (int32_t)p_pattern[i * max_strobe_w + j + 1] -
						(int32_t)p_pattern[(i + 1) * max_strobe_w + j - 1] +
						(int32_t)p_pattern[(i + 1) * max_strobe_w + j + 1];

					D = (int32_t)sqrtf((float)((B * B) + (C * C)));

					A = (int32_t)p_full_mask[(i - 1) * max_strobe_w + j - 1] * 1 +
						(int32_t)p_full_mask[(i - 1) * max_strobe_w + j] * 2 +
						(int32_t)p_full_mask[(i - 1) * max_strobe_w + j + 1] * 1 +
						(int32_t)p_full_mask[i * max_strobe_w + j - 1] * 2 +
						(int32_t)p_full_mask[i * max_strobe_w + j] * 3 +
						(int32_t)p_full_mask[i * max_strobe_w + j + 1] * 2 +
						(int32_t)p_full_mask[(i + 1) * max_strobe_w + j - 1] * 1 +
						(int32_t)p_full_mask[(i + 1) * max_strobe_w + j] * 2 +
						(int32_t)p_full_mask[(i + 1) * max_strobe_w + j + 1] * 1;

					A = (D * PAT_MUL_IN_MASK) + A;
					A = A / 16;

					D = (int32_t)p_curr_strobe[i * max_strobe_w + j] - (int32_t)p_prev_strobe[i * max_strobe_w + j];

					p_sko[i * max_strobe_w + j] =
						p_sko[i * max_strobe_w + j] * trackerData.patternUpdateCoeff + (1.0f - trackerData.patternUpdateCoeff) * sqrtf((float)(D * D));

					A = A - (int32_t)(SKO_MULTIPLICATOR * p_sko[i * max_strobe_w + j]) + MASK_ADD_VALUE;

					if (A < MIN_MASK_VALUE)
						A = MIN_MASK_VALUE;
					if (A > MAX_MASK_VALUE)
						A = MAX_MASK_VALUE;
				}

				p_reduced_mask[i * max_strobe_w + j] = (uint8_t)A;
				B = (int32_t)p_pattern[i * max_strobe_w + j] - pat_med;
				A = A / MIN_MASK_VALUE;
				p_razn_pattern[i * max_strobe_w + j] = (B * A) / PAT_DIVIDER;

				if (i >= strobe_y0 && i < strobe_y1 && j >= strobe_x0 && j < strobe_x1)
					pat_znam_sum[thread_num] += (B * B * A) / ZNAM_DIVIDER;
			}
		}

#pragma omp barrier

		// Calculate znam value by first thread.
		if (thread_num == 0)
		{
			for (int32_t i = 0; i < num_threads; ++i)
				pat_znam += pat_znam_sum[i];
			pat_znam = (int32_t)sqrt(pat_znam);
		}

		// Copy mask data.
		for (int32_t i = strobe_h0; i < strobe_h1; ++i)
		{
			for (int32_t j = 0; j < max_strobe_w; ++j)
			{
				p_full_mask[i * max_strobe_w + j] = p_reduced_mask[i * max_strobe_w + j];
				p_reduced_mask[i * max_strobe_w + j] = p_reduced_mask[i * max_strobe_w + j] / MIN_MASK_VALUE;
			}
		}

		// Create razn wind and doubled razn wind.
		register int32_t wind_h0 = wind_y0 + ((wind_y1 - wind_y0) * thread_num) / num_threads;
		register int32_t wind_h1 = wind_y0 + ((wind_y1 - wind_y0) * (thread_num + 1)) / num_threads;
		for (int32_t i = wind_h0; i < wind_h1; ++i)
		{
			for (int32_t j = wind_x0; j < wind_x1; ++j)
			{
				A = p_frame[(i + frame_y0) * frame_w + (j + frame_x0)] - part_med;
				p_razn_window[i * wind_w + j] = A;
				p_double_razn_window[i * wind_w + j] = (uint32_t)(A * A);
			}
		}


#pragma omp barrier

		// Calculate correlation.
		corr_h0 = corr_y0 + ((corr_y1 - corr_y0) * thread_num) / num_threads;
		corr_h1 = corr_y0 + ((corr_y1 - corr_y0) * (thread_num + 1)) / num_threads;
		for (int32_t i = corr_h0; i < corr_h1; ++i)
		{
			if ((i - corr_y0) % CORR_STEP != 0)
				continue;

			for (int32_t j = corr_x0; j < corr_x1; j = j + CORR_STEP)
			{
				chisl = 0;
				for (int32_t k = strobe_y0; k < strobe_y1; ++k)
				{
					tmp_p_razn_pattern = &p_razn_pattern[k * max_strobe_w + strobe_x0];
					tmp_p_razn_window = &p_razn_window[(i + k - strobe_y0) * wind_w + j];
					for (int32_t t = strobe_x0; t < strobe_x1; ++t)
					{
						chisl += (*tmp_p_razn_window++) * (*tmp_p_razn_pattern++);
					}
				}

				if (chisl < 0)
				{
					p_chisl[i * corr_w + j] = 0;
					continue;
				}

				part_znam = 0;
				for (int32_t k = strobe_y0; k < strobe_y1; ++k)
				{
					tmp_p_double_razn_window = &p_double_razn_window[(i + k - strobe_y0) * wind_w + j];
					tmp_p_reduced_mask = &p_reduced_mask[k * max_strobe_w + strobe_x0];
					for (int32_t t = strobe_x0; t < strobe_x1; ++t)
					{
						part_znam += (*tmp_p_double_razn_window++) * (uint32_t)(*tmp_p_reduced_mask++);
					}
				}
				part_znam = (uint32_t)sqrt(part_znam);

				if (part_znam != 0 && pat_znam != 0)
					corr_k = (float)chisl / (float)(part_znam * pat_znam);
				else
					corr_k = 0.0f;

				p_chisl[i * corr_w + j] = chisl;

				p_corr[i * corr_w + j] =
					corr_k * expf(-(((float)j - Xcorrect) * ((float)j - Xcorrect)) / (8.0f * XCovariance)) *
					expf(-(((float)i - Ycorrect) * ((float)i - Ycorrect)) / (8.0f * YCovariance));
			}
		}

#pragma omp barrier

		// Find maximums.
		if (thread_num == 0)
		{
			for (int32_t i = corr_y0; i < corr_y1; i = i + CORR_STEP)
			{
				for (int32_t j = corr_x0; j < corr_x1; j = j + CORR_STEP)
				{
					if (correlationSurface[i * corr_w + j] > localCorrMax[0])
					{
						localCorrMax[0] = correlationSurface[i * corr_w + j];
						localMaxX[0] = j;
						localMaxY[0] = i;
					}
				}
			}

			// Find second maximum.
			for (int32_t i = corr_y0; i < corr_y1; i = i + CORR_STEP)
			{
				for (int32_t j = corr_x0; j < corr_x1; j = j + CORR_STEP)
				{
					if (correlationSurface[i * corr_w + j] > localCorrMax[1])
					{
						if (abs(localMaxX[0] - j) > LOCAL_MAX_DELTA && abs(localMaxY[0] - i) > LOCAL_MAX_DELTA)
						{
							localCorrMax[1] = correlationSurface[i * corr_w + j];
							localMaxX[1] = j;
							localMaxY[1] = i;
						}
					}
				}
			}
		}


#pragma omp barrier


		for (int32_t m = 0; m < 2; ++m)
		{
			if (localCorrMax[m] <= 0.0f)
				continue;

			int32_t csx0, csx1, csy0, csy1;

			csx0 = localMaxX[m] - LOCAL_MAX_BORDER;
			if (csx0 < corr_x0)
				csx0 = corr_x0;
			csx1 = localMaxX[m] + LOCAL_MAX_BORDER + 1;
			if (csx1 > corr_x1)
				csx1 = corr_x1;
			csy0 = localMaxY[m] - LOCAL_MAX_BORDER;
			if (csy0 < corr_y0)
				csy0 = corr_y0;
			csy1 = localMaxY[m] + LOCAL_MAX_BORDER + 1;
			if (csy1 > corr_y1)
				csy1 = corr_y1;

			corr_h0 = csy0 + ((csy1 - csy0) * thread_num) / num_threads;
			corr_h1 = csy0 + ((csy1 - csy0) * (thread_num + 1)) / num_threads;
			for (int32_t i = corr_h0; i < corr_h1; ++i)
			{
				for (int32_t j = csx0; j < csx1; ++j)
				{
					if ((j - corr_x0) % CORR_STEP != 0 ||
						(i - corr_y0) % CORR_STEP != 0)
					{
						chisl = 0;
						for (int32_t k = strobe_y0; k < strobe_y1; ++k)
						{
							tmp_p_razn_pattern = &p_razn_pattern[k * max_strobe_w + strobe_x0];
							tmp_p_razn_window = &p_razn_window[(i + k - strobe_y0) * wind_w + j];
							for (int32_t t = strobe_x0; t < strobe_x1; ++t)
							{
								chisl += (*tmp_p_razn_window++) * (*tmp_p_razn_pattern++);
							}
						}

						if (chisl < 0)
						{
							p_chisl[i * corr_w + j] = 0;
							continue;
						}

						part_znam = 0;
						for (int32_t k = strobe_y0; k < strobe_y1; ++k)
						{
							tmp_p_double_razn_window = &p_double_razn_window[(i + k - strobe_y0) * wind_w + j];
							tmp_p_reduced_mask = &p_reduced_mask[k * max_strobe_w + strobe_x0];
							for (int32_t t = strobe_x0; t < strobe_x1; ++t)
							{
								part_znam += (*tmp_p_double_razn_window++) * (uint32_t)(*tmp_p_reduced_mask++);
							}
						}
						part_znam = (uint32_t)sqrt(part_znam);

						if (part_znam != 0 && pat_znam != 0)
							corr_k = (float)chisl / (float)(part_znam * pat_znam);
						else
							corr_k = 0.0f;

						p_chisl[i * corr_w + j] = chisl;

						p_corr[i * corr_w + j] =
							corr_k * expf(-(((float)j - Xcorrect) * ((float)j - Xcorrect)) / (8.0f * XCovariance)) *
							expf(-(((float)i - Ycorrect) * ((float)i - Ycorrect)) / (8.0f * YCovariance));
					}
				}
			}
		}

	}//omp parallel...
}


inline int32_t vtracker::CorrelationVideoTracker::FindFrameID(
	const int32_t x,
	const int32_t y,
	const uint8_t* descriptor)
{
	// Allocate memory for descriptors.
	int32_t width = trackerData.frameWidth;
	int32_t height = trackerData.frameHeight;
	int32_t descriptor_width = CVT_MAXIMUM_TRACKING_RECTANGLE_WIDTH / 8;
	int32_t descriptor_height = CVT_MAXIMUM_TRACKING_RECTANGLE_HEIGHT / 8;
	uint8_t** descriptors;
	descriptors = new uint8_t *[trackerData.frameBufferSize];
	for (int32_t i = 0; i < trackerData.frameBufferSize; ++i)
		descriptors[i] = new uint8_t[(size_t)descriptor_width * (size_t)descriptor_height];

	// Calculate descriptors.
	int32_t frame_x0 = x - CVT_MAXIMUM_TRACKING_RECTANGLE_WIDTH / 2;
	int32_t frame_y0 = y - CVT_MAXIMUM_TRACKING_RECTANGLE_HEIGHT / 2;
	for (int32_t n = 0; n < trackerData.frameBufferSize; ++n)
	{
		for (int32_t i = 0; i < descriptor_height; ++i)
		{
			for (int32_t j = 0; j < descriptor_width; ++j)
			{
				if (frame_y0 + i * 8 < 0 ||
					frame_y0 + i * 8 + 8 > height ||
					frame_x0 + j * 8 < 0 ||
					frame_x0 + j * 8 + 8 > width)
				{
					descriptors[n][i * descriptor_width + j] = 0;
				}
				else
				{
					uint32_t medium_value = 0;
					for (int32_t k = frame_y0 + i * 8; k < frame_y0 + i * 8 + 8; ++k)
						for (int32_t t = frame_x0 + j * 8; t < frame_x0 + j * 8 + 8; ++t)
							medium_value += frameBuffer[n][k * width + t];
					medium_value = medium_value / 64;

					descriptors[n][i * descriptor_width + j] = (uint8_t)medium_value;
				}
			}
		}
	}

	// Calculate medium value of input descriptor.
	int32_t pat_med = 0;
	for (int32_t i = 0; i < descriptor_height; ++i)
		for (int32_t j = 0; j < descriptor_width; ++j)
			pat_med += (int32_t)descriptor[i * descriptor_width + j];
	pat_med = pat_med / (descriptor_width * descriptor_height);

	// Calculate znam value of input descriptor.
	int32_t pat_znam = 0;
	for (int32_t i = 0; i < descriptor_height; ++i)
		for (int32_t j = 0; j < descriptor_width; ++j)
			pat_znam += ((int32_t)descriptor[i * descriptor_width + j] - pat_med) * ((int32_t)descriptor[i * descriptor_width + j] - pat_med);
	pat_znam = (int32_t)sqrt((double)pat_znam);

	// Find appropriate frame.
	int32_t frame_ID = 0;
	double max_corr = 0.0f;
	for (int32_t n = 0; n < trackerData.frameBufferSize; ++n)
	{
		// Calculate medium value of frame descriptor.
		int32_t part_med = 0;
		for (int32_t i = 0; i < descriptor_height; ++i)
			for (int32_t j = 0; j < descriptor_width; ++j)
				part_med += (int32_t)descriptors[n][i * descriptor_width + j];
		part_med = part_med / (descriptor_width * descriptor_height);

		// Calculate znam value of frame descriptor.
		int32_t part_znam = 0;
		for (int32_t i = 0; i < descriptor_height; ++i)
			for (int32_t j = 0; j < descriptor_width; ++j)
				part_znam += ((int32_t)descriptors[n][i * descriptor_width + j] - part_med) * ((int32_t)descriptors[n][i * descriptor_width + j] - part_med);
		part_znam = (int32_t)sqrt((double)part_znam);

		// Calculate chisl value.
		int32_t chisl = 0;
		for (int32_t i = 0; i < descriptor_height; ++i)
			for (int32_t j = 0; j < descriptor_width; ++j)
				chisl += ((int32_t)descriptor[i * descriptor_width + j] - pat_med) * ((int32_t)descriptors[n][i * descriptor_width + j] - part_med);

		// Calculate correlation value.
		float corr_k = 0.0f;
		if (pat_znam > 0 && part_znam > 0)
			corr_k = (float)chisl / (float)(pat_znam * part_znam);

		// Compare.
		if (corr_k > max_corr)
		{
			max_corr = corr_k;
			frame_ID = n;
		}
	}

	// Free memory.
	for (int32_t i = 0; i < trackerData.frameBufferSize; ++i)
		delete[] descriptors[i];
	delete[] descriptors;

	// Check correlation value.
	if (max_corr < MIN_CORR_VALUE)
		return -1;

	return frame_ID;
}
