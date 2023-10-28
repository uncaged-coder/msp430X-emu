#include "Port6.h"

#define P6DIR 0x36
#define P6IN 0x34
#define P6OUT 0x35
#define P6REN 0x13
#define P6SEL 0x37

Port6::Port6() : Port("Port6", P6REN, P6IN, P6OUT, P6DIR, P6SEL, 0, 0, 0) {}
