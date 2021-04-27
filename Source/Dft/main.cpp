#include <noice/noice.h>
#include <ClParser.h>
#include <sstream>
#include <iostream>
#include <string>

namespace 
{

enum class ReturnCodes : int
{
    Success = 0,
    IoError = 1,
    BadCmdArgs = 2,
    DftError = 3
};

struct ArgParameters
{
    const char* inputExr = nullptr;
    const char* outputPrefix = nullptr;
    unsigned threadCount = 16;
    bool quiet = false;
    bool printHelp = false;
};

void printHeader()
{
    std::cout << ">>Noice-Dft<<" << std::endl;
    std::cout << "copyright (c) 2021 Kleber Garcia " << std::endl << std::endl;
}

bool prepareCliSchema(noice::ClParser& p, ArgParameters& object)
{
    using namespace noice;
    ClParser::GroupId generalGid = p.createGroup("General", "General parameters:");
    p.bind(generalGid, &object);
    CliSwitch(generalGid, "Print help dialog", "h", "help", Bool, ArgParameters, printHelp);
    CliSwitch(generalGid, "Source input exr file", "i", "input", String, ArgParameters, inputExr);
    CliSwitch(generalGid, "Output prefix for output dft files", "p", "prefix", String, ArgParameters, outputPrefix);
    CliSwitch(generalGid, "Disables all the standard output.", "q", "quiet", Bool, ArgParameters, quiet);
    CliSwitch(generalGid, 
        "Number of software threads to utilize for computation (default is 16)",
        "t", "threads", 
        Uint, ArgParameters, threadCount);

    return true;
}

void printExample()
{
    std::cout << "Usage: noice-dft [-flag|--name=value|-flag=value]*  " << std::endl;
    std::cout << "Example that creates a dft analysis for an input noise exr " << std::endl;
    std::cout << "noice-dft -i noise.exr " << std::endl << std::endl;
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
    {
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

    if (parameters.inputExr == nullptr)
    {
        std::cerr << "Must specify a valid exr input file." << std::endl;
        return (int)ReturnCodes::BadCmdArgs;
    }

    noice::TextureComponentHandle components[4];
    noice::Error err = noice::loadTextureFromFile(parameters.inputExr, components);
    if (err != noice::Error::Ok)
    {
        std::cerr << "Failed to read texture file. Error \"" << getErrorString(err) << "\"" << std::endl;
        return (int)ReturnCodes::IoError;
    }

    if (parameters.threadCount == 0)
    {
        std::cerr << "Error, thread count must be > 0" << std::endl;
        return (int)ReturnCodes::BadCmdArgs;
    }

    noice::DftOptions dftOptions;
    dftOptions.threadCount = parameters.threadCount;
    noice::TextureComponentHandle dfts[4][2];
    const char* channelNames[4] = { "red", "green", "blue", "alpha" };
    const char* componentName[2] = { "intensity", "direction" };
    for (int i = 0; i < 4; ++i)
    {
        auto& c = components[i];
        if (!c.valid())
            continue;

        noice::Error dftErr = noice::generateDft(c, dfts[i], dftOptions);
        if (dftErr != noice::Error::Ok)
        {
            std::cerr << "Error generating dft: \"" << noice::getErrorString(dftErr) << "\"" << std::endl;
            return (int)ReturnCodes::DftError;
        }

        for (int dftC = 0; dftC < 2; ++dftC)
        {
            std::stringstream ss;
            if (parameters.outputPrefix != nullptr)
                ss << parameters.outputPrefix << ".";
            ss << "dft." << channelNames[i] << "." << componentName[dftC] << ".exr";
            std::string filename = ss.str();
            noice::TextureFileDesc outDesc;
            outDesc.filename = filename.c_str();
            outDesc.channels[0] = dfts[i][dftC];
            noice::Error fileErr = saveTextureToFile(outDesc);
            if (fileErr != noice::Error::Ok)
            {
                std::cerr << "failed writting file " << filename
                    << " with error \"" << noice::getErrorString(fileErr) << "\"" << std::endl;
                return (int)ReturnCodes::IoError;
            }
            else if (!parameters.quiet)
            {
                std::cout << "DFT analysis result: \"" << filename << "\""<< std::endl;
            }
            
        }

        deleteComponent(c);
    }

    return (int)ReturnCodes::Success;
}
