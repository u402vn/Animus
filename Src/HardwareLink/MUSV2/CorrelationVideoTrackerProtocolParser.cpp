#include "CorrelationVideoTrackerProtocolParser.h"
#include "CorrelationVideoTrackerVersion.h"
#include <memory.h>


cr::vtracker::CorrelationVideoTrackerProtocolParser::CorrelationVideoTrackerProtocolParser()
{
    memset(&m_inputTrackerData, 0, sizeof(vtracker::CorrelationVideoTrackerResultData));
    m_inputCommand = vtracker::CorrelationVideoTrackerCommand::RESET;
    m_inputArg1 = 0;
    m_inputArg2 = 0;
    m_inputArg3 = 0;
    m_inputProperty = vtracker::CorrelationVideoTrackerProperty::NUM_THREADS;
    m_inputPropertyValue = 0;
}


cr::vtracker::CorrelationVideoTrackerProtocolParser::~CorrelationVideoTrackerProtocolParser()
{
	// Nothing.
}


bool cr::vtracker::CorrelationVideoTrackerProtocolParser::encodeSetPropertyCommand(
	uint8_t* commandData,
	const uint32_t commandBufferSize,
	uint32_t& commandDataSize,
	const vtracker::CorrelationVideoTrackerProperty propertyID,
	const double propertyValue)
{
	// Check command buffer size.
	if (commandBufferSize < 15)
		return false;

	// Encode command.
	commandData[0] = 1;
	commandData[1] = CORRELATION_VIDEO_TRACKER_MAJOR_VERSION;
	commandData[2] = CORRELATION_VIDEO_TRACKER_MINOR_VERSION;
	int32_t intPropertyID = (int32_t)propertyID;
	memcpy(&commandData[3], &intPropertyID, 4);
	memcpy(&commandData[7], &propertyValue, 8);

	commandDataSize = 15;

	return true;
}


bool cr::vtracker::CorrelationVideoTrackerProtocolParser::encodeCommand(
	uint8_t* commandData,
	const uint32_t commandBufferSize,
	uint32_t& commandDataSize,
    const cr::vtracker::CorrelationVideoTrackerCommand commandID,
	const int32_t arg1,
	const int32_t arg2,
	const int32_t arg3,
	const uint8_t* frame_mono8,
	const int32_t frame_width,
	const int32_t frame_height)
{
	// Check command bufffer size.
	if (commandBufferSize < 19)
		return false;

	// Encode command.
	commandData[0] = 2;
	commandData[1] = CORRELATION_VIDEO_TRACKER_MAJOR_VERSION;
	commandData[2] = CORRELATION_VIDEO_TRACKER_MINOR_VERSION;
	int32_t intCommandID = (int32_t)commandID;
	memcpy(&commandData[3], &intCommandID, 4);
	memcpy(&commandData[7], &arg1, 4);
	memcpy(&commandData[11], &arg2, 4);
	memcpy(&commandData[15], &arg3, 4);

	if (commandID == vtracker::CorrelationVideoTrackerCommand::CAPTURE)
	{
		if (frame_mono8 != nullptr && frame_width > 0 && frame_height > 0)
		{
			if (commandBufferSize < 275)
				return false;

			// Reset frame ID.
			int32_t frame_ID = -1;
			memcpy(&commandData[15], &frame_ID, 4);

			// Calculate frame descriptor.
			int32_t descriptor_width = CVT_MAXIMUM_TRACKING_RECTANGLE_WIDTH / 8;
			int32_t descriptor_height = CVT_MAXIMUM_TRACKING_RECTANGLE_HEIGHT / 8;
			uint8_t* frame_descriptor = new uint8_t[(size_t)descriptor_width * (size_t)descriptor_height];
			memset(frame_descriptor, 0, (size_t)descriptor_width * (size_t)descriptor_height);

			int32_t frame_x0 = arg1 - CVT_MAXIMUM_TRACKING_RECTANGLE_WIDTH / 2;
			int32_t frame_y0 = arg2 - CVT_MAXIMUM_TRACKING_RECTANGLE_HEIGHT / 2;

			for (int32_t i = 0; i < descriptor_height; ++i)
			{
				for (int32_t j = 0; j < descriptor_width; ++j)
				{
					if (frame_y0 + i * 8 < 0 ||
						frame_y0 + i * 8 + 8 > frame_height ||
						frame_x0 + j * 8 < 0 ||
						frame_x0 + j * 8 + 8 > frame_width)
					{
						frame_descriptor[i * descriptor_width + j] = 0;
					}
					else
					{
						uint32_t medium_value = 0;
						for (int32_t k = frame_y0 + i * 8; k < frame_y0 + i * 8 + 8; ++k)
							for (int32_t t = frame_x0 + j * 8; t < frame_x0 + j * 8 + 8; ++t)
								medium_value += frame_mono8[k * frame_width + t];
						medium_value = medium_value / 64;

						frame_descriptor[i * descriptor_width + j] = (uint8_t)medium_value;
					}
				}
			}

			memcpy(&commandData[19], frame_descriptor, (size_t)descriptor_width * (size_t)descriptor_height);

			delete[] frame_descriptor;

			commandDataSize = 275;
			return true;
		}
	}	

	commandDataSize = 19;

	return true;
}


