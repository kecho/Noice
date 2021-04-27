#include <noice/noice.h>
#include <ClParser.h>
#include <iostream>
#include <string>

namespace 
{

enum class ReturnCodes : int
{
    Success = 0,
    IoError = 1,
    BadCmdArgs = 2
};

struct ArgParameters
{
    const char* inputExr = nullptr;
    const char* outputPrefix = "";
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
    CliSwitch(generalGid, "Disables all the standard output.", "q", "quiet", String, ArgParameters, quiet);
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
        return (int)ReturnCodes::BadCmdArgs;
    }

    for (auto& c : components)
        deleteComponent(c);

    return (int)ReturnCodes::Success;
}
