#pragma once
/**
\file Header file of video tracking class.
\brief The file includes a description of the class of
automatic tracking of objects in the video.

\authors Sergey Zaplatnikov (s.zaplatnikov@solidtech.systems), Alexey Shein (a.shein@solidtech.systems)
\date 10.92.2019
\version 3.1

SOLID Ltd.
*/
#include <string>
#include "RfVideoTrackerDataStructures.h"

namespace rf {

	class VideoTracker;

	/**
	\brief Class RfVideoTracker.
	\detailed The class is designed for automatic tracking of objects in the video.
	*/
	class RfVideoTracker {

	public:
		/**
		\brief Class constructor.
		*/
		RfVideoTracker();
		/**
		\brief Class destructor.
		*/
		~RfVideoTracker();
		/**
		\brief The method is intended to obtain the version number of the program library.
		\return String of version number.
		*/
		std::string GetVersion();
		/**
		\brief Method to set property value.
		\param[in] propertyID Property index.
		\param[in] propertyValue Property value.
		\return The method returns the TRUE in case of successful setting
		of the property. The method returns FALSE if it was not possible 
		to establish the property.
		*/
		bool SetProperty(RfVideoTrackerProperty propertyID, float propertyValue);
		/**
		\brief Method to get property value.
		\param[in] propertyID Property index.
		\return Method returns property value or returns -1.0 in case any errors.
		*/
		float GetProperty(RfVideoTrackerProperty propertyID);
		/**
		\brief Method adds frame to frame buffer fo processing.
		\param[in] frame Pointer to frame data with mono_8 pixel format.
		\return Method returns frame index in frame buffer or return -1 if
		frame buffer is not initialized.
		*/
		int32_t AddFrame(uint8_t *frame);
		/**
		\brief Method for executing command.
		\detailed The method executes the command that is specified in the parameters.
		The value of the transmitted parameters for each command is different:
		for CAPTURE command: arg1 - strobe x position, arg2 - strobe y position, arg3 - frame index, arg4 - capture mask.
		for RESET command: arg1 - not used, arg2 - not used, arg3 - not used, arg4 - not used.
		for SET_INERTIAL_MODE command: arg1 - not used, arg2 - not used, arg3 - not used, arg4 - not used.
		for SET_LOST_MODE command: arg1 - not used, arg2 - not used, arg3 - not used, arg4 - not used.
		for SET_STATIC_MODE command: arg1 - not used, arg2 - not used, arg3 - not used, arg4 - not used.
		for SET_STROBE_AUTO_SIZE command: arg1 - not used, arg2 - not used, arg3 - not used, arg4 - not used.
		for MOVE_STROBE command: arg1 - strobe offset x, arg2 - strobe offset y, arg3 - not used, arg4 - not used.
		for PROCESS_FRAME command: arg1 - maximum number of frames for processing,  arg2 - not used y, arg3 - not used, arg4 - not used.
		for MOVE_SEARCH_WINDOW command: arg1 - search window offset x,  arg2 - search window offset y,
		arg3 - 0: offset only fo 1 frame 1: offset permanently, arg4 - not used.
		\param[in] commandID Command index.
		\param[in] arg1 First parameter.
		\param[in] arg2 Second parameter.
		\param[in] arg3 Third parameter.
		\param[in] arg4 Fouth parameter.
		\return The method returns the TROE if the command is successful. the method returns
		FALSE in case any error.
		*/
		bool ExecuteCommand(RfVideoTrackerCommand commandID, int32_t arg1 = -1, int32_t arg2 = -1, int32_t arg3 = -1, uint8_t* arg4 = nullptr);
		/**
		\brief Method to get tracker data structure.
		\return Method returns tracker data structure.
		*/
		RfVideoTrackerData GetTrackerData();
		/**
		\brief Mathod get internal matrix (image).
		\detailed The method fills the array passed in by the pointer
		with the data specified in the matrix parameters.
		\param[in] imageID Idex of matrix (PATTERN_IMAGE, MASK_IMAGE, CORR_IMAGE).
		\param[in] img Pointer to buffer for filling by method.
		\return Method returns TRUE in case success or FALSE in case any errors.
		*/
		bool GetImage(RfImageType imageID, uint8_t *img);

	private:

		VideoTracker *tracker;

	};//class...

}//namespace...