bool cr::vtracker::CorrelationVideoTrackerProtocolParser::encodeTrackerResultData(
	uint8_t* resultData,
	const uint32_t resultBufferSize,
	uint32_t& resultDataSize,
    cr::vtracker::CorrelationVideoTrackerResultData& trackerData,
    cr::vtracker::CorrelationVideoTrackerResultFieldsMask* fieldsMask)
{
	// Check result data buffer size.
	if (resultBufferSize < 145)
		return false;

	// Fill header data.
	resultData[0] = 0;	// Header of packet
	resultData[1] = CORRELATION_VIDEO_TRACKER_MAJOR_VERSION;
	resultData[2] = CORRELATION_VIDEO_TRACKER_MINOR_VERSION;

	// Fill packet.
	size_t pos = 8;
	if (fieldsMask == nullptr)
	{
		resultData[3] = 0xFF;
		resultData[4] = 0xFF;
		resultData[5] = 0xFF;
		resultData[6] = 0xFF;
		resultData[7] = 0xFF;
		
		memcpy(&resultData[pos], &trackerData.trackingRectangleCenterX, 4);
		pos += 4;
		memcpy(&resultData[pos], &trackerData.trackingRectangleCenterY, 4);
		pos += 4;
		memcpy(&resultData[pos], &trackerData.trackingRectangleWidth, 4);
		pos += 4;
		memcpy(&resultData[pos], &trackerData.trackingRectangleHeight, 4);
		pos += 4;
		memcpy(&resultData[pos], &trackerData.objectCenterX, 4);
		pos += 4;
		memcpy(&resultData[pos], &trackerData.objectCenterY, 4);
		pos += 4;
		memcpy(&resultData[pos], &trackerData.objectWidth, 4);
		pos += 4;
		memcpy(&resultData[pos], &trackerData.objectHeight, 4);
		pos += 4;
		memcpy(&resultData[pos], &trackerData.frameCounterInLostMode, 4);
		pos += 4;
		memcpy(&resultData[pos], &trackerData.frameCounter, 4);
		pos += 4;
		memcpy(&resultData[pos], &trackerData.frameWidth, 4);
		pos += 4;
		memcpy(&resultData[pos], &trackerData.frameHeight, 4);
		pos += 4;
		memcpy(&resultData[pos], &trackerData.correlationSurfaceWidth, 4);
		pos += 4;
		memcpy(&resultData[pos], &trackerData.correlationSurfaceHeight, 4);
		pos += 4;
		memcpy(&resultData[pos], &trackerData.pixelDeviationThreshold, 4);
		pos += 4;
		memcpy(&resultData[pos], &trackerData.lostModeOption, 4);
		pos += 4;
		memcpy(&resultData[pos], &trackerData.frameBufferSize, 4);
		pos += 4;
		memcpy(&resultData[pos], &trackerData.maximumNumberOfFramesInLostMode, 4);
		pos += 4;
		memcpy(&resultData[pos], &trackerData.trackerFrameID, 4);
		pos += 4;
		memcpy(&resultData[pos], &trackerData.bufferFrameID, 4);
		pos += 4;
		memcpy(&resultData[pos], &trackerData.searchWindowCenterX, 4);
		pos += 4;
		memcpy(&resultData[pos], &trackerData.searchWindowCenterY, 4);
		pos += 4;
		memcpy(&resultData[pos], &trackerData.numThreads, 4);
		pos += 4;
		memcpy(&resultData[pos], &trackerData.trackingRectangleCenterFX, 4);
		pos += 4;
		memcpy(&resultData[pos], &trackerData.trackingRectangleCenterFY, 4);
		pos += 4;
		memcpy(&resultData[pos], &trackerData.horizontalObjectValocity, 4);
		pos += 4;
		memcpy(&resultData[pos], &trackerData.verticalObjectVelocity, 4);
		pos += 4;
		memcpy(&resultData[pos], &trackerData.objectLossThreshold, 4);
		pos += 4;
		memcpy(&resultData[pos], &trackerData.objectDetectionThreshold, 4);
		pos += 4;
		memcpy(&resultData[pos], &trackerData.probabilityAdaptiveThreshold, 4);
		pos += 4;
		memcpy(&resultData[pos], &trackerData.patternUpdateCoeff, 4);
		pos += 4;
		memcpy(&resultData[pos], &trackerData.velocityUpdateCoeff, 4);
		pos += 4;
		memcpy(&resultData[pos], &trackerData.probabilityUpdateCoeff, 4);
		pos += 4;
		memcpy(&resultData[pos], &trackerData.objectDetectionProbability, 4);
		pos += 4;
		resultData[pos] = trackerData.mode;
		pos += 1;
	}
	else
	{
		resultData[3] = 0;
		resultData[3] = resultData[3] | (fieldsMask->trackingRectangleCenterX == true ? (uint8_t)128 : (uint8_t)0);
		resultData[3] = resultData[3] | (fieldsMask->trackingRectangleCenterY == true ? (uint8_t)64 : (uint8_t)0);
		resultData[3] = resultData[3] | (fieldsMask->trackingRectangleWidth == true ? (uint8_t)32 : (uint8_t)0);
		resultData[3] = resultData[3] | (fieldsMask->trackingRectangleHeight == true ? (uint8_t)16 : (uint8_t)0);
		resultData[3] = resultData[3] | (fieldsMask->objectCenterX == true ? (uint8_t)8 : (uint8_t)0);
		resultData[3] = resultData[3] | (fieldsMask->objectCenterY == true ? (uint8_t)4 : (uint8_t)0);
		resultData[3] = resultData[3] | (fieldsMask->objectWidth == true ? (uint8_t)2 : (uint8_t)0);
		resultData[3] = resultData[3] | (fieldsMask->objectHeight == true ? (uint8_t)1 : (uint8_t)0);

		resultData[4] = 0;
		resultData[4] = resultData[4] | (fieldsMask->frameCounterInLostMode == true ? (uint8_t)128 : (uint8_t)0);
		resultData[4] = resultData[4] | (fieldsMask->frameCounter == true ? (uint8_t)64 : (uint8_t)0);
		resultData[4] = resultData[4] | (fieldsMask->frameWidth == true ? (uint8_t)32 : (uint8_t)0);
		resultData[4] = resultData[4] | (fieldsMask->frameHeight == true ? (uint8_t)16 : (uint8_t)0);
		resultData[4] = resultData[4] | (fieldsMask->correlationSurfaceWidth == true ? (uint8_t)8 : (uint8_t)0);
		resultData[4] = resultData[4] | (fieldsMask->correlationSurfaceHeight == true ? (uint8_t)4 : (uint8_t)0);
		resultData[4] = resultData[4] | (fieldsMask->pixelDeviationThreshold == true ? (uint8_t)2 : (uint8_t)0);
		resultData[4] = resultData[4] | (fieldsMask->lostModeOption == true ? (uint8_t)1 : (uint8_t)0);

		resultData[5] = 0;
		resultData[5] = resultData[5] | (fieldsMask->frameBufferSize == true ? (uint8_t)128 : (uint8_t)0);
		resultData[5] = resultData[5] | (fieldsMask->maximumNumberOfFramesInLostMode == true ? (uint8_t)64 : (uint8_t)0);
		resultData[5] = resultData[5] | (fieldsMask->trackerFrameID == true ? (uint8_t)32 : (uint8_t)0);
		resultData[5] = resultData[5] | (fieldsMask->bufferFrameID == true ? (uint8_t)16 : (uint8_t)0);
		resultData[5] = resultData[5] | (fieldsMask->searchWindowCenterX == true ? (uint8_t)8 : (uint8_t)0);
		resultData[5] = resultData[5] | (fieldsMask->searchWindowCenterY == true ? (uint8_t)4 : (uint8_t)0);
		resultData[5] = resultData[5] | (fieldsMask->numThreads == true ? (uint8_t)2 : (uint8_t)0);
		resultData[5] = resultData[5] | (fieldsMask->trackingRectangleCenterFX == true ? (uint8_t)1 : (uint8_t)0);

		resultData[6] = 0;
		resultData[6] = resultData[6] | (fieldsMask->trackingRectangleCenterFY == true ? (uint8_t)128 : (uint8_t)0);
		resultData[6] = resultData[6] | (fieldsMask->horizontalObjectValocity == true ? (uint8_t)64 : (uint8_t)0);
		resultData[6] = resultData[6] | (fieldsMask->verticalObjectVelocity == true ? (uint8_t)32 : (uint8_t)0);
		resultData[6] = resultData[6] | (fieldsMask->objectLossThreshold == true ? (uint8_t)16 : (uint8_t)0);
		resultData[6] = resultData[6] | (fieldsMask->objectDetectionThreshold == true ? (uint8_t)8 : (uint8_t)0);
		resultData[6] = resultData[6] | (fieldsMask->probabilityAdaptiveThreshold == true ? (uint8_t)4 : (uint8_t)0);
		resultData[6] = resultData[6] | (fieldsMask->patternUpdateCoeff == true ? (uint8_t)2 : (uint8_t)0);
		resultData[6] = resultData[6] | (fieldsMask->velocityUpdateCoeff == true ? (uint8_t)1 : (uint8_t)0);

		resultData[7] = 0;
		resultData[7] = resultData[7] | (fieldsMask->probabilityUpdateCoeff == true ? (uint8_t)128 : (uint8_t)0);
		resultData[7] = resultData[7] | (fieldsMask->objectDetectionProbability == true ? (uint8_t)64 : (uint8_t)0);
		resultData[7] = resultData[7] | (fieldsMask->mode == true ? (uint8_t)32 : (uint8_t)0);

		if (fieldsMask->trackingRectangleCenterX == true)
		{
			memcpy(&resultData[pos], &trackerData.trackingRectangleCenterX, 4);
			pos += 4;
		}
		if (fieldsMask->trackingRectangleCenterY == true)
		{
			memcpy(&resultData[pos], &trackerData.trackingRectangleCenterY, 4);
			pos += 4;
		}
		if (fieldsMask->trackingRectangleWidth == true)
		{
			memcpy(&resultData[pos], &trackerData.trackingRectangleWidth, 4);
			pos += 4;
		}
		if (fieldsMask->trackingRectangleHeight == true)
		{
			memcpy(&resultData[pos], &trackerData.trackingRectangleHeight, 4);
			pos += 4;
		}
		if (fieldsMask->objectCenterX == true)
		{
			memcpy(&resultData[pos], &trackerData.objectCenterX, 4);
			pos += 4;
		}
		if (fieldsMask->objectCenterY == true)
		{
			memcpy(&resultData[pos], &trackerData.objectCenterY, 4);
			pos += 4;
		}
		if (fieldsMask->objectWidth == true)
		{
			memcpy(&resultData[pos], &trackerData.objectWidth, 4);
			pos += 4;
		}
		if (fieldsMask->objectHeight == true)
		{
			memcpy(&resultData[pos], &trackerData.objectHeight, 4);
			pos += 4;
		}
		if (fieldsMask->frameCounterInLostMode == true)
		{
			memcpy(&resultData[pos], &trackerData.frameCounterInLostMode, 4);
			pos += 4;
		}
		if (fieldsMask->frameCounter == true)
		{
			memcpy(&resultData[pos], &trackerData.frameCounter, 4);
			pos += 4;
		}
		if (fieldsMask->frameWidth == true)
		{
			memcpy(&resultData[pos], &trackerData.frameWidth, 4);
			pos += 4;
		}
		if (fieldsMask->frameHeight == true)
		{
			memcpy(&resultData[pos], &trackerData.frameHeight, 4);
			pos += 4;
		}
		if (fieldsMask->correlationSurfaceWidth == true)
		{
			memcpy(&resultData[pos], &trackerData.correlationSurfaceWidth, 4);
			pos += 4;
		}
		if (fieldsMask->correlationSurfaceHeight == true)
		{
			memcpy(&resultData[pos], &trackerData.correlationSurfaceHeight, 4);
			pos += 4;
		}
		if (fieldsMask->pixelDeviationThreshold == true)
		{
			memcpy(&resultData[pos], &trackerData.pixelDeviationThreshold, 4);
			pos += 4;
		}
		if (fieldsMask->lostModeOption == true)
		{
			memcpy(&resultData[pos], &trackerData.lostModeOption, 4);
			pos += 4;
		}
		if (fieldsMask->frameBufferSize == true)
		{
			memcpy(&resultData[pos], &trackerData.frameBufferSize, 4);
			pos += 4;
		}
		if (fieldsMask->maximumNumberOfFramesInLostMode == true)
		{
			memcpy(&resultData[pos], &trackerData.maximumNumberOfFramesInLostMode, 4);
			pos += 4;
		}
		if (fieldsMask->trackerFrameID == true)
		{
			memcpy(&resultData[pos], &trackerData.trackerFrameID, 4);
			pos += 4;
		}
		if (fieldsMask->bufferFrameID == true)
		{
			memcpy(&resultData[pos], &trackerData.bufferFrameID, 4);
			pos += 4;
		}
		if (fieldsMask->searchWindowCenterX == true)
		{
			memcpy(&resultData[pos], &trackerData.searchWindowCenterX, 4);
			pos += 4;
		}
		if (fieldsMask->searchWindowCenterY == true)
		{
			memcpy(&resultData[pos], &trackerData.searchWindowCenterY, 4);
			pos += 4;
		}
		if (fieldsMask->numThreads == true)
		{
			memcpy(&resultData[pos], &trackerData.numThreads, 4);
			pos += 4;
		}
		if (fieldsMask->trackingRectangleCenterFX == true)
		{
			memcpy(&resultData[pos], &trackerData.trackingRectangleCenterFX, 4);
			pos += 4;
		}
		if (fieldsMask->trackingRectangleCenterFY == true)
		{
			memcpy(&resultData[pos], &trackerData.trackingRectangleCenterFY, 4);
			pos += 4;
		}
		if (fieldsMask->horizontalObjectValocity == true)
		{
			memcpy(&resultData[pos], &trackerData.horizontalObjectValocity, 4);
			pos += 4;
		}
		if (fieldsMask->verticalObjectVelocity == true)
		{
			memcpy(&resultData[pos], &trackerData.verticalObjectVelocity, 4);
			pos += 4;
		}
		if (fieldsMask->objectLossThreshold == true)
		{
			memcpy(&resultData[pos], &trackerData.objectLossThreshold, 4);
			pos += 4;
		}
		if (fieldsMask->objectDetectionThreshold == true)
		{
			memcpy(&resultData[pos], &trackerData.objectDetectionThreshold, 4);
			pos += 4;
		}
		if (fieldsMask->probabilityAdaptiveThreshold == true)
		{
			memcpy(&resultData[pos], &trackerData.probabilityAdaptiveThreshold, 4);
			pos += 4;
		}
		if (fieldsMask->patternUpdateCoeff == true)
		{
			memcpy(&resultData[pos], &trackerData.patternUpdateCoeff, 4);
			pos += 4;
		}
		if (fieldsMask->velocityUpdateCoeff == true)
		{
			memcpy(&resultData[pos], &trackerData.velocityUpdateCoeff, 4);
			pos += 4;
		}
		if (fieldsMask->probabilityUpdateCoeff == true)
		{
			memcpy(&resultData[pos], &trackerData.probabilityUpdateCoeff, 4);
			pos += 4;
		}
		if (fieldsMask->objectDetectionProbability == true)
		{
			memcpy(&resultData[pos], &trackerData.objectDetectionProbability, 4);
			pos += 4;
		}
		if (fieldsMask->mode == true)
		{
			resultData[pos] = trackerData.mode;
			pos += 1;
		}
	}

	resultDataSize = (uint32_t)pos;

	return true;
}


