#include <noice/noice.h>
#include <ImfImage.h>
#include <ImfOutputFile.h>
#include <ImfIO.h>
#include <iostream>
#include "Image.h"
#include "ImageStream.h"

namespace noice
{

class StreamWrapper : public Imf::OStream
{
public:
    StreamWrapper(OutputStream& ostream, const char* filename) : m_ostream(ostream), OStream(filename) {}

    virtual void write (const char c[], int n) 
    {
        m_ostream.write(c, n);
        m_p += n;
    }

    virtual Imath::Int64 tellp ()
    {
        return (Imath::Int64)m_p;
    }

    virtual void seekp (Imath::Int64 pos) 
    {
        m_p = pos;
    }

private:
    Imath::Int64 m_p = 0;
    OutputStream& m_ostream;

};

Error streamOutImage(
    const char* filename,
    OutputStream& output,
    int width, int height, int depth,
    const Channel* rgba[4])
{
    if (width == 0 || height == 0 || depth == 0)
        return Error::BadArgs;
    
    const char* channelNames[] = { "R", "G", "B", "A" };
    StreamWrapper sw(output, filename);
    int scanLines = height * depth;
    Imf::Header header(width, scanLines);

    for (int c = 0; c < 4; ++c)
    {
        if (rgba[c] == nullptr || rgba[c]->image == nullptr)
            continue;

        header.channels().insert(channelNames[c], Imf::Channel(Imf::FLOAT));
    }

    try {
        Imf::OutputFile file(sw, header);
        Imf::FrameBuffer frameBuffer;
        for (int c = 0; c < 4; ++c)
        {
            if (rgba[c] == nullptr || rgba[c]->image == nullptr)
                continue;

            frameBuffer.insert(
                channelNames[c], Imf::Slice(
                    Imf::FLOAT, (char*)rgba[c]->image->img().data,
                    sizeof(float),
                    sizeof(float) * width));

        }

        file.setFrameBuffer(frameBuffer);
        file.writePixels(scanLines);
    } catch (const std::exception& exc) {
        std::cerr << exc.what() << std::endl;
        return Error::IoIssue;
    }

    return Error::Ok;
}

}
