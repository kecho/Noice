#include <Noice/DistanceKernel.ispc.h>
#include <vector>
#include <iostream>
#include <chrono>
#include <ImfImage.h>
#include <ImfRgbaFile.h>
#include "KernelRunner.h"
#include "SimdSearcher.h"

namespace noice
{

class DistanceKernel
{
public:
    inline void operator()(int x0, int y0, int x1, int y1, ispc::Image& image)
    {
        ispc::DistanceKernel(m_pX, m_pY, x0, y0, x1, y1, m_w, m_h, m_rho2, image);
    }

    inline void init(float rho2, int w, int h)
    {
        m_w = w;
        m_h = h;
        m_rho2 = rho2;
    }

    inline void args(int px, int py)
    {
        m_pX = px;
        m_pY = py;
    }

private:
    int m_pX = 0;
    int m_pY = 0;
    int m_w = 0;
    int m_h = 0;
    float m_rho2 = 2.1f;
};

void makeBlueNoise(int w, int h, int threadCount)
{
    auto t0 = std::chrono::high_resolution_clock::now();
    std::srand(0xdeadbeef); // use current time as seed for random generator

    int pixelCount = w * h;
    std::vector<float> pixels(w*h);
    std::vector<ispc::PixelState> imgScatterState(pixelCount);
    std::vector<ispc::PixelState> tmpScatterStates[2];
    for (auto& s : tmpScatterStates)
        s.resize(pixelCount >> 1);

    SimdSearcher searcher(w, h, threadCount);
    KernelRunner<DistanceKernel> distanceKernel(w, h, threadCount);
    const float rho2= 2.1f * 2.1f;
    distanceKernel.kernel().init(rho2, w, h);

    std::vector<float> distancePixels(w*h, 0.0f);
    ispc::Image distanceImg = { w, h, distancePixels.data() };

    ispc::PixelState currentPixel = {};
    for (int pixelIt = 0; pixelIt < pixelCount; ++pixelIt)
    {
        float rank = (float)pixelIt / (float)pixelCount;
        searcher.setValidity(currentPixel, false);
        int offset = ispc::GetOffset(currentPixel);
        pixels[offset] = rank;
        
        int currX = offset % w;
        int currY = offset / w;
        
        distanceKernel.kernel().args(currX, currY);
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
