#include <assert.h>
#include <bits/fs_fwd.h>
#include <cstdint>
#include <iomanip> // for std::hex
#include <iostream>
#include <sstream>
#include <stdio.h>

#include "MSP430InstructionHelper.h"

using namespace std;

uint8_t decodeMajorOpcode(uint16_t rawInstruction)
{
    uint8_t majorOpcode;

    majorOpcode = (rawInstruction >> 8) & 0xFC;
    if (((majorOpcode & 0xF0) >= 0x40) || ((majorOpcode & 0xF0) == 0x00))
    {
        majorOpcode &= 0xF0;
    }
    return majorOpcode;
}

uint8_t wordSizeToBytes(enum WORD_SIZE wordSize)
{
    switch (wordSize)
    {
    case BYTE:
        return 1;
    case WORD:
        return 2;
    case _20B_WORD:
        return 4;
    default:
        assert(0);
        return 0;
    };
}

int wordSizeInBits(enum WORD_SIZE wordSize)
{
    switch (wordSize)
    {
    case BYTE:
        return 8;
    case WORD:
        return 16;
    case _20B_WORD:
        return 20;
    default:
        assert(0);
        return 0;
    };
}

const char *addressingModeStr(enum ADDRESSING_MODE addrMode)
{
    switch (addrMode)
    {
    case ADDR_MODE_INDEXED:
        return "IDX";
    case ADDR_MODE_REGISTER:
        return "REG";
    case ADDR_MODE_ABSOLUTE:
        return "ABS";
    case ADDR_MODE_SYMBOLIC:
        return "SYM";
    case ADDR_MODE_IMMEDIATE:
        return "IMM";
    case ADDR_MODE_INDIRECT_REGISTER:
        return "IND";
    case ADDR_MODE_INDIRECT_AUTOINCREMENT:
        return "IAI";
    case ADDR_MODE_INVALID:
        return "INV";
    default:
        return "???";
    };
}

static const char *wordSizeStr(enum WORD_SIZE wordSize)
{
    switch (wordSize)
    {
    case BYTE:
        return "BT";
    case WORD:
        return "WD";
    case _20B_WORD:
        return "20";
    default:
        return "??";
    };
}

static void printOperand(const char *operandStr, const InstructionOperand *opd)
{
    printf(
        "%s:[adm:%s size=%s value:%X CG=%d reg=%d addr=%X axFlag=%X nu=%d]\n",
        operandStr, addressingModeStr(opd->addrMode),
        wordSizeStr(opd->wordSize), opd->value, opd->usedConstantGenerator,
        opd->reg, opd->address, opd->axFlag, opd->needUpdateValue);
}

void printInstruction(const Instruction *i)
{
    printf("%X:%X format:%d ext:%d rep:%d %X\n", i->majorOpcode, i->minorOpcode,
           i->format, i->extended, i->repetition, i->rawInstruction[0]);

    printOperand("src", &(i->source));
    printOperand("dst", &(i->destination));
}

enum WORD_SIZE addrModeToWordSize(enum ADDRESSING_MODE addrMode,
                                  WORD_SIZE wordSize)
{
    if ((addrMode == ADDR_MODE_ABSOLUTE) || (addrMode == ADDR_MODE_INDEXED) ||
        (addrMode == ADDR_MODE_IMMEDIATE) ||
        (addrMode == ADDR_MODE_INDIRECT_AUTOINCREMENT))
    {
        return wordSize;
    }
    else
    {
        return WORD;
    }
}

enum WORD_SIZE bwAlFlagToWordSize(uint8_t bwFlag, uint8_t alFlag, bool extended)
{
    if (extended == false)
    {
        return (bwFlag == 0) ? WORD : BYTE;
    }
    else if ((alFlag == 0) && (bwFlag == 1))
    {
        return _20B_WORD;
    }
    else if ((alFlag == 1) && (bwFlag == 0))
    {
        return WORD;
    }
    else if ((alFlag == 1) && (bwFlag == 1))
    {
        return BYTE;
    }
    else
    {
        /* Reseved */
        assert(0);
        return WORD;
    }
}

enum ADDRESSING_MODE axFlagToAddrMode(uint8_t axFlag, uint8_t value)
{
    switch (axFlag)
    {
    case 0:
        return ADDR_MODE_REGISTER;
    case 1:
        if (value == 0)
        {
            return ADDR_MODE_SYMBOLIC;
        }
        else if (value == 2)
        {
            return ADDR_MODE_ABSOLUTE;
        }
        else
        {
            return ADDR_MODE_INDEXED;
        }
    case 2:
        return ADDR_MODE_INDIRECT_REGISTER;
    case 3:
        if (value == 0)
        {
            return ADDR_MODE_IMMEDIATE;
        }
        else
        {
            return ADDR_MODE_INDIRECT_REGISTER;
        }
    default:
        assert(0);
        return ADDR_MODE_REGISTER;
    };
}

enum ADDRESSING_MODE adFlagToAddrMode(Instruction *instr)
{
    if (instr->format == 1)
    {
        assert(instr->destination.axFlag <= 1);
    }
    return axFlagToAddrMode(instr->destination.axFlag, instr->destination.reg);
}

enum ADDRESSING_MODE asFlagToAddrMode(Instruction *instr)
{
    return axFlagToAddrMode(instr->source.axFlag, instr->source.reg);
}

uint8_t opcodeToFormat(uint8_t opcode)
{
    if ((opcode == 0x00) || (opcode >= 0x40))
    {
        return 1;
    }
    else if ((opcode == 0x10) || (opcode == 0x14))
    {
        return 2;
    }
    else if ((opcode >= 0x20) && (opcode <= 0x3C))
    {
        return 3;
    }
    else
    {
        assert(0);
        return 0;
    }
}

uint32_t getValueFromConstantGenerator(uint32_t source, uint8_t asFlag)
{
    /* R3 */
    if (source == 3)
    {
        switch (asFlag)
        {
        case 0b00:
            return 0;
        case 0b01:
            return 1;
        case 0b10:
            return 2;
        case 0b11:
            return 0xFFFF; /* -1 */
        default:
            assert(0);
            return 0;
        };
    }
    /* R2 */
    else if (source == 2)
    {
        switch (asFlag)
        {
        case 0b10:
            return 4;
        case 0b11:
            return 8;
        default:
            assert(0);
            return 0;
        };
    }
    else
    {
        assert(0);
        return 0;
    }
}
