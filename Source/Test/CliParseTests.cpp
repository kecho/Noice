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

static bool compFloat(float a, float b)
{
    return std::abs(a - b) < 0.000001f;
}

bool cliTokenizer2()
{
    struct Case
    {
        const char* str;
        float val;
        bool expectSuccess;
    };

    Case cases[] = {
        { "0", 0.0f, true } ,
        { "57e12", 57e12f, true },
        { ".44", 0.44f, true },
        { "0.1", 0.1f, true },
        { "1.0", 1.0f, true },
        { "57e-5", 57e-5f, true },        
        { "57e--12", 57e-12f, false },
        { "-3.5567", -3.5567f, true },
        { "-3a.ffe1", -3.112f, false },
        { "3e.-ffe1", 0.0f, false }
    };

    for (const Case& c : cases)
    {
        std::string s = c.str;
        float output = 0.0f;
        int charsParsed = 0;
        bool success = ClTokenizer::parseFloat(s, output, charsParsed);
        bool matches = compFloat(output, c.val);
        if (success == c.expectSuccess && success)
            if(matches)
                continue;            
            else
            {
                std::cerr << "Incorrect parsing, got \"" << c.str << "\" resulted in " << output << std::endl;
                return false;
            }

        if (!success && c.expectSuccess)
        {
            std::cerr << "Failed parsing: \"" << c.str << "\" returned: " << output << std::endl;
            return false;
        }

        if (success && !c.expectSuccess)
        {
            std::cerr << "False success parsing: \"" << c.str << "\" returned: " << output << std::endl;
            return false;
        }
    }

    return true;
}

static inline bool compareNumber(int a, int b) { return a == b; }
static inline bool compareNumber(float a, float b) { return compFloat(a, b); }

template<typename T>
bool compLists(const std::vector<T>& a, const std::vector<T>& b)
{
    if (a.size() != b.size())
        return false;

    for (int i = 0; i < (int)a.size(); ++i)
    {
        if (!compareNumber(a[i], b[i]))
            return false;
    }

    return true;
}

bool cliTokenizer3()
{
    struct CaseF
    {
        const char* str;
        std::vector<float> expected;
        bool expectSuccess;
    };

    CaseF cases[] = {
        { "", {}, true },
        { "1.0x2e-5x79x-10.2", { 1.0f, 2e-5f, 79.0f, -10.2f }, true },
        { "x12x29xxx9", { 0.0f, 12.0f, 29.0f, 0.0f, 0.0f, 9.0f }, false }
    };

    auto compFloat = [](float a, float b)
    {
        return std::abs(a - b) < 0.000001f;
    };

    for (const CaseF& c : cases)
    {
        std::string inputStr = c.str;
        std::vector<float> outList;
        bool result = ClTokenizer::parseFloatList(outList, inputStr, 'x');
        bool match = compLists(outList, c.expected);
        if (result == c.expectSuccess && result)\
            if (!match)
            {
                std::cerr << "Incorrect parsing: " << c.str << std::endl;
                return false;
            }
            else
                continue;

        if (!result && c.expectSuccess)
        {
            std::cerr << "Failed parsing: " << c.str << std::endl;
            return false;
        }

        if (result && !c.expectSuccess)
        {
            std::cerr << "False success parsing :" << c.str << std::endl;
            return false;
        }
    }

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
        const char* name2;
    };

    char params[][64] ={ "exe", "-i=10", "-j", "-40", "--name0=MyNamE", "-n", "AnotherName", "-btrue", "-q=false", "-o2.0" };
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
        "o",
        "name2",
        CliParamType::String,
        offsetof(SimpleStruct, name2)));

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

    if (std::string(obj.name2) != "2.0")
    {
        return false;
    }

    if (!obj.boolTest0)
        return false;

    if (obj.boolTest1)
        return false;

    return true;
}


}
