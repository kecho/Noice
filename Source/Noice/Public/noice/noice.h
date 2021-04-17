#pragma once

namespace noice
{

//////////////////////////////////
// Support types for main  API  //
//////////////////////////////////

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

struct WhiteNoiseGenDesc
{
    int width  = 64;
    int height = 64;
    int depth  = 1;
    unsigned seed = 0xdeadbeef;
};

struct TextureFileDesc
{
    const char* filename = nullptr;
    TextureComponentHandle channels[4] = {};
};

//////////////////////////////////////
// Main API to build noise textures //
//////////////////////////////////////

TextureComponentHandle createTextureComponent();
void deleteComponent(TextureComponentHandle& component);
Error generateWhiteNoise    (TextureComponentHandle component, const WhiteNoiseGenDesc& desc, int threadCount);
Error generateBlueNoise     (TextureComponentHandle component, const BlueNoiseGenDesc& desc, int threadCount);
Error saveTextureToFile     (const TextureFileDesc& desc);
Error saveTextureToStream   (const TextureFileDesc& desc, OutputStream& outStream);


////////////////////////////////////////////////////////
//Attachments to texture components for various things//
////////////////////////////////////////////////////////

struct EventArguments
{
    void* userData;
    int pixelsProcessed;
};

typedef void (*EventCallback)(const EventArguments&);
void attachEventCallback(TextureComponentHandle handle, EventCallback callback, void* userData, int sampleFrequency);

struct Stopwatch
{
    unsigned int microseconds = 0ull;
};
void attachStopwatch(TextureComponentHandle handle, Stopwatch* stopwatchObject);


}
