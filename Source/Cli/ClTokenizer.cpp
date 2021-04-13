#include "ClTokenizer.h"

namespace noice
{

namespace
{

bool isDigit(char digit)
{
    return digit >= '0' && digit <= '9';
}

bool parseInteger(const std::string& p, int& output, int& charsParsed)
{
    charsParsed = 0;
    const char* s = p.c_str();
    int multiplier = 1;
    if (*s == '-')
    {
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
        int intVal = 0;
        bool boolVal = 0;
        int charsParsed = 0;
        if (parseInteger(clStr, intVal, charsParsed))
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


