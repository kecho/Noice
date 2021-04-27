#pragma once

#include <vector>
#include <string>
#include <functional>
#include <unordered_map>
#include <memory>
#include <iostream>
#include "ClTokenizer.h"

//Helper macros
//In order to use them:
// 1) Have a noice::ClParser p named as p in the scope.
// 2) Put these inside a function that returns a bool
// 3) you must include iostream (because of std::cerr)
// 4) do a using namespace noice at the top of the function
#define CliSwitch(id, desc, sn, ln, tp, st, mem) \
    if (!p.addParam(id, ClParser::ParamData(desc, sn, ln, CliParamType::tp, offsetof(st, mem))))\
    { std::cerr << "Found duplicate schema name :" << sn << " " << ln << std::endl; return false; }

#define CliSwitchAction(id, desc, sn, ln, tp, st, mem, lst, lmbda) \
    if (!p.addParam(id, ClParser::ParamData(desc, sn, ln, CliParamType::tp, offsetof(st, mem), lst, lmbda)))\
    { std::cerr << "Found duplicate schema name :" << sn << " " << ln << std::endl; return false; }


namespace noice
{

class ClParser
{
public:
    using GroupId = unsigned int;
    using ParamId = unsigned int;
    struct ParamData;
    using ParamCallback = std::function<void(const ParamData&, GroupId, const void*)>;
    using OnErrorCallback = std::function<void(const std::string&)>;

    struct ParamData
    {
        std::string description;
        std::string shortName;
        std::string longName;
        CliParamType type;
        uint64_t offset;
        std::vector<std::string> enumNames;
        ParamCallback onSet;

        ParamData(
            std::string pdescription,
            std::string pshortName,
            std::string plongName,
            CliParamType ptype,
            uint64_t poffset)
            : description(pdescription)
            , shortName(pshortName)
            , longName(plongName)
            , type(ptype)
            , offset(poffset)
            , onSet(nullptr)
        {
        }

        ParamData(
            std::string pdescription,
            std::string pshortName,
            std::string plongName,
            CliParamType ptype,
            uint64_t poffset,
            std::vector<std::string> penumNames,
            ParamCallback ponSet)
            : description(pdescription)
            , shortName(pshortName)
            , longName(plongName)
            , type(ptype)
            , offset(poffset)
            , enumNames(penumNames)
            , onSet(ponSet)
        {
        }
    };

    struct Group
    {
        GroupId groupId = -1;
        std::string name;
        std::string description;
        std::vector<ParamData> params;
    };

    ClParser() {}

    GroupId createGroup(const char* name, const char* description);

    const Group& group(GroupId gid) const { return m_schema.groups[gid]; }
    int groupCounts() const { return m_schema.groups.size(); }

    bool addParam(GroupId gid, ParamData param);
    void bind(GroupId gid, void* object);
    void setOnErrorCallback(OnErrorCallback cb) { m_onError = cb; }
    bool parse(int argc, char* argv[]);
    void printTokens(int argc, char* argv[]);
    const char* appPath() const { return m_appPath.c_str(); }
    void prettyPrintHelp();

private:
    struct Schema
    {
        std::vector<Group> groups;
    };

    struct ParamLoc
    {
        GroupId gid;
        int paramIndex;
    };

    void reportError(const char* msg) const;
    void reportErrorStr(const std::string& msg) const;
    bool parseParamName(const ClTokenizer::Name& nm, ParamLoc& outLoc);
    bool parseParamValue(const ParamLoc& loc, ClTokenizer::Imm value);

    Schema m_schema;
    OnErrorCallback m_onError = nullptr;
    using ParamMap = std::unordered_map<std::string, ParamLoc>;
    ParamMap m_usedParamNames;
    ParamMap m_usedParamShortNames;
    std::string m_appPath;
    std::vector<void*> m_groupBinds;
    std::vector<std::unique_ptr<std::string>> m_supportStrings;
};

}
