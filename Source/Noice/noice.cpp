#include <Noice/BlueNoiseKernel.ispc.h>
#include <vector>
#include <iostream>
#include <ctime>
#include <ImfImage.h>
#include <ImfRgbaFile.h>

void makeBlueNoise(int w, int h)
{
   // while (true);
    std::srand(0xdeadbeef); // use current time as seed for random generator

    int pixelCount = w * h;
    std::vector<float> pixels(w*h);
    std::vector<ispc::BNKPixelScatterState> imgScatterState(pixelCount);
    std::vector<ispc::BNKPixelScatterState> tmpScatterStates[2];
    for (auto& s : tmpScatterStates)
        s.resize(pixelCount >> 1);

    ispc::BNKInitPixelScatterArray(imgScatterState.data(), (unsigned)imgScatterState.size());

    std::vector<float> distancePixels(w*h, 0.0f);
    ispc::Image distanceImg = { w, h, distancePixels.data() };

    const float rho2= 2.1f * 2.1f;
    ispc::BNKPixelScatterState currentPixel = {};
    for (int pixelIt = 0; pixelIt < pixelCount; ++pixelIt)
    {
        float rank = (float)pixelIt / (float)pixelCount;
        imgScatterState[currentPixel.offset].valid = 0;
        pixels[currentPixel.offset] = rank;
        
        int currX = currentPixel.offset % w;
        int currY = currentPixel.offset / h;
        int selectedOutput = 0;
        ispc::BNKDistance(currX, currY, w, h, rho2, distanceImg);
        for (int mipIt = 0, mipPixelCount = pixelCount; mipPixelCount > 1; ++mipIt, mipPixelCount >>= 1)
        {
            selectedOutput = (mipIt + 1) & 0x1;
            ispc::BNKPixelScatterState* inputState = mipIt == 0 ? imgScatterState.data() : tmpScatterStates[mipIt & 0x1].data();
            ispc::BNKPixelScatterState* outputState = tmpScatterStates[selectedOutput].data();
            int randVal = std::rand();
            ispc::BNKFindMaxDistance(
                distanceImg, (unsigned)randVal, mipPixelCount, inputState, outputState);
        } 

        currentPixel = tmpScatterStates[selectedOutput][0];
    }

    //////////////////////

    ispc::Image img;
    img.width = w;
    img.height = h;
    img.data = pixels.data();

    std::vector<Imf::Rgba> outputPixels(w*h);
    for (unsigned i = 0; i < (unsigned)outputPixels.size(); ++i)
    {
        outputPixels[i] = Imf::Rgba(pixels[i], 1.0f, 1.0f, 1.0f);
    }

    Imf::RgbaOutputFile file ("testImage.exr", w, h, Imf::WRITE_R);
    file.setFrameBuffer (outputPixels.data(), 1, w);
    file.writePixels(h);
}

