#include <Noice/noisekernels.ispc.h>

float* makeGradient(int w, int h)
{
    ispc::Image img;
    img.width = w;
    img.height = h;
    img.data = new float[w*h];
    ispc::MakeGradient(img);
    return img.data;
}

