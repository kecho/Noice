#include "Image.h"

namespace noice
{
    void Image::init(int width, int height, int depth, int channels)
    {
        m_support.resize(width*height*depth*channels);
        m_img.width = width;
        m_img.height = height;
        m_img.depth = depth;
        m_img.channels = channels;
        m_img.dimensions.v[0] = width;
        m_img.dimensions.v[1] = height;
        m_img.dimensions.v[2] = depth;
        for (int i = 0; i < 3; ++i)
        {
            m_img.texelSize.v[i] = 1.0f / m_img.dimensions.v[i];
            m_img.halfTexelSize.v[i] = 0.5f * m_img.texelSize.v[i];
        }

        m_img.data = m_support.data();
    }

    void Image::clear(float value)
    {
        for (float& f : m_support)
            f = value;
    }

    void Image::startStopwatch()
    {
        if (!m_stopwatchObject)
            return;
        m_timeStart = std::chrono::high_resolution_clock::now();
    }

    void Image::endStopwatch()
    {
        if (!m_stopwatchObject)
            return;
        
        m_stopwatchObject->microseconds = (unsigned int)std::chrono::duration_cast<std::chrono::microseconds>
            (std::chrono::high_resolution_clock::now() - m_timeStart).count();
    }

}
