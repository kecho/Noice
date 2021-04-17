#include "Common.ispch"

export void RandomDirKernel(
    uniform int x0, uniform int y0,
    uniform int x1, uniform int y1,
    uniform int z,
    uniform int width, uniform int height, uniform int depth,
    uniform Image& whiteNoise,
    uniform Image& result)
{
    uniform int sliceOffset = z * width * height;
    uniform float* dataPtr = result.data + 3*sliceOffset;
    uniform float* whiteNoiseInput = whiteNoise.data + sliceOffset;
	for (uniform uint y = y0; y < y1; ++y)
	{
        uniform int rowOffset = y*width;
		float* row = dataPtr + 3*rowOffset;
        float* whiteNoiseRow = whiteNoiseInput + rowOffset;

		foreach(x=x0...x1)
        {
            #pragma ignore warning(perf)
			row[3*x] = whiteNoiseRow[x];
        }
    }
}
