#include <Noice/BlueNoiseKernel.ispc.h>
#include <vector>
#include <iostream>
#include <chrono>
#include <execution>
#include <ImfImage.h>
#include <ImfRgbaFile.h>

namespace noice
{

class DistanceSearcher
{
public:
    DistanceSearcher(int pixelCount, int jobCounts);

    const ispc::BNKPixelScatterState& findMin(ispc::Image& distanceImage);
    void setValidity(const ispc::BNKPixelScatterState& state, bool validity);

private:
    const ispc::BNKPixelScatterState& findMin(ispc::Image& distanceImage, int offset, int pixelCount);
    int m_pixelCount;
    int m_jobCount;

    struct JobContext
    {
        int offset;
        int size;
        ispc::BNKPixelScatterState result;
    };

    std::vector<JobContext> m_jobContexts;
    std::vector<ispc::BNKPixelScatterState> m_imgScatterState;
    std::vector<ispc::BNKPixelScatterState> m_tmpScatterStates[2];
};

DistanceSearcher::DistanceSearcher(int pixelCount, int jobCount)
: m_imgScatterState(pixelCount)
, m_pixelCount(pixelCount)
, m_jobCount(jobCount)
{
    for (auto& s : m_tmpScatterStates)
        s.resize(pixelCount >> 1);

    ispc::BNKInitPixelScatterArray(m_imgScatterState.data(), (unsigned)m_imgScatterState.size());

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

void DistanceSearcher::setValidity(const ispc::BNKPixelScatterState& state, bool validity)
{
    m_imgScatterState[state.offset].valid = validity ? 1 : 0;
}

const ispc::BNKPixelScatterState& DistanceSearcher::findMin(ispc::Image& distanceImg)
{
    if (m_jobContexts.size() == 1)
        return findMin(distanceImg, 0, m_pixelCount);

    std::for_each(std::execution::par, m_jobContexts.begin(), m_jobContexts.end(), [this, &distanceImg](JobContext& j) {
        j.result = findMin(distanceImg, j.offset, j.size);
    });

    int minIndex = 0;
    for (int i = 1; i < (int)m_jobContexts.size(); ++i)
    {
        ispc::BNKPixelScatterState& r0 = m_jobContexts[i].result;
        ispc::BNKPixelScatterState& r1 = m_jobContexts[minIndex].result;
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

const ispc::BNKPixelScatterState& DistanceSearcher::findMin(ispc::Image& distanceImg, int offset, int pixelCount)
{
    int offsetHalf = offset >> 1;
    int selectedOutput = 0;
    for (int mipIt = 0, mipPixelCount = pixelCount; mipPixelCount > 1; ++mipIt, mipPixelCount >>= 1)
    {
        selectedOutput = (mipIt + 1) & 0x1;
        ispc::BNKPixelScatterState* inputState = mipIt == 0 ? m_imgScatterState.data() + offset : m_tmpScatterStates[mipIt & 0x1].data() + offsetHalf;
        ispc::BNKPixelScatterState* outputState = m_tmpScatterStates[selectedOutput].data() + offsetHalf;
        int randVal = std::rand();
        ispc::BNKFindMaxDistance(
            distanceImg, (unsigned)randVal, mipPixelCount, inputState, outputState);
    } 
    
    return m_tmpScatterStates[selectedOutput][offsetHalf];
}

void makeBlueNoise(int w, int h, int threadCount)
{
    auto t0 = std::chrono::high_resolution_clock::now();
   // while (true);
    std::srand(0xdeadbeef); // use current time as seed for random generator

    int pixelCount = w * h;
    std::vector<float> pixels(w*h);
    std::vector<ispc::BNKPixelScatterState> imgScatterState(pixelCount);
    std::vector<ispc::BNKPixelScatterState> tmpScatterStates[2];
    for (auto& s : tmpScatterStates)
        s.resize(pixelCount >> 1);

    DistanceSearcher searcher(pixelCount, threadCount);

    std::vector<float> distancePixels(w*h, 0.0f);
    ispc::Image distanceImg = { w, h, distancePixels.data() };

    const float rho2= 2.1f * 2.1f;
    ispc::BNKPixelScatterState currentPixel = {};
    for (int pixelIt = 0; pixelIt < pixelCount; ++pixelIt)
    {
        float rank = (float)pixelIt / (float)pixelCount;
        searcher.setValidity(currentPixel, false);
        pixels[currentPixel.offset] = rank;
        
        int currX = currentPixel.offset % w;
        int currY = currentPixel.offset / h;
        int selectedOutput = 0;
        ispc::BNKDistance(currX, currY, w, h, rho2, distanceImg);
        currentPixel = searcher.findMin(distanceImg);
    }

    auto t1 = std::chrono::high_resolution_clock::now();
    auto ms_result = std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0);
    std::cout << "Duration: " << ms_result.count() << " ms " << std::endl;

    //////////////////////

    ispc::Image img;
    img.width = w;
    img.height = h;
    img.data = pixels.data();

    std::vector<Imf::Rgba> outputPixels(w*h);
    for (unsigned i = 0; i < (unsigned)outputPixels.size(); ++i)
    {
        outputPixels[i] = Imf::Rgba(pixels[i], 1.0f, 1.0f, 1.0f);
    }

    Imf::RgbaOutputFile file ("testImage.exr", w, h, Imf::WRITE_R);
    file.setFrameBuffer (outputPixels.data(), 1, w);
    file.writePixels(h);

}

}
