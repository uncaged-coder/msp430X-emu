#pragma once

#include <stdint.h>

/* Word sizes for MSP430 instructions */
enum WORD_SIZE
{
    __RESERVED = 0, /* Reserved value */
    _20B_WORD,      /* 20 bits word size */
    WORD,           /* 16 bits (standard) word size */
    BYTE            /* byte size */
};

/* Addressing modes for MSP430 instructions */
enum ADDRESSING_MODE
{
    ADDR_MODE_INVALID,
    ADDR_MODE_REGISTER,               /* Rn 00/0*/
    ADDR_MODE_INDEXED,                /* X(Rn) 01/1 */
    ADDR_MODE_SYMBOLIC,               /* ADDR 01/1 */
    ADDR_MODE_ABSOLUTE,               /* &ADDR 01/1 */
    ADDR_MODE_INDIRECT_REGISTER,      /* @Rn 01/~ */
    ADDR_MODE_INDIRECT_AUTOINCREMENT, /* @Rn+ !!/~ */
    ADDR_MODE_IMMEDIATE               /* #N 11/~ */
};

typedef struct
{
    uint32_t value;                // Operand value
    uint32_t address;              // address.
    uint8_t reg;                   // register number (if register addr mode)
    uint8_t axFlag;                // Addressing mode flag (asFlag or adFlag)
    enum WORD_SIZE wordSize;       // operand word size
    enum ADDRESSING_MODE addrMode; // operand addressing mode
    bool needUpdateValue; // if true, fetch additional instruction if needed
    bool processAsSource; //
    uint16_t additionalRawInstruction; // if applicable
    bool usedConstantGenerator;        // for debuging purpose
} InstructionOperand;

// major opcode
static constexpr uint8_t MAJOR_OPCODE_00 = 0x00;
static constexpr uint8_t MAJOR_OPCODE_10 = 0x10;
static constexpr uint8_t MAJOR_OPCODE_14 = 0x14;
static constexpr uint8_t MAJOR_OPCODE_18 = 0x18;
static constexpr uint8_t MAJOR_OPCODE_1C = 0x1C;
static constexpr uint8_t MAJOR_OPCODE_JNE_JNZ = 0x20;
static constexpr uint8_t MAJOR_OPCODE_JEQ_JZ = 0x24;
static constexpr uint8_t MAJOR_OPCODE_JNC = 0x28;
static constexpr uint8_t MAJOR_OPCODE_JC = 0x2C;
static constexpr uint8_t MAJOR_OPCODE_JN = 0x30;
static constexpr uint8_t MAJOR_OPCODE_JGE = 0x34;
static constexpr uint8_t MAJOR_OPCODE_JL = 0x38;
static constexpr uint8_t MAJOR_OPCODE_JMP = 0x3C;
static constexpr uint8_t MAJOR_OPCODE_MOV = 0x40;
static constexpr uint8_t MAJOR_OPCODE_ADD = 0x50;
static constexpr uint8_t MAJOR_OPCODE_ADDC = 0x60;
static constexpr uint8_t MAJOR_OPCODE_SUBC = 0x70;
static constexpr uint8_t MAJOR_OPCODE_SUB = 0x80;
static constexpr uint8_t MAJOR_OPCODE_CMP = 0x90;
static constexpr uint8_t MAJOR_OPCODE_DADD = 0xA0;
static constexpr uint8_t MAJOR_OPCODE_BIT = 0xB0;
static constexpr uint8_t MAJOR_OPCODE_BIC = 0xC0;
static constexpr uint8_t MAJOR_OPCODE_BIS = 0xD0;
static constexpr uint8_t MAJOR_OPCODE_XOR = 0xE0;
static constexpr uint8_t MAJOR_OPCODE_AND = 0xF0;

