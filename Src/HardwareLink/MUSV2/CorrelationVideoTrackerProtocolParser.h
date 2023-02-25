#ifndef CORRELATION_VIDEO_TRACKER_PROTOCOL_PARSER_H
#define CORRELATION_VIDEO_TRACKER_PROTOCOL_PARSER_H
#include <vector>
#include <string>
#include "CorrelationVideoTrackerDataStructures.h"


namespace cr
{
namespace vtracker
{

	/**
	 * @brief Class for encoding and decoding packets.
	*/
	class CorrelationVideoTrackerProtocolParser
	{
	public:

		/**
		 * @brief Class constructor.
		*/
		CorrelationVideoTrackerProtocolParser();

		/**
		 * @brief Class destructor.
		*/
		~CorrelationVideoTrackerProtocolParser();

		/**
		 * @brief Method to encode SET PROPERTY command.
		 * @param commandData Pointer to output data buffer.
		 * @param CommandBufferSize Size of buffer for command.
		 * @param commandDataSize Size of encoded command data.
		 * @param propertyID ID of property.
		 * @param propertyValue Property value.
		 * @return TRUE if command encoded or FALSE.
		*/
        bool encodeSetPropertyCommand(
			uint8_t* commandData,
			const uint32_t commandBufferSize,
			uint32_t& commandDataSize,
			const vtracker::CorrelationVideoTrackerProperty propertyID,
			const double propertyValue);

		/**
		 * @brief Method to encode Tracker command.
		 * @param commandData Pointer to output data buffer.
		 * @param commandBufferSize Size of buffer for command data.
		 * @param commandID ID of command.
		 * @param arg1 First argument.
		 * @param arg2 Second argument.
		 * @param arg3 Third argument.
         * @param frameMono8 Poiunter to frame in mono8 format (grayscale) if you want encode frame descriptor.
         * @param frameWidth Frame width.
         * @param frameHeight Frame height.
		 * @return TRUE if the command encoded or FALSE.
		*/
        bool encodeCommand(
			uint8_t* commandData,
			const uint32_t commandBufferSize,
			uint32_t& commandDataSize,
			const vtracker::CorrelationVideoTrackerCommand commandID,
			const int32_t arg1 = -1,
			const int32_t arg2 = -1,
			const int32_t arg3 = -1,
            const uint8_t* frameMono8 = nullptr,
            const int32_t frameWidth = 0,
            const int32_t frameHeight = 0);

		/**
		 * @brief Method to encode vodeo tracker resuld data.
		 * @param resultData Pointer to output data buffer.
		 * @param resultBufferSize Size of buffer for result data.
		 * @param resultDataSize Size of encoded data.
		 * @param trackerData Tracker data.
		 * @return TRUE if data encoded or FALSE.
		*/
        bool encodeTrackerResultData(
			uint8_t* resultData,
			const uint32_t resultBufferSize,
			uint32_t& resultDataSize,
			vtracker::CorrelationVideoTrackerResultData& trackerData,
			vtracker::CorrelationVideoTrackerResultFieldsMask* fieldsMask = nullptr);

		/**
		 * @brief Method to decode input packet.
		 * @param packetData Pointer to data for decoding.
		 * @param packetSize Size of packet.
		 * @return (-1) - error in input data, (-2) - not valid RfVideoTracker version, (0) - tracker data,
		           (1) - set property command, (2) - command.
		*/
        int32_t decodeInputPacket(
			const uint8_t* packetData,
			const uint32_t packetSize);

		/**
		 * @brief Method to get input tracker results data.
		 * @return Tracker result data structure.
		*/
        cr::vtracker::CorrelationVideoTrackerResultData getInputTrackerResultData();

		/**
		* @brief Method to get input property.
		* @param property ID of input property.
		* @param propertyValue Value of input property.
		*/
        void getInputProperty(
			vtracker::CorrelationVideoTrackerProperty& propertyID,
			double& propertyValue);

		/**
         * @brief Method to get input tracker command.
         * @param commandID Tracker command ID.
         * @param arg1 First argument.
         * @param arg2 Second argument.
         * @param arg3 Third argument.
         * @param arg4 Pointer to frame descriptor.
		*/
        void getInputCommand(
            cr::vtracker::CorrelationVideoTrackerCommand& commandID,
			int32_t& arg1,
			int32_t& arg2,
			int32_t& arg3,
            uint8_t* frameDescriptor = nullptr);

        /**
         * @brief Method to get version string.
         * @return String of version.
        */
        static std::string getVersion();

	private:

        vtracker::CorrelationVideoTrackerCommand m_inputCommand;
        int32_t m_inputArg1;
        int32_t m_inputArg2;
        int32_t m_inputArg3;
        uint8_t m_inputFrameDescriptor[256];
        CorrelationVideoTrackerProperty m_inputProperty;
        double m_inputPropertyValue;
        CorrelationVideoTrackerResultData m_inputTrackerData;

        int32_t decodeSetPropetyCommand(const uint8_t* data, const uint32_t dataSize);

        int32_t decodeTrackerData(const uint8_t* data, const uint32_t dataSize);

        int32_t decodeTrackerCommand(const uint8_t* data, const uint32_t dataSize);
	};
}
}
#endif // CORRELATION_VIDEO_TRACKER_PROTOCOL_PARSER_H

