#include <arpa/inet.h>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <map>
#include <regex>
#include <sstream>
#include <string>
#include <vector>

#include "MSP430InstructionHelper.h"
#include "MSP430TestHelper.h"
#include "ReplayDs.h"

struct Change
{
    int reg_or_offset;
    uint32_t old_value;
    uint32_t new_value;
};

struct ParsedInstruction
{
    int number;
    int address;
    std::vector<uint16_t> rawInstruction;
    std::string mnemonic;
    std::string emulatedMnemonic;
    enum ADDRESSING_MODE srcAddressingMode;
    enum ADDRESSING_MODE destAddressingMode;
    std::vector<Change> registerChanges;
    std::vector<Change> stackChanges;
};

std::map<std::string, int> register_map = {
    {"PC", 0},   {"SP", 1},   {"SR", 2},   {"R3", 3},  {"R4", 4},   {"R5", 5},
    {"R6", 6},   {"R7", 7},   {"R8", 8},   {"R9", 9},  {"R10", 10}, {"R11", 11},
    {"R12", 12}, {"R13", 13}, {"R14", 14}, {"R15", 15}};

// FIXME: why +2 ?
static constexpr uint32_t stackAddr = 0x30E0 - 0x300 + 2;

void printInstruction(const ParsedInstruction &instr)
{
    std::cout << "Instruction Number: " << instr.number << std::endl;
    std::cout << "Address: " << std::hex << instr.address << std::endl;
    std::cout << "Raw Instruction: ";
    for (const auto &raw : instr.rawInstruction)
    {
        std::cout << std::hex << raw << " ";
    }
    std::cout << std::endl;
    std::cout << "Mnemonic: " << instr.mnemonic << std::endl;
    if (instr.mnemonic != instr.emulatedMnemonic)
    {
        std::cout << "Emulated Mnemonic: " << instr.emulatedMnemonic
                  << std::endl;
    }
    std::cout << "Src Addressing Mode: "
              << addressingModeStr(instr.srcAddressingMode) << std::endl;
    std::cout << "Dest Addressing Mode: "
              << addressingModeStr(instr.destAddressingMode) << std::endl;

    std::cout << "Changed Registers: " << std::endl;
    for (const auto &reg : instr.registerChanges)
    {
        std::cout << "  R" << reg.reg_or_offset << ": Old = " << reg.old_value
                  << ", New = " << reg.new_value << std::endl;
    }

    std::cout << "Changed Stack: " << std::endl;
    for (const auto &stack_change : instr.stackChanges)
    {
        std::cout << "  Offset " << std::hex << stack_change.reg_or_offset
                  << "  Addr " << stackAddr + stack_change.reg_or_offset
                  << ": Old = " << stack_change.old_value
                  << ", New = " << stack_change.new_value << std::endl;
    }
}

std::vector<ParsedInstruction>
extractInstructionsFromLog(const std::string &log_file)
{
    std::ifstream in(log_file);
    std::string line;
    std::vector<ParsedInstruction> instructions;
    std::regex instruction_pattern(R"(Instruction (\d+) (\w+):(\w+))");
    std::regex change_pattern(
        R"(Changed (Reg|Stack): (\w+), Old: (\w+), New: (\w+))");

    while (std::getline(in, line))
    {
        std::smatch match;
        if (std::regex_match(line, match, instruction_pattern))
        {
            ParsedInstruction inst;
            inst.number = std::stoi(match[1]);
            inst.address = std::stoi(match[2], 0, 16);
            inst.rawInstruction.push_back(std::stoi(match[3], 0, 16));
            instructions.push_back(inst);
        }
        else if (std::regex_match(line, match, change_pattern))
        {
            Change change;
            if (match[1] == "Reg")
            {
                change.reg_or_offset = register_map[match[2]];
            }
            else
            {
                change.reg_or_offset = std::stoi(
                    match[2], nullptr, 10); // Assuming stack offset is in hex
            }
            change.old_value =
                std::stoi(match[3], nullptr, 16); // Assuming values are in hex
            change.new_value =
                std::stoi(match[4], nullptr, 16); // Assuming values are in hex
            if (match[1] == "Reg")
            {
                instructions.back().registerChanges.push_back(change);
            }
            else
            {
                instructions.back().stackChanges.push_back(change);
            }
        }
    }

    std::cout << "==============================" << std::endl;
    return instructions;
}

