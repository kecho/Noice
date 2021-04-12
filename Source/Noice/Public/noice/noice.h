#pragma once

namespace noice
{

class OutputStream
{
public:
    virtual void write(const char* buffer, int bufferSize) = 0;
};

struct BlueNoiseGenDesc
{
    int width  = 64;
    int height = 64;
    int depth  = 1;
    float rho2 = 2.1f;
    unsigned seed = 0xdeadbeef;
};

enum class Error
{
    Ok,
    IoIssue
};

Error blueNoise(
    const char* fileName,
    const BlueNoiseGenDesc& desc,
    int threadCount);

Error blueNoise(
    OutputStream& outStream,
    const BlueNoiseGenDesc& desc,
    int threadCount);

void makeBlueNoise(int w, int h, int d, int threadCount);

}
