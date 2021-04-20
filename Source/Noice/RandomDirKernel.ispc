#include "Common.ispch"

export void RandomDirKernel(
    uniform int x0, uniform int y0,
    uniform int x1, uniform int y1,
    uniform int z,
    uniform int width, uniform int height, uniform int depth,
    const uniform Image& scrambleTexture,
    uniform Image& result)
{
    if (depth == 1)
    {
        uniform int N = width * height;
        for (uniform uint y = y0; y < y1; ++y)
        {
            uniform int rowOffset = y * width;
            float4* row = (float4*)result.data + rowOffset;
            foreach(x=x0...x1)
            {
                float texValue = scrambleTexture.data[rowOffset + x];
                float2 value = discSample(texValue);
                #pragma ignore warning(perf)
                row[x] = value.xyxy;
            }
        }
    }
    else
    {
        uniform int N = width * height * depth;
        uniform int sliceOffset = z * width * height;
        for (uniform uint y = y0; y < y1; ++y)
        {
            uniform int rowOffset = y * width + sliceOffset;
            float4* row = (float4*)(result.data) + rowOffset;
            foreach(x=x0...x1)
            {
                int pixelId = (int)(scrambleTexture.data[rowOffset + x] * (float)N + 0.5);
                
                float2 uv = hammersley2d(pixelId, N);
                float3 value = sphereSample(uv.x, uv.y);
                #pragma ignore warning(perf)
                row[x] = value.xyzz;
            }
        }
    }
}
