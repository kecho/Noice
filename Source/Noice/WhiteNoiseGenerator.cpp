#include "WhiteNoiseGenerator.h"
#include "KernelRunner.h"
#include "Image.h"
#include <Noice/EnumerationKernel.ispc.h>
#include <random>

namespace noice
{ 

class EnumKernel
{
public:
    inline void operator()(
        int jobId,
        int x0, int y0,
        int x1, int y1, int z, ispc::Image& image)
    {
        ispc::EnumerationKernel(
            x0, y0, x1, y1, z,
            m_w, m_h, m_d, image);
    }

    inline void init(int w, int h, int d, int threadCount)
    {
        m_w = w;
        m_h = h;
        m_d = d;
    }

private:
    int m_w = 0;
    int m_h = 0;
    int m_d = 0;
};

Error whiteNoiseGenerator(
    const WhiteNoiseGenDesc& desc,
    int threadCount,
    Image& output)
{
    output.startStopwatch();

    EventArguments callbackArgs;
    callbackArgs.userData = output.getEventUserData();

    //initialize values in simd
    {
        KernelRunner<EnumKernel> enumKernel(output.width(), output.height(), output.depth(), threadCount);
        enumKernel.run(output.img());
    }

    //scramble
    {
        using RandomFunction = std::mt19937;
        auto rand = RandomFunction(desc.seed);
        for (int i = output.pixelCount() - 1; i >= 0; --i)
        {
            int j = rand() % (i + 1);
            std::swap(output[i], output[j]);
        }
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
