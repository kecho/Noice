#include "common.ispch"

struct BNKPixelScatterState
{
    int offset;
    int valid;
};

export void BNKInitPixelScatterArray(
    uniform BNKPixelScatterState states[],
    uniform uint counts)
{
	foreach(i=0...counts)
    {
        BNKPixelScatterState* s = &states[i];

        #pragma ignore warning(perf)
        s->offset = i;

        #pragma ignore warning(perf)
        s->valid = 1;
    }
} 

export void BNKFindMaxDistance(
    const uniform Image& distanceImg,
    uniform uint randomNumber,
    uniform uint inputCount,
    uniform BNKPixelScatterState inputStates[],
    uniform BNKPixelScatterState outputStates[])
{
    uniform uint outputCount = inputCount >> 1;
    uint rnd = irng(randomNumber);
    uint offset0 = (rnd) & 0x1;
    uint offset1 = (rnd + 0x1) & 0x1;

    foreach (i=0...outputCount)
    {
        uint oi = ((i + randomNumber) & (outputCount - 1));
        uint si = oi << 1;
        BNKPixelScatterState* p0 = &inputStates[si + offset0];
        BNKPixelScatterState* p1 = &inputStates[si + offset1];

        #pragma ignore warning(perf)
		if (p0->valid == 1 && p1->valid == 1)
		{
            #pragma ignore warning(perf)
            float d0 = distanceImg.data[p0->offset];

            #pragma ignore warning(perf)
            float d1 = distanceImg.data[p1->offset];

            #pragma ignore warning(perf)
			outputStates[oi] = d0 < d1 ? *p0 : *p1;
		}
		else
		{
            #pragma ignore warning(perf)
			outputStates[oi] = p0->valid == 1 ? *p0 : *p1;
		}
    }
}
 
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




