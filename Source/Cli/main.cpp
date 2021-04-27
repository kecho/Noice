#include <noice/noice.h>
#include <ClParser.h>
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
#define PERLIN_TYPE "perlin"
#define DEFAULT_NOISE_TYPE PERLIN_TYPE

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
    int seed = 0x1fee1bad;
    const char* channelName = "";
    const char* noiseType = DEFAULT_NOISE_TYPE;
    float rho2 = 2.1f;

    const char* perlinFrequenciesString = "";
    const char* perlinWeightsString   = "";
    std::vector<float> perlinFrequencies;
    std::vector<float> perlinWeights;

    bool readPerlinFrequencies()
    {
        if (perlinFrequenciesString == nullptr || !*perlinFrequenciesString)
            return true;

        std::string ss = perlinFrequenciesString;
        std::vector<float> result;
        if (!noice::ClTokenizer::parseFloatList(result, ss, ':'))
            return false;

        perlinFrequencies = std::move(result);
        return true;
    }

    bool readPerlinWeights()
    {
        if (perlinWeightsString == nullptr || !*perlinWeightsString)
            return true;

        std::string ss = perlinWeightsString;
        std::vector<float> result;
        if (!noice::ClTokenizer::parseFloatList(result, ss, ':'))
            return false;

        perlinWeights = std::move(result);
        return true;
    }

};

struct ArgParameters
{
    const char* dimensionsString = "128";
    int width  = -1;
    int height = -1;
    int depth  = -1;
    int threadCount = 16;
    bool printHelp = false;
    bool quiet = false;
    bool disableProgbar = false;
    bool pipe = false;
    const char* outputName = "noise.exr";
    ChannelParametes channels[4];

    bool readDimensions()
    {
        if (dimensionsString == nullptr)
            return false;

        std::string ds = dimensionsString;
        std::vector<int> result;
        if (!noice::ClTokenizer::parseIntList(result, ds, 'x'))
            return false;

        if (result.empty() || result.size() > 3)
            return false;

        width = result[0];
        if (result.size() >= 2)
            height = result[1];
        else
            height = width;

        if (result.size() >= 3)
            depth = result[2];
        else
            depth = 1;

        return true;
    }
};

class StdStreamOut : public noice::OutputStream
{
public:
    StdStreamOut() { fdopen(fileno(stdout), "wb"); }
    virtual void write(const char* buffer, int bufferSize)
    {
        fwrite(buffer, 1, bufferSize, stdout);
    }

private:
    FILE* m_stdout = nullptr;
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
    std::cout << "noice -d 256 -cR -n blue -cG -n white -o output.exr" << std::endl << std::endl;
}

