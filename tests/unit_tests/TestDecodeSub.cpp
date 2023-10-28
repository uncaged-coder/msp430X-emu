#include "MSP430InstructionHelper.h"
#include "MSP430TestFixture.h"
#include "MSP430TestHelper.h"
#include <catch2/catch.hpp>

TEST_CASE_METHOD(MSP430TestFixture, "SUB Tests", "[SUB]")
{

    SECTION("SUBA Immediate Mode to Register")
    {

        Instruction instr;

        // Specific to this test
        uint16_t code[] = {0x00B1, 0x000E};            // SUBA #14, SP
        sim.testSetRegister(MSP430::REG_IDX_SP, 0x20); // Set SP to 32

        // Shared functionality (loading the code and executing it)
        instr = loadCodeAndRun(code, sizeof(code));

        REQUIRE(instr.majorOpcode == 0x00);

        // Checks specific to this test
        REQUIRE(instr.source.addrMode == ADDR_MODE_IMMEDIATE);
        REQUIRE(instr.source.value == 0xe);
        REQUIRE(sim.testGetRegister(MSP430::REG_IDX_SP) ==
                0x12); // 32 - 14 = 18

        // Reset for the next test
        sim.testSetRegister(MSP430::REG_IDX_SR, 0x0);
    }

    SECTION("SUBA Register to Register")
    {
        Instruction instr;

        uint16_t code[] = {0x06F7};   // SUBA R6, R7
        sim.testSetRegister(7, 0x10); // R7 = 16
        sim.testSetRegister(6, 0xA);  // R6 = 10

        instr = loadCodeAndRun(code, sizeof(code));

        REQUIRE(sim.testGetRegister(7) == 0x6);

        // Reset for the next test
        sim.testSetRegister(MSP430::REG_IDX_SR, 0x0);
    }

    SECTION("SUB Immediate Mode to Register")
    {
        Instruction instr;

        uint16_t code[] = {0x8034, 0x000E}; // SUB #14, R4
        sim.testSetRegister(4, 0x20);       // Set R0 to 32

        instr = loadCodeAndRun(code, sizeof(code));

        REQUIRE(sim.testGetRegister(4) == 0x12);

        // Reset for the next test
        sim.testSetRegister(MSP430::REG_IDX_SR, 0x0);
    }

    SECTION("SUB Register to Register")
    {
        Instruction instr;

        uint16_t code[] = {0x8907};   // SUB R9, R7
        sim.testSetRegister(7, 0x10); // R7 = 16
        sim.testSetRegister(9, 0x8);  // R9 = 8

        instr = loadCodeAndRun(code, sizeof(code));

        REQUIRE(sim.testGetRegister(7) == 0x8);

        // Reset for the next test
        sim.testSetRegister(MSP430::REG_IDX_SR, 0x0);
    }
}
