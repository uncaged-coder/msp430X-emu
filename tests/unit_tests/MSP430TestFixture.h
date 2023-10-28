#pragma once

#include <stdint.h>

#include "MSP430TestHelper.h"

struct MSP430TestFixture
{
    MSP430TestHelper sim;

    MSP430TestFixture()
    {
        // Shared setup actions go here...
    }

    ~MSP430TestFixture()
    {
        // Any shared cleanup actions go here...
    }

    Instruction loadCodeAndRun(const uint16_t *code, size_t size)
    {
        sim.testLoadCode(code, size);
        Instruction instr = sim.testDecodeInstruction();
        sim.testRunInstruction(&instr);
        return instr;
    }
};
