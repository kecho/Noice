#include "Common.ispch"

float distanceFunction(int3 dims, float rho2, int3 p0, int3 p1)
{
    int3 dp = abs_i3(p1 - p0);
    float3 z = (float3)min_i3(dp, dims - dp);
    float zSqr = dot_f3(z, z);
    return exp(-zSqr / rho2);
}

export void DistanceKernel(
    uniform int pX, uniform int pY, uniform int pZ,
    uniform int x0, uniform int y0,
    uniform int x1, uniform int y1,
    uniform int z,
    uniform int width, uniform int height, uniform int depth,
    uniform float rho2,
    uniform Image& result)
{
    uniform int3 p = { pX, pY, pZ };
    uniform int3 dims = { width, height, depth };
    uniform int sliceOffset = z * width * height;
    uniform float* dataPtr = result.data + sliceOffset;
    for (uniform uint y = y0; y < y1; ++y)
    {
        float* row = dataPtr + y*result.width;
        foreach(x=x0...x1)
        {
            int3 xyz = { x, y, z };
            row[x] += distanceFunction(dims, rho2, xyz, p);
        }
    }
}




