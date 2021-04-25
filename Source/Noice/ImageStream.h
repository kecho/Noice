#pragma once

#include <noice/noice.h>

namespace noice
{

class Image;
class OutputStream;

struct Channel
{
    Image* image = nullptr;
};

Error streamOutImage(
    const char* filename,
    OutputStream& output,
    int width, int height, int depth,
    const Channel* rgba[4]);

Error streamInImage(
    const char* filename,
    Channel outputChannels[4]);


}
