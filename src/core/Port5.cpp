#include "Port5.h"

#define P5DIR 0x32
#define P5IN 0x30
#define P5OUT 0x31
#define P5REN 0x12
#define P5SEL 0x33

Port5::Port5() : Port("Port5", P5REN, P5IN, P5OUT, P5DIR, P5SEL, 0, 0, 0) {}
