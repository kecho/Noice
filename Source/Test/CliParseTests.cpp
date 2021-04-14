#include "CliParseTests.h"
#include "Cli/ClTokenizer.h"
#include "Cli/ClTokenizer.cpp"
#include "Cli/ClParser.h"
#include "Cli/ClParser.cpp"
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
    char params[][64] ={ "=-T=asf", "91"};

    std::vector<char*> paramsV;
    for (auto& p : params)
        paramsV.push_back(p);

    ClTokenizer tokenizer;
    tokenizer.init((int)paramsV.size(), paramsV.data());

    if (!expectEq(tokenizer))
        return false;

    if (!expectName(tokenizer, "T", true))
        return false;

    if (!expectEq(tokenizer))
        return false;

    if (!expectString(tokenizer, "asf"))
        return false;

    if (!expectInt(tokenizer, 91))
        return false;

    ClTokenizer::Token endToken;
    if (tokenizer.next(endToken) != ClTokenizer::Result::End)
        return false;

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

bool cliGrammar0()
{
    struct SimpleStruct
    {
        int intTest0;
        int intTest1;
        const char* name0;
        bool boolTest0;
        bool boolTest1;
        const char* name1;
    };

    char params[][64] ={ "exe", "-i=10", "-j", "-40", "--name0=MyNamE", "-n", "AnotherName", "-btrue", "-q=false" };
    std::vector<char*> paramsV;
    for (auto& p : params)
        paramsV.push_back(p);

    ClParser parser;
    ClParser::GroupId rootId = parser.createGroup("root", "");
    parser.addParam(rootId, ClParser::ParamData(
        "",
        "i",
        "intTest0",
        CliParamType::Int,
        offsetof(SimpleStruct, intTest0)));

    parser.addParam(rootId, ClParser::ParamData(
        "",
        "j",
        "intTest1",
        CliParamType::Int,
        offsetof(SimpleStruct, intTest1)));

    parser.addParam(rootId, ClParser::ParamData(
        "",
        "m",
        "name0",
        CliParamType::String,
        offsetof(SimpleStruct, name0)));

    parser.addParam(rootId, ClParser::ParamData(
        "",
        "n",
        "name1",
        CliParamType::String,
        offsetof(SimpleStruct, name1)));

    parser.addParam(rootId, ClParser::ParamData(
        "",
        "b",
        "boolTest0",
        CliParamType::Bool,
        offsetof(SimpleStruct, boolTest0)));

    parser.addParam(rootId, ClParser::ParamData(
        "",
        "q",
        "boolTest1",
        CliParamType::Bool,
        offsetof(SimpleStruct, boolTest1)));
    
    parser.setOnErrorCallback([](const std::string& err)
    {
        std::cerr << err << " ";
    });

    SimpleStruct obj = {};
    parser.bind(rootId, &obj);
    if (!parser.parse((int)paramsV.size(), paramsV.data()))
        return false;

    if (obj.intTest0 != 10)
        return false;

    if (obj.intTest1 != -40)
        return false;

    if (std::string(obj.name0) != "MyNamE")
        return false;

    if (std::string(obj.name1) != "AnotherName")
        return false;

    if (!obj.boolTest0)
        return false;

    if (obj.boolTest1)
        return false;

    return true;
}


}
