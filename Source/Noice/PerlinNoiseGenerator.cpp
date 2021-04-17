#include "PerlinNoiseGenerator.h"
#include "WhiteNoiseGenerator.h"
#include "KernelRunner.h"
#include "Image.h"
#include <Noice/RandomDirKernel.ispc.h>
#include <random>

namespace noice
{ 

class DirectionKernel
{
public:
    using RandomFunction = std::mt19937;

    inline void init(int w, int h, int d, int threadCount)
    {
        m_w = w;
        m_h = h;
        m_d = d;
    }

    inline void setWhiteNoise(ispc::Image& whiteNoise) { m_whiteNoise = &whiteNoise; }

    inline void operator()(
        int jobId,
        int x0, int y0,
        int x1, int y1, int z, ispc::Image& image)
    {
        ispc::RandomDirKernel(
            x0, y0, x1, y1, z,
            m_w, m_h, m_d, *m_whiteNoise, image);
    }

private:
    ispc::Image* m_whiteNoise = nullptr;
    int m_w = 0;
    int m_h = 0;
    int m_d = 0;
};

Error perlinNoiseGenerator(
    const PerlinNoiseGenDesc& desc,
    int threadCount,
    Image& output)
{
    output.init(desc.width, desc.height, desc.depth);
    output.startStopwatch();
    EventArguments callbackArgs;
    callbackArgs.userData = output.getEventUserData();

    Image whiteNoiseImg;
    {
        WhiteNoiseGenDesc whiteNoiseDesc;
        whiteNoiseDesc.width = desc.width;
        whiteNoiseDesc.height = desc.height;
        whiteNoiseDesc.depth = desc.depth;
        whiteNoiseDesc.seed = desc.seed;
        Error ret = whiteNoiseGenerator(whiteNoiseDesc, threadCount, whiteNoiseImg);
        if (ret != Error::Ok)
            return ret;
    }

    Image randomDirectionsImg;
    {
        randomDirectionsImg.init(desc.width * 3/*rgb direction*/, desc.height, desc.depth);
        KernelRunner<DirectionKernel> randDirKernel(desc.width, desc.height, desc.depth, threadCount);
        randDirKernel.kernel().setWhiteNoise(whiteNoiseImg.img());
        randDirKernel.run(randomDirectionsImg.img());
    }

    if (output.getEventCb() != nullptr)
    {
        callbackArgs.pixelsProcessed = (int)output.pixelCount();
        output.getEventCb()(callbackArgs);
    }

    output.endStopwatch();
    return Error::Ok;
}

}
