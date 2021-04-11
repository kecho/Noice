#include <Noice/DistanceKernel.ispc.h>
#include <vector>
#include <iostream>
#include <chrono>
#include <ImfImage.h>
#include <ImfRgbaFile.h>
#include <ImfOutputFile.h>
#include "KernelRunner.h"
#include "SimdSearcher.h"


namespace noice
{

class DistanceKernel
{
public:
    inline void operator()(
        int x0, int y0,
        int x1, int y1, int z, ispc::Image& image)
    {
        ispc::DistanceKernel(
            m_pX, m_pY, m_pZ,
            x0, y0, x1, y1,
            z, m_w, m_h, m_d, m_rho2, image);
    }

    inline void init(float rho2, int w, int h, int d)
    {
        m_w = w;
        m_h = h;
        m_d = d;
        m_rho2 = rho2;
    }

    inline void args(int px, int py, int pz)
    {
        m_pX = px;
        m_pY = py;
        m_pZ = pz;
    }

private:
    int m_pX = 0;
    int m_pY = 0;
    int m_pZ = 0;
    int m_w = 0;
    int m_h = 0;
    int m_d = 0;
    float m_rho2 = 2.1f;
};

void makeBlueNoise(int w, int h, int d, int threadCount)
{
    auto t0 = std::chrono::high_resolution_clock::now();
    int pixelCount = w * h * d;
    int wh = w * h;
    std::vector<float> pixels(pixelCount);

    SimdSearcher searcher(w, h, d, 0xdeadbeef, threadCount);
    KernelRunner<DistanceKernel> distanceKernel(w, h, d, threadCount);
    const float rho2= 2.1f * 2.1f;
    distanceKernel.kernel().init(rho2, w, h, d);

    std::vector<float> distancePixels(w*h*d, 0.0f);
    ispc::Image distanceImg = { w, h, d, distancePixels.data() };

    ispc::PixelState currentPixel = {};
    for (int pixelIt = 0; pixelIt < pixelCount; ++pixelIt)
    {
        float rank = (float)pixelIt / (float)pixelCount;
        searcher.setValidity(currentPixel, false);
        int offset = ispc::GetOffset(currentPixel);
        pixels[offset] = rank;
        
        int currX = offset % w;
        int currY = (offset / w) % h;
        int currZ = (offset / wh);
        
        distanceKernel.kernel().args(currX, currY, currZ);
        distanceKernel.run(distanceImg);
        currentPixel = searcher.findMin(distanceImg);
    }

    auto t1 = std::chrono::high_resolution_clock::now();
    auto ms_result = std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0);
    std::cout << "Duration: " << ms_result.count() << " ms " << std::endl;

    //////////////////////

    #if 0
    ispc::Image img;
    img.width = w;
    img.height = h;
    img.data = pixels.data();


    Imf::RgbaOutputFile file ("testImage.exr", w, h, Imf::WRITE_R);
    file.setFrameBuffer (outputPixels.data(), 1, w);
    file.writePixels(h);
    #else

    int scanLines = h * d;
    Imf::Header header(w, scanLines);
    header.channels().insert("R", Imf::Channel(Imf::FLOAT));    
    try {
        Imf::OutputFile file("multiLayer2.exr", header);
        Imf::FrameBuffer frameBuffer;

        frameBuffer.insert("R", Imf::Slice(Imf::FLOAT, (char*)(pixels.data()),
            sizeof(float), sizeof(float) * w));

        file.setFrameBuffer(frameBuffer);
        file.writePixels(scanLines);
    } catch (const std::exception& exc) {
        std::cout << exc.what() << std::endl;
    }
    
    #endif
}

}
