#include "SimdSearcher.h"
#include "Utils.h"
#include <execution>

namespace noice
{

SimdSearcher::SimdSearcher(int w, int h, int jobCount)
: m_pixelCount(nextPowOf2(w*h))
, m_imgScatterState(m_pixelCount)
, m_w(w)
, m_h(h)
, m_jobCount(jobCount)
{
    for (auto& s : m_tmpScatterStates)
        s.resize(m_pixelCount >> 1);

    ispc::InitPixelStates(m_imgScatterState.data(), (unsigned)m_imgScatterState.size(), (unsigned)m_w, (unsigned)m_h);

    int jobPixelSz = divUp<int>(m_pixelCount, jobCount); 

    int offset = 0;
    int pixelsLeft = m_pixelCount;
    for (int i = 0; i < jobCount && pixelsLeft > 0; ++i)
    {
        m_jobContexts.emplace_back();
        JobContext& j = m_jobContexts.back();
        j.offset = offset;
        j.size = std::min(jobPixelSz, pixelsLeft);
        j.result = {};
        offset += j.size;
        pixelsLeft -= j.size;
    }
}

void SimdSearcher::setValidity(const ispc::PixelState& state, bool validity)
{
    m_imgScatterState[ispc::GetOffset(state)] = ispc::MakePixelState(ispc::GetOffset(state), validity ? 1 : 0);
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
        if (ispc::IsValid(r0) && ispc::IsValid(r1))
        {
            minIndex = distanceImg.data[ispc::GetOffset(r0)] < distanceImg.data[ispc::GetOffset(r1)] ? i : minIndex;
        }
        else
        {
            minIndex = ispc::IsValid(r0) ? i : minIndex;
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
