#pragma once
#include <vector>
#include <string>
#include <iostream>

#include "VideoCodec.h"

namespace IMSDK {
namespace Decode{ class Decoder; }
namespace Encode{ class Encoder; }
}

namespace vcodec
{

class VideoCodecIntelMedia : public VideoCodec
{
public:

    static std::string getVersion();
    /**
     * @brief Class constructor.
     */
    VideoCodecIntelMedia();

    /**
     * @brief Class destructor.
     */
    ~VideoCodecIntelMedia();

    /**
     * @brief Method to set property.
     * @param key name of property to set.
     * @param value Value of property to set.
     * @return TRUE if property set or FALSE.
     */
    bool setProperty(std::string key, double value);

    /**
     * @brief Method to get property value.
     * @param key name of property to get.
     * @param value Value of property to get.
     * @return TRUE if property set or FALSE.
     */
    bool getProperty(std::string key, double& value);

    /**
     * @brief Method to trascode frame.
     * @param input Input video frame data.
     * @param output Output video frame data.
     */
    bool transcode(vsource::Frame& input, vsource::Frame& output);

private:
    std::vector<std::string> m_decodeParams;
    std::vector<std::string> m_encodeParams;
    bool m_isDecodeChangeParams;
    bool m_isEncodeChangeParams;

    double m_bitRate;
    double m_fps;
    int m_gopSize;

    IMSDK::Decode::Decoder* m_decoder;
    IMSDK::Encode::Encoder* m_encoder;

    vsource::Frame m_inputFrame;
    vsource::Frame m_outputFrame;

};

}


