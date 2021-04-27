#include "ClTokenizer.h"
#include <sstream>

namespace noice
{

namespace
{

bool isDigit(char digit)
{
    return digit >= '0' && digit <= '9';
}

bool parseBool(const std::string& p, bool& output, int& charsParsed)
{
    charsParsed = 0;
    charsParsed += p.size();
    if (p == "true")
    {
        output = true;
        return true;
    }
    else if (p == "false")
    {
        output = false;
        return true;
    }
    else
        return false;
}

}

bool ClTokenizer::parseInteger(const std::string& p, int& output, bool& hasSign, int& charsParsed)
{
    hasSign = false;
    charsParsed = 0;
    const char* s = p.c_str();
    int multiplier = 1;
    if (*s == '-')
    {
        hasSign = true;
        multiplier = -1;
        ++s;
    }

    if (!isDigit(*s))
        return false;
    
    int number = 0;
    while (*s && s != nullptr)
    {
        char digit = *s;
        if (!isDigit(digit))
            break;

        int d = digit - '0';
        number *= 10;
        number += d;
        ++s;
    }
    
    charsParsed = s - p.c_str();
    output = number * multiplier;
    return true;
}

bool ClTokenizer::parseFloat(const std::string& p, float& output, int& charsParsed)
{
    int first = 0;
    bool hasSign = false;
    if (p[0] != '.')
    {
        if (!parseInteger(p, first, hasSign, charsParsed))
            return false;
    }

    const char* cs = p.c_str() + charsParsed;
    if (!*cs)
    {
        output = (float)first;
        return true;
    }

    int tail = 0;
    char separator = *cs;
    if (separator != '.' && separator != 'e')
        return false;

    ++charsParsed;
    ++cs;
    int charsParsed2 = 0;
    std::string tailString = cs;
    if (!parseInteger(tailString, tail, hasSign, charsParsed2))
        return false;

    if (hasSign && separator == '.')
        return false;

    cs = tailString.c_str() + charsParsed2;

    charsParsed += charsParsed2;
    float multi = 1.0f;
    if (separator == '.')
    {
        for (int i = 0; i < charsParsed2; ++i)
            multi *= 10.0f;
        float sign = first >= 0.0f ? 1.0f : -1.0f;
        output = (float)first + sign*(float)tail*(1.0f/(float)multi);
        return true;
    }
    else if (separator == 'e')
    {
        if (tail > 64)
            return false;

        if (tail > 0)
            for (int i = 0; i < tail; ++i)
                multi *= 10.0f;
        else
            for (int i = 0; i < std::abs(tail); ++i)
                multi /= 10.0f;

        output = (float)first * (float)multi;
        return true;
    }

    return false;
}

std::vector<std::string> ClTokenizer::splitString(const std::string& s, char splitChar)
{
    std::vector<std::string> split;

    std::stringstream tmp;
    const char* cs = s.c_str();
    while (cs && *cs)
    {
        tmp.write(cs, 1);
        ++cs;
        if (!*cs || *cs == splitChar)
        {
            if (*cs == splitChar) ++cs;
            split.push_back(tmp.str());
            tmp = std::stringstream();
        }
    }

    return split;
}

bool ClTokenizer::parseIntList  (std::vector<int>& outList, const std::string& inputString, char token)
{
    outList.clear();
    auto stringList = splitString(inputString, token);
    bool unusedVal;
    int parsedCount;
    for (const auto& s : stringList)
    {
        int intOut = 0;
        if (!parseInteger(s, intOut, unusedVal, parsedCount))
            return false;

        if (s.size() != parsedCount)
            return false;

        outList.push_back(intOut);
    }

    return true;
}

bool ClTokenizer::parseFloatList(std::vector<float>& outList, const std::string& inputString, char token)
{
    outList.clear();
    auto stringList = splitString(inputString, token);
    int parsedCount;
    for (const auto& s : stringList)
    {
        float floatOut = 0;
        if (!parseFloat(s, floatOut, parsedCount))
            return false;

        if (parsedCount != s.size())
            return false;

        outList.push_back(floatOut);
    }

    return true;
}

std::string ClTokenizer::toString(const ClTokenizer::Token& t)
{
    std::stringstream ss;
    if (auto imm = std::get_if<Imm>(&t))
    {
        switch (imm->type)
        {
        case CliParamType::Int:
            ss << imm->scalar.i;
            break;
        case CliParamType::Float:
            ss << imm->scalar.f;
            break;
        case CliParamType::Uint:
            ss << imm->scalar.u;
            break;
        case CliParamType::Bool:
            ss << (imm->scalar.b ? "true" : "false");
            break;
        case CliParamType::String:
            ss << imm->strValue;
            break;
        }
    }
    else if (auto name = std::get_if<Name>(&t))
    {
        ss << name->name;
    }
    else if (auto eq = std::get_if<Equal>(&t))
    {
        ss << "=";
    }
    else 
    {
        ss << "<unknown>";
    }
    return ss.str();
}

const char* ClTokenizer::toString(CliParamType type)
{
    switch (type)
    {
    case CliParamType::Uint:
        return "Uint";
    case CliParamType::Int:
        return "Int";
    case CliParamType::Float:
        return "Float";
    case CliParamType::Bool:
        return "Bool";
    case CliParamType::String:
        return "String";
    }
    return "<unknown>";
}

ClTokenizer::Result ClTokenizer::next(ClTokenizer::Token& outToken)
{
    if (m_index >= m_argc)
        return Result::End;

    const char* origin = m_argv[m_index] + m_strOffset;
    const char* str = origin;
    std::string clStr = str;

    if (clStr.size() == 0)
        return Result::ErrorEmptyToken;

    int dashes = clStr[0] == '-' ? 1 : 0;

    if (clStr.size() <= 1 && dashes >= 1)
        return Result::ErrorMissingParamName;

    dashes += dashes > 0 && clStr[1] == '-' ? 1 : 0;

    if (clStr.size() <= 2 && dashes >= 2)
        return Result::ErrorMissingParamName;

    bool isParam = dashes > 0 && !isDigit(str[dashes]);
    if (isParam)
    {
        str += dashes;
        Name nm;
        nm.isShortParam = dashes == 1;
        if (nm.isShortParam)
        {
            char c = *(str++);
            nm.name = c;
        }
        else
        {
            nm.name = "";
            while (*str != '\0' &&  *str != '=')
            {
                nm.name += *(str++);
            }
        }

        outToken = nm;
    }
    else if (*str == '=')
    {
        ++str;
        outToken = Equal{ 0 };
    }
    else
    {
        Imm imm;
        imm.hasSign = false;
        int intVal = 0;
        float floatVal = 0;
        bool boolVal = 0;
        int charsParsed = 0;
        if (parseInteger(clStr, intVal, imm.hasSign, charsParsed) && clStr.size() == charsParsed)
        {
            imm.type = CliParamType::Int;
            imm.scalar.i = intVal;
        }
        else if (parseBool(clStr, boolVal, charsParsed) && clStr.size() == charsParsed)
        {
            imm.type = CliParamType::Bool;
            imm.scalar.b = boolVal;
        }
        else if (parseFloat(clStr, floatVal, charsParsed) && clStr.size() == charsParsed)
        {
            imm.type = CliParamType::Float;
            imm.scalar.f = floatVal;
        }
        else
        {
            imm.type = CliParamType::String;
            charsParsed = clStr.size();
        }

        imm.strValue = clStr;
        outToken = imm;
        str += charsParsed;
    }

    m_strOffset += (str - origin);
    clStr = str;

    if (clStr.size() == 0)
    {
        ++m_index;
        m_strOffset = 0;
    }

    return Result::Success;
}

}


