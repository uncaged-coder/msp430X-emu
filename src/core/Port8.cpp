#include "Port8.h"

#define P8DIR 0x3d
#define P8IN 0x39
#define P8OUT 0x3b
#define P8REN 0x15
#define P8SEL 0x3f

Port8::Port8() : Port("Port8", P8REN, P8IN, P8OUT, P8DIR, P8SEL, 0, 0, 0) {}
