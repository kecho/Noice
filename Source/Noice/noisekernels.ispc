#include "common.ispch"

export void MakeGradient(uniform Image& outputImage)
{
    uniform float widthF = (float)outputImage.width;
	for (uniform uint y = 0; y < outputImage.height; ++y)
	{
		uniform uint offset = y*outputImage.width;
		foreach(x=0...outputImage.width)
		{
            float color = ((float)x / widthF);
            outputImage.data[offset + x] = color;
        }
    }
}
