#pragma once
#include <noice/noice.h>

namespace noice
{

class Image;
Error whiteNoiseGenerator(const WhiteNoiseGenDesc& desc, int threadCount, Image& output);

}
