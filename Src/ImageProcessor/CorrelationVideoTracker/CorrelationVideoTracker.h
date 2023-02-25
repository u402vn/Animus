#pragma once
#include <mutex>
#include "CorrelationVideoTrackerDataStructures.h"

namespace vtracker
{
	/**
	 * @brief Class of correlation video tracker.
	*/
	class CorrelationVideoTracker
	{
	public:

		/**
		 * @brief Class constructor.
		*/
		CorrelationVideoTracker();

		/**
		 * @brief Class destructor.
		*/
		~CorrelationVideoTracker();

		/**
		 * @brief Method to set property.
		 * @param propertyID ID of property to set.
		 * @param propertyValue Value of property to set.
		 * @return TRUE if property set or FALSE.
		*/
		bool SetProperty(CorrelationVideoTrackerProperty propertyID, double propertyValue);

		/**
		 * @brief Method to get property value.
		 * @param propertyID ID of property to get.
		 * @return Value of property or -1.
		*/
		double GetProperty(CorrelationVideoTrackerProperty propertyID);

		/**
		 * @brief Method to process frame.
		 * @param frame_mono8 Pointer to frame data in mono8 (GRAYSCALE) format.
		 * @param width Width of frame.
		 * @param height Height of frame.
		 * @param timeoutMs Time (milliseconds) during which calculations must be performed.
		 * @return TRUE if at least one frame has been processed or FALSE.
		*/
		bool ProcessFrame(uint8_t *frame_mono8, int32_t width, int32_t height, const uint32_t timeoutMs = 0);

		/**
		 * @brief Method to perform command.
		 * @param commandID ID of command.
		 * @param arg1 First argument.
		 * @param arg2 Second argument.
		 * @param arg3 Third argument.
		 * @param arg4 Fourth argument.
		 * @param arg5 Fifth argument.
		 * @return TRUE if the command has been performed or FALSE.
		*/
		bool ExecuteCommand(
			CorrelationVideoTrackerCommand commandID,
			const int32_t arg1 = -1,
			const int32_t arg2 = -1,
			const int32_t arg3 = -1,
			const uint8_t* arg4 = nullptr,
			const uint8_t* arg5 = nullptr);

		/**
		 * @brief Method to get result data of video tracker.
		 * @return Current structure of result data.
		*/
		CorrelationVideoTrackerResultData GetTrackerResultData();

		/**
		 * @brief Method to get image of internal surfaces.
		 * @param imageType Type of image to get.
		 * @param image Pointer to image buffer. Must be 128x128 = 16384 bytes.
		 * @return TRUE if the image was filled or FALSE.
		*/
		bool GetImage(CorrelationVideoTrackerImageType imageType, uint8_t* image);

	private:

		uint8_t** frameBuffer;							/// Frame buffer.
		uint8_t* patternImage;							/// Patter image.
		int32_t* differencePattern;						/// Difference pattern image.
		uint8_t* reducedMask;							/// Reduced mask surface.
		uint8_t* fullMask;								/// Full mask surface.
		uint8_t** trackingRectangleImage;				/// Images under tracking rectangle.
		int32_t* differenceWindow;						/// Difference search window.
		uint32_t* doubledDifferenceWindow;				/// Doubled difference search window.
		float* deviationSurface;						/// Deviation surface.
		float* correlationSurface;						/// Correlation surface.
		int32_t* chislSurface;							/// Service surface.
		float Xcorrect;									/// Horizontal probable object position.
		float Ycorrect;									/// Vertical probable object position.
		float XCovariance;								/// Horizontal position covariance.					
		float YCovariance;								/// Vertical position covariance.
		float RX;										/// Horizontal deviation.
		float RY;										/// Vertical deviation.
		uint32_t trackingRectangleIndex;				/// Index of current tracking rectamgle image.
		CorrelationVideoTrackerResultData trackerData;	/// Result data structure of video tracker.
		std::mutex accessManageMutex;					/// Mutex to manage access to class methos from different threads.
		int32_t last_chisl_value;						/// Service variable.


		/**
		 * @brief Method to capture object.
		 * @param objectCenterX Horizontal position of object center.
		 * @param objectCenterY Vertical position of object center.
		 * @param frameID ID of frame to capture object. Use if it necessary.
		 * @param frameDescriptor Descriptor of object area to capture instead frame ID.
		 * @param objectMask Mask of object to better capture.
		 * @return TRUE if object captured of FALSE.
		*/
		bool Capture(
			const int32_t objectCenterX,
			const int32_t objectCenterY,
			const int32_t frameID = -1,
			const uint8_t* frameDescriptor = nullptr,
			const uint8_t* objectMask = nullptr);

		/**
		 * @brief Method to reset tracker.
		*/
		void Reset();

		/**
		 * @brief Method to check reset criteria.
		 * @return TRUE if algorithm shoukd be reset or FALSE.
		*/
		bool CheckResetCriteria();

		/**
		 * @brief Method to set INERTIAL mode.
		 * @return TRUE if INERTIAL mode set or FALSE.
		*/
		bool SetInertialMode();

		/**
		 * @brief Method to set LOST mode.
		 * @return TRUE if LOST mode set or FALSE.
		*/
		bool SetLostMode();

		/**
		 * @brief Method to set STATIC mode.
		 * @return TRUE if STATIC mode set or FALSE.
		*/
		bool SetStaticMode();

		/**
		 * @brief Method to check object presense on video frame before capture.
		 * @param frameData Pointer to frame data.
		 * @param objectCenterX Horizontal position of object center to check.
		 * @param objectCenterY Vertical position of object center to check.
		 * @return TRUE if some object is present under the capture rectangle or FALSE.
		*/
		bool CheckObjectPresence(uint8_t* frameData, const int32_t objectCenterX, const int32_t objectCenterY);

		/**
		 * @brief Method to set tracking rectangle size automatically.
		 * @return TRUE if size set automatically or FALSE.
		*/
		bool SetTrackingRectangleSizeAutomatically();

		/**
		 * @brief Method to move tracking rectangle.
		 * @param offsetX Horizontal offset.
		 * @param offestY Vertical offset.
		 * @return TRUE if tracking rectangle moved or FALSE.
		*/
		bool MoveTrackingRectangle(int32_t offsetX, int32_t offsetY);

		/**
		 * @brief Method to calculate object rectangle.
		*/
		void CalculateObjectRectangle();

		/**
		 * @brief Method to update coordinate filter.
		 * @param newMaxX New horizontal position of maxinun in correlation threshold.
		 * @param newMaxY New vertical position of maxinun in correlation threshold.
		*/
		void UpdateCooordinatesFilter(int32_t newMaxX, int32_t newMaxY);

		/**
		 * @brief Method to calculate correlation surface.
		*/
		inline void CalculateCorrelationSurface();

		/**
		 * @brief Method to calculate correlation surface.
		*/
		inline void CalculateCorrelationSurface_omp();

		/**
		 * @brief Method to analize correlation surface.
		*/
		inline void AnalizeCorrelationSurface();

		/**
		 * @brief Method to find frame_ID by descriptor.
		*/
		inline int32_t FindFrameID(
			const int32_t x,
			const int32_t y,
			const uint8_t* descriptor);

	};
}
