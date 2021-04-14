#include <noice/noice.h>
#include <iostream>
#include <fstream>
#include <chrono>
#include "Image.h"
#include "BlueNoiseGenerator.h"
#include "ImageStream.h"
#include <stdio.h>
#include <fcntl.h>
#include <io.h>


namespace noice
{

class StdStreamOut : public OutputStream
{
public:
    StdStreamOut() { setmode(fileno(stdout), O_BINARY); }
    virtual void write(const char* buffer, int bufferSize)
    {
        ::write(::fileno(stdout), buffer, bufferSize);
    }
};

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

Error generateBlueNoise(const BlueNoiseGenDesc& desc, int threadCount, TextureComponentHandle* component)
{
    if (component == nullptr)
        return Error::HandleIsNull;

    if (component->opaquePtr != 0u)
        return Error::OpaqueNotNull;

    auto img = new noice::Image();
    *component = img->asHandle();

    return blueNoiseGenerator(desc, threadCount, *img);
}

static bool isValidFileName(const TextureFileDesc& desc)
{
    if (desc.filename == nullptr || desc.filename[0] == '\0')
        return false;
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
    for (int c = 0; c < (int)Channels::Count; ++c)
    {
        const TextureComponentHandle* component = desc.channels[c];
        if (component == nullptr || component->opaquePtr == 0)
            continue;

        Image* img = Image::get(*component);
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

void deleteComponent(TextureComponentHandle* component)
{
    if (component == nullptr || component->opaquePtr == nullptr)
        return;

    Image* img = Image::get(*component);
    delete img;
    *component = TextureComponentHandle();
}

}
