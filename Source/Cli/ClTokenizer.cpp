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

bool parseInteger(const std::string& p, int& output, bool& hasSign, int& charsParsed)
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
        bool boolVal = 0;
        int charsParsed = 0;
        if (parseInteger(clStr, intVal, imm.hasSign, charsParsed))
        {
            imm.type = CliParamType::Int;
            imm.scalar.i = intVal;
        }
        else if (parseBool(clStr, boolVal, charsParsed))
        {
            imm.type = CliParamType::Bool;
            imm.scalar.b = boolVal;
        }
        else
        {
            imm.type = CliParamType::String;
            imm.strValue = clStr;
            charsParsed = clStr.size();
        }

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


