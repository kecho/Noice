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
    NoPixels,
    CorruptedHandle,
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

TextureComponentHandle createTextureComponent();
void deleteComponent(TextureComponentHandle& component);
Error generateBlueNoise     (const BlueNoiseGenDesc& desc, int threadCount, TextureComponentHandle component);
Error saveTextureToFile     (const TextureFileDesc& desc);
Error saveTextureToStream   (const TextureFileDesc& desc, OutputStream& outStream);

//event handles, to track progress or even cancel jobs.
struct EventArguments
{
    void* userData;
};

typedef void (*EventCallback)(const EventArguments&);
void attachEventHandle(EventCallback callback, void* userData, TextureComponentHandle handle);


}