bool prepareCliSchema(noice::ClParser& p, ArgParameters& object)
{
    using namespace noice;
    ArgParameters* objectPtr = &object;
    ClParser::GroupId generalGid = p.createGroup("General", "General parameters:");
    ClParser::GroupId channelGid = p.createGroup("Channel", "Channel parameters:");

    p.bind(generalGid, objectPtr);
    p.bind(channelGid, &objectPtr->channels);

    CliSwitch(generalGid, 
        "Print help dialog.",
        "h", "help", 
        Bool, ArgParameters, printHelp);

    CliSwitch(generalGid,
        "The texture name. This application always writes .exr files. The extension .exr gets automatically appended if not included.",
        "o", "output", 
        String, ArgParameters, outputName);

    CliSwitch(generalGid, 
        "Dimension string describing the output texture. Each dimension size must be separated by the 'x' character."
        " '-d 32' would mean a texture of 32x32x1. '-d 32x15' would mean a texture of 32x15x1. '-d 15x16x15' would mean a texture of 15x16x15 and so on.", "d", "dimensions", 
        String, ArgParameters, dimensionsString);

    CliSwitch(generalGid, 
        "Disables all the standard output, when writting a texture to a file", "q", "quiet", 
        Bool, ArgParameters, quiet);

    CliSwitch(generalGid,
        "Disables the progress bar output", "g", "disable_progbar", 
        Bool, ArgParameters, disableProgbar);

    //Turning off feature for now:
    //TODO figure out how to do this cross platform: turns out it isnt as easy :)
    //CliSwitch(generalGid,
    //    "Streams the texture file through the standard output. Only binary data is output. "
    //    "When enabled this program automatically goes into quiet mode.", "p", "pipe", 
    //    Bool, ArgParameters, pipe);

    CliSwitch(generalGid, 
        "Number of software threads to utilize for computation (default is 16)",
        "t", "threads", 
        Uint, ArgParameters, threadCount);

    std::vector<std::string> channelEnumNames = { "R", "r", "red", "G", "g", "green", "B", "b", "blue", "A", "a", "alpha", "x", "y", "z", "w" };
    auto onChannelSet = [objectPtr, &p](const ClParser::ParamData& paramData, ClParser::GroupId gid, const void* value){
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
            p.bind(gid, &objectPtr->channels[activeChannel]);};

    CliSwitchAction(channelGid,
        "Sets the channel for which all the following channel parameters will affect.\n"
        "\tFor example, doing -cR means that we activate red channel and all the following\n"
        "\tchannel switches will affect such channel.\n"
        "\tWhen the command line encounters a new -cG then it will switch to Green, and so on.",
        "c", "channel",
        String, ChannelParametes, channelName, channelEnumNames, onChannelSet);

    CliSwitch(channelGid,
        "Random number seed utilized, for deterministic generation of noise.", "s", "seed", 
        Int, ChannelParametes, seed);

    std::vector<std::string> noiseTypes = { BLUE_TYPE, WHITE_TYPE, PERLIN_TYPE };
    CliSwitchAction(channelGid,
        "Sets the noise type on the particular active channel.",
        "n", "noise", 
        String, ChannelParametes, noiseType,
        noiseTypes, nullptr);

    CliSwitch(channelGid,
        "Colon separated values with the perlin noise grid frequencies. For example, 3:4.0:24.0:64.0 etc.\n"
        "\tIf set, the weights must be set as well, with one corresponding weight per frequency.",
        "f", "frequencies", 
        String, ChannelParametes, perlinFrequenciesString);

    CliSwitch(channelGid,
        "Colon separated numbers with the perlin noise grid weights corresponding to each frequency.\n"
        "\tThe weights must correspond to one per frequency.",
        "w", "weights", 
        String, ChannelParametes, perlinWeightsString);

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
    noice::Stopwatch stopwatch = {};
    for (int i = 0; i < 4; ++i)
    {
        const ChannelParametes& channelParmeters = parameters.channels[i];
        noice::TextureComponentHandle& currentHandle = outDesc.channels[i];
        if (!channelParmeters.enabled)
            continue;

        std::string noiseType = channelParmeters.noiseType;
        
        noice::TextureComponentDesc componentDesc;
        componentDesc.width  = (int)parameters.width;
        componentDesc.height = (int)parameters.height;
        componentDesc.depth  = (int)parameters.depth;
        currentHandle = noice::createTextureComponent(componentDesc);
        if (!parameters.quiet && !parameters.disableProgbar)
        {
            progressInfo.msgPrefix = channelNames[i];
            noice::attachEventCallback(currentHandle, &printProgress, &progressInfo, 300);
            std::cout << std::endl;
        }

        if (!parameters.quiet)
        {
            stopwatch = {};
            noice::attachStopwatch(currentHandle, &stopwatch);
        }

        noice::Error err = noice::Error::Ok;
        if (noiseType == BLUE_TYPE) 
        {
            noice::BlueNoiseGenDesc desc;
            desc.seed = (unsigned)channelParmeters.seed;
            desc.rho2 = channelParmeters.rho2;
            err = generateBlueNoise(currentHandle, desc, parameters.threadCount);
        }
        else if (noiseType == WHITE_TYPE)
        {
            noice::WhiteNoiseGenDesc desc;
            desc.seed = (unsigned)channelParmeters.seed;
            err = generateWhiteNoise(currentHandle, desc, parameters.threadCount);
        }
        else if (noiseType == PERLIN_TYPE)
        {
            noice::PerlinNoiseGenDesc desc;
            desc.seed = (unsigned)channelParmeters.seed;
            desc.frequencies = channelParmeters.perlinFrequencies.data();
            desc.weights = channelParmeters.perlinWeights.data();
            desc.frequencyCounts = (int)channelParmeters.perlinFrequencies.size();
            err = generatePerlinNoise(currentHandle, desc, parameters.threadCount);
        }

        if (!parameters.quiet)
            std::cout << " " << channelNames[i] << " channel finished, time: " << (float)stopwatch.microseconds / (1000.0f*1000.0f) << " seconds" << std::endl;

        if (err != noice::Error::Ok)
        {
            std::cerr << "Generator error" << noice::getErrorString(err);
            return ReturnCodes::InternalError;
        }

        usedHandles.push_back(currentHandle);
    }

    noice::Error err = noice::Error::Ok;
    if (parameters.pipe)
    {
        StdStreamOut stdStreamOut;
        err = saveTextureToStream(outDesc, stdStreamOut);
    }
    else
    {
        err = saveTextureToFile(outDesc);
    }

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

ReturnCodes validateAndProcessParameters(ArgParameters& parameters)
{
    //parameter validation
    for (auto& channel : parameters.channels)
    {
        if (!channel.enabled)
            continue;

        if (!channel.readPerlinFrequencies())
        {
            std::cerr << "Could not read perlin frequencies (f). Must be a list of ':' separated numbers. Got \"" << channel.perlinFrequenciesString << "\" instead." <<std::endl;
            return ReturnCodes::BadCmdArgs;
        }

        if (!channel.readPerlinWeights())
        {
            std::cerr << "Could not read perlin weights (w). Must be a list of ':' separated numbers. Got \"" << channel.perlinWeightsString << "\" instead." <<std::endl;
            return ReturnCodes::BadCmdArgs;
        }

        if (channel.perlinWeights.size() != channel.perlinFrequencies.size())
        {
            std::cerr << "The perlin list of frequencies and weights must match in count. " << std::endl;
            return ReturnCodes::BadCmdArgs;
        }
        for (auto f : channel.perlinFrequencies)
        {
            if (f <= 0.0f)
            {
                std::cerr << "Found negative or 0 frequency in the list of frequencies. All frequencies must be greater than 0. " << std::endl;
                return ReturnCodes::BadCmdArgs;
            }

            if (f > 4000)
            {
                std::cerr << "Found an extremely high perlin frequency, please dont use values greater than 4000. " << std::endl;
                return ReturnCodes::BadCmdArgs;
            }
        }
        for (auto f : channel.perlinWeights)
        {
            if (f < 0.0f)
            {
                std::cerr << "Found negative in the list of weights. All weights must be greater than or equal to 0. " << std::endl;
                return ReturnCodes::BadCmdArgs;
            }
        }
    }

    if (parameters.width <= 0)
    {
        std::cerr << "Width cannot be less or equal to 0." << std::endl;
        return ReturnCodes::BadCmdArgs;
    }

    if (parameters.height <= 0)
    {
        std::cerr << "Height cannot be less or equal to 0." << std::endl;
        return ReturnCodes::BadCmdArgs;
    }

    if (parameters.depth <= 0)
    {
        std::cerr << "Depth cannot be less or equal to 0." << std::endl;
        return ReturnCodes::BadCmdArgs;
    }

    if (parameters.width > 4096)
    {
        std::cerr << "Width cannot be greater than 4096." << std::endl;
        return ReturnCodes::BadCmdArgs;
    }

    if (parameters.height > 4096)
    {
        std::cerr << "Height cannot be greater than 4096." << std::endl;
        return ReturnCodes::BadCmdArgs;
    }

    if (parameters.depth > 4096)
    {
        std::cerr << "Depth cannot be greater than 4096." << std::endl;
        return ReturnCodes::BadCmdArgs;
    }

    return ReturnCodes::Success;
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
    {
        return (int)ReturnCodes::BadCmdArgs;
    }
    
    if (parameters.pipe)
        parameters.quiet = true;

    if (!parameters.readDimensions())
    {
        std::cerr << "Failed to parse dimensions (d). String passed was \"" << parameters.dimensionsString << "\" " << std::endl;
        return (int)ReturnCodes::BadCmdArgs;
    }

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

    auto validationResult = validateAndProcessParameters(parameters);
    if (validationResult != ReturnCodes::Success)
        return (int)validationResult;

    if (!parameters.quiet)
    {
        std::cout << "Generating " << parameters.width << "x" << parameters.height;
        if (parameters.depth > 1) 
            std::cout << "x" << parameters.depth;
        std::cout << " [";
        std::cout << (parameters.channels[0].enabled ? "r" : "")
            << (parameters.channels[1].enabled ? "g" : "")
            << (parameters.channels[2].enabled ? "b" : "")
            << (parameters.channels[3].enabled ? "a" : "");
        std::cout << "] - " << parameters.outputName << std::endl;
    }

    return (int)work(parameters);
}


