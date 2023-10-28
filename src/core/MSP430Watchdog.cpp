#include <assert.h>
#include <iostream>

#include "MSP430Watchdog.h"

MSP430Watchdog::MSP430Watchdog() : Device(0x120, 0x121, "wdog") { init(); }

MSP430Watchdog::~MSP430Watchdog()
{
    // Destructor implementation, if needed
}

void MSP430Watchdog::init()
{
    // Initialize the Watchdog Timer Control Register
    WDTCTL = 0x6900;
}

void MSP430Watchdog::destroy()
{
    // Cleanup if any is required when destroying the device
}

void MSP430Watchdog::update()
{
    // Update the device state, if required
}

uint16_t MSP430Watchdog::readWord(uint32_t address)
{
    // Address check is needed to ensure we're reading the correct register
    if (address == 0x0120) // The address of WDTCTL based on your map file
    {
        std::cout << "read from wdog register" << std::hex << WDTCTL
                  << std::endl;
        return WDTCTL;
    }
    // If the address doesn't match any known register, trigger an error
    assert(false);
    return 0;
}

void MSP430Watchdog::writeWord(uint32_t address, uint16_t value)
{
    std::cout << "write in wdog register" << std::endl;

    // Address check is needed to ensure we're writing to the correct register
    if (address == 0x0120)
    {
        WDTCTL = value & 0xFFFF; // Make sure only the lower 16 bits are used
    }
    else
    {
        // If the address doesn't match any known register, trigger an error
        assert(false);
    }
}
