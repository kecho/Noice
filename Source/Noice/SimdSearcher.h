#pragma once
#include <vector>
#include <random>
#include <Noice/SimdSearcher.ispc.h>

namespace ispc
{
    struct Image;
    typedef int PixelState;

    inline int MakePixelState(int offset, int valid)
    {
        return (offset << 1) | (valid & 0x1);
    }

    inline int IsValid(PixelState ps)
    {
        return ps & 0x1;
    }
    
    inline int GetOffset(PixelState ps)
    {
        return ps >> 1;
    }
}

namespace noice
{

class SimdSearcher
{
public:
    SimdSearcher(int w, int h, int d, unsigned seed, int jobCounts);

    const ispc::PixelState& findMin(ispc::Image& distanceImage);
    void setValidity(const ispc::PixelState& state, bool validity);

private:

    using RandomFunction = std::mt19937;

    const ispc::PixelState& findMin(ispc::Image& distanceImage, RandomFunction& rand, int offset, int pixelCount);
    int m_w;
    int m_h;
    int m_d;
    int m_pixelCount;
    int m_jobCount;

    struct JobContext
    {
        int offset;
        int size;
        RandomFunction rand;
        ispc::PixelState result;
    };

    std::vector<JobContext> m_jobContexts;
    std::vector<ispc::PixelState> m_imgScatterState;
    std::vector<ispc::PixelState> m_tmpScatterStates[2];
};

}
