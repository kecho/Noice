#include "Image.h"

namespace noice
{
    void Image::init(int width, int height, int depth)
    {
        m_support.resize(width*height*depth);
        m_img.width = width;
        m_img.height = height;
        m_img.depth = depth;
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
