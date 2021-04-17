#include "WhiteNoiseGenerator.h"
#include "KernelRunner.h"
#include "Image.h"
#include <Noice/EnumerationKernel.ispc.h>

namespace noice
{ 

class EnumKernel
{
public:
    inline void operator()(
        int x0, int y0,
        int x1, int y1, int z, ispc::Image& image)
    {
        ispc::EnumerationKernel(
            x0, y0, x1, y1, z,
            m_w, m_h, m_d, image);
    }

    inline void init(int w, int h, int d)
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
    output.init(desc.width, desc.height, desc.depth);
    KernelRunner<EnumKernel> enumKernel(desc.width, desc.height, desc.depth, threadCount);
    enumKernel.run(output.img());
    return Error::Ok;
}

}
