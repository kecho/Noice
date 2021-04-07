#include "common.ispch"

struct PixelState
{
    int offset;
    int valid;
};

export void InitPixelStates(
    uniform PixelState states[],
    uniform uint counts)
{
	foreach(i=0...counts)
    {
        PixelState* s = &states[i];

        #pragma ignore warning(perf)
        s->offset = i;

        #pragma ignore warning(perf)
        s->valid = 1;
    }
} 

export void SimdFindMax(
    const uniform Image& distanceImg,
    uniform uint randomNumber,
    uniform uint inputCount,
    uniform PixelState inputStates[],
    uniform PixelState outputStates[])
{
    uniform uint outputCount = inputCount >> 1;
    uint rnd = irng(randomNumber);
    uint offset0 = (rnd) & 0x1;
    uint offset1 = (rnd + 0x1) & 0x1;

    foreach (i=0...outputCount)
    {
        uint oi = ((i + randomNumber) & (outputCount - 1));
        uint si = oi << 1;
        PixelState* p0 = &inputStates[si + offset0];
        PixelState* p1 = &inputStates[si + offset1];

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
 
