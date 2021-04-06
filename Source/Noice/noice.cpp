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

static inline int divUp(int a, int b)
{
    return (a + b - 1) / b;
}

template <typename KernelT>
class KernelRunner
{
public:
    KernelRunner(int w, int h, int subdivision)
    : m_w(w), m_h(h)
    {
        int tileW = divUp(w, subdivision);
        int tileH = divUp(h, subdivision);
        for (int tX = 0; tX < subdivision; ++tX) 
        {
            for (int tY = 0; tY < subdivision; ++tY)
            {
                m_jobContexts.emplace_back();
                JobContext& jc = m_jobContexts.back();
                jc.x0 = tX * tileW;
                jc.x1 = std::min(jc.x0 + tileW, m_w);
                jc.y0 = tY * tileH;
                jc.y1 = std::min(jc.y0 + tileH, m_h);
            }
        }
    }

    KernelT& kernel() { return m_kernel; }

    void run(ispc::Image& image)
    {
        if ((int)m_jobContexts.size() == 1)
        {
            m_kernel(0, 0, m_w, m_h, image);
            return;
        }

        std::for_each(std::execution::par, m_jobContexts.begin(), m_jobContexts.end(), [this, &image](JobContext& j) {
            m_kernel(j.x0, j.y0, j.x1, j.y1, image);
        });
    }

private:
    struct JobContext
    {
        int x0;
        int x1;
        int y0;
        int y1;
        int offset;
        int size;
    };

    std::vector<JobContext> m_jobContexts;
    int m_w;
    int m_h;
    KernelT m_kernel;
};

class DistanceKernel
{
public:
    int pX = 0;
    int pY = 0;
    int w = 0;
    int h = 0;
    float rho2 = 2.1f;
    void operator()(int x0, int y0, int x1, int y1, ispc::Image& image)
    {
        ispc::BNKDistance(pX, pY, x0, y0, x1, y1, w, h, rho2, image);
    }
};

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
    KernelRunner<DistanceKernel> distanceKernel(w, h, 8);
    distanceKernel.kernel().w = w;
    distanceKernel.kernel().h = h;

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
        
        distanceKernel.kernel().pX = currX;
        distanceKernel.kernel().pY = currY;
        distanceKernel.run(distanceImg);
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
