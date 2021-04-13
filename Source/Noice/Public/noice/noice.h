#pragma once

namespace noice
{

class OutputStream
{
public:
    virtual void write(const char* buffer, int bufferSize) = 0;
};

enum class Error
{
    Ok,
    BadArgs,
    CantOpenFile,
    HandleIsNull,
    OpaqueNotNull,
    InconsistentDimensions,
    NoInputImageSpecified,
    IoIssue
};

struct TextureComponentHandle
{
    void* opaquePtr = 0;
};

enum class Channels
{
    R, G, B, A, Count
};

struct BlueNoiseGenDesc
{
    int width  = 64;
    int height = 64;
    int depth  = 1;
    float rho2 = 2.1f;
    unsigned seed = 0xdeadbeef;
};

Error generateBlueNoise  (const BlueNoiseGenDesc& desc, int threadCount, TextureComponentHandle* component);
Error saveTextureToFile  (const TextureComponentHandle* channels[(int)Channels::Count], const char* filename);
Error saveTextureToStream(const TextureComponentHandle* channels[(int)Channels::Count], const char* filename, OutputStream& outStream);

void deleteComponent(TextureComponentHandle* component);

}
