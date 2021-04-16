#pragma once
#include <noice/noice.h>
#include <Noice/DistanceKernel.ispc.h>
#include <vector>

namespace noice
{

class Image
{
public:
    void init(int width, int height, int depth);
    void clear(float value);
    ispc::Image& img() { return m_img; }
    const ispc::Image& img() const { return m_img; }
    int pixelCount() const { return (int)m_support.size(); }
    inline float& operator[](int index) { return m_support[index]; }
    inline float  operator[](int index) const { return m_support[index]; }

    TextureComponentHandle asHandle() 
    {
        return TextureComponentHandle { this };
    }

    static Image* get(const TextureComponentHandle& handle) 
    {
        return (Image*)handle.opaquePtr;
    }

    bool hasEventCb() const  { return m_eventCb != nullptr; }

    void attachEventCb(EventCallback cb, void* userData)
    {
        m_eventCb = cb;
        m_userData = userData;
    }

    void attachStopwatchObject(Stopwatch* stopwatch)
    {
        m_stopwatchObject = stopwatch;
    }

    void dispatchEvent()
    {
        EventArguments args = { m_userData };
        m_eventCb(args);
    }

private:
    //pixel support vector
    std::vector<float> m_support;
    ispc::Image m_img;

    //attachments
    EventCallback m_eventCb = nullptr;
    void* m_userData = nullptr;
    Stopwatch* m_stopwatchObject = nullptr;
};

}
