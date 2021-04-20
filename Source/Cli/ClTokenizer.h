#pragma once

#include <string>
#include <variant>
#include <vector>

namespace noice
{

enum class CliParamType
{
    Uint, Int, Bool, String
};

class ClTokenizer
{
public:
    
    struct Imm
    {
        CliParamType type;
        bool hasSign;
        std::string strValue;
        union {
            unsigned u;
            int i;
            bool b;
        } scalar;
    };

    struct Name
    {
        bool isShortParam;
        std::string name;
    };

    struct Equal
    {
        int dummy;
    };

    enum class Result
    {
        Success,
        ErrorEmptyToken,
        ErrorMissingParamName,
        End
    };

    using Token = std::variant<Imm, Name, Equal>;

    static std::string toString(const Token& t);
    static const char* toString(CliParamType type);
    static std::vector<std::string> splitString(const std::string& s, char splitChar);
    static bool parseInteger(const std::string& p, int& output, bool& hasSign, int& charsParsed);
    static bool parseFloat(const std::string& p, float& output, int& charsParsed);
    static bool parseIntList  (std::vector<int>& outList, const std::string& inputString, char token);
    static bool parseFloatList(std::vector<float>& outList, const std::string& inputString, char token);

    void init(int argc, char* argv[])
    {
        m_strOffset = 0;
        m_index = 0;
        m_argc = argc;
        m_argv = argv;
    }

    Result next(Token& outToken);

private:
    int m_strOffset = 0;
    int m_index = 0;
    int m_argc = 0;
    char** m_argv;
};


}
