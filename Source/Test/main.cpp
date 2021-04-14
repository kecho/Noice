#include <iostream>
#include <functional>
#include <string>
#include "CliParseTests.h"


enum : int { FailCode = 0xfa11 };
using TestFn = std::function<bool()>;

struct TestCase
{
    std::string name;
    TestFn testFn;
};


TestCase g_testRegistry [] = {
    { "Cli Tokenization 0", noice::cliTokenizer0 },
    { "Cli Tokenization 1", noice::cliTokenizer1 },
};


int main(int argc, char* argv[])
{
    int fails = 0;
    for (auto test : g_testRegistry)
    {
        std::cout << "Testing: " << test.name << " Result: ";
        bool result = test.testFn();
        std::cout << (result ? "Pass" : "Fail") << std::endl;
        fails += !result ? 1 : 0;
    }

    return fails == 0 ? 0 : (int)FailCode;
}
