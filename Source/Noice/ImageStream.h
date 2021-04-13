#pragma once

#include <noice/noice.h>

namespace noice
{

class Image;
class OutputStream;

struct Channel
{
    const Image* image = nullptr;
};

Error streamOutImage(
    const char* filename,
    OutputStream& output,
    int width, int height, int depth,
    const Channel* rgba[4]);


}
