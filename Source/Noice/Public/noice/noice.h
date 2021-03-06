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
    bool valid() const { return opaquePtr != 0; }
};

struct TextureComponentDesc
{
    int width  = 64;
    int height = 64;
    int depth  = 1;
};

struct BlueNoiseGenDesc
{
    float rho2 = 2.1f;
    unsigned seed = 0xdeadbeef;
};

struct WhiteNoiseGenDesc
{
    unsigned seed = 0xdeadbeef;
};

struct PerlinNoiseGenDesc
{
    unsigned seed = 0xdeadbeef;
    const float* frequencies = nullptr;
    const float* weights = nullptr;
    int frequencyCounts = 0;
};

struct TextureFileDesc
{
    const char* filename = nullptr;
    TextureComponentHandle channels[4] = {};
};

struct DftOptions
{
    int threadCount = 16;
};

//////////////////////////////////////
// Main API to build noise textures //
//////////////////////////////////////

TextureComponentHandle createTextureComponent(const TextureComponentDesc& componentDesc);
float* getPixels(TextureComponentHandle component);
void deleteComponent(TextureComponentHandle& component);
Error generateWhiteNoise    (TextureComponentHandle component, const WhiteNoiseGenDesc& desc,  int threadCount);
Error generateBlueNoise     (TextureComponentHandle component, const BlueNoiseGenDesc& desc,   int threadCount);
Error generatePerlinNoise   (TextureComponentHandle component, const PerlinNoiseGenDesc& desc, int threadCount);
Error saveTextureToFile     (const TextureFileDesc& desc);
Error saveTextureToStream   (const TextureFileDesc& desc, OutputStream& outStream);
Error loadTextureFromFile   (const char* fileName, TextureComponentHandle components[4]);
Error generateDft           (TextureComponentHandle componentInput, TextureComponentHandle outputHandles[2], const DftOptions& options);


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
