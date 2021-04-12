#pragma once
#include <noice/noice.h>

namespace noice
{

class Image;
struct BlueNoiseGenDesc;

Error blueNoiseGenerator(
    const BlueNoiseGenDesc& desc,
    int threadCount,
    Image& output);

}
