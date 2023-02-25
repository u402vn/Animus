#pragma once
#include <vector>
#include <string>
#include <iostream>

#include "VideoCodec.h"


namespace vcodec
{
    class VideoCodecGstreamer : public VideoCodec
    {
    public:

        static std::string getVersion();
        /**
         * @brief Class constructor.
        */
        VideoCodecGstreamer();

        /**
         * @brief Class destructor.
        */
        ~VideoCodecGstreamer();

        /**
         * @brief Method to set property.
         * @param property_name name of property to set.
         * @param property_value Value of property to set.
         * @return TRUE if property set or FALSE.
         */
        bool setProperty(std::string property_name, double property_value);

        /**
         * @brief Method to get property value.
         * @param property_name name of property to get.
         * @param property_value Value of property to set.
         * @return TRUE if property set or FALSE.
         */
        bool getProperty(std::string property_name, double& property_value);

        /**
         * @brief Method to trascode frame.
         * @param inFrame Input video frame data.
         * @param outFrame Output video frame data.
         */
        bool transcode(vsource::Frame& inFrame, vsource::Frame& outFrame);

    private:
        bool m_isDecodeChangeParams;
        bool m_isEncodeChangeParams;

        int m_bitRate;
        int m_FPS;
        int m_GOPsize;

        void* decoder;
        void* encoder;

        vsource::Frame m_inputFrame;
        vsource::Frame m_outputFrame;

    };

}

