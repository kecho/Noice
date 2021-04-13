#include "CliParseTests.h"
#include "Cli/ClTokenizer.h"
#include "Cli/ClTokenizer.cpp"
#include <vector>

namespace noice
{

bool cliTokenizer0()
{
    return true;
}

bool cliTokenizer1()
{
    return true;
}

bool cliTokenizer2()
{
    char params[][64] ={ "-1", "testStr", "--param=20" };

    std::vector<char*> paramsV;
    for (auto& p : params)
        paramsV.push_back(p);

    ClTokenizer tokenizer;
    tokenizer.init((int)paramsV.size(), paramsV.data());
    ClTokenizer::Token token;

    if (tokenizer.next(token) != ClTokenizer::Result::Success)
        return false;

    if (auto imm = std::get_if<ClTokenizer::Imm>(&token))
    {
        if (imm->type != CliParamType::Int)
            return false;
    }
    else
        return false;

    return true;
}


}
