#include <noice/noice.h>
#include <iostream>
#include <fstream>
#include <chrono>
#include "Image.h"
#include "BlueNoiseGenerator.h"
#include "WhiteNoiseGenerator.h"
#include "PerlinNoiseGenerator.h"
#include "DftGenerator.h"
#include "ImageStream.h"
#include <stdio.h>

namespace noice
{

const char* g_noiceVersion = "1.0.0";

static const char* sErrorString[(int)Error::Count] = 
{
    "Ok",
    "bad args",
    "No pixels",
    "Corrupted Handle",
    "Cant open file",
    "Handle is null",
    "Opaque not null",
    "Inconsistent dimensions",
    "No input image specified",
    "Invalid file name",
    "Io issue"
};

const char* getErrorString(Error err)
{
    if ((int)err >= (int)Error::Count)
        return "Unknown error";

    return sErrorString[(int)err];
}

class FileStreamOut : public OutputStream
{
public:
    FileStreamOut(const char* filename)
    : m_filename(filename)
    {
    }

    bool open()
    {
        m_outstream.open(m_filename, std::ios::out | std::ios::trunc | std::ios::binary);
        return m_outstream.is_open();
    }

    void close()
    {
        m_outstream.close();
    }

    virtual void write(const char* buffer, int bufferSize)
    {
        m_outstream.write(buffer, bufferSize);
    }

private:
    std::ofstream m_outstream;
    std::string m_filename;
};

TextureComponentHandle createTextureComponent(const TextureComponentDesc& componentDesc)
{
    auto img = new noice::Image();
    img->init(componentDesc.width, componentDesc.height, componentDesc.depth, 1);
    return img->asHandle();
}

Error generateBlueNoise(TextureComponentHandle component, const BlueNoiseGenDesc& desc, int threadCount)
{
    if (component.opaquePtr == nullptr)
        return Error::HandleIsNull;

    Image* img = Image::get(component);
    if (img == nullptr)
        return Error::CorruptedHandle;

    return blueNoiseGenerator(desc, threadCount, *img);
}

Error generateWhiteNoise(TextureComponentHandle component, const WhiteNoiseGenDesc& desc, int threadCount)
{
    if (component.opaquePtr == nullptr)
        return Error::HandleIsNull;

    Image* img = Image::get(component);
    if (img == nullptr)
        return Error::CorruptedHandle;

    return whiteNoiseGenerator(desc, threadCount, *img);
}

Error generatePerlinNoise(TextureComponentHandle component, const PerlinNoiseGenDesc& desc, int threadCount)
{
    if (component.opaquePtr == nullptr)
        return Error::HandleIsNull;

    Image* img = Image::get(component);
    if (img == nullptr)
        return Error::CorruptedHandle;

    return perlinNoiseGenerator(desc, threadCount, *img);
}

static bool isValidFileName(const TextureFileDesc& desc)
{
    if (desc.filename == nullptr || desc.filename[0] == '\0')
        return false;

    return true;
}

Error saveTextureToStream(const TextureFileDesc& desc, OutputStream& outStream)
{
    if (!isValidFileName(desc))
        return Error::InvalidFileName;

    Channel rgbaContent[4];
    const Channel* rgbaArgs[4] = { nullptr };
    int width  = -1;
    int height = -1;
    int depth  = -1;
    for (int c = 0; c < 4; ++c)
    {
        const TextureComponentHandle& component = desc.channels[c];
        if (component.opaquePtr == nullptr)
            continue;

        Image* img = Image::get(component);
        if (width == -1)
            width = img->img().width;
        if (height == -1)
            height = img->img().height;
        if (depth == -1)
            depth = img->img().depth;

        if (width != img->img().width || height != img->img().height || depth != img->img().depth)
            return Error::InconsistentDimensions;

        rgbaContent[c].image = img;
        rgbaArgs[c] = &rgbaContent[c];
    }

    if (width == -1)
        return Error::NoInputImageSpecified;

    return streamOutImage(desc.filename,
            outStream, width, height, depth, rgbaArgs);
}

Error saveTextureToFile(const TextureFileDesc& desc)
{
    if (!isValidFileName(desc))
        return Error::InvalidFileName;
        
    FileStreamOut fileStream(desc.filename);
    if (!fileStream.open())
        return Error::CantOpenFile;
    Error err = saveTextureToStream(desc, fileStream);
    fileStream.close();
    return err;
}

Error loadTextureFromFile(const char* filename, TextureComponentHandle outputChannels[4])
{
    Channel channels[4];
    Error ret = streamInImage(filename, channels);
    for (int c = 0; c < 4; ++c)
    {
        if (channels[c].image == nullptr)
            continue;
        
        outputChannels[c] = channels[c].image->asHandle();
    }
    return ret;
}

Error generateDft (TextureComponentHandle componentInput, TextureComponentHandle outputHandles[2])
{
    if (!componentInput.valid())
        return Error::HandleIsNull;

    Image* inputTexture = Image::get(componentInput);
    Image* outputTextures[2] = { nullptr, nullptr };
    for (int i = 0; i < 2; ++i)
    {
        auto& outputHandle = outputHandles[i];
        if (!outputHandle.valid())
        {
            TextureComponentDesc desc;
            desc.width  = inputTexture->width();
            desc.height = inputTexture->height();
            desc.depth  = inputTexture->depth();
            outputHandle = createTextureComponent(desc);
        }

        outputTextures[i] = Image::get(outputHandle);
    }

    return dftGenerator(*inputTexture, outputTextures);
}

float* getPixels(TextureComponentHandle component)
{
    if (component.opaquePtr == nullptr)
        return nullptr;

    Image* img = Image::get(component);
    if (img == nullptr || img->pixelCount() == 0)
        return nullptr;

    return img->img().data;
}

void deleteComponent(TextureComponentHandle& component)
{
    if (component.opaquePtr == nullptr)
        return;

    Image* img = Image::get(component);
    delete img;
    component = TextureComponentHandle();
}

void attachEventCallback(TextureComponentHandle component, EventCallback callback, void* userData, int sampleFrequency)
{
    if (component.opaquePtr == nullptr)
        return;

    Image* img = Image::get(component);
    img->attachEventCb(callback, sampleFrequency, userData);
}

void attachStopwatch(TextureComponentHandle component, Stopwatch* stopwatchObject)
{
    if (component.opaquePtr == nullptr)
        return;

    Image* img = Image::get(component);
    img->attachStopwatchObject(stopwatchObject);
}

}
