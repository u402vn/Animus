#pragma once

#include "VideoDataStructures.h"


namespace vsource
{

#define VIDEO_DATA_PROTOCOL_PARSER_MAJOR_VERSION 1	///< Major version of video data protocol parser.
#define VIDEO_DATA_PROTOCOL_PARSER_MINOR_VERSION 0	///< Minor version of video data protocol parser.

	
	/**
	 * @brief Class for encoding and decoding video data. 
	*/
	class VideoDataProtocolParser
	{
	public:

		/**
		 * @brief Class constructor.
		*/
		VideoDataProtocolParser();

		/**
		 * @brief Class destructor.
		*/
		~VideoDataProtocolParser();

		/**
		 * @brief Method to encode video frame data.
		 * @param frame Frame to encode.
		 * @param dataBuffer Pointer to output data buffer.
		 * @param dataBufferSize Size of data buffer.
		 * @param outputSize Size of encoded data.
		 * @return TRUE if data encoded or FALSE.
		*/
        bool encode(Frame& frame, uint8_t* dataBuffer, const uint32_t dataBufferSize, uint32_t& outputSize);

		/**
		 * @brief Method to decode frame data.
		 * @param frame Output frame.
		 * @param frameData Frame data to decode.
		 * @param frameDataSize Size of frame data.
		 * @return TRUE if frame data decoded or FALSE.
		*/
        bool decode(Frame& frame, const uint8_t* frameData, const uint32_t frameDataSize);

	};
}
