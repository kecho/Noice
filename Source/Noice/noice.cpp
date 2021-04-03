#define OPENEXR_DLL
#include <Noice/noisekernels.ispc.h>
#include <vector>
#include <ImfImage.h>
#include <ImfRgbaFile.h>

void makeGradient(int w, int h)
{
    std::vector<float> pixels(w*h);

    ispc::Image img;
    img.width = w;
    img.height = h;
    img.data = pixels.data();
    ispc::MakeGradient(img);

    std::vector<Imf::Rgba> outputPixels(w*h);
    for (unsigned i = 0; i < (unsigned)outputPixels.size(); ++i)
    {
        outputPixels[i] = Imf::Rgba(pixels[i], 1.0f, 1.0f, 1.0f);
    }

    Imf::RgbaOutputFile file ("testImage", w, h, Imf::WRITE_R);
    file.setFrameBuffer (outputPixels.data(), 1, w);
    file.writePixels(h);
}

