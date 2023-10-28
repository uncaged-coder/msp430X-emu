#include "Uart.h"
#include <stdio.h>

Uart::Uart() : Peripheral() { port = 3; }

Uart::~Uart() {}

void Uart::txCb(uint8_t value)
{
    if (value & 0x10)
    {
        printf("uart tx bit set\n");
    }
    else
    {
        printf("uart tx bit unset\n");
    }
}

uint8_t Uart::rxCb() { return 0; }
