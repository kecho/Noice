#include <noice/noice.h>
#include "ClParser.h"
#include <iostream>


namespace
{

enum class ReturnCodes : int
{
    Success = 0,
    InternalError = 1,
    IoError = 2,
    BadCmdArgs = 3
};

#define BLUE_TYPE "blue"
#define WHITE_TYPE "white"

bool isAlias(std::string name, const char** aliases)
{
    const char** a = aliases;
    while (*a != nullptr)
    {
        if (name == *a)
            return true;
    }

    return false;
}

struct ChannelParametes
{
    bool enabled = false;
    int seed = 0x12feebade;
    const char* channelName = "";
    const char* noiseType = BLUE_TYPE;
    float rho2 = 2.1f;
};

struct ArgParameters
{
    int width = 128;
    int height = 0;
    int depth = 1;
    int threadCount = 16;
    bool printHelp = false;
    bool quiet = false;
    bool pipe = false;
    const char* outputName = "noise.exr";
    ChannelParametes channels[4];
};

void printHeader()
{
    std::cout << " noice - copyright (c) 2021 Kleber Garcia " << std::endl;
}

void printExample()
{
    std::cout << "Usage: noice [-flag|--name=value|-flag=value]*  " << std::endl;
    std::cout << "Example that generates blue noise on red channel and white on green: " << std::endl;
    std::cout << "noice -W 256 -cR -n blue -cG -n white -o output.exr" << std::endl << std::endl;
}

void prepareCliSchema(noice::ClParser& p, ArgParameters& object)
{
    using namespace noice;
    ArgParameters* objectPtr = &object;
    ClParser::GroupId generalGid = p.createGroup("General", "General parameters:");
    p.bind(generalGid, objectPtr);
    p.addParam(generalGid, ClParser::ParamData(
        "Print help dialog.",
        "h", "help", 
        CliParamType::Bool, offsetof(ArgParameters, printHelp)
    ));
    p.addParam(generalGid, ClParser::ParamData(
        "The texture name. This application always writes .exr files. The extension .exr gets automatically appended if not included.",
        "o", "output", 
        CliParamType::String, offsetof(ArgParameters, outputName)
    ));
    p.addParam(generalGid, ClParser::ParamData(
        "Width of the target texture (defaults to 128)", "W", "width", 
        CliParamType::Uint, offsetof(ArgParameters, width)
    ));   
    p.addParam(generalGid, ClParser::ParamData(
        "Height of the target texture (defaults to width)", "H", "height", 
        CliParamType::Uint, offsetof(ArgParameters, height)
    ));
    p.addParam(generalGid, ClParser::ParamData(
        "Depth of the target texture (defaults to 1 pixel, or a 2d texture)", "D", "depth", 
        CliParamType::Uint, offsetof(ArgParameters, depth)
    ));   
    p.addParam(generalGid, ClParser::ParamData(
        "Disables all the standard output, when writting a texture to a file", "q", "quiet", 
        CliParamType::Bool, offsetof(ArgParameters, quiet)
    ));
    p.addParam(generalGid, ClParser::ParamData(
        "Streams the texture file through the standard output. Only binary data is output. "
        "When enabled this program automatically goes into quiet mode.", "p", "pipe", 
        CliParamType::Bool, offsetof(ArgParameters, pipe)
    ));
    p.addParam(generalGid, ClParser::ParamData(
        "Number of software threads to utilize for computation (default is 16)",
        "t", "threads", 
        CliParamType::Bool, offsetof(ArgParameters, threadCount)
    ));

    ClParser::GroupId channelGid = p.createGroup("Channel", "Channel parameters:");
    p.addParam(channelGid, ClParser::ParamData(
        "Sets the channel for which all the following channel parameters will affect.\n"
        "\tFor example, doing -cR means that we activate red channel and all the following\n"
        "\tchannel switches will affect such channel.\n"
        "\tWhen the command line encounters a new -cG then it will switch to Green, and so on.",
        "c", "channel",
        CliParamType::String, offsetof(ChannelParametes, channelName), { "R", "r", "red", "G", "g", "green", "B", "b", "blue", "A", "a", "alpha", "x", "y", "z", "w" },
        [objectPtr, &p](const ClParser::ParamData& paramData, ClParser::GroupId gid, const void* value){
            std::string choice = (const char*)value;
            int activeChannel = 0;
            if (choice == "R" || choice == "r" || choice == "x" || choice == "red")
                activeChannel = 0;
            else if (choice == "G" || choice == "g" || choice == "y" || choice == "green")
                activeChannel = 1;
            else if (choice == "B" || choice == "b" || choice == "z" || choice == "blue")
                activeChannel = 2;
            else if (choice == "A" || choice == "a" || choice == "w" || choice == "alpha")
                activeChannel = 3;
            objectPtr->channels[activeChannel].enabled = true;
            p.bind(gid, &objectPtr->channels[activeChannel]);
        }
    ));
    p.addParam(channelGid, ClParser::ParamData(
        "Random number seed utilized, for deterministic generation of noise.", "s", "seed", 
        CliParamType::Int, offsetof(ChannelParametes, seed)
    ));
    p.addParam(channelGid, ClParser::ParamData(
        "Sets the noise type on the particular active channel.",
        "n", "noise", 
        CliParamType::String, offsetof(ChannelParametes, noiseType),
        { BLUE_TYPE, WHITE_TYPE }, nullptr
    ));
}

}

