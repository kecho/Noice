#include "NoiseTests.h"
#include <iostream>
#include <vector>
#include <noice/noice.h>

namespace noice
{

static bool checkDitherTexture(TextureComponentHandle component, int pixelCount)
{
    const float* pixels = getPixels(component);
    std::vector<bool> ditherStates(pixelCount, false);
    for (int i = 0; i < pixelCount; ++i)
    {
        float p = pixels[i];
        int id = (int)(p * pixelCount);
        if (ditherStates[id])
        {
            std::cerr << "dither state is incorrect, some pixels have duplicate values" << std::endl;
            return false;
        }

        ditherStates[id] = true;
    }

    for (auto pixelState : ditherStates)
    {
        if (!pixelState)
        {
            std::cerr << "some pixels in the dither state were never visited" << std::endl;
            return false;
        }
    }

    return true;
}

bool blueNoiseTest()
{
    TextureComponentHandle component = createTextureComponent();
    BlueNoiseGenDesc desc;
    desc.width = 16;
    desc.height = 16;
    desc.depth = 16;
    Error errCode = generateBlueNoise(component, desc, 8u);
    if (errCode != Error::Ok)
    {
        std::cerr << "blueNoiseTest failed with error code :" << (int)errCode <<  std::endl;
        return false;
    }

    //validate
    int pixelCount = desc.width * desc.height* desc.depth;
    if (!checkDitherTexture(component, pixelCount))
        return false;

    deleteComponent(component);
    return true;
}

bool whiteNoiseTest()
{
    TextureComponentHandle component = createTextureComponent();
    WhiteNoiseGenDesc desc;
    desc.width = 16;
    desc.height = 16;
    desc.depth = 16;
    Error errCode = generateWhiteNoise(component, desc, 8u);
    if (errCode != Error::Ok)
    {
        std::cerr << "whiteNoiseTest failed with error code :" << (int)errCode <<  std::endl;
        return false;
    }

    //validate
    int pixelCount = desc.width * desc.height* desc.depth;
    if (!checkDitherTexture(component, pixelCount))
        return false;

    deleteComponent(component);
    return true;
}

static bool perlinTestCommon(int depth)
{
    TextureComponentHandle component = createTextureComponent();
    PerlinNoiseGenDesc desc;
    desc.width = 32;
    desc.height = 32;
    desc.depth = depth;
    Error err = generatePerlinNoise(component, desc, 8u);
    if (err != Error::Ok)
        return false;
    deleteComponent(component);
    return true;
}

bool perlin2dTest()
{
    return perlinTestCommon(1);
}

bool perlin3dTest()
{
    return perlinTestCommon(32);
}

}
