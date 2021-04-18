#include "PerlinNoiseGenerator.h"
#include "WhiteNoiseGenerator.h"
#include "KernelRunner.h"
#include "Image.h"
#include <Noice/RandomDirKernel.ispc.h>
#include <Noice/PerlinKernel.ispc.h>
#include <random>

namespace noice
{ 

class DirectionKernel
{
public:
    inline void init(int w, int h, int d, int threadCount)
    {
        m_w = w;
        m_h = h;
        m_d = d;
    }

    inline void setSampleOffset(float u, float v)
    {
        m_uOffset = u;
        m_vOffset = u;
    }

    inline void operator()(
        int jobId,
        int x0, int y0,
        int x1, int y1, int z, ispc::Image& image)
    {
        ispc::RandomDirKernel(
            x0, y0, x1, y1, z,
            m_w, m_h, m_d, *m_scrambleTexture, image);
    }

    inline void setScrambleTexture(const ispc::Image& scrambleTexture)
    {
        m_scrambleTexture = &scrambleTexture;
    }

private:
    int m_w = 0;
    int m_h = 0;
    int m_d = 0;
    const ispc::Image* m_scrambleTexture = nullptr;
    float m_uOffset = 0.0f;
    float m_vOffset = 0.0f;
};

class PerlinKernel 
{
public:
    inline void init(int w, int h, int d, int threadCount)
    {
        m_w = w;
        m_h = h;
        m_d = d;
    }

    inline void operator()(
        int jobId,
        int x0, int y0,
        int x1, int y1, int z, ispc::Image& image)
    {
        ispc::PerlinKernel(
            x0, y0, x1, y1, z,
            m_w, m_h, m_d, *m_directions, image);
    }

    inline void setDirection(const ispc::Image& directions)
    {
        m_directions = &directions;
    }

private:
    const ispc::Image* m_directions = nullptr;
    int m_w = 0;
    int m_h = 0;
    int m_d = 0;
};

Error perlinNoiseGenerator(
    const PerlinNoiseGenDesc& desc,
    int threadCount,
    Image& output)
{
    Error result = Error::Ok;
    output.init(desc.width, desc.height, desc.depth);
    output.startStopwatch();
    EventArguments callbackArgs;
    callbackArgs.userData = output.getEventUserData();

    float frequency = 4.0f;
    int gridWidth  = (int)std::ceil(frequency);
    int gridHeight = (int)std::ceil(frequency);
    int gridDepth  = (int)std::ceil(desc.depth > 1 ? frequency : 1);

    Image randomDirectionsImg;
    {
        WhiteNoiseGenDesc whiteNoiseDesc;
        Image scrambleTexture;
        whiteNoiseDesc.width  = gridWidth;
        whiteNoiseDesc.height = gridHeight;
        whiteNoiseDesc.depth  = gridDepth;
        whiteNoiseDesc.seed   = desc.seed;
        result = whiteNoiseGenerator(whiteNoiseDesc, threadCount, scrambleTexture);
        if (result != Error::Ok)
            return result;
        randomDirectionsImg.init(gridWidth, gridHeight, gridDepth, 4);
        KernelRunner<DirectionKernel> randDirKernel(gridWidth, gridHeight, gridDepth, threadCount);
        randDirKernel.kernel().setScrambleTexture(scrambleTexture.img());
        randDirKernel.run(randomDirectionsImg.img());
    }

    {
        KernelRunner<PerlinKernel> perlinKernel(desc.width, desc.height, desc.depth, threadCount);
        perlinKernel.kernel().setDirection(randomDirectionsImg.img());
        perlinKernel.run(output.img());
    }

    if (output.getEventCb() != nullptr)
    {
        callbackArgs.pixelsProcessed = (int)output.pixelCount();
        output.getEventCb()(callbackArgs);
    }

    output.endStopwatch();
    return result;
}

}
