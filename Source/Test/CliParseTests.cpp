#include "CliParseTests.h"
#include "Cli/ClTokenizer.h"
#include "Cli/ClTokenizer.cpp"
#include <vector>

namespace noice
{

namespace
{

bool expectInt(ClTokenizer& tokenizer, int expectedInt)
{
    ClTokenizer::Token token;
    if (tokenizer.next(token) != ClTokenizer::Result::Success)
        return false;

    auto imm = std::get_if<ClTokenizer::Imm>(&token);
    if (imm == nullptr)
        return false;

    if (imm->type != CliParamType::Int)
        return false;

    return imm->scalar.i == expectedInt;
}

bool expectString(ClTokenizer& tokenizer, std::string expectedString)
{
    ClTokenizer::Token token;
    if (tokenizer.next(token) != ClTokenizer::Result::Success)
        return false;

    auto imm = std::get_if<ClTokenizer::Imm>(&token);
    if (imm == nullptr)
        return false;

    if (imm->type != CliParamType::String)
        return false;

    return imm->strValue == expectedString;
}

bool expectBool(ClTokenizer& tokenizer, bool expectedBool)
{
    ClTokenizer::Token token;
    if (tokenizer.next(token) != ClTokenizer::Result::Success)
        return false;

    auto imm = std::get_if<ClTokenizer::Imm>(&token);
    if (imm == nullptr)
        return false;

    if (imm->type != CliParamType::Bool)
        return false;

    return imm->scalar.b == expectedBool;
}

bool expectName(ClTokenizer& tokenizer, std::string name, bool isShort)
{
    ClTokenizer::Token token;
    if (tokenizer.next(token) != ClTokenizer::Result::Success)
        return false;

    auto nameToken = std::get_if<ClTokenizer::Name>(&token);
    if (nameToken == nullptr)
        return false;

    if (nameToken->isShortParam != isShort)
        return false;

    return nameToken->name == name;
}

bool expectEq(ClTokenizer& tokenizer)
{
    ClTokenizer::Token token;
    if (tokenizer.next(token) != ClTokenizer::Result::Success)
        return false;

    auto name = std::get_if<ClTokenizer::Equal>(&token);
    if (name == nullptr)
        return false;

    return true;
}

}

bool cliTokenizer0()
{
    return true;
}

bool cliTokenizer1()
{
    char params[][64] ={ "-1", "testStr", "--param=20" };

    std::vector<char*> paramsV;
    for (auto& p : params)
        paramsV.push_back(p);

    ClTokenizer tokenizer;
    tokenizer.init((int)paramsV.size(), paramsV.data());
    if (!expectInt(tokenizer, -1))
        return false;

    if (!expectString(tokenizer, "testStr"))
        return false;

    if (!expectName(tokenizer, "param", false))
        return false;

    if (!expectEq(tokenizer))
        return false;

    if (!expectInt(tokenizer, 20))
        return false;

    ClTokenizer::Token endToken;
    if (tokenizer.next(endToken) != ClTokenizer::Result::End)
        return false;

    return true;
}


}
