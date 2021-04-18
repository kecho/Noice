#include "Common.ispch"

export void PerlinKernel(
    uniform int x0, uniform int y0,
    uniform int x1, uniform int y1,
    uniform int z,
    uniform int width, uniform int height, uniform int depth,
    const uniform Image& directions,
    uniform Image& output)
{
    if (output.depth == 1)
    {
        float4* directionsData = (float4*)directions.data;
	    for (uniform uint y = y0; y < y1; ++y)
	    {
            uniform int rowOffset = y * width;
	    	float* row = output.data + rowOffset;
	    	foreach(x=x0...x1)
            {
                float2 coord = { ((float)x + 0.5f), ((float)y + 0.5f) };
                float2 uv = coord * output.texelSize.xy;
                float4 weights;
                float2 weightDims;
                int4 offsets;
                linearSample2dInfo(directions, uv, weights, weightDims, offsets);
                int2 samplePos[4];
                samplePos[0] = (int2)floor_f2((uv - directions.halfTexelSize.xy) * directions.dimensions.xy);
                samplePos[1] = samplePos[0];
                samplePos[1].x += 1;
                samplePos[2] = samplePos[0];
                samplePos[2].y += 1;
                samplePos[3] = samplePos[2];
                samplePos[3].x += 1;
                
                #pragma ignore warning(perf)
	    		float2 grid0 = directionsData[offsets.x].xy;
                #pragma ignore warning(perf)
	    		float2 grid1 = directionsData[offsets.y].xy;
                #pragma ignore warning(perf)
	    		float2 grid2 = directionsData[offsets.z].xy;
                #pragma ignore warning(perf)
	    		float2 grid3 = directionsData[offsets.w].xy;

                float2 posInGrid = uv * directions.dimensions.xy;
                float2 dir0 = posInGrid - (float2)samplePos[0];
                float2 dir1 = posInGrid - (float2)samplePos[1];
                float2 dir2 = posInGrid - (float2)samplePos[2];
                float2 dir3 = posInGrid - (float2)samplePos[3];

                float d0 = dot_f2(grid0, dir0);
                float d1 = dot_f2(grid1, dir1);
                float d2 = dot_f2(grid2, dir2);
                float d3 = dot_f2(grid3, dir3);

                float2 txy = (3.0f - 2.0f*weightDims) * weightDims * weightDims;
                float d01 = lerp(d0, d1, txy.x);
                float d23 = lerp(d2, d3, txy.x);
            
                
                float noiseValue = lerp(d01, d23, txy.y);
                row[x] = noiseValue * 0.5 + 0.5;
            }
        }
    }
    else
    {
#if 0
        uniform int sliceOffset = z * width * height;
        float4* directionsData = (float4*)directions.data;
	    for (uniform uint y = y0; y < y1; ++y)
	    {
            uniform int rowOffset = y * width + sliceOffset;
	    	float* row = output.data + rowOffset;
	    	foreach(x=x0...x1)
            {
            }
        }
#endif
    }
}