// minor opcode for extended major opcode 0x00
static constexpr uint8_t MINOR_EXT00_MOVA_INDIRECT_REGISTER = 0x0;
static constexpr uint8_t MINOR_EXT00_MOVA_INDIRECT_AUTOINCREMENT = 0x1;
static constexpr uint8_t MINOR_EXT00_MOVA_ABSOLUTE_SOURCE = 0x2;
static constexpr uint8_t MINOR_EXT00_MOVA_INDEXED_SOURCE = 0x3;
static constexpr uint8_t MINOR_EXT00_MOVA_ABSOLUTE_DESINTATION = 0x6;
static constexpr uint8_t MINOR_EXT00_MOVA_INDEXED_DESTINATION = 0x7;
static constexpr uint8_t MINOR_EXT00_MOVA_IMMEDIATE = 0x8;
static constexpr uint8_t MINOR_EXT00_MOVA_REGISTER = 0xC;
static constexpr uint8_t MINOR_EXT00_CMPA_IMMEDIATE = 0x9;
static constexpr uint8_t MINOR_EXT00_CMPA_REGISTER = 0xD;
static constexpr uint8_t MINOR_EXT00_ADDA_IMMEDIATE = 0xA;
static constexpr uint8_t MINOR_EXT00_ADDA_REGISTER = 0xE;
static constexpr uint8_t MINOR_EXT00_SUBA_IMMEDIATE = 0xB;
static constexpr uint8_t MINOR_EXT00_SUBA_REGISTER = 0xF;
static constexpr uint8_t MINOR_EXT00_RR_RL_A_OPCODE = 0x4;
static constexpr uint8_t MINOR_EXT00_RR_RL_W_OPCODE = 0x5;

// minor opcode for extended major opcode 0x10
static constexpr uint16_t MINOR_EXT10_RETI = 0x130;
static constexpr uint16_t MINOR_EXT10_CALLA_REGISTER = 0x134;
static constexpr uint16_t MINOR_EXT10_CALLA_INDEXED = 0x135;
static constexpr uint16_t MINOR_EXT10_CALLA_INDIRECT_REGISTER = 0x136;
static constexpr uint16_t MINOR_EXT10_CALLA_INDIRECT_AUTOINCREMENT = 0x137;
static constexpr uint16_t MINOR_EXT10_CALLA_ABSOLUTE = 0x138;
static constexpr uint16_t MINOR_EXT10_CALLA_SYMBOLIC = 0x139;
static constexpr uint16_t MINOR_EXT10_CALLA_IMMEDIATE = 0x13B;

// Structure representing a decoded MSP430 instruction
typedef struct InstructionStruct
{
    uint8_t extended : 1;   // extended flag
    uint8_t zc : 1;         // if 0, use the status of carry bit.
    uint8_t repetition : 4; // only for extended instruction.

    // Major opcode of the instruction. (first byte of rawinstruction)
    uint8_t majorOpcode;
    uint16_t minorOpcode;

    InstructionOperand source;
    InstructionOperand destination;

    // Internal data.
    uint8_t format : 2;         // Instruction format (I, II or III)
    uint16_t rawInstruction[4]; // Raw instruction word
} Instruction;

uint8_t decodeMajorOpcode(uint16_t rawInstruction);
void printInstruction(const Instruction *i);

const char *addressingModeStr(enum ADDRESSING_MODE addrMode);
uint8_t wordSizeToBytes(enum WORD_SIZE wordSize);
int wordSizeInBits(enum WORD_SIZE wordSize);
enum WORD_SIZE bwAlFlagToWordSize(uint8_t bwFlag, uint8_t alFlag,
                                  bool extended);
enum WORD_SIZE addrModeToWordSize(enum ADDRESSING_MODE addrMode,
                                  enum WORD_SIZE wordSize);
enum ADDRESSING_MODE adFlagToAddrMode(Instruction *instr);
enum ADDRESSING_MODE asFlagToAddrMode(Instruction *instr);
uint32_t getValueFromConstantGenerator(uint32_t source, uint8_t asFlag);

uint8_t opcodeToFormat(uint8_t opcode);
