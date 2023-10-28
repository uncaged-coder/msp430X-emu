#include <memory>

#include "DevicesManager.h"
#include "MSP430InstructionHelper.h"
#include "MSP430TestHelper.h"
#include "Memory.h"

MSP430TestHelper::MSP430TestHelper() : MSP430() {}

Instruction MSP430TestHelper::testDecodeInstruction()
{
    return decodeInstruction();
}

void MSP430TestHelper::testRunInstruction(Instruction *instr)
{
    runOneInstruction(instr);
}

uint32_t MSP430TestHelper::testGetRegister(uint8_t reg)
{
    return getRegister(reg);
}

void MSP430TestHelper::testSetRegister(uint8_t reg, uint32_t value)
{
    setRegister(reg, value);
}

std::shared_ptr<Memory> MSP430TestHelper::testGetMemory()
{
    return devicesManager_.getMemoryDevice();
}

uint8_t MSP430TestHelper::testDecodeMajorOpcode(uint16_t rawInstruction)
{
    return decodeMajorOpcode(rawInstruction);
}

void MSP430TestHelper::testLoadCode(const uint16_t *code, const size_t codeSize)
{
    auto mem = devicesManager_.getMemoryDevice();

    for (size_t i = 0; i < codeSize; i++)
    {
        mem->writeByte(2 * i, code[i] & 0xFF);
        mem->writeByte(2 * i + 1, code[i] >> 8);
    }

    setRegister(REG_IDX_PC, 0);
    setRegister(REG_IDX_SP, 32);
}

bool MSP430TestHelper::testLoadROM(const std::string romFile)
{
    return loadROM(romFile);
}

void MSP430TestHelper::testRegDump() { regDump(); }
