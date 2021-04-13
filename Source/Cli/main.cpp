#include <Noice/noice.h>
#include <iostream>

void main()
{
    //int threadCounts[] = { 64 };
    noice::BlueNoiseGenDesc desc;
    desc.width = 256;
    desc.height = 256;
    noice::TextureComponentHandle bluenoise;
    noice::Error err = noice::generateBlueNoise(desc, 16, &bluenoise);
    if (err != noice::Error::Ok)
    {
        std::cerr << "error: " << (int)err << std::endl;
        return;
    }

    const noice::TextureComponentHandle* components[4] = {};
    components[0] = &bluenoise;
    components[1] = &bluenoise;
    err = noice::saveTextureToFile(components, "outputTex.exr");
    if (err != noice::Error::Ok)
    {
        std::cerr << "io error" << (int)err << std::endl;
        return;
    }

    deleteComponent(&bluenoise);

    std::cout << "Success." << std::endl;
}

