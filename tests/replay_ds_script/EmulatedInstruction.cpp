#include <map>
#include <regex>
#include <sstream>
#include <string>

enum INSTRUCTION_TYPE
{
    SIMPLE,        // Simple instructions with no size modifiers
    SIZE_SPECIFIC, // Instructions with .B, .A size modifiers
};

struct EmulationInfo
{
    INSTRUCTION_TYPE type;
    std::string replacement;
};

// Map to associate specific mnemonics with their emulated forms
static std::map<std::string, EmulationInfo> mappings = {
    {"RETA", {SIMPLE, "MOVA @SP+,PC"}},
    {"DINT", {SIMPLE, "BIC #8,SR"}},
    {"NOP", {SIMPLE, "MOV #0,R3"}},
    {"ADC", {SIZE_SPECIFIC, "ADDC%s #0,%s"}},
    {"BR", {SIMPLE, "MOV %s,PC"}},
    {"CLR", {SIZE_SPECIFIC, "MOV%s #0,%s"}},
    {"CLRC", {SIMPLE, "BIC #1,SR"}},
    {"CLRN", {SIMPLE, "BIC #4,SR"}},
    {"CLRZ", {SIMPLE, "BIC #2,SR"}},
    {"DADC", {SIZE_SPECIFIC, "DADD%s #0,%s"}},
    {"DEC", {SIZE_SPECIFIC, "SUB%s #1,%s"}},
    {"DECD", {SIZE_SPECIFIC, "SUB%s #2,%s"}},
    {"DECDA", {SIMPLE, "SUBA #2,%s"}},
    {"EINT", {SIMPLE, "BIS #8,SR"}},
    {"INC", {SIZE_SPECIFIC, "ADD%s #1,%s"}},
    {"INCD", {SIZE_SPECIFIC, "ADD%s #2,%s"}},
    {"INV", {SIZE_SPECIFIC, "XOR%s #–1,%s"}},
    {"RLA", {SIZE_SPECIFIC, "ADD%s %s,%s"}},
    {"RLC", {SIZE_SPECIFIC, "ADDC%s %s,%s"}},
    {"SBC", {SIZE_SPECIFIC, "SUBC%s #0,%s"}},
    {"SETC", {SIMPLE, "BIS #1,SR"}},
    {"SETN", {SIMPLE, "BIS #4,SR"}},
    {"SETZ", {SIMPLE, "BIS #2,SR"}},
    {"TST", {SIZE_SPECIFIC, "CMP%s #0,%s"}},
    {"ADCX", {SIZE_SPECIFIC, "ADDCX%s #0,%s"}},
    {"BRA", {SIMPLE, "MOVA %s,PC"}},
    {"RETA", {SIMPLE, "MOVA @SP+,PC"}},
    {"CLRA", {SIMPLE, "MOV #0,%s"}},
    {"CLRX", {SIZE_SPECIFIC, "MOVX%s #0,%s"}},
    {"DADCX", {SIZE_SPECIFIC, "DADDX%s #0,%s"}},
    {"DECX", {SIZE_SPECIFIC, "SUBX%s #1,%s"}},
    {"DECDX", {SIZE_SPECIFIC, "SUBX%s #2,%s"}},
    {"INCX", {SIZE_SPECIFIC, "ADDX%s #1,%s"}},
    {"INCDX", {SIZE_SPECIFIC, "ADDX%s #2,%s"}},
    {"INVX", {SIZE_SPECIFIC, "XORX%s #–1,%s"}},
    {"RLAX", {SIZE_SPECIFIC, "ADDX%s %s,%s"}},
    {"RLCX", {SIZE_SPECIFIC, "ADDCX%s %s,%s"}},
    {"SBCX", {SIZE_SPECIFIC, "SUBCX%s #0,%s"}},
    {"TSTA", {SIMPLE, "CMPA #0,%s"}},
    {"TSTX", {SIZE_SPECIFIC, "CMPX%s #0,%s"}},
    {"POPX", {SIZE_SPECIFIC, "MOVX%s @SP+,%s"}}};

bool isDirectMnemonic(const std::string &mnemonic)
{
    return mappings.count(mnemonic) > 0;
}

std::string computeEmulatedMnemonic(const std::string &mnemonic_)
{
    std::string mnemonic = mnemonic_;

    std::regex rpt_pattern(R"(RPT \w+ (\w+\.\w+ \w+))");
    std::smatch rpt_match;
    if (std::regex_match(mnemonic, rpt_match, rpt_pattern))
    {
        mnemonic = rpt_match[1].str();
    }

    if (isDirectMnemonic(mnemonic))
    {
        return mappings[mnemonic].replacement;
    }

    std::regex generic_pattern(R"((\w+)\.?(\w*)\s+(.*))");

    std::smatch match;
    if (std::regex_match(mnemonic, match, generic_pattern))
    {
        std::string instruction = match[1].str();
        std::string size = match[2].str();
        std::string operand = match[3].str();

        if (mappings.count(instruction))
        {
            auto info = mappings[instruction];
            char buffer[256];
            switch (info.type)
            {
            case SIMPLE:
                snprintf(buffer, sizeof(buffer), info.replacement.c_str(),
                         operand.c_str());
                return buffer;
            case SIZE_SPECIFIC:
                snprintf(buffer, sizeof(buffer), info.replacement.c_str(),
                         size.c_str(), operand.c_str(), operand.c_str());
                return buffer;
            }
        }
    }

    return mnemonic;
}