int32_t cr::vtracker::CorrelationVideoTrackerProtocolParser::decodeInputPacket(
	const uint8_t* packetData,
	const uint32_t packetSize)
{
	// Check packet size.
	if (packetSize < 7)
		return -1;

	// Check protocol version.
	if (packetData[1] != CORRELATION_VIDEO_TRACKER_MAJOR_VERSION ||
		packetData[2] != CORRELATION_VIDEO_TRACKER_MINOR_VERSION)
		return -2;

	// Check data header.
	switch (packetData[0])
	{
	case 0:
        return decodeTrackerData(packetData, packetSize);

	case 1:
        return decodeSetPropetyCommand(packetData, packetSize);

	case 2:
        return decodeTrackerCommand(packetData, packetSize);

	default:
		return -1;
	}

	return -1;
}


cr::vtracker::CorrelationVideoTrackerResultData cr::vtracker::CorrelationVideoTrackerProtocolParser::getInputTrackerResultData()
{
    return m_inputTrackerData;
}


void cr::vtracker::CorrelationVideoTrackerProtocolParser::getInputProperty(vtracker::CorrelationVideoTrackerProperty& propertyID, double& propertyValue)
{
    propertyID = m_inputProperty;
    propertyValue = m_inputPropertyValue;
}


void cr::vtracker::CorrelationVideoTrackerProtocolParser::getInputCommand(vtracker::CorrelationVideoTrackerCommand& commandID, int32_t& arg1, int32_t& arg2, int32_t& arg3, uint8_t* frame_descriptor)
{
    commandID = m_inputCommand;
    arg1 = m_inputArg1;
    arg2 = m_inputArg2;
    arg3 = m_inputArg3;

	if (frame_descriptor != nullptr)
        memcpy(frame_descriptor, m_inputFrameDescriptor, 256);
}


