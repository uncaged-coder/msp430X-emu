#include <catch2/catch.hpp>

#include "MSP430InstructionHelper.h"
#include "MSP430TestHelper.h"

TEST_CASE("CALL test", "[CALL]")
{
    /* CALLA tests */
    SECTION("CALLA #imm20")
    {
#if 0
        MSP430TestHelper sim;
        Memory *mem = sim.testGetMemory();

        // Machine code for CALLA #imm20
        uint16_t code[] = {0x13B1, 0x1824};

        sim.testLoadCode(code, sizeof(code));
        Instruction instr = sim.testDecodeInstruction();

        REQUIRE(instr.addrMode == ADDR_MODE_IMMEDIATE);
        REQUIRE(instr.source == 0);
        REQUIRE(instr.destination == 0x11824); // R10

        sim.testRunInstruction(&instr);

        // Ensure the stack pointer decremented by 10 (5 words * 2 bytes each)
        //REQUIRE(sim.testGetRegister(REG_IDX_PC) ==  0x11824);
#endif
    }
}
