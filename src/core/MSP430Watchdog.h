#pragma once

#include <stdint.h>

#include "Device.h"

class MSP430Watchdog : public Device
{
public:
    MSP430Watchdog();
    ~MSP430Watchdog();

    // Override functions from Device interface
    void init() override;
    void destroy() override;
    void update() override;
    uint16_t readWord(uint32_t address) override;
    void writeWord(uint32_t address, uint16_t value) override;

    void writeByte(uint32_t address, uint8_t value) override {}

private:
    uint16_t WDTCTL; // Watchdog Timer Control Register
};