int main(int argc, char* argv[])
{
    noice::ClParser parser;
    parser.setOnErrorCallback([](const std::string& msg)
    {
        std::cerr << "Command line error: " << msg << std::endl;
        std::cerr << "For documentation on usage, use --help or -h.";
    });

    ArgParameters parameters;
    prepareCliSchema(parser, parameters);

    if (!parser.parse(argc, argv))
        return (int)ReturnCodes::BadCmdArgs;

    if (!parameters.quiet)
    {
        printHeader();
        if (parameters.printHelp)
        {
            printExample();
            parser.prettyPrintHelp();
            return (int)ReturnCodes::Success;
        }
    }

    if (parameters.height == 0)
        parameters.height = parameters.width;


    bool useDefaultChannel = true;
    for (int i = 0; i < 4; ++i)
    {
        if (parameters.channels[i].enabled)
        {
            useDefaultChannel = false;
            break;
        }
    }

    if (useDefaultChannel)
    {
        parameters.channels[0].enabled = true;
    }

    noice::TextureFileDesc outDesc;
    outDesc.filename = parameters.outputName;
    std::vector<noice::TextureComponentHandle> usedHandles;
    std::cout << "Generating '" << outDesc.filename << "'" << std::endl;
    for (int i = 0; i < 4; ++i)
    {
        const ChannelParametes& channelParmeters = parameters.channels[i];
        noice::TextureComponentHandle& currentHandle = outDesc.channels[i];
        if (!channelParmeters.enabled)
            continue;

        std::string noiseType = channelParmeters.noiseType;
        if (noiseType == BLUE_TYPE) 
        {
            noice::BlueNoiseGenDesc bnd;
            bnd.width  = (int)parameters.width;
            bnd.height = (int)parameters.height;
            bnd.depth  = (int)parameters.depth;
            bnd.seed = (unsigned)channelParmeters.seed;
            bnd.rho2 = channelParmeters.rho2;
            noice::Error err = generateBlueNoise(bnd, parameters.threadCount, currentHandle);
            if (err != noice::Error::Ok)
            {
                std::cerr << "Internal error: " << (int)err;
                return (int)ReturnCodes::InternalError;
            }

            usedHandles.push_back(currentHandle);
        }
    }

    noice::Error err = saveTextureToFile(outDesc);
    if (err != noice::Error::Ok)
    {
        std::cerr << "Internal error: " << (int)err;
        return (int)ReturnCodes::IoError;
    }

    for (auto usedHandle : usedHandles)
        noice::deleteComponent(usedHandle);

    return (int)ReturnCodes::Success;
}

