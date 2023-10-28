#include "Port7.h"

#define P7DIR 0x3c
#define P7IN 0x38
#define P7OUT 0x3a
#define P7REN 0x14
#define P7SEL 0x3e

Port7::Port7() : Port("Port7", P7REN, P7IN, P7OUT, P7DIR, P7SEL, 0, 0, 0) {}
