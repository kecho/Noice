#include "ClParser.h"

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

bool ClParser::parse(int argc, const char* argv[])
{
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
}

}
