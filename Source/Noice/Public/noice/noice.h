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
    InvalidFileName,
    IoIssue,
    Count
};

const char* getErrorString(Error err);

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

struct TextureFileDesc
{
    const char* filename = nullptr;
    TextureComponentHandle channels[4] = {};
};

Error generateBlueNoise  (const BlueNoiseGenDesc& desc, int threadCount, TextureComponentHandle& outputComponent);
Error saveTextureToFile  (const TextureFileDesc& desc);
Error saveTextureToStream(const TextureFileDesc& desc, OutputStream& outStream);

void deleteComponent(TextureComponentHandle& component);

}
