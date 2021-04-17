#pragma once
#include <noice/noice.h>

namespace noice
{

class Image;
Error perlinNoiseGenerator(const PerlinNoiseGenDesc& desc, int threadCount, Image& output);

}
