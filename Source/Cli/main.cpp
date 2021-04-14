#include <noice/noice.h>
#include "ClParser.h"
#include <iostream>


namespace
{

enum class ReturnCodes : int
{
    Success = 0,
    BadCmdArgs
};

#define BLUE_TYPE "blue-noise"
const char* blueNoiseAlias[] = { "blue", "b", nullptr };

#define WHITE_TYPE "white-noise"
const char* whiteNoiseAlias[] = { "white", "w", nullptr };

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
    const char* channelName = "";
    const char* noiseType = BLUE_TYPE;
    float phi2 = 2.1f;
};

struct ArgParameters
{
    int width = 128;
    int height = 0;
    int depth = 0;
    int seed = 0x12feebade;
    bool printHelp = false;
    bool quiet = false;
    bool pipe = false;
    const char* outputName = "noise.exr";
    ChannelParametes channels[4];
};

void printHeader()
{
    std::cout << "   +-----------------------------------+" << std::endl;
    std::cout << "   |               noice               |" << std::endl;
    std::cout << "   | A texture noise generator utility |" << std::endl;
    std::cout << "   | Copyright (c) 2021 Kleber Garcia  |" << std::endl;
    std::cout << "   +-----------------------------------+" << std::endl << std::endl;
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
        "Random number seed utilized, for deterministic generation of noise.", "s", "seed", 
        CliParamType::Int, offsetof(ArgParameters, seed)
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
        "Sets the blue noise type on the particular active channel.",
        "n", "noise", 
        CliParamType::String, offsetof(ChannelParametes, noiseType),
        { "blue", "white" }, nullptr
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

    if (parameters.printHelp)
    {
        printHeader();
        parser.prettyPrintHelp();
        return (int)ReturnCodes::Success;
    }

    if (parameters.height == 0)
        parameters.height = parameters.width;


    for (int i = 0; i < 4; ++i)
    {
        if (parameters.channels[i].enabled)
            std::cout << "Channel " << i << " enabled" << std::endl;
    }
    return (int)ReturnCodes::Success;
}

