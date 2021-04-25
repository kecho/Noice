## Noise v1.0.2

### Bug fix
* Improved produced blue noise dithering at the end of the spectrum by doing the 2 steps in void and cluster algorithm.

## Noise v1.0.1

### Features
* Added support for intel sse/sse2/avx2/avx512, ISPC chooses the best architecture for the CPU in place.

### Bug fix
* Fixed issue when specifying a non power of two thread count, it would produce black blue noise texture.
