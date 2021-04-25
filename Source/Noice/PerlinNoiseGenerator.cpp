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
            m_w, m_h, m_d, m_weight, *m_directions, image);
    }

    inline void setParameters(float weight, const ispc::Image& directions)
    {
        m_weight = weight;
        m_directions = &directions;
    }

private:
    const ispc::Image* m_directions = nullptr;
    float m_weight = 1.0f;
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
    output.startStopwatch();
    output.clear(0.0f); 
    EventArguments callbackArgs;
    callbackArgs.userData = output.getEventUserData();

    static float defaultFrequencies[] = { 2.0f, 4.0f, 8.0f, 16.0f, 32.0f, 64.0f };
    static float defaultWeights[]     = { 0.5f, 0.3f, 0.2f, 0.1f, 0.05f, 0.02f };

    const float* frequencies = desc.frequencies;
    const float* weights = desc.weights;
    int freqCounts = desc.frequencyCounts;

    if ((frequencies == nullptr && weights == nullptr) || freqCounts == 0)
    {
        frequencies = defaultFrequencies;
        weights = defaultWeights;
        freqCounts = sizeof(defaultFrequencies)/sizeof(defaultFrequencies[0]);
    }
    else if (frequencies == nullptr || weights == nullptr)
    {
        return Error::BadArgs;
    }

    for (int i = 0; i < freqCounts; ++i)
    {
        float weight = weights[i];
        float frequency = frequencies[i];
        int gridWidth  = (int)std::ceil(frequency);
        int gridHeight = (int)std::ceil(frequency);
        int gridDepth  = (int)std::ceil(output.depth() > 1 ? frequency : 1);
    
        Image randomDirectionsImg;
        {
            Image scrambleTexture;
            scrambleTexture.init(gridWidth, gridHeight, gridDepth, 1u);
            WhiteNoiseGenDesc whiteNoiseDesc;
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
            KernelRunner<PerlinKernel> perlinKernel(output.width(), output.height(), output.depth(), threadCount);
            perlinKernel.kernel().setParameters(weight, randomDirectionsImg.img());
            perlinKernel.run(output.img());
        }
    }

    if (output.pixelCount() > 0)
    {
        ispc::Image& outImg = output.img();
        float minVal = outImg.data[0];
        float maxVal = minVal;
        for (int i = 1; i < output.pixelCount(); ++i)
        {
            float pixelVal = outImg.data[i];
            minVal = std::min(minVal, pixelVal);
            maxVal = std::max(maxVal, pixelVal);
        }
        float range = 1.0f / (maxVal - minVal);
        for (int i = 1; i < output.pixelCount(); ++i)
        {
            float pixelVal = outImg.data[i];
            outImg.data[i] = (pixelVal - minVal) * range;
        }
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
