#!/bin/bash
../t2-output/linux-gcc-production-default/noice -cR -n blue -d 256 -o BlueNoise_256x256.exr
../t2-output/linux-gcc-production-default/noice -cR -n blue -d 32x32x32 -o BlueNoise_32x32x32.exr
../t2-output/linux-gcc-production-default/noice -cR -n perlin -d 128x128x128 -o PerlinNoise_128x128x128.exr
../t2-output/linux-gcc-production-default/noice -cR -n perlin -d 32x32x32 -o PerlinNoise_32x32x32.exr
../t2-output/linux-gcc-production-default/noice -cR -n perlin -d 512 -o PerlinNoise_512x512.exr
../t2-output/linux-gcc-production-default/noice -cR -n white -cG -n blue -d 256 -o WhiteBlue_256x256.exr
