#pragma once

#include <stdint.h>

#include "Peripheral.h"

class Uart : public Peripheral
{
public:
    Uart();
    ~Uart();

    void txCb(uint8_t value);
    uint8_t rxCb();
};
