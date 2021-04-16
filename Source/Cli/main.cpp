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
    bool disableProgbar = false;
    bool pipe = false;
    const char* outputName = "noise.exr";
    ChannelParametes channels[4];
};

void printHeader()
{
    std::cout << ">>Noice<<" << std::endl;
    std::cout << "copyright (c) 2021 Kleber Garcia " << std::endl << std::endl;
}

void printExample()
{
    std::cout << "Usage: noice [-flag|--name=value|-flag=value]*  " << std::endl;
    std::cout << "Example that generates blue noise on red channel and white on green: " << std::endl;
    std::cout << "noice -W 256 -cR -n blue -cG -n white -o output.exr" << std::endl << std::endl;
}

bool prepareCliSchema(noice::ClParser& p, ArgParameters& object)
{
    #define Check(x) if (!(x)) { std::cerr << "Found duplicate schema name line number:" << __LINE__ << std::endl; return false; }
    using namespace noice;
    ArgParameters* objectPtr = &object;
    ClParser::GroupId generalGid = p.createGroup("General", "General parameters:");
    p.bind(generalGid, objectPtr);
    Check(p.addParam(generalGid, ClParser::ParamData(
        "Print help dialog.",
        "h", "help", 
        CliParamType::Bool, offsetof(ArgParameters, printHelp)
    )))
    Check(p.addParam(generalGid, ClParser::ParamData(
        "The texture name. This application always writes .exr files. The extension .exr gets automatically appended if not included.",
        "o", "output", 
        CliParamType::String, offsetof(ArgParameters, outputName)
    )))
    Check(p.addParam(generalGid, ClParser::ParamData(
        "Width of the target texture (defaults to 128)", "W", "width", 
        CliParamType::Uint, offsetof(ArgParameters, width)
    )))
    Check(p.addParam(generalGid, ClParser::ParamData(
        "Height of the target texture (defaults to width)", "H", "height", 
        CliParamType::Uint, offsetof(ArgParameters, height)
    )))
    Check(p.addParam(generalGid, ClParser::ParamData(
        "Depth of the target texture (defaults to 1 pixel, or a 2d texture)", "D", "depth", 
        CliParamType::Uint, offsetof(ArgParameters, depth)
    )))
    Check(p.addParam(generalGid, ClParser::ParamData(
        "Disables all the standard output, when writting a texture to a file", "q", "quiet", 
        CliParamType::Bool, offsetof(ArgParameters, quiet)
    )))
    Check(p.addParam(generalGid, ClParser::ParamData(
        "Disables the progress bar output", "g", "disable_progbar", 
        CliParamType::Bool, offsetof(ArgParameters, disableProgbar)
    )))
    Check(p.addParam(generalGid, ClParser::ParamData(
        "Streams the texture file through the standard output. Only binary data is output. "
        "When enabled this program automatically goes into quiet mode.", "p", "pipe", 
        CliParamType::Bool, offsetof(ArgParameters, pipe)
    )))
    Check(p.addParam(generalGid, ClParser::ParamData(
        "Number of software threads to utilize for computation (default is 16)",
        "t", "threads", 
        CliParamType::Bool, offsetof(ArgParameters, threadCount)
    )))

    ClParser::GroupId channelGid = p.createGroup("Channel", "Channel parameters:");
    Check(p.addParam(channelGid, ClParser::ParamData(
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
    )))
    Check(p.addParam(channelGid, ClParser::ParamData(
        "Random number seed utilized, for deterministic generation of noise.", "s", "seed", 
        CliParamType::Int, offsetof(ChannelParametes, seed)
    )))
    Check(p.addParam(channelGid, ClParser::ParamData(
        "Sets the noise type on the particular active channel.",
        "n", "noise", 
        CliParamType::String, offsetof(ChannelParametes, noiseType),
        { BLUE_TYPE, WHITE_TYPE }, nullptr
    )))

    return true;
}

struct ProgressInfo
{
    const char* msgPrefix = "";
    float maxPixels = 0.0f;
};

void printProgress(const noice::EventArguments& args)
{
    const auto* progressInfo = (const ProgressInfo*)args.userData;
    float perc = (float)args.pixelsProcessed / progressInfo->maxPixels;
    const int spaces = 30;
    const float spacesf = (const float)spaces;
    int progressCount = (int)(spacesf * perc);
    char spaceStr[spaces + 1];
    for (int i = 0; i < progressCount; ++i)
        spaceStr[i] = '|';
    for (int i = progressCount; i < spaces; ++i)
        spaceStr[i] = ' ';
    spaceStr[spaces] = '\0';
    bool isFinished = args.pixelsProcessed == progressInfo->maxPixels;
    std::cout << "\r [" << progressInfo->msgPrefix << "] progress: [" << spaceStr << "]" << std::flush;
    if (isFinished)
        std::cout << std::endl;
}

ReturnCodes work(const ArgParameters& parameters)
{
    const char* channelNames[] = { "r", "g", "b", "a" };

    noice::TextureFileDesc outDesc;
    outDesc.filename = parameters.outputName;
    std::vector<noice::TextureComponentHandle> usedHandles;
    ProgressInfo progressInfo;
    progressInfo.maxPixels = (float)parameters.width * parameters.height * parameters.depth;
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
            currentHandle = noice::createTextureComponent();
            if (!parameters.quiet && !parameters.disableProgbar)
            {
                progressInfo.msgPrefix = channelNames[i];
                noice::attachEventCallback(&printProgress, &progressInfo, 300, currentHandle);
            }
            noice::Error err = generateBlueNoise(bnd, parameters.threadCount, currentHandle);
            if (err != noice::Error::Ok)
            {
                std::cerr << "Generator error" << noice::getErrorString(err);
                return ReturnCodes::InternalError;
            }

            usedHandles.push_back(currentHandle);
        }
    }

    noice::Error err = saveTextureToFile(outDesc);
    if (err != noice::Error::Ok)
    {
        std::cerr << "Texture output error: " << noice::getErrorString(err);
        return ReturnCodes::IoError;
    }

    for (auto usedHandle : usedHandles)
        noice::deleteComponent(usedHandle);

    if (!parameters.quiet)
        std::cout << "Success." << std::endl;

    return ReturnCodes::Success;
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

    if (!parameters.quiet)
    {
        std::cout << "Generating '" << parameters.outputName << "'" << std::endl;
    }

    return (int)work(parameters);
}


