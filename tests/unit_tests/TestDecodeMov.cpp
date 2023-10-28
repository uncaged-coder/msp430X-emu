#include <catch2/catch.hpp>

#include "MSP430InstructionHelper.h"
#include "MSP430TestFixture.h"
#include "MSP430TestHelper.h"

TEST_CASE_METHOD(MSP430TestFixture, "MOV Tests", "[MOV]")
{
    /******** 0x00 instructions ****************/

    /* MOVA Rsrc,Rdst */
    SECTION("MOVA Register Mode")
    {
        // Machine code for MOVA R4, R5
        uint16_t code[] = {0x04C5};
        sim.testSetRegister(4, 1234);
        sim.testSetRegister(5, 0);

        Instruction instr = loadCodeAndRun(code, sizeof(code));

        REQUIRE(instr.majorOpcode == 0x00);

        REQUIRE(instr.source.addrMode == ADDR_MODE_REGISTER);
        REQUIRE(instr.destination.addrMode == ADDR_MODE_REGISTER);
        REQUIRE(instr.source.reg == 4);
        REQUIRE(instr.destination.reg == 5);

        sim.testRunInstruction(&instr);

        // Check that R5 now has the value 1234 (which was in R4)
        REQUIRE(sim.testGetRegister(5) == 1234);
    }

    /******** 0x18 instructions ****************/

    /* CLRX.A  0x00000(SP) */
    SECTION("CLRX.A")
    {
        auto mem = sim.testGetMemory();
        uint16_t code[] = {0x1800, 0x43C1, 0x0000};

        sim.testSetRegister(MSP430::REG_IDX_SP, 0x20); // Set SP to 32
        // 0100 0011 1100 1111
        REQUIRE(sim.testGetRegister(MSP430::REG_IDX_SP) == 0x20);
        Instruction instr = loadCodeAndRun(code, sizeof(code));

        // source used constant generator to get 0
        REQUIRE(sim.testGetRegister(MSP430::REG_IDX_SP) == 0x20);
        REQUIRE(instr.source.usedConstantGenerator == true);
        REQUIRE(instr.source.value == 0);

        REQUIRE(instr.destination.addrMode == ADDR_MODE_INDEXED);
        REQUIRE(instr.destination.address == 0x20); // SP + 0

        sim.testRunInstruction(&instr);
        REQUIRE(mem->readWord(instr.destination.address) == 0);
    }
}
