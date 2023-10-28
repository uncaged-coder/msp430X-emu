#include "Port1.h"

#define P1DIR 0x22
#define P1IE 0x25
#define P1IES 0x24
#define P1IFG 0x23
#define P1IN 0x20
#define P1OUT 0x21
#define P1REN 0x27
#define P1SEL 0x26

Port1::Port1()
    : Port("Port1", P1REN, P1IN, P1OUT, P1DIR, P1SEL, P1IFG, P1IES, P1IE)
{
    value_ = 0x91;
}
