#include <catch2/catch.hpp>

#include "MSP430InstructionHelper.h"
#include "MSP430TestHelper.h"

TEST_CASE("PUSHM", "[PUSHM]")
{
    /* PUSHM.A tests */
    SECTION("PUSHM.A #5,R10")
    {
        MSP430TestHelper sim;
        // Memory *mem = sim.testGetMemory();

        // Machine code for PUSHM.A #5, R10
        uint16_t code[] = {0x144A};

        sim.testLoadCode(code, sizeof(code));
        Instruction instr = sim.testDecodeInstruction();

        REQUIRE(instr.source.addrMode == ADDR_MODE_IMMEDIATE);
        REQUIRE(instr.source.value == 5);     // Since we're pushing 5 words
        REQUIRE(instr.destination.reg == 10); // R10

        // Set specific values to the registers R10 through R6
        const uint16_t testValues[5] = {0xAAAA, 0xBBBB, 0xCCCC, 0xDDDD, 0xEEEE};
        sim.testSetRegister(10, testValues[0]);
        sim.testSetRegister(9, testValues[1]);
        sim.testSetRegister(8, testValues[2]);
        sim.testSetRegister(7, testValues[3]);
        sim.testSetRegister(6, testValues[4]);

        // Backup the stack pointer before executing the instruction
        // uint16_t prevSP = sim.testGetRegister(1);

        sim.testRunInstruction(&instr);

        // Ensure the stack pointer decremented by 10 (5 words * 2 bytes each)
        // REQUIRE(sim.testGetRegister(1) == prevSP - 10);

        // Ensure the values on the stack match the previous register values
        for (int i = 0; i < 5; i++)
        {
            // REQUIRE(mem->read(prevSP - (i + 1) * 2) == testValues[i]);
        }
    }
}
