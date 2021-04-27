#include "Common.ispch"

float2 cmul(float2 a, float2 b)
{
    float2 ri = {a.x*b.x - a.y*b.y, a.x*b.y + a.y*b.x};
    return ri;
}

inline float2 polar(float m, float a)
{
    float2 cosSin = { cos(a), sin(a) };
    return m*cosSin;   
}

extern void DftTranspose(
    uniform int x0, uniform int y0,
    uniform int x1, uniform int y1,
    uniform int z,
    uniform int width, uniform int height,
    uniform Image& input,
    uniform Image& output)
{
    uniform int sliceOffset = z * width * height;
    uniform float4* inputDataPtr = (uniform float4*)(input.data + sliceOffset);
    uniform float4* dataPtr = (uniform float4*)(output.data + sliceOffset);
    for (uniform uint y = y0; y < y1; ++y)
    {
        float4* inputRow = inputDataPtr + y*input.width;
        foreach(x=x0...x1)
        {
            float4* outputRow = dataPtr + x*output.width;
            #pragma ignore warning(perf)
            outputRow[y] = inputRow[x];
        }
    }
}

extern void DftKernel(
    uniform int x0, uniform int y0,
    uniform int x1, uniform int y1,
    uniform int z,
    uniform int width, uniform int height,
    uniform int size,
    uniform Image& input,
    uniform Image& output)
{
    uniform float tau = atan(1.0)*8.0;
    uniform int sliceOffset = z * width * height;
    uniform float4* inputDataPtr = (uniform float4*)(input.data + sliceOffset);
    uniform float4* dataPtr = (uniform float4*)(output.data + sliceOffset);
    float halfSize = (float)size * 0.5;
    float tauSizeInv = tau * (1.0 / (float)size);
    for (uniform uint y = y0; y < y1; ++y)
    {
        float4* inputRow = inputDataPtr + y*width;
        float4* row = dataPtr + y*width;
        foreach(x=x0...x1)
        {
            float w = x - (halfSize);
            float2 value = { 0.0f, 0.0f };
            cfor (int k = 0; k < size; ++k)
            {
                #pragma ignore warning(perf)
                float4 sample = inputRow[x + k];
                float a = -(tauSizeInv * w * (float)k);
        
                value += cmul(sample.xy, polar(1.0, a));
            }    

            float4 result = { value.x, value.y, 0.0f, 0.0f };
            #pragma ignore warning(perf)
            row[x] = result;
        }
    }
}
