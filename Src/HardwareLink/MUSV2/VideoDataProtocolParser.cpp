#include "VideoDataProtocolParser.h"

#define FRAME_DATA_OVERHEAD_SIZE 63	///< Size of frame overhead + 3 service bytes


vsource::VideoDataProtocolParser::VideoDataProtocolParser()
{

}


vsource::VideoDataProtocolParser::~VideoDataProtocolParser()
{

}


bool vsource::VideoDataProtocolParser::encode(Frame& frame, uint8_t* dataBuffer, const uint32_t dataBufferSize, uint32_t& outputSize)
{
	// Check frame data.
	if (frame.size == 0)
		return false;

	// Check data buffer size.
	if (dataBufferSize < FRAME_DATA_OVERHEAD_SIZE + frame.size)
		return false;

	outputSize = FRAME_DATA_OVERHEAD_SIZE + frame.size;

	// Encode data.
	dataBuffer[0] = 0;
	dataBuffer[1] = VIDEO_DATA_PROTOCOL_PARSER_MAJOR_VERSION;
	dataBuffer[2] = VIDEO_DATA_PROTOCOL_PARSER_MINOR_VERSION;

	memcpy(&dataBuffer[3], &frame.width, 4);
	memcpy(&dataBuffer[7], &frame.height, 4);
	memcpy(&dataBuffer[11], &frame.size, 4);
	memcpy(&dataBuffer[15], &frame.fourcc, 4);
	memcpy(&dataBuffer[19], &frame.sourceID, 4);
	memcpy(&dataBuffer[23], &frame.frameID, 4);

	memcpy(&dataBuffer[27], &frame.transformMatrix[0][0], 4);
	memcpy(&dataBuffer[31], &frame.transformMatrix[0][1], 4);
	memcpy(&dataBuffer[35], &frame.transformMatrix[0][2], 4);
	memcpy(&dataBuffer[39], &frame.transformMatrix[1][0], 4);
	memcpy(&dataBuffer[43], &frame.transformMatrix[1][1], 4);
	memcpy(&dataBuffer[47], &frame.transformMatrix[1][2], 4);
	memcpy(&dataBuffer[51], &frame.transformMatrix[2][0], 4);
	memcpy(&dataBuffer[55], &frame.transformMatrix[2][1], 4);
	memcpy(&dataBuffer[59], &frame.transformMatrix[2][2], 4);

	memcpy(&dataBuffer[63], frame.data, frame.size);

	return true;
}


bool vsource::VideoDataProtocolParser::decode(Frame& frame, const uint8_t* frameData, const uint32_t frameDataSize)
{
	// Check frame data size.
	if (frameDataSize < FRAME_DATA_OVERHEAD_SIZE + 1)
		return false;

	// Check header version.
	if (frameData[0] != 0 ||
		frameData[1] != VIDEO_DATA_PROTOCOL_PARSER_MAJOR_VERSION ||
		frameData[2] != VIDEO_DATA_PROTOCOL_PARSER_MINOR_VERSION)
		return false;

	// Decode data.
	uint32_t width = 0;
	uint32_t height = 0;
	uint32_t size = 0;
	uint32_t fourcc = 0;
	uint32_t sourceID = 0;
	uint32_t frameID = 0;
	float transformMatrix[3][3] = { 0 };

	memcpy(&width, &frameData[3], 4);
	memcpy(&height, &frameData[7], 4);
	memcpy(&size, &frameData[11], 4);
	memcpy(&fourcc, &frameData[15], 4);
	memcpy(&sourceID, &frameData[19], 4);
	memcpy(&frameID, &frameData[23], 4);
			
	memcpy(&transformMatrix[0][0], &frameData[27], 4);
	memcpy(&transformMatrix[0][1], &frameData[31], 4);
	memcpy(&transformMatrix[0][2], &frameData[35], 4);
	memcpy(&transformMatrix[1][0], &frameData[39], 4);
	memcpy(&transformMatrix[1][1], &frameData[43], 4);
	memcpy(&transformMatrix[1][2], &frameData[47], 4);
	memcpy(&transformMatrix[2][0], &frameData[51], 4);
	memcpy(&transformMatrix[2][1], &frameData[55], 4);
	memcpy(&transformMatrix[2][2], &frameData[59], 4);

	// Checke frame size.
	if (width == 0 || height == 0 || size == 0)
		return false;

	// Check if the input data valid.
	switch (fourcc) {
	case (uint32_t)ValidFourccCodes::RGB24:
		if (size != width * height * 3)
			return false;
		if (frameDataSize != size + FRAME_DATA_OVERHEAD_SIZE)
			return false;
		break;
	case (uint32_t)ValidFourccCodes::BGR24:
		if (size != width * height * 3)
			return false;
		if (frameDataSize != size + FRAME_DATA_OVERHEAD_SIZE)
			return false;
		break;
	case (uint32_t)ValidFourccCodes::UYVY:
		if (size != width * height * 2)
			return false;
		if (frameDataSize != size + FRAME_DATA_OVERHEAD_SIZE)
			return false;
		break;
	case (uint32_t)ValidFourccCodes::YUY2:
		if (size != width * height * 2)
			return false;
		if (frameDataSize != size + FRAME_DATA_OVERHEAD_SIZE)
			return false;
		break;
	case (uint32_t)ValidFourccCodes::Y800:
		if (size != width * height)
			return false;
		if (frameDataSize != size + FRAME_DATA_OVERHEAD_SIZE)
			return false;
		break;
	case (uint32_t)ValidFourccCodes::NV12:
		if (size != width * (height + height / 2))
			return false;
		if (frameDataSize != size + FRAME_DATA_OVERHEAD_SIZE)
			return false;
		break;
	case (uint32_t)ValidFourccCodes::YUV1:
		if (size != width * height * 3)
			return false;
		if (frameDataSize != size + FRAME_DATA_OVERHEAD_SIZE)
			return false;
		break;
	case (uint32_t)ValidFourccCodes::JPEG:
		// We can't calculate size for JPEG.
		break;
	case (uint32_t)ValidFourccCodes::JPG2:
		// We can't calculate size for JPG2.
		break;
	case (uint32_t)ValidFourccCodes::H264:
		// We can't calculate size for H264.
		break;
	case (uint32_t)ValidFourccCodes::H265:
		// We can't calculate size for H265.
		break;
	default:
		// Incorrect FOURCC code.
		return false;
	}

	// Check output frame initialization
	if (frame.width != width ||
		frame.height != height ||
		frame.fourcc != fourcc)
	{
		// Release frame.
		frame.Release();
		// Create new frame.
		frame = Frame(width, height, fourcc);
	}

	// Copy frame data.
	memcpy(frame.data, &frameData[63], size);

	// Copy atributes.
	frame.frameID = frameID;
	frame.sourceID = sourceID;
	frame.size = size;
	frame.transformMatrix[0][0] = transformMatrix[0][0];
	frame.transformMatrix[0][1] = transformMatrix[0][1];
	frame.transformMatrix[0][2] = transformMatrix[0][2];
	frame.transformMatrix[1][0] = transformMatrix[1][0];
	frame.transformMatrix[1][1] = transformMatrix[1][1];
	frame.transformMatrix[1][2] = transformMatrix[1][2];
	frame.transformMatrix[2][0] = transformMatrix[2][0];
	frame.transformMatrix[2][1] = transformMatrix[2][1];
	frame.transformMatrix[2][2] = transformMatrix[2][2];

	return true;
}