enum ADDRESSING_MODE inferOperandAddressingMode(const std::string &operand)
{
    std::cout << "find addr mode for opd " << operand << std::endl;
    // Check if the operand is indirect
    if (operand.find("@R") != std::string::npos ||
        operand.find("@SP") != std::string::npos ||
        operand.find("@PC") != std::string::npos)
    {
        return operand.find("+") != std::string::npos
                   ? ADDR_MODE_INDIRECT_AUTOINCREMENT
                   : ADDR_MODE_INDIRECT_REGISTER;
    }

    if (operand.find("#") != std::string::npos)
        return ADDR_MODE_IMMEDIATE;

    if (operand.find("&") != std::string::npos)
        return ADDR_MODE_ABSOLUTE;

    // Check for indexed addressing mode
    if ((operand.find("(") != std::string::npos &&
         operand.find(")") != std::string::npos) ||
        operand.find("($C$L") != std::string::npos)
    {
        std::cout << "indexed" << std::endl;
        return ADDR_MODE_INDEXED;
    }

    if (operand.find("R") != std::string::npos ||
        operand.find("SP") != std::string::npos ||
        operand.find("PC") != std::string::npos)
        return ADDR_MODE_REGISTER;

    std::cout << "invalid" << std::endl;
    return ADDR_MODE_INVALID;
}

enum ADDRESSING_MODE extractAddressingMode(const std::string &mnemonic,
                                           bool isSource)
{
    std::stringstream ss(mnemonic);
    std::string opcode, operands, operand1, operand2;

    ss >> opcode >> operands;

    size_t commaPos = operands.find(',');
    if (commaPos != std::string::npos)
    {
        operand1 = operands.substr(0, commaPos);
        operand2 = operands.substr(commaPos + 1);
    }
    else
    {
        operand1 = operands;
    }
    std::cout << "****** process operand " << operand1 << " opd2: " << operand2
              << "is source ?" << isSource << std::endl;

    return isSource ? inferOperandAddressingMode(operand1)
                    : inferOperandAddressingMode(operand2);
}

uint32_t extractMainAddress(const std::string &asmFile)
{
    std::ifstream in(asmFile);
    std::string line;
    std::regex mainPattern(R"((\w+): +main:)");

    while (std::getline(in, line))
    {
        std::smatch match;
        if (std::regex_search(line, match, mainPattern))
        {
            return (uint32_t) std::stoi(match[1].str(), nullptr, 16);
        }
    }
    // If not found, return a sentinel value or throw an exception
    throw std::runtime_error("Main label not found in assembly file");
}

std::string stripAndReduceSpaces(const std::string &input)
{
    // First, remove leading and trailing spaces
    std::string trimmed = std::regex_replace(input, std::regex("^ +| +$"), "");

    // Then replace multiple spaces with a single space
    return std::regex_replace(trimmed, std::regex(" +"), " ");
}

