#include <Noice/noice.h>
#include <iostream>

void main()
{
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

    noice::TextureFileDesc texDesc = {};
    texDesc.filename = "outputTex.exr";
    texDesc.channels[0] = &bluenoise;
    texDesc.channels[1] = &bluenoise;
    err = noice::saveTextureToFile(texDesc);
    if (err != noice::Error::Ok)
    {
        std::cerr << "io error" << (int)err << std::endl;
        return;
    }

    deleteComponent(&bluenoise);

    std::cout << "Success." << std::endl;
}

