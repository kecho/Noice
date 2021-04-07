#include "SimdSearcher.h"
#include <execution>

namespace noice
{

SimdSearcher::SimdSearcher(int pixelCount, int jobCount)
: m_imgScatterState(pixelCount)
, m_pixelCount(pixelCount)
, m_jobCount(jobCount)
{
    for (auto& s : m_tmpScatterStates)
        s.resize(pixelCount >> 1);

    ispc::InitPixelStates(m_imgScatterState.data(), (unsigned)m_imgScatterState.size());

    int jobPixelSz = pixelCount / jobCount; 
    m_jobContexts.resize(jobCount);
    for (int i = 0; i < jobCount; ++i)
    {
        JobContext& j = m_jobContexts[i];
        j.offset = i * jobPixelSz;
        j.size = jobPixelSz;
        j.result = {};
    }
}

void SimdSearcher::setValidity(const ispc::PixelState& state, bool validity)
{
    m_imgScatterState[state.offset].valid = validity ? 1 : 0;
}

const ispc::PixelState& SimdSearcher::findMin(ispc::Image& distanceImg)
{
    if (m_jobContexts.size() == 1)
        return findMin(distanceImg, 0, m_pixelCount);

    std::for_each(std::execution::par, m_jobContexts.begin(), m_jobContexts.end(), [this, &distanceImg](JobContext& j) {
        j.result = findMin(distanceImg, j.offset, j.size);
    });

    int minIndex = 0;
    for (int i = 1; i < (int)m_jobContexts.size(); ++i)
    {
        ispc::PixelState& r0 = m_jobContexts[i].result;
        ispc::PixelState& r1 = m_jobContexts[minIndex].result;
        if (r0.valid && r1.valid)
        {
            minIndex = distanceImg.data[r0.offset] < distanceImg.data[r1.offset] ? i : minIndex;
        }
        else
        {
            minIndex = r0.valid ? i : minIndex;
        }
    }

    return m_jobContexts[minIndex].result;
    
}

const ispc::PixelState& SimdSearcher::findMin(ispc::Image& distanceImg, int offset, int pixelCount)
{
    int offsetHalf = offset >> 1;
    int selectedOutput = 0;
    for (int mipIt = 0, mipPixelCount = pixelCount; mipPixelCount > 1; ++mipIt, mipPixelCount >>= 1)
    {
        selectedOutput = (mipIt + 1) & 0x1;
        ispc::PixelState* inputState = mipIt == 0 ? m_imgScatterState.data() + offset : m_tmpScatterStates[mipIt & 0x1].data() + offsetHalf;
        ispc::PixelState* outputState = m_tmpScatterStates[selectedOutput].data() + offsetHalf;
        int randVal = std::rand();
        ispc::SimdFindMax(
            distanceImg, (unsigned)randVal, mipPixelCount, inputState, outputState);
    } 
    
    return m_tmpScatterStates[selectedOutput][offsetHalf];
}


}