void extractAssemblyData(const std::string &asm_file,
                         std::vector<ParsedInstruction> &instructions)
{
    std::ifstream in(asm_file);
    std::string line;
    std::regex asm_pattern(R"((\w+): (\w+) +(.+))");

    while (std::getline(in, line))
    {
        std::smatch match;
        if (std::regex_search(line, match, asm_pattern))
        {
            int address = std::stoi(match[1], 0, 16);
            int code = std::stoi(match[2], 0, 16);
            std::string mnemonic = stripAndReduceSpaces(match[3]);
            // std::cout << "got mnemo:" << mnemonic << std::endl;

            // Find the matching instruction based on the address
            for (auto &inst : instructions)
            {
                if (inst.address == address)
                {
                    inst.mnemonic = mnemonic;
                    inst.emulatedMnemonic = computeEmulatedMnemonic(mnemonic);
                    inst.srcAddressingMode =
                        extractAddressingMode(inst.emulatedMnemonic, true);
                    inst.destAddressingMode =
                        extractAddressingMode(inst.emulatedMnemonic, false);

                    // If the raw instruction doesn't match, it's an error
                    if (inst.rawInstruction[0] != htons(code))
                    {
                        std::cerr << "Mismatch in instruction code for address "
                                  << address << " :" << inst.rawInstruction[0]
                                  << " vs " << code << std::endl;
                    }
                    printInstruction(inst);
                }
            }
        }
    }
}

template <typename T>
bool testValues(const std::string &message, const T &value1, const T &value2)
{
    if (value1 != value2)
    {
        std::cerr << "**** ERROR on " << message << ": " << std::hex << value1
                  << " != " << std::hex << value2 << std::endl;
        return false;
    }

    return true;
}

bool checkDecodedInstruction(const Instruction &instr,
                             const ParsedInstruction &parsedInstr)
{
    if (!testValues("raw instruction", instr.rawInstruction[0],
                    parsedInstr.rawInstruction[0]))
    {
        return false;
    }
    if (!testValues("source addr mode", instr.source.addrMode,
                    parsedInstr.srcAddressingMode))
    {
        return false;
    }
    if (!testValues("dest addr mode", instr.destination.addrMode,
                    parsedInstr.destAddressingMode))
    {
        return false;
    }

    return true;
}

bool checkRegisters(MSP430TestHelper &sim, const ParsedInstruction &parsedInstr)
{
    bool ret = true;

    for (const Change &change : parsedInstr.registerChanges)
    {
        uint32_t emulatedRegValue = sim.testGetRegister(change.reg_or_offset);
        if (!testValues("Register mismatch R" +
                            std::to_string(change.reg_or_offset),
                        change.new_value, emulatedRegValue))
        {
            ret = false;
        }
    }

    return ret;
}

bool checkStack(MSP430TestHelper &sim, const ParsedInstruction &parsedInstr)
{
    bool ret = true;
    auto emulatedMemory = sim.testGetMemory();

    for (const Change &change : parsedInstr.stackChanges)
    {
        uint32_t addr = stackAddr + change.reg_or_offset;
        uint16_t emulatedStackValue = emulatedMemory->readWord(addr);

        // Convert number to hex string
        std::stringstream ss;
        ss << std::hex << addr;
        std::string hexAddr = ss.str();

        if (!testValues("Stack mismatch at address " + hexAddr,
                        (uint16_t) change.new_value, emulatedStackValue))
        {
            ret = false;
        }
    }

    return ret;
}

