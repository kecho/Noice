#pragma once
#include <noice/noice.h>
#include <Noice/DistanceKernel.ispc.h>
#include <chrono>
#include <vector>

namespace noice
{

class Image
{
public:
    void init(int width, int height, int depth, int channels = 1);
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

    void attachEventCb(EventCallback cb, int eventSampleFrequency, void* userData)
    {
        m_eventSampleFrequency = eventSampleFrequency;
        m_eventCb = cb;
        m_userData = userData;
    }

    void attachStopwatchObject(Stopwatch* stopwatch)
    {
        m_stopwatchObject = stopwatch;
    }

    int getEventFrequency() const { return m_eventSampleFrequency; }
    EventCallback getEventCb() const { return m_eventCb; }
    void* getEventUserData() const { return m_userData; }
    Stopwatch* getStopwatchObject() { return m_stopwatchObject; }
    void startStopwatch();
    void endStopwatch();

private:
    //pixel support vector
    std::vector<float> m_support;
    ispc::Image m_img;

    //attachments for events
    int m_eventSampleFrequency = 0;
    EventCallback m_eventCb = nullptr;
    void* m_userData = nullptr;

    //attachment for stopwatch
    using TimeType = std::chrono::time_point<std::chrono::high_resolution_clock>;
    TimeType m_timeStart = {};
    Stopwatch* m_stopwatchObject = nullptr;
};

}
