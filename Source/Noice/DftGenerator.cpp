#include "DftGenerator.h"
#include "Image.h"

namespace noice
{

Error dftGenerator(const Image& input, Image* outputs[2], const DftOptions& options)
{
    for (int i = 0; i < 2; ++i)
    {
        auto* output = outputs[i];
        if (output->width() != input.width()
            || output->height() != input.height()
            || output->depth() != input.depth()
            || output->channels() != input.channels()
            || input.channels() > 1)
            return Error::BadArgs;

        output->clear(0.0f);
    }

    return Error::Ok;
}

}