int main(int argc, char *argv[])
{
    MSP430TestHelper sim;

    if (argc != 4)
    {
        std::cerr << "Usage: " << argv[0] << " <fw_file> <log_file> <asm_file>"
                  << std::endl;
        return 1;
    }

    const std::string fwFile = argv[1];
    const std::string logFile = argv[2];
    const std::string asmFile = argv[3];

    if (sim.testLoadROM(fwFile) == false)
    {
        std::cout << "could not load FW " << fwFile << std::endl;
        return 1;
    }

    const uint32_t mainPC = extractMainAddress(asmFile);

    std::vector<ParsedInstruction> instructions =
        extractInstructionsFromLog(logFile);
    extractAssemblyData(asmFile, instructions);

    // Sort instructions by their number in ascending order
    std::sort(instructions.begin(), instructions.end(),
              [](const ParsedInstruction &a, const ParsedInstruction &b)
              { return a.number < b.number; });

    // CCS DS script start at main function.
#if 0
    sim.testSetRegister(MSP430::REG_IDX_PC, mainPC);
    sim.testSetRegister(MSP430::REG_IDX_SP, 0x30fc);
    sim.testSetRegister(2, 1);
    sim.testSetRegister(3, 0); // ?
    sim.testSetRegister(4, 0);
    sim.testSetRegister(5, 0x40);
    sim.testSetRegister(6, 0x89);
    sim.testSetRegister(7, 0x3c);
    sim.testSetRegister(8, 0x7f);
    sim.testSetRegister(9, 0x61b4);
    sim.testSetRegister(0xa, 0x0);
    sim.testSetRegister(0xb, 0x0); // ?
    sim.testSetRegister(0xc, 0x3f9c);
    sim.testSetRegister(0xd, 0x203c);
    sim.testSetRegister(0xe, 0);
    sim.testSetRegister(0xf, 0x5a80);
#else
    sim.testSetRegister(MSP430::REG_IDX_PC, mainPC);
    sim.testSetRegister(MSP430::REG_IDX_SP, 0x30fc);
    sim.testSetRegister(2, 3);
    sim.testSetRegister(3, 0); // ?
    sim.testSetRegister(4, 0);
    sim.testSetRegister(5, 0x40);
    sim.testSetRegister(6, 0x89);
    sim.testSetRegister(7, 0x3c);
    sim.testSetRegister(8, 0x7f);
    sim.testSetRegister(9, 0x61bd);
    sim.testSetRegister(0xa, 0x0);
    sim.testSetRegister(0xb, 0xfff);
    sim.testSetRegister(0xc, 0x0);
    sim.testSetRegister(0xd, 0x203c);
    sim.testSetRegister(0xe, 0);
    sim.testSetRegister(0xf, 0);
#endif
    auto emulatedMemory = sim.testGetMemory();
    emulatedMemory->writeWord(0x30d2, 0x61b4);

    std::cout << "==============================" << std::endl;

    // DS script has a bug: it skip first instruction log.
    {
        Instruction instr = sim.testDecodeInstruction();
        sim.testRunInstruction(&instr);
    }

    Instruction instr;
    memset(&instr, 0, sizeof(Instruction));

    const std::string SEPARATOR = "====================";
    const std::string ERR_MSG = "**** exit because check error\n";

    // Run through each parsed instruction
    for (const auto &parsedInstr : instructions)
    {
        uint32_t initialPC = sim.testGetRegister(MSP430::REG_IDX_PC);

        std::cout << "===========PC: " << std::hex << initialPC
                  << " SP: " << std::hex
                  << sim.testGetRegister(MSP430::REG_IDX_SP) << " " << SEPARATOR
                  << std::endl;

        // Display the parsed instruction (from CCS)
        printInstruction(parsedInstr);

        int currentRepetition = 0;
        bool hasError = false;

        do
        {
            // Decode and check the instruction from the firmware
            instr = sim.testDecodeInstruction();
            if (!checkDecodedInstruction(instr, parsedInstr))
            {
                std::cout << ERR_MSG;
                return 1;
            }

            // Display repetition info if applicable
            if (instr.repetition)
            {
                std::cout << "Repetition " << std::dec << instr.repetition
                          << " reg=" << std::hex << sim.testGetRegister(12)
                          << std::endl;
            }

            // Run the instruction
            sim.testRunInstruction(&instr);

            // Check for remaining repetitions
            if (instr.repetition > currentRepetition)
            {
                // Restore initial PC for next repetition
                sim.testSetRegister(MSP430::REG_IDX_PC, initialPC);
                currentRepetition++;
                continue;
            }
            break;
        } while (true);

        // Check registers and stack
        if (!checkRegisters(sim, parsedInstr) || !checkStack(sim, parsedInstr))
        {
            hasError = true;
        }

        if (hasError)
        {
            std::cout << ERR_MSG;
            return 1;
        }

        sim.testRegDump();
    }

    return 0;
}
