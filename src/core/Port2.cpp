#include "Port2.h"

#define P2DIR 0x2a
#define P2IE 0x2d
#define P2IES 0x2c
#define P2IFG 0x2b
#define P2IN 0x28
#define P2OUT 0x29
#define P2REN 0x2f
#define P2SEL 0x2e

Port2::Port2()
    : Port("Port2", P2REN, P2IN, P2OUT, P2DIR, P2SEL, P2IFG, P2IES, P2IE)
{
    value_ = 0xef;
}
