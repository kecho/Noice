#include "common.ispch"

float distanceFunction(int2 dims, float rho2, int2 p0, int2 p1)
{
	int2 dp = abs_i2(p1 - p0);
	float2 z = (float2)min_i2(dp, dims - dp);
	float zSqr = dot_f2(z, z);
	return exp(-zSqr / rho2);
}

export void BNKDistance(
    uniform int pX, uniform int pY,
    uniform int x0, uniform int y0,
    uniform int x1, uniform int y1,
    uniform int width, uniform int height,
    uniform float rho2,
    uniform Image& result)
{
    uniform int2 p = { pX, pY };
    uniform int2 dims = { width, height };
	for (uniform uint y = y0; y < y1; ++y)
	{
		float* row = result.data + y*result.width;
		foreach(x=x0...x1)
		{
			int2 xy = { x, y };
			row[x] += distanceFunction(dims, rho2, xy, p);
		}
	}
}




