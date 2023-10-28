#pragma once

#include <stdint.h>

class Peripheral
{
public:
    Peripheral(){};
    ~Peripheral(){};

    // MSP430 port number the device is connected to
    uint8_t port;

    virtual void txCb(uint8_t value) = 0;
    virtual uint8_t rxCb() = 0;
};
