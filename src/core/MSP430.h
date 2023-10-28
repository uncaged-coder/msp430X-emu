#pragma once

#include <functional>
#include <stdint.h>
#include <string>
#include <vector>

#include "DevicesManager.h"
#include "MSP430InstructionHelper.h"
#include "Peripheral.h"

// Forward declarations
class MSP430InstructionHelper;
class MSP430TestHelper;
class Peripheral;
class DevicesManager;

class MSP430
{
public:
    // Status Register Structure
    union regStatus
    {
        struct __attribute__((packed))
        {
            uint8_t carry : 1;
            uint8_t zero : 1;
            uint8_t negative : 1;
            uint8_t GIE : 1;
            uint8_t CPUOFF : 1;
            uint8_t OSCOFF : 1;
            uint8_t SCG0 : 1;
            uint8_t SCG1 : 1;
            uint8_t overflow : 1;
            uint8_t reserved : 7;
            uint16_t unused;
        } status;
        uint32_t value;
    };

    MSP430();
    ~MSP430();

    void run();
    void runOneInstruction(Instruction *instr);
    bool loadROM(std::string filename);

    DevicesManager &getDevicesManager() { return devicesManager_; }

    static constexpr uint8_t NB_REGISTERS = 16;
    static constexpr uint8_t REG_IDX_PC = 0;
    static constexpr uint8_t REG_IDX_SP = 1;
    static constexpr uint8_t REG_IDX_SR = 2;
    static constexpr uint8_t REG_IDX_CG2 = 3;

private:
    uint32_t registers_[NB_REGISTERS];
    DevicesManager devicesManager_;

    // Register-related Methods
    uint16_t fetch();
    uint32_t *getRegPtr(uint8_t reg);
    void regIncPc();
    void setRegister(uint8_t reg, uint32_t value);
    uint32_t getRegister(uint8_t reg);
    regStatus getStatusRegister();
    void regInc(uint8_t reg, uint32_t value);
    void resetRegisters();
    void regSetOverflow(regStatus &status, bool value);
    void regSetNegative(regStatus &status, bool value);
    void regSetZero(regStatus &status, bool value);
    void regSetCarry(regStatus &status, bool value);
    void regDump();

    // Instruction-related Methods
    void updateInstructionSource(InstructionOperand *operand);
    void updateInstructionValue(InstructionOperand *operand);
    void updateInstructionDestination(InstructionOperand *operand);
    void decodeInstructionMajorOpcode00(Instruction *instr);
    void decodeInstructionMajorOpcode10(Instruction *instr);
    void decodeInstructionMajorOpcode14(Instruction *instr);
    void decodeCoreInstruction(Instruction *instr, bool extended);
    Instruction decodeInstruction();
    uint32_t getInstructionMaskValue(InstructionOperand *operand);
    uint32_t getInstructionSignMask(InstructionOperand *operand);

    // Function related to instruction run
    bool checkCondition(uint8_t opcode);
    void instructionWrite(Instruction *instr, uint32_t value, std::string str,
                          bool alwaysUpdatePc = true);
    void runCoreMSP430Instruction(Instruction *instr);
    void runExtendedMSP430Instruction(Instruction *instr);
    void runJumpInstruction(Instruction *instr, uint8_t opcode);
    void runMovInstruction(Instruction *instr);
    void runAddInstruction(Instruction *instr, bool isAddc = false);
    void runRrRlInstruction(Instruction *instr);
    void runSubInstruction(Instruction *instr, bool isSubc = false);
    void runCmpInstruction(Instruction *instr);
    void runDaddInstruction(Instruction *instr);
    uint32_t addBCD(Instruction *instr);
    void runRrcInstruction(Instruction *instr);
    void runRraInstruction(Instruction *instr);
    void runSwpbInstruction(Instruction *instr);
    void runSxtInstruction(Instruction *instr);
    void executeLogicalOp(Instruction *instr,
                          std::function<uint32_t(uint32_t, uint32_t)> op,
                          bool updateStatus, uint32_t operand1 = 0,
                          uint32_t operand2 = 0, bool isXor = false);
    void runBitInstruction(Instruction *instr);
    void runBicInstruction(Instruction *instr);
    void runBisInstruction(Instruction *instr);
    void runXorInstruction(Instruction *instr);
    void runAndInstruction(Instruction *instr);
    void runPushPopInstruction(Instruction *instr);
    void regUpdateStatusForLogicalOp(uint32_t result, uint32_t signMask,
                                     uint32_t operand1 = 0,
                                     uint32_t operand2 = 0, bool isXor = false);
    void handleTypeExt00(Instruction *instr);
    void handleTypeExt10(Instruction *instr);
    void handleTypeExt18(Instruction *instr);
    void handleType00(Instruction *instr);
    void handleType10(Instruction *instr);

    void displayDebugInformation();

    // Miscellaneous methods
    bool parseHexFile(std::string filename, std::vector<uint8_t> &data);
    void dumpDataToFile(const std::vector<uint8_t> &data, std::string filename);

    friend class ::MSP430TestHelper;
};
