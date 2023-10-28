#pragma once

#include <array>

#include "Device.h"

// or the actual max address based on the datasheet
static constexpr uint32_t MSP430F2618_MAX_MEMORY_ADDRESS = 0x1FFFFF;

class Memory : public Device
{
public:
    Memory();

    // Implémentations des méthodes de Device
    void init() override;
    void destroy() override;
    void update() override;

    uint8_t readByte(uint32_t address) override;
    uint16_t readWord(uint32_t address) override;
    uint32_t readDWord(uint32_t address) override;

    void writeByte(uint32_t address, uint8_t value) override;
    void writeWord(uint32_t address, uint16_t value) override;
    void writeDWord(uint32_t address, uint32_t value) override;

    void dump(uint32_t address, uint32_t len) override;

private:
    std::array<uint8_t, MSP430F2618_MAX_MEMORY_ADDRESS> memory;
};
