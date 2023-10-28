#pragma once

#include <memory>

#include "MSP430.h"
#include "MSP430InstructionHelper.h"
#include "Memory.h"

class MSP430TestHelper : public MSP430
{
public:
    MSP430TestHelper();

    bool testLoadROM(const std::string romFile);
    void testLoadCode(const uint16_t *code, const size_t codeSize);
    Instruction testDecodeInstruction();
    void testRunInstruction(Instruction *instr);
    uint32_t testGetRegister(uint8_t reg);
    void testSetRegister(uint8_t reg, uint32_t value);
    uint8_t testDecodeMajorOpcode(uint16_t rawInstruction);
    std::shared_ptr<Memory> testGetMemory();
    void testRegDump();
};
