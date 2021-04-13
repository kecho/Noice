#include "ClParser.h"
#include <variant>
#include <iostream>


namespace noice
{

ClParser::GroupId ClParser::createGroup(const char* name)
{
    GroupId gid = m_schema.groups.size();
    m_groupBinds.emplace_back();
    m_groupBinds.back() = nullptr;
    m_schema.groups.emplace_back();
    Group& g = m_schema.groups.back();
    g.groupId = gid;
    g.name = name;
    return gid;
}

bool ClParser::addParam(ClParser::GroupId gid, const ClParser::ParamData& param)
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
    
bool ClParser::parse(int argc, char* argv[])
{
#if 0
    enum class States { PARAM, VALUE };
    States s = States::PARAM;
    ParamLoc paramLoc = {};
    auto errCb = [this](const std::string& msg)
    {
        if (m_onError == nullptr)
            return;
        m_onError(msg);
    };
    auto errCbStr = [this](const char* msg)
    {
        if (m_onError == nullptr)
            return;
        std::string msgStr = msg;
        m_onError(msgStr);
    };


    int argumentIndex = 1;
    if (argumentIndex >= argc)
        return;

    const char* currentArg = argv[argumentIndex];
    while (currentArg != nullptr)
    {
        if (s == States::PARAM)
        {
            std::string arg = currentArg;
            if (arg.size() == 0)
                continue;

            if (arg.size() == 1 || arg[0] != '-')
            {
                errCbStr("Found invalid argument/switch, must start with '-'.");
                return false;
            }

            bool isLongName = arg[1] == '-';
            if (isLongName && arg.size() <= 2)
            {
                errCbStr("Found invalid parameter/switch, must start with '--' followed by the property name.");
                return false;
            }
        
            ParamMap::iterator it;
            if (isLongName)
            {
                it = m_usedParamNames.find(std::string(rawStr+2));
                if (it == m_usedParamNames.end())
                    return false;
            }
            else
            {
                it = m_usedParamShortNames.find(std::string(rawStr+1));
                if (it == m_usedParamShortNames.end())
                    return false;
            }
            paramLoc = it->second;
            s = States::VALUE;
        }
        else if (s == States::VALUE)
        {
            //TODO parse and set values.
            s = States::VALUE; 
        }
    }

    return true;
#else
    return true;
#endif
}

}
