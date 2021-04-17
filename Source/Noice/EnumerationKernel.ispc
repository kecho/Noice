#include "Common.ispch"

export void EnumerationKernel(
    uniform int x0, uniform int y0,
    uniform int x1, uniform int y1,
    uniform int z,
    uniform int width, uniform int height, uniform int depth,
    uniform Image& result)
{
    uniform float totalPixelCountF = width * height * depth;
    uniform int sliceStride = z * width * height;
    uniform float sliceStrideF = sliceStride;
    uniform float* dataPtr = result.data + sliceStride;
	for (uniform uint y = y0; y < y1; ++y)
	{
        uniform int rowStride = y*result.width;
        uniform float idOffset = sliceStrideF + rowStride;
		float* row = dataPtr + rowStride;
		foreach(x=x0...x1)
        {
            float id = idOffset + x; 
			row[x] = id / totalPixelCountF;
        }
    }
}
