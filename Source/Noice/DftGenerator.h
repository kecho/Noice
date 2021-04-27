#pragma once
#include <noice/noice.h>

namespace noice
{

class Image;

Error dftGenerator(const Image& input, Image* outputs[2], const DftOptions& options);

}