int32_t cr::vtracker::CorrelationVideoTrackerProtocolParser::decodeSetPropetyCommand(const uint8_t* data, const uint32_t dataSize)
{
	// Check data size.
	if (dataSize != 15)
		return -1;

	// Decode command.
    memcpy(&m_inputProperty, &data[3], 4);
    memcpy(&m_inputPropertyValue, &data[7], 8);

	return 1;
}


int32_t cr::vtracker::CorrelationVideoTrackerProtocolParser::decodeTrackerCommand(const uint8_t* data, const uint32_t dataSize)
{
	// Check data size.
	if (dataSize == 19)
	{
		// Decode command.
        memcpy(&m_inputCommand, &data[3], 4);
        memcpy(&m_inputArg1, &data[7], 4);
        memcpy(&m_inputArg2, &data[11], 4);
        memcpy(&m_inputArg3, &data[15], 4);

		return 2;
	}
	else
	{
		if (dataSize == 275)
		{
			// Decode command.
            memcpy(&m_inputCommand, &data[3], 4);
            memcpy(&m_inputArg1, &data[7], 4);
            memcpy(&m_inputArg2, &data[11], 4);
            memcpy(&m_inputArg3, &data[15], 4);

			// Copy frame descriptor.
            memcpy(m_inputFrameDescriptor, &data[19], 256);

			return 2;
		}
	}

	return -1;
}


