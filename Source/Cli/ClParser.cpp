#include "ClParser.h"
#include <variant>
#include <iostream>
#include <iomanip>
#include <sstream>


namespace noice
{

ClParser::GroupId ClParser::createGroup(const char* name, const char* description)
{
    GroupId gid = m_schema.groups.size();
    m_groupBinds.emplace_back();
    m_groupBinds.back() = nullptr;
    m_schema.groups.emplace_back();
    Group& g = m_schema.groups.back();
    g.groupId = gid;
    g.name = name;
    g.description = description;
    return gid;
}

bool ClParser::addParam(ClParser::GroupId gid, ClParser::ParamData param)
{
    if (gid >= (GroupId)m_schema.groups.size())
        return false;

    {
        auto shortNameIt = m_usedParamShortNames.find(param.shortName);
        if (shortNameIt != m_usedParamShortNames.end())
            return false;

        auto longNameIt = m_usedParamNames.find(param.longName);
        if (longNameIt != m_usedParamNames.end())
            return false;
    }

    Group& g = m_schema.groups[gid];
    int paramIndex = g.params.size();
    g.params.push_back(param);
    ParamLoc loc = { gid, paramIndex };
    m_usedParamShortNames[param.shortName] = loc;
    m_usedParamNames[param.longName] = loc;
    return true;
}

void ClParser::bind(ClParser::GroupId gid, void* object)
{
    if (gid >= (GroupId)m_groupBinds.size())
        return;
    m_groupBinds[gid] = object;
}

void ClParser::printTokens(int argc, char* argv[])
{
    ClTokenizer tokenizer;
    tokenizer.init(argc, argv);

    ClTokenizer::Result tokResult = ClTokenizer::Result::Success;
    while(tokResult == ClTokenizer::Result::Success)
    {
        ClTokenizer::Token token;
        tokResult = tokenizer.next(token);
        if (tokResult == ClTokenizer::Result::End)
            break;

        if (tokResult != ClTokenizer::Result::Success)
        {
            std::cout << "Error in tokenizer " << (int)tokResult;
            break;
        }

        if (auto imm = std::get_if<ClTokenizer::Imm>(&token))
        {
            switch (imm->type)
            {
            case CliParamType::Int:
                std::cout << imm->scalar.u << " ";
                break;
            case CliParamType::Uint:
                std::cout << imm->scalar.i << " ";
                break;
            case CliParamType::Bool:
                std::cout << (imm->scalar.b ? "true" : "false") << " ";
                break;
            case CliParamType::String:
                std::cout << "\"" << imm->strValue << "\"" << " ";
                break;
            }
        }
        else if (auto name = std::get_if<ClTokenizer::Name>(&token))
        {
            std::cout << (name->isShortParam ? "-" : "--") << name->name << " ";
        }
        else if (auto name = std::get_if<ClTokenizer::Equal>(&token))
        {
            std::cout << "= ";
        }
    }

    std::cout << std::endl;
}
    
void ClParser::reportError(const char* msg) const
{
    if (m_onError == nullptr)
        return;
    
    std::string msgStr = msg;
    m_onError(msg);
}

void ClParser::reportErrorStr(const std::string& msg) const
{
    if (m_onError == nullptr)
        return;

    m_onError(msg);
}

bool ClParser::parse(int argc, char* argv[])
{
    ClTokenizer tokenizer;
    tokenizer.init(argc, argv);

    ClTokenizer::Result result;
    ClTokenizer::Token t;
    result = tokenizer.next(t);

    if (result != ClTokenizer::Result::Success)
    {
        reportError("Cannot get applications path.");
        return false;
    }

    if (auto imm = std::get_if<ClTokenizer::Imm>(&t))
    {
        if (imm->type == CliParamType::String)
            m_appPath = imm->strValue;
    }

    enum class State
    {
        ParamName,
        ParamValue
    };

    result = tokenizer.next(t);
    State state = State::ParamName;

    ParamLoc currentParamLoc = {};
    while (result != ClTokenizer::Result::End)
    {
        if (result != ClTokenizer::Result::Success)
        {
            switch (result)
            {
            case ClTokenizer::Result::ErrorEmptyToken:
                reportError("Empty token in command line.");
                return false;
            case ClTokenizer::Result::ErrorMissingParamName:
                reportError("Did not specified a parameter name.");
                return false;
            }
        }

        if (state == State::ParamName)
        {
            auto* name = std::get_if<ClTokenizer::Name>(&t);
            if (name == nullptr)
            {
                std::stringstream ss;
                ss << "Expected a parameter name. Found '" << ClTokenizer::toString(t) << "' instead.";
                reportErrorStr(ss.str());
                return false;
            }

            if (!parseParamName(*name, currentParamLoc))
                return false;

            state = State::ParamValue;
            result = tokenizer.next(t);

            if (result == ClTokenizer::Result::Success || result == ClTokenizer::Result::End)
            {
                Group& group = m_schema.groups[currentParamLoc.gid];
                ParamData& paramType = group.params[currentParamLoc.paramIndex];
                if (paramType.type == CliParamType::Bool && std::get_if<ClTokenizer::Name>(&t) != nullptr)
                {
                    ClTokenizer::Imm imm;
                    imm.type = CliParamType::Bool;
                    imm.scalar.b = true;
                    if (!parseParamValue(currentParamLoc, imm))
                        return false;

                    state = State::ParamName;
                }
            }
        }
        else if (state == State::ParamValue)
        {
            if (auto eq = std::get_if<ClTokenizer::Equal>(&t))
            {
                result = tokenizer.next(t);
                continue;
            }
            else if (auto imm = std::get_if<ClTokenizer::Imm>(&t))
            {
                if (!parseParamValue(currentParamLoc, *imm))
                    return false;
            }
            else
            {
                std::stringstream ss;
                ss << "Unexpected token found: '" << ClTokenizer::toString(t) << "'.";
                reportErrorStr(ss.str());
                return false;
            }

            state = State::ParamName;
            result = tokenizer.next(t);
            continue;
        }
    }

    if (state == State::ParamValue)
    {
        Group& group = m_schema.groups[currentParamLoc.gid];
        ParamData& paramType = group.params[currentParamLoc.paramIndex];
        std::stringstream ss;
        ss << "Missing argument for parameter: " << paramType.longName << "(" << paramType.shortName << ") .";
        reportErrorStr(ss.str());
        return false;
    }

    return true;
}

bool ClParser::parseParamName(const ClTokenizer::Name& nm, ClParser::ParamLoc& outLoc)
{
    bool found = false;
    if (nm.isShortParam)
    {
        auto it = m_usedParamShortNames.find(nm.name);
        found = it != m_usedParamShortNames.end();
        if (found)
            outLoc = it->second;
    }
    else
    {
        auto it = m_usedParamNames.find(nm.name);
        found = it != m_usedParamNames.end();
        if (found)
            outLoc = it->second;
    }

    if (!found)
    {
        std::stringstream ss;
        ss << "Unknown parameter/switch '" << nm.name << "' passed" << std::endl; 
        reportErrorStr(ss.str());
        return false;
    }

    return true;
}

bool ClParser::parseParamValue(const ClParser::ParamLoc& loc, const ClTokenizer::Imm& value)
{
    char* obj = (char*)m_groupBinds[loc.gid];
    Group& group = m_schema.groups[loc.gid];
    ParamData& paramType = group.params[loc.paramIndex];

    const bool valueIsInt   = value.type == CliParamType::Uint || value.type == CliParamType::Int;
    const bool requestIsUnsigned = paramType.type == CliParamType::Uint;
    const bool valHasSign = value.type == CliParamType::Int && value.hasSign;
    const bool compatibleTypes = paramType.type == value.type || (requestIsUnsigned && (valueIsInt && !valHasSign));
    if (!compatibleTypes)
    {
        std::stringstream ss;
        ss << "Parameter " << paramType.longName << " ("
            << paramType.shortName << ") must be of type " << ClTokenizer::toString(paramType.type)
            << " but instead found type " << ClTokenizer::toString(value.type) << " of value: "
            << ClTokenizer::toString(value);

        reportErrorStr(ss.str());
        return false;
    }

    
    if (value.type == CliParamType::String && !paramType.enumNames.empty())
    {
        bool found = false;
        for (auto& enumName : paramType.enumNames)
        {
            if (enumName == value.strValue)
            {
                found = true;
                break;
            }
        }

        if (!found)
        {
            std::stringstream ss;
            ss << "Parameter " << paramType.longName << " (" << paramType.shortName << ") with value '" << value.strValue << "' must contain one of the following values: ";
            for (int i = 0; i < paramType.enumNames.size(); ++i)
                ss  << "\'" << paramType.enumNames[i] << "'"
                    << (i != (paramType.enumNames.size() - 1) ? "," : "");
            reportErrorStr(ss.str());
            return false;
        }
    }

    const void* resultPtr = nullptr;
    int valueBlobSize = 0;
    switch (value.type)
    {
    case CliParamType::String:
        {
            m_supportStrings.push_back(std::make_unique<std::string>());
            (*m_supportStrings.back()) = value.strValue;
            if (obj)
            {
                *((const char**)(obj + paramType.offset)) = m_supportStrings.back().get()->c_str();
            }
            resultPtr = m_supportStrings.back().get()->c_str();
        }
        break;
    case CliParamType::Int:
    case CliParamType::Uint:
        {
            resultPtr = &value.scalar.i;
            if (obj)
                *(int*)(obj + paramType.offset) = value.scalar.i;
        }
        break;
    case CliParamType::Bool:
        {
            resultPtr = &value.scalar.b;
            if (obj)
                *(bool*)(obj + paramType.offset) = value.scalar.b;
        }
        break;
    }

    if (paramType.onSet != nullptr)
        paramType.onSet(paramType, loc.gid, resultPtr);

    return true;
}

void ClParser::prettyPrintHelp()
{
    for (const auto& g : m_schema.groups)
    {
        std::cout << g.description << std::endl << std::endl;
        for (const auto& p : g.params)
        {
            std::cout << std::setw(4) << "-" << p.shortName << ", --"  << p.longName << " (" << ClTokenizer::toString(p.type) << ")" << std::endl;
            std::cout << "\t" << p.description << std::endl;
            if (p.type == CliParamType::String && !p.enumNames.empty())
            {
                std::cout << "\t" << "Possible values:  " << std::endl << "\t";
                for (int e = 0; e < (int)p.enumNames.size(); ++e)
                {
                    std::cout << p.enumNames[e] << (e != ((int)p.enumNames.size() - 1) ? ", " : "" );
                }
            }
            std::cout << std::endl << std::endl;
        }
    }
}

}
