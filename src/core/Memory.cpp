#include <assert.h>
#include <stdint.h>
#include <stdio.h>

#include "Memory.h"

Memory::Memory() : Device(0, MSP430F2618_MAX_MEMORY_ADDRESS, "mem") { init(); }

void Memory::init()
{
    // No specific initialization required for memory}
}

void Memory::destroy() {}

void Memory::update()
{
    // No update action required for memory
}

uint8_t Memory::readByte(uint32_t address)
{
    assert(address < memory.size());
    return memory[address];
}

uint16_t Memory::readWord(uint32_t address)
{
    assert(address + 1 < memory.size());
    uint16_t lowByte = memory[address];
    uint16_t highByte = memory[address + 1];
    return (highByte << 8) | lowByte;
}

uint32_t Memory::readDWord(uint32_t address)
{
    assert(address + 3 < memory.size());
    uint32_t result = 0;
    for (uint32_t i = 0; i < 4; i++)
    {
        result |= memory[address + i] << (8 * i);
    }
    return result;
}

void Memory::writeByte(uint32_t address, uint8_t value)
{
    assert(address < memory.size());
    memory[address] = value;
}

void Memory::writeWord(uint32_t address, uint16_t value)
{
    assert(address + 1 < memory.size());
    memory[address] = value & 0xFF;
    memory[address + 1] = (value >> 8) & 0xFF;
}

void Memory::writeDWord(uint32_t address, uint32_t value)
{
    assert(address + 3 < memory.size());
    for (uint32_t i = 0; i < 4; i++)
    {
        memory[address + i] = (value >> (8 * i)) & 0xFF;
    }
}

void Memory::dump(uint32_t address, uint32_t len)
{
    for (size_t line = 0; line < len / 16; line++)
    {
        printf("%3X: ", (unsigned int) (address + line * 16));
        for (size_t byte = 0; byte < 16; byte++)
        {
            if (address + line * 16 + byte >= memory.size())
            {
                break;
            }
            printf("%02X ", memory[address + line * 16 + byte]);
        }
        printf("\n");
    }
}
