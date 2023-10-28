#include "Port4.h"

#define P4DIR 0x1e
#define P4IN 0x1c
#define P4OUT 0x1d
#define P4REN 0x11
#define P4SEL 0x1f

Port4::Port4() : Port("Port4", P4REN, P4IN, P4OUT, P4DIR, P4SEL, 0, 0, 0) {}
