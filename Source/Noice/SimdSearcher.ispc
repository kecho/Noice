#include "Common.ispch"

#define PixelState int

inline int MakePixelState(int offset, int valid)
{
    return (offset << 1) | (valid & 0x1);
}

inline int IsValid(PixelState ps)
{
    return ps & 0x1;
}

inline int GetOffset(PixelState ps)
{
    return ps >> 1;
}

export void InitPixelStates(
    uniform PixelState states[],
    uniform uint counts,
    uniform uint w,
    uniform uint h,
    uniform uint d)
{
    uniform int wh = w*h;
    foreach(i=0...counts)
    {
        #pragma ignore warning(perf)
        uint x = i % w;
        #pragma ignore warning(perf)
        uint y = (i / w) % h;
        #pragma ignore warning(perf)
        uint z = (i / (wh));
        uint validity = x < w && y < h && z < d ? 1 : 0;
        states[i] = MakePixelState(i, validity);
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
        #pragma ignore warning(perf)
        PixelState p0 = inputStates[si + offset0];
        #pragma ignore warning(perf)
        PixelState p1 = inputStates[si + offset1];

        #pragma ignore warning(perf)
        if (IsValid(p0) == 1 && IsValid(p1) == 1)
        {
            #pragma ignore warning(perf)
            float d0 = distanceImg.data[GetOffset(p0)];

            #pragma ignore warning(perf)
            float d1 = distanceImg.data[GetOffset(p1)];

            #pragma ignore warning(perf)
            outputStates[oi] = d0 < d1 ? p0 : p1;
        }
        else
        {
            #pragma ignore warning(perf)
            outputStates[oi] = IsValid(p0) == 1 ? p0 : p1;
        }
    }
}
 
