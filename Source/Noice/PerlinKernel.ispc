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
                float2 weights;
                int4 offsets;
                linearSample2dInfo(directions, uv, weights, offsets);
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

                float2 txy = (3.0f - 2.0f*weights) * weights * weights;
                float d01 = lerp(d0, d1, txy.x);
                float d23 = lerp(d2, d3, txy.x);
                
                float noiseValue = lerp(d01, d23, txy.y);
                row[x] = noiseValue * 0.5 + 0.5;
            }
        }
    }
    else
    {
        uniform int sliceOffset = z * width * height;
        float4* directionsData = (float4*)directions.data;
	    for (uniform uint y = y0; y < y1; ++y)
	    {
            uniform int rowOffset = y * width + sliceOffset;
	    	float* row = output.data + rowOffset;
	    	foreach(x=x0...x1)
            {
                float3 coord = { ((float)x + 0.5f), ((float)y + 0.5f), ((float)z + 0.5f) };
                float3 uvw = coord * output.texelSize.xyz;
                float3 weights;
                int4 offsets0;
                int4 offsets1;
                linearSample3dInfo(directions, uvw, weights, offsets0, offsets1);
                int3 samplePos[8];
                samplePos[0] = (int3)floor_f3((uvw - directions.halfTexelSize.xyz) * directions.dimensions.xyz);
                samplePos[1] = samplePos[0];
                samplePos[1].x += 1;
                samplePos[2] = samplePos[0];
                samplePos[2].y += 1;
                samplePos[3] = samplePos[2];
                samplePos[3].x += 1;
                for (int i = 0; i < 4; ++i)
                {
                    samplePos[i + 4] = samplePos[i];
                    samplePos[i + 4].z += 1;
                }
                
                float3 gridSamples[8];
                #pragma ignore warning(perf)
	    		gridSamples[0] = directionsData[offsets0.x].xyz;
                #pragma ignore warning(perf)
	    		gridSamples[1] = directionsData[offsets0.y].xyz;
                #pragma ignore warning(perf)
	    		gridSamples[2] = directionsData[offsets0.z].xyz;
                #pragma ignore warning(perf)
	    		gridSamples[3] = directionsData[offsets0.w].xyz;
                #pragma ignore warning(perf)
	    		gridSamples[4] = directionsData[offsets1.x].xyz;
                #pragma ignore warning(perf)
	    		gridSamples[5] = directionsData[offsets1.y].xyz;
                #pragma ignore warning(perf)
	    		gridSamples[6] = directionsData[offsets1.z].xyz;
                #pragma ignore warning(perf)
	    		gridSamples[7] = directionsData[offsets1.w].xyz;

                float3 posInGrid = uvw * directions.dimensions.xyz;
                float3 dirs[8];
                for (int i = 0; i < 8; ++i)
                    dirs[i] = posInGrid - (float3)samplePos[i];

                
                float d[8];
                for (int i = 0; i < 8; ++i)
                    d[i] = dot_f3(gridSamples[i], dirs[i]);

                float3 txyz = (3.0f - 2.0f*weights) * weights * weights;
                float d001 = lerp(d[0], d[1], txyz.x);
                float d023 = lerp(d[2], d[3], txyz.x);
                float d0 = lerp(d001, d023, txyz.y);

                float d101 = lerp(d[4], d[5], txyz.x);
                float d123 = lerp(d[6], d[7], txyz.x);
                float d1 = lerp(d101, d123, txyz.y);

                float noiseValue = lerp(d0, d1, txyz.z);

                row[x] = noiseValue * 0.5 + 0.5;
            }
        }
    }
}
