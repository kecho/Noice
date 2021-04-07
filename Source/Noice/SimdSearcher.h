#pragma once
#include <vector>
#include <Noice/SimdSearcher.ispc.h>

namespace ispc
{
    struct Image;
    struct PixelState;
}

namespace noice
{

class SimdSearcher
{
public:
    SimdSearcher(int pixelCount, int jobCounts);

    const ispc::PixelState& findMin(ispc::Image& distanceImage);
    void setValidity(const ispc::PixelState& state, bool validity);

private:
    const ispc::PixelState& findMin(ispc::Image& distanceImage, int offset, int pixelCount);
    int m_pixelCount;
    int m_jobCount;

    struct JobContext
    {
        int offset;
        int size;
        ispc::PixelState result;
    };

    std::vector<JobContext> m_jobContexts;
    std::vector<ispc::PixelState> m_imgScatterState;
    std::vector<ispc::PixelState> m_tmpScatterStates[2];
};

}
