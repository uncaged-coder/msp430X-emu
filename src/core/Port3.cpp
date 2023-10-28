#include "Port3.h"

#define P3DIR 0x1a
#define P3IN 0x18
#define P3OUT 0x19
#define P3REN 0x10
#define P3SEL 0x1b

Port3::Port3() : Port("Port3", P3REN, P3IN, P3OUT, P3DIR, P3SEL, 0, 0, 0) {}