int32_t cr::vtracker::CorrelationVideoTrackerProtocolParser::decodeTrackerData(const uint8_t* data, const uint32_t dataSize)
{
	// Check data size.
	if (dataSize > 145 || dataSize < 8)
		return -1;

	// Decode tracker data.
	size_t pos = 8;
	if ((data[3] & (uint8_t)128) == (uint8_t)128)
	{
        memcpy(&m_inputTrackerData.trackingRectangleCenterX, &data[pos], 4);
		pos += 4;
	}
	else
	{
        m_inputTrackerData.trackingRectangleCenterX = -1;
	}
	if ((data[3] & (uint8_t)64) == (uint8_t)64)
	{
        memcpy(&m_inputTrackerData.trackingRectangleCenterY, &data[pos], 4);
		pos += 4;
	}
	else
	{
        m_inputTrackerData.trackingRectangleCenterY = -1;
	}
	if ((data[3] & (uint8_t)32) == (uint8_t)32)
	{
        memcpy(&m_inputTrackerData.trackingRectangleWidth, &data[pos], 4);
		pos += 4;
	}
	else
	{
        m_inputTrackerData.trackingRectangleWidth = -1;
	}
	if ((data[3] & (uint8_t)16) == (uint8_t)16)
	{
        memcpy(&m_inputTrackerData.trackingRectangleHeight, &data[pos], 4);
		pos += 4;
	}
	else
	{
        m_inputTrackerData.trackingRectangleHeight = -1;
	}
	if ((data[3] & (uint8_t)8) == (uint8_t)8)
	{
        memcpy(&m_inputTrackerData.objectCenterX, &data[pos], 4);
		pos += 4;
	}
	else
	{
        m_inputTrackerData.objectCenterX = -1;
	}
	if ((data[3] & (uint8_t)4) == (uint8_t)4)
	{
        memcpy(&m_inputTrackerData.objectCenterY, &data[pos], 4);
		pos += 4;
	}
	else
	{
        m_inputTrackerData.objectCenterY = -1;
	}
	if ((data[3] & (uint8_t)2) == (uint8_t)2)
	{
        memcpy(&m_inputTrackerData.objectWidth, &data[pos], 4);
		pos += 4;
	}
	else
	{
        m_inputTrackerData.objectWidth = -1;
	}
	if ((data[3] & (uint8_t)1) == (uint8_t)1)
	{
        memcpy(&m_inputTrackerData.objectHeight, &data[pos], 4);
		pos += 4;
	}
	else
	{
        m_inputTrackerData.objectHeight = -1;
	}
	if ((data[4] & (uint8_t)128) == (uint8_t)128)
	{
        memcpy(&m_inputTrackerData.frameCounterInLostMode, &data[pos], 4);
		pos += 4;
	}
	else
	{
        m_inputTrackerData.frameCounterInLostMode = -1;
	}
	if ((data[4] & (uint8_t)64) == (uint8_t)64)
	{
        memcpy(&m_inputTrackerData.frameCounter, &data[pos], 4);
		pos += 4;
	}
	else
	{
        m_inputTrackerData.frameCounter = -1;
	}
	if ((data[4] & (uint8_t)32) == (uint8_t)32)
	{
        memcpy(&m_inputTrackerData.frameWidth, &data[pos], 4);
		pos += 4;
	}
	else
	{
        m_inputTrackerData.frameWidth = -1;
	}
	if ((data[4] & (uint8_t)16) == (uint8_t)16)
	{
        memcpy(&m_inputTrackerData.frameHeight, &data[pos], 4);
		pos += 4;
	}
	else
	{
        m_inputTrackerData.frameHeight = -1;
	}
	if ((data[4] & (uint8_t)8) == (uint8_t)8)
	{
        memcpy(&m_inputTrackerData.correlationSurfaceWidth, &data[pos], 4);
		pos += 4;
	}
	else
	{
        m_inputTrackerData.correlationSurfaceWidth = -1;
	}
	if ((data[4] & (uint8_t)4) == (uint8_t)4)
	{
        memcpy(&m_inputTrackerData.correlationSurfaceHeight, &data[pos], 4);
		pos += 4;
	}
	else
	{
        m_inputTrackerData.correlationSurfaceHeight = -1;
	}
	if ((data[4] & (uint8_t)2) == (uint8_t)2)
	{
        memcpy(&m_inputTrackerData.pixelDeviationThreshold, &data[pos], 4);
		pos += 4;
	}
	else
	{
        m_inputTrackerData.pixelDeviationThreshold = -1;
	}
	if ((data[4] & (uint8_t)1) == (uint8_t)1)
	{
        memcpy(&m_inputTrackerData.lostModeOption, &data[pos], 4);
		pos += 4;
	}
	else
	{
        m_inputTrackerData.lostModeOption = -1;
	}
	if ((data[5] & (uint8_t)128) == (uint8_t)128)
	{
        memcpy(&m_inputTrackerData.frameBufferSize, &data[pos], 4);
		pos += 4;
	}
	else
	{
        m_inputTrackerData.frameBufferSize = -1;
	}
	if ((data[5] & (uint8_t)64) == (uint8_t)64)
	{
        memcpy(&m_inputTrackerData.maximumNumberOfFramesInLostMode, &data[pos], 4);
		pos += 4;
	}
	else
	{
        m_inputTrackerData.maximumNumberOfFramesInLostMode = -1;
	}
	if ((data[5] & (uint8_t)32) == (uint8_t)32)
	{
        memcpy(&m_inputTrackerData.trackerFrameID, &data[pos], 4);
		pos += 4;
	}
	else
	{
        m_inputTrackerData.trackerFrameID = -1;
	}
	if ((data[5] & (uint8_t)16) == (uint8_t)16)
	{
        memcpy(&m_inputTrackerData.bufferFrameID, &data[pos], 4);
		pos += 4;
	}
	else
	{
        m_inputTrackerData.bufferFrameID = -1;
	}
	if ((data[5] & (uint8_t)8) == (uint8_t)8)
	{
        memcpy(&m_inputTrackerData.searchWindowCenterX, &data[pos], 4);
		pos += 4;
	}
	else
	{
        m_inputTrackerData.searchWindowCenterX = -1;
	}
	if ((data[5] & (uint8_t)4) == (uint8_t)4)
	{
        memcpy(&m_inputTrackerData.searchWindowCenterY, &data[pos], 4);
		pos += 4;
	}
	else
	{
        m_inputTrackerData.searchWindowCenterY = -1;
	}
	if ((data[5] & (uint8_t)2) == (uint8_t)2)
	{
        memcpy(&m_inputTrackerData.numThreads, &data[pos], 4);
		pos += 4;
	}
	else
	{
        m_inputTrackerData.numThreads = -1;
	}
	if ((data[5] & (uint8_t)1) == (uint8_t)1)
	{
        memcpy(&m_inputTrackerData.trackingRectangleCenterFX, &data[pos], 4);
		pos += 4;
	}
	else
	{
        m_inputTrackerData.trackingRectangleCenterFX = -1;
	}
	if ((data[6] & (uint8_t)128) == (uint8_t)128)
	{
        memcpy(&m_inputTrackerData.trackingRectangleCenterFY, &data[pos], 4);
		pos += 4;
	}
	else
	{
        m_inputTrackerData.trackingRectangleCenterFY = -1;
	}
	if ((data[6] & (uint8_t)64) == (uint8_t)64)
	{
        memcpy(&m_inputTrackerData.horizontalObjectValocity, &data[pos], 4);
		pos += 4;
	}
	else
	{
        m_inputTrackerData.horizontalObjectValocity = -1;
	}
	if ((data[6] & (uint8_t)32) == (uint8_t)32)
	{
        memcpy(&m_inputTrackerData.verticalObjectVelocity, &data[pos], 4);
		pos += 4;
	}
	else
	{
        m_inputTrackerData.verticalObjectVelocity = -1;
	}
	if ((data[6] & (uint8_t)16) == (uint8_t)16)
	{
        memcpy(&m_inputTrackerData.objectLossThreshold, &data[pos], 4);
		pos += 4;
	}
	else
	{
        m_inputTrackerData.objectLossThreshold = -1;
	}
	if ((data[6] & (uint8_t)8) == (uint8_t)8)
	{
        memcpy(&m_inputTrackerData.objectDetectionThreshold, &data[pos], 4);
		pos += 4;
	}
	else
	{
        m_inputTrackerData.objectDetectionThreshold = -1;
	}
	if ((data[6] & (uint8_t)4) == (uint8_t)4)
	{
        memcpy(&m_inputTrackerData.probabilityAdaptiveThreshold, &data[pos], 4);
		pos += 4;
	}
	else
	{
        m_inputTrackerData.probabilityAdaptiveThreshold = -1;
	}
	if ((data[6] & (uint8_t)2) == (uint8_t)2)
	{
        memcpy(&m_inputTrackerData.patternUpdateCoeff, &data[pos], 4);
		pos += 4;
	}
	else
	{
        m_inputTrackerData.patternUpdateCoeff = -1;
	}
	if ((data[6] & (uint8_t)1) == (uint8_t)1)
	{
        memcpy(&m_inputTrackerData.velocityUpdateCoeff, &data[pos], 4);
		pos += 4;
	}
	else
	{
        m_inputTrackerData.velocityUpdateCoeff = -1;
	}
	if ((data[7] & (uint8_t)128) == (uint8_t)128)
	{
        memcpy(&m_inputTrackerData.probabilityUpdateCoeff, &data[pos], 4);
		pos += 4;
	}
	else
	{
        m_inputTrackerData.probabilityUpdateCoeff = -1;
	}
	if ((data[7] & (uint8_t)64) == (uint8_t)64)
	{
        memcpy(&m_inputTrackerData.objectDetectionProbability, &data[pos], 4);
		pos += 4;
	}
	else
	{
        m_inputTrackerData.objectDetectionProbability = -1;
	}
	if ((data[7] & (uint8_t)32) == (uint8_t)32)
	{
        m_inputTrackerData.mode = data[pos];
		pos += 1;
	}
	else
	{
        m_inputTrackerData.mode = 255;
	}

	return 0;
}



std::string cr::vtracker::CorrelationVideoTrackerProtocolParser::getVersion()
{
    return (std::to_string(CORRELATION_VIDEO_TRACKER_MAJOR_VERSION) + "." +
            std::to_string(CORRELATION_VIDEO_TRACKER_MINOR_VERSION) + "." +
            std::to_string(CORRELATION_VIDEO_TRACKER_PATCH_VERSION));
}
