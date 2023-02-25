#pragma once
#include <string>
#include <iostream>

#include "VideoDataStructures.h"


namespace vcodec
{
/**
 * @brief Interface class of video codecs.
 */
class VideoCodec
{
public:
    /**
     * @brief Method to set property.
     * @param key name of property to set.
     * @param value Value of property to set.
     * @return TRUE if property set or FALSE.
     */
    virtual bool setProperty(std::string key, double value) = 0;

    /**
     * @brief Method to get property value.
     * @param key name of property to get.
     * @param value Value of property to get.
     * @return TRUE if property set or FALSE.
     */
    virtual bool getProperty(std::string key, double& value) = 0;

    /**
     * @brief Method to trascode frame.
     * @param input Input video frame data.
     * @param output Output video frame data.
     */
    virtual bool transcode(vsource::Frame& input, vsource::Frame& output) = 0;

};

}
