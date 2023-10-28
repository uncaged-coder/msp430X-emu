#include <assert.h>
#include <cstdint>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string.h>
#include <string>
#include <unistd.h>

#include "DevicesManager.h"
#include "MSP430.h"
#include "MSP430InstructionHelper.h"
#include "Peripheral.h"

using namespace std;

MSP430::MSP430() : devicesManager_()
{
    // Initialize microcontroller
    resetRegisters();
}

MSP430::~MSP430()
{
    // Destructor
}

void MSP430::resetRegisters()
{
    memset(registers_, 0, sizeof(registers_));
    setRegister(REG_IDX_SP, 0x2de0); // not necessary - initialized by FW
    //  through c_int00
    setRegister(REG_IDX_PC, 0x471c);
}

bool MSP430::parseHexFile(std::string filename, std::vector<uint8_t> &data)
{
    uint32_t extAddr = 0;

    // Open the file
    std::ifstream file(filename);

    // Check if the file was opened successfully
    if (!file.is_open())
    {
        std::cerr << "Failed to open file: " << filename << std::endl;
        return false;
    }

    // String to hold each line
    std::string line;

    // Initialize data to the maximum addressable size of MSP430 (assuming
    // 16-bit address space)
    data.resize(116 * 10240, 0);

    // Read and parse each line
    while (std::getline(file, line))
    {
        // Check if the line is a valid hex record (starts with ':')
        if (line[0] != ':')
        {
            continue;
        }

        // Remove the starting ':'
        line = line.substr(1);

        // Read the byte count, address, and record type
        int byteCount = std::stoi(line.substr(0, 2), nullptr, 16);
        int address = std::stoi(line.substr(2, 4), nullptr, 16);
        int recordType = std::stoi(line.substr(6, 2), nullptr, 16);

        // Process data records
        if (recordType == 0)
        {
            // Read the data
            for (int i = 0; i < byteCount; ++i)
            {
                uint8_t byte =
                    std::stoi(line.substr(8 + i * 2, 2), nullptr, 16);
                // Write byte to the corresponding address in the data vector
                assert(extAddr + address + i < 116 * 10240);

                data[extAddr + address + i] = byte;
            }
        }
        else if (recordType == 4)
        {
            int offset = std::stoi(line.substr(8, 4), nullptr, 16);
            std::cout << "offset = " << offset << std::endl;
            extAddr = (offset << 16);
            // assert(offset == 1);
        }
        else
        {
            std::cout << "not managed record" << recordType << std::endl;
        }
    }

    file.close();
    return true;
}

void MSP430::dumpDataToFile(const std::vector<uint8_t> &data,
                            std::string filename)
{
    // Open the file for writing
    std::ofstream file(filename, std::ios::binary);

    // Write each byte to the file
    for (uint8_t byte : data)
    {
        file.put(static_cast<char>(byte));
    }

    // Close the file
    file.close();
}

bool MSP430::loadROM(std::string filename)
{
    std::vector<uint8_t> data;

    // Parse the ROM file
    if (!parseHexFile(filename, data))
    {
        std::cerr << "Failed to parse the ROM file." << std::endl;
        return false;
    }

    // Dump the parsed data to a file for debugging
    dumpDataToFile(data, "dump.bin");

    // Check that the ROM file was large enough to provide a reset vector
    if (data.size() < 2)
    {
        std::cerr << "ROM file is too small." << std::endl;
        return false;
    }

    // Set the pc to the value at the reset vector
    // regs.pc = (static_cast<uint32_t>(data[data.size() - 2]) << 8) |
    // data[data.size() - 1];

    // Load the data into the emulator's memory
    for (size_t i = 0; i < data.size(); i++)
    {
        // std::cout << "write " << i << std::endl;
        devicesManager_.writeByte(i, data[i]);
    }
    // regs.pc = devicesManager_.read(0x31FE, 2);
    // regs.pc = devicesManager_.read(0x3100, 2);
    // regs.pc = 0x3100;
    // regs.pc = devicesManager_.read(0xFFFE, 2);
    // setRegister(REG_IDX_PC, 0x9128);
    std::cout << "vect reset addr=" << std::hex << getRegister(REG_IDX_PC)
              << std::endl;
    // regs.pc = devicesManager_.read(regs.pc, 2);
    // std::cout << "pc =" << std::hex << regs.pc << std::endl;

    std::cout << "done " << data.size() << std::endl;
    return true;
}

uint32_t *MSP430::getRegPtr(uint8_t reg)
{
    assert(reg < NB_REGISTERS);
    return &registers_[reg];
}

void MSP430::regInc(uint8_t reg, uint32_t value) { registers_[reg] += value; }

void MSP430::regIncPc(void) { regInc(REG_IDX_PC, 2); }

// Note: Additional methods for overflow and carry will depend on the specific
// operation being performed.
void MSP430::setRegister(uint8_t reg, uint32_t value)
{
    cout << "**** set reg" << std::dec << reg + 0 << " : " << std::hex << value
         << endl;
    registers_[reg] = value;
}

uint32_t MSP430::getRegister(uint8_t reg) { return registers_[reg]; }

MSP430::regStatus MSP430::getStatusRegister()
{
    regStatus st;
    st.value = getRegister(REG_IDX_SR);
    return st;
}

void MSP430::regSetOverflow(regStatus &status, bool value)
{
    status.status.overflow = value ? 1 : 0;
}

void MSP430::regSetNegative(regStatus &status, bool value)
{
    status.status.negative = value ? 1 : 0;
}

void MSP430::regSetZero(regStatus &status, bool value)
{
    status.status.zero = (value == 0) ? 1 : 0;
}

void MSP430::regSetCarry(regStatus &status, bool value)
{
    status.status.carry = value ? 1 : 0;
}

void MSP430::regDump()
{
    for (int i = 0; i < NB_REGISTERS; i++)
    {
        if (i == REG_IDX_PC)
        {
            printf("PC");
        }
        else if (i == REG_IDX_SP)
        {
            printf("SP");
        }
        else if (i == REG_IDX_SR)
        {
            printf("SR");
        }
        else if (i == REG_IDX_CG2)
        {
            printf("CG2");
        }
        else
        {
            printf("R%d", i);
        }
        printf(": %02X ", getRegister(i));
    }
    printf("\n");
}

uint16_t MSP430::fetch()
{
    uint16_t instruction = devicesManager_.readWord(getRegister(REG_IDX_PC));
    printf("PC:%X instruction:%X\n", getRegister(REG_IDX_PC), instruction);

    return instruction;
}

/**
 * \brief Decodes the core MSP430 instruction set.
 *
 * This function decodes the primary set of instructions that the MSP430 uses.
 * While it primarily handles standard instructions, it can also process
 * extended instructions when preceded by 0x18 or 0x1C. Note that it does not
 * decode pure MSP430 extended instructions like MOVA, and it does not process
 * 0x18 instructions directly.
 *
 * \param[in,out] instr Pointer to the Instruction structure to be decoded.
 * \param[in] extended Flag indicating if the instruction is extended.
 *
 * \return  none
 */
void MSP430::decodeCoreInstruction(Instruction *instr, bool isExtended)
{
    assert(instr != nullptr);

    InstructionOperand *src = &(instr->source);
    InstructionOperand *dst = &(instr->destination);
    uint16_t rawInstruction =
        isExtended ? instr->rawInstruction[1] : instr->rawInstruction[0];

    // Set basic instruction attributes
    instr->extended = isExtended;
    instr->minorOpcode = decodeMajorOpcode(rawInstruction);
    instr->format = opcodeToFormat(instr->minorOpcode);
    assert(instr->minorOpcode != 0x18 && instr->minorOpcode != 0x1C);

    uint8_t bwFlag = (rawInstruction & 0x0040) >> 6;
    uint8_t alFlag =
        isExtended ? ((instr->rawInstruction[0] & 0x0040) >> 6) : 0;

    // Assign source and destination attributes
    src->axFlag = (rawInstruction >> 4) & 0x3;
    dst->axFlag = (rawInstruction >> 7) & 0x1;
    dst->reg = (rawInstruction & 0xF);
    src->reg = ((rawInstruction >> 8) & 0xF);
    src->wordSize = dst->wordSize =
        bwAlFlagToWordSize(bwFlag, alFlag, isExtended);
    src->needUpdateValue = dst->needUpdateValue =
        (instr->format == 3) ? false : true;

    if (instr->format == 3)
    {
        src->addrMode = ADDR_MODE_INDEXED;
        dst->addrMode = ADDR_MODE_INVALID;
        dst->value = instr->rawInstruction[0] & 0x1FF;
        src->value = 0;
        return;
    }

    // Set addressing modes for src and dst
    src->addrMode = asFlagToAddrMode(instr);
    dst->addrMode = adFlagToAddrMode(instr);

    // Handle standard instruction
    if (!isExtended)
    {
        src->value = (src->addrMode == ADDR_MODE_INDEXED ||
                      src->addrMode == ADDR_MODE_ABSOLUTE)
                         ? 0
                         : src->reg;
        dst->value = (dst->addrMode == ADDR_MODE_INDEXED ||
                      dst->addrMode == ADDR_MODE_ABSOLUTE)
                         ? 0
                         : dst->reg;
        return;
    }

    // Handle extended instructions
    dst->value = instr->rawInstruction[0] & 0xF;
    src->value = (src->addrMode == ADDR_MODE_INDEXED ||
                  src->addrMode == ADDR_MODE_ABSOLUTE ||
                  src->addrMode == ADDR_MODE_IMMEDIATE)
                     ? (instr->rawInstruction[0] >> 7) & 0xF
                     : 0;
}

/**
 * Decodes MSP430 instructions with a major opcode of 0x10.
 *
 * MSP430 instructions with a major opcode of 0x10 are primarily associated with
 * CALLA and RETI operations. The function decodes these instructions and sets
 * the relevant fields in the passed instruction structure.
 *
 * @param instr Pointer to the instruction structure to be decoded.
 */
void MSP430::decodeInstructionMajorOpcode00(Instruction *instr)
{
    InstructionOperand *src = &(instr->source);
    InstructionOperand *dst = &(instr->destination);

    // Set default values for the instruction.
    instr->format = 1;
    instr->minorOpcode = (instr->rawInstruction[0] >> 4) & 0xF;
    instr->extended = 1;
    src->addrMode = ADDR_MODE_REGISTER;
    dst->addrMode = ADDR_MODE_REGISTER;
    src->reg = (instr->rawInstruction[0] >> 8) & 0xF;
    dst->reg = instr->rawInstruction[0] & 0xF;
    src->value = src->reg;
    dst->value = dst->reg;
    dst->needUpdateValue = true;
    src->needUpdateValue = true;
    // as/ad Flag not applicable
    src->axFlag = 0;
    dst->axFlag = 0;

    // Decode addressing mode based on minor opcode.
    switch (instr->minorOpcode)
    {
    case MINOR_EXT00_MOVA_INDIRECT_REGISTER:
        src->addrMode = ADDR_MODE_INDIRECT_REGISTER;
        break;

    case MINOR_EXT00_MOVA_INDIRECT_AUTOINCREMENT:
        src->addrMode = ADDR_MODE_INDIRECT_AUTOINCREMENT;
        break;

    case MINOR_EXT00_MOVA_ABSOLUTE_SOURCE:
        src->addrMode = ADDR_MODE_ABSOLUTE;
        break;

    case MINOR_EXT00_MOVA_INDEXED_SOURCE:
        src->addrMode = ADDR_MODE_INDEXED;
        break;

    case MINOR_EXT00_RR_RL_A_OPCODE:
    case MINOR_EXT00_RR_RL_W_OPCODE:
        src->addrMode = ADDR_MODE_IMMEDIATE;
        if (instr->minorOpcode == MINOR_EXT00_RR_RL_W_OPCODE)
        {
            src->wordSize = WORD; // .W extension
        }
        // source is immediate addr mode but fit in main raw code
        src->needUpdateValue = false;
        // source is only 2 bits (bits 10 and 11)
        src->value >>= 2;
        break;

    case MINOR_EXT00_MOVA_ABSOLUTE_DESINTATION:
        dst->addrMode = ADDR_MODE_ABSOLUTE;
        break;

    case MINOR_EXT00_MOVA_INDEXED_DESTINATION:
        dst->addrMode = ADDR_MODE_INDEXED;
        break;

    case MINOR_EXT00_MOVA_IMMEDIATE:
    case MINOR_EXT00_CMPA_IMMEDIATE:
    case MINOR_EXT00_ADDA_IMMEDIATE:
    case MINOR_EXT00_SUBA_IMMEDIATE:
        src->addrMode = ADDR_MODE_IMMEDIATE;
        break;

    case MINOR_EXT00_MOVA_REGISTER:
    case MINOR_EXT00_CMPA_REGISTER:
        // srcAddrMode and dstAddrMode are already set to register mode
        break;

    case MINOR_EXT00_ADDA_REGISTER:
    case MINOR_EXT00_SUBA_REGISTER:
        src->axFlag = 2; // FIXME ?
        // srcAddrMode and dstAddrMode are already set to register mode
        break;

    default:
        assert(0); // Invalid minor opcode
    }

    // Set the word size.
    if (instr->minorOpcode != MINOR_EXT00_RR_RL_W_OPCODE)
    {
        src->wordSize = addrModeToWordSize(src->addrMode, _20B_WORD);
    }
    dst->wordSize = addrModeToWordSize(dst->addrMode, _20B_WORD);
}

void MSP430::decodeInstructionMajorOpcode10(Instruction *instr)
{
    InstructionOperand *src = &(instr->source);
    InstructionOperand *dst = &(instr->destination);

    /* Not yet implemented */
    instr->format = 2;
    instr->extended = 1;
    dst->addrMode = ADDR_MODE_INVALID;
    dst->wordSize = WORD;
    src->wordSize = _20B_WORD;
    src->reg = instr->rawInstruction[0] & 0xF;
    dst->reg = 0;
    src->value = src->reg;
    dst->value = dst->reg;
    instr->minorOpcode = ((instr->rawInstruction[0] >> 4) & 0xFFF);
    dst->needUpdateValue = true;
    src->needUpdateValue = true;
    dst->processAsSource = false; // true;
    // as/ad Flag not applicable
    src->axFlag = 0xFF;
    dst->axFlag = 0xFF;

    /* RETI */
    if (instr->minorOpcode == MINOR_EXT10_RETI)
    {
        src->addrMode = ADDR_MODE_INVALID;
    }
    /* CALLA Rdst */
    else if (instr->minorOpcode == MINOR_EXT10_CALLA_REGISTER)
    {
        src->addrMode = ADDR_MODE_REGISTER;
    }
    /* CALLA x(Rdst) */
    else if (instr->minorOpcode == MINOR_EXT10_CALLA_INDEXED)
    {
        src->addrMode = ADDR_MODE_INDEXED;
    }
    /* CALLA @Rdst */
    else if (instr->minorOpcode == MINOR_EXT10_CALLA_INDIRECT_REGISTER)
    {
        src->addrMode = ADDR_MODE_INDIRECT_REGISTER;
    }
    /* CALLA @Rdst+ */
    else if (instr->minorOpcode == MINOR_EXT10_CALLA_INDIRECT_AUTOINCREMENT)
    {
        src->addrMode = ADDR_MODE_INDIRECT_AUTOINCREMENT;
    }
    /* CALLA &abs20 */
    else if (instr->minorOpcode == MINOR_EXT10_CALLA_ABSOLUTE)
    {
        src->addrMode = ADDR_MODE_ABSOLUTE;
    }
    /* CALLA EDE / CALLA x(PC) */
    else if (instr->minorOpcode == MINOR_EXT10_CALLA_SYMBOLIC)
    {
        src->addrMode = ADDR_MODE_SYMBOLIC;
    }
    /* CALLA #imm20 */
    else if (instr->minorOpcode == MINOR_EXT10_CALLA_IMMEDIATE)
    {
        src->addrMode = ADDR_MODE_IMMEDIATE;
    }
    else
    {
        cout << "Unimplemented Maj:00 Minor:" << hex << instr->minorOpcode
             << endl;
        /* Not yet implemented */
        src->addrMode = ADDR_MODE_INVALID;
        assert(0);
    }
}

void MSP430::decodeInstructionMajorOpcode14(Instruction *instr)
{
    InstructionOperand *src = &(instr->source);
    InstructionOperand *dst = &(instr->destination);
    instr->extended = 1;
    instr->format = 2;
    src->wordSize = WORD;
    src->addrMode = ADDR_MODE_IMMEDIATE;
    dst->addrMode = ADDR_MODE_REGISTER;
    instr->minorOpcode = (instr->rawInstruction[0] >> 8) & 0xFF;
    src->reg = (instr->rawInstruction[0] >> 4) & 0xF;
    dst->reg = instr->rawInstruction[0] & 0xF;
    src->value = src->reg;
    dst->value = dst->reg;
    dst->needUpdateValue = false;
    src->needUpdateValue = false;
    // as/ad Flag not applicable
    src->axFlag = 0xFF;
    dst->axFlag = 0xFF;

    /* POPM.A and PUSHM.A */
    if ((instr->minorOpcode == 0x14) || (instr->minorOpcode == 0x16))
    {
        // dst->wordSize = _20B_WORD;
        dst->wordSize = WORD;
    }
    else if ((instr->minorOpcode == 0x15) || (instr->minorOpcode == 0x17))
    {
        dst->wordSize = WORD;
    }
    else
    {
        assert(0);
    }
}

void MSP430::updateInstructionValue(InstructionOperand *opd)
{
    assert(opd != nullptr);
    uint32_t newValue = opd->value;
    uint32_t mask = getInstructionMaskValue(opd);
    uint8_t wordSizeBytes = wordSizeToBytes(opd->wordSize);

    bool fetchExtraData = false;

    switch (opd->addrMode)
    {
    case ADDR_MODE_REGISTER:
    case ADDR_MODE_INDIRECT_AUTOINCREMENT:
        break;
    case ADDR_MODE_IMMEDIATE:
    case ADDR_MODE_INDEXED:
    case ADDR_MODE_ABSOLUTE:
        fetchExtraData = true;
        break;
    default:
        fetchExtraData = (opd->wordSize != BYTE);
    }

    // Extend source by fetching extra bits on the next instruction.
    if (fetchExtraData && opd->needUpdateValue)
    {
        regIncPc();
        opd->additionalRawInstruction = fetch();
        newValue = (opd->value << 16) | opd->additionalRawInstruction;
        opd->value = newValue;
    }

    switch (opd->addrMode)
    {
    case ADDR_MODE_REGISTER:
        newValue = (*(getRegPtr(opd->reg)) & mask);
        break;

    case ADDR_MODE_INDEXED:
        opd->address = getRegister(opd->reg) + opd->value;
        newValue = devicesManager_.read(opd->address, wordSizeBytes) & mask;
        printf("read at @%X val=%X wz=%d\n", opd->address, newValue,
               wordSizeBytes);
        break;

    case ADDR_MODE_SYMBOLIC:
        opd->address = getRegister(REG_IDX_PC) + opd->value;
        newValue = devicesManager_.read(opd->address, wordSizeBytes) & mask;
        break;

    case ADDR_MODE_ABSOLUTE:
        opd->address = opd->value;
        newValue = devicesManager_.read(opd->address, wordSizeBytes) & mask;
        break;

    case ADDR_MODE_INDIRECT_REGISTER:
        opd->address = *getRegPtr(opd->reg);
        newValue = devicesManager_.read(opd->address, wordSizeBytes) & mask;
        break;

    case ADDR_MODE_INDIRECT_AUTOINCREMENT:
        newValue =
            devicesManager_.read(*getRegPtr(opd->reg), wordSizeBytes) & mask;
        *getRegPtr(opd->reg) += wordSizeBytes;
        break;

    case ADDR_MODE_IMMEDIATE:
        // Immediate values don't need to be fetched from memory
        break;

    case ADDR_MODE_INVALID:
        // No operation for invalid addressing mode
        break;

    default:
        assert(0); // Unknown addressing mode
    }

    if (opd->needUpdateValue)
    {
        opd->value = newValue;
    }
}

void MSP430::updateInstructionSource(InstructionOperand *src)
{
    // handle case of source defined by constant generator
    if (((src->reg == 2) && ((src->axFlag > 1) && (src->axFlag <= 3))) ||
        ((src->reg == 3) && (src->axFlag <= 3)))
    {
        src->value = getValueFromConstantGenerator(src->reg, src->axFlag);
        src->addrMode = ADDR_MODE_IMMEDIATE;
        src->usedConstantGenerator = true;
        return;
    }

    // This kind of operand (soruce or destination) is not used for current
    // instruction.
    if (src->addrMode == ADDR_MODE_INVALID)
    {
        return;
    }

    updateInstructionValue(src);
}

void MSP430::updateInstructionDestination(InstructionOperand *dst)
{
    // This kind of operand (soruce or destination) is not used for current
    // instruction.
    if (dst->addrMode == ADDR_MODE_INVALID)
    {
        return;
    }

    assert((dst->addrMode == ADDR_MODE_REGISTER) ||
           (dst->addrMode == ADDR_MODE_INDEXED) ||
           (dst->addrMode == ADDR_MODE_SYMBOLIC) ||
           (dst->addrMode == ADDR_MODE_ABSOLUTE));

    updateInstructionValue(dst);
}

/**
 * Decodes an MSP430 instruction.
 *
 * The function determines the type of instruction based on its major opcode,
 * and then decodes it accordingly. Extended instructions are handled
 * separately. After decoding, the function updates the source and destination
 * values for the instruction.
 *
 * @return The decoded instruction.
 */
Instruction MSP430::decodeInstruction(void)
{
    Instruction instr;

    cout << "******* start decode instruction" << endl;

    memset(&instr, 0, sizeof(instr));
    instr.rawInstruction[0] = fetch();
    instr.majorOpcode = decodeMajorOpcode(instr.rawInstruction[0]);
    assert(instr.rawInstruction[0]);

    // std::cout << "MajorOpcode=" << std::hex << (int)instr.majorOpcode <<
    // std::endl;

    // Handle extended instructions like MOVA, CMPA, ADDA, SUBA.
    // Referenced from p164 in MSP430 design manual.
    if (instr.majorOpcode == MAJOR_OPCODE_00)
    {
        decodeInstructionMajorOpcode00(&instr);
    }
    // Handle RETI, CALLA, and some reserved codes. Also includes RRC, SWPB,
    // RRA, SXT. Referenced from p165 in MSP430 design manual.
    else if (instr.majorOpcode == MAJOR_OPCODE_10)
    {
        decodeInstructionMajorOpcode10(&instr);
    }
    // Handle POPM and PUSHM.
    // Referenced from p165 in MSP430 design manual.
    else if (instr.majorOpcode == MAJOR_OPCODE_14)
    {
        decodeInstructionMajorOpcode14(&instr);
    }
    // If the instruction is an extension word (0x1800 or 0x1900).
    // Referenced from p151 in MSP430 design manual.
    else if ((instr.majorOpcode == MAJOR_OPCODE_18) ||
             (instr.majorOpcode == MAJOR_OPCODE_1C))
    {
        // check '#' repetition flag
        if (instr.rawInstruction[0] & 0x80)
        {
            instr.repetition = instr.rawInstruction[0] & 0xF;
            instr.repetition = getRegister(instr.repetition);
        }
        instr.zc = (instr.rawInstruction[0] & 0x100) ? 1 : 0;
        // Update the source and destination values of the instruction depending
        // on the addressing mode.
        regIncPc();
        instr.rawInstruction[1] = fetch();
        decodeCoreInstruction(&instr, true);
    }
    // Handle non-extended format.
    else
    {
        decodeCoreInstruction(&instr, false);
    }

    // Update the source and destination values of the instruction depending on
    // address mode.
    updateInstructionSource(&(instr.source));
    updateInstructionDestination(&(instr.destination));

    printInstruction(&instr);
    cout << "******* done decode instruction" << endl;
    return instr;
}

void MSP430::handleType00(Instruction *instr) { assert(0); }

void MSP430::handleTypeExt00(Instruction *instr)
{
    switch (instr->minorOpcode)
    {
    case MINOR_EXT00_MOVA_INDIRECT_REGISTER:
    case MINOR_EXT00_MOVA_INDIRECT_AUTOINCREMENT:
    case MINOR_EXT00_MOVA_ABSOLUTE_SOURCE:
    case MINOR_EXT00_MOVA_INDEXED_SOURCE:
    case MINOR_EXT00_MOVA_ABSOLUTE_DESINTATION:
    case MINOR_EXT00_MOVA_INDEXED_DESTINATION:
    case MINOR_EXT00_MOVA_IMMEDIATE:
    case MINOR_EXT00_MOVA_REGISTER:
        runMovInstruction(instr);
        break;

    case MINOR_EXT00_CMPA_IMMEDIATE:
    case MINOR_EXT00_CMPA_REGISTER:
        runCmpInstruction(instr);
        break;

    case MINOR_EXT00_ADDA_IMMEDIATE:
    case MINOR_EXT00_ADDA_REGISTER:
        runAddInstruction(instr);
        break;

    case MINOR_EXT00_SUBA_IMMEDIATE:
    case MINOR_EXT00_SUBA_REGISTER:
        runSubInstruction(instr);
        break;

    case MINOR_EXT00_RR_RL_A_OPCODE:
    case MINOR_EXT00_RR_RL_W_OPCODE:
        runRrRlInstruction(instr);
        break;

    default:
        assert(0);
    };
}

void MSP430::handleType10(Instruction *instr) { assert(0); }

void MSP430::handleTypeExt10(Instruction *instr)
{
    switch (instr->minorOpcode)
    {
    /* RETI */
    case MINOR_EXT10_RETI:
        assert(0);
        break;

    /* CALLA */
    case MINOR_EXT10_CALLA_REGISTER:
    case MINOR_EXT10_CALLA_INDEXED:
    case MINOR_EXT10_CALLA_INDIRECT_REGISTER:
    case MINOR_EXT10_CALLA_INDIRECT_AUTOINCREMENT:
    case MINOR_EXT10_CALLA_ABSOLUTE:
    case MINOR_EXT10_CALLA_SYMBOLIC:
    case MINOR_EXT10_CALLA_IMMEDIATE:
        regIncPc();
        /*  Push current PC onto the stack */
        regInc(REG_IDX_SP, -2);
        devicesManager_.writeWord(getRegister(REG_IDX_SP),
                                  getRegister(REG_IDX_PC) >> 16);
        regInc(REG_IDX_SP, -2);
        devicesManager_.writeWord(getRegister(REG_IDX_SP),
                                  getRegister(REG_IDX_PC) & 0xFFFF);
        setRegister(REG_IDX_PC, instr->source.value);
        printf("CALLA #imm20 %X\n", getRegister(REG_IDX_PC));
        break;

    /* Reserved */
    default:
        assert(0);
        break;
    };
}

void MSP430::runPushPopInstruction(Instruction *instr)
{
    uint32_t dst = instr->destination.value;
    uint32_t nMinus1 = instr->source.value;

    uint32_t wordSizeInBytes =
        (instr->minorOpcode == 0x14 || instr->minorOpcode == 0x16) ? 4 : 2;

    switch (instr->minorOpcode)
    {
    /* PUSHM.A */
    case 0x14:
    /* PUSHM.W */
    case 0x15:
        printf("PUSHM.X %X %X\n", nMinus1, dst);

        // Push the n registers onto the stack
        for (uint32_t i = 0; i <= nMinus1; ++i)
        {
            // Decrement the stack pointer
            regInc(REG_IDX_SP, -wordSizeInBytes);

            // Get the value of the register to push
            assert(dst - i <= 0xf);
            uint32_t reg_value = *getRegPtr(dst - i);

            // Push the value onto the stack
            devicesManager_.write(getRegister(REG_IDX_SP), reg_value,
                                  wordSizeInBytes);
            printf("wrote reg:%d at %X: %X (%d)\n", i, getRegister(REG_IDX_SP),
                   reg_value, wordSizeInBytes);
        }
        regIncPc();
        break;

    /* POPM.A */
    case 0x16:
    /* POPM.W */
    case 0x17:
        printf("POPM.X %X %X\n", nMinus1, dst);
        // Pop the n registers from the stack
        for (uint32_t i = 0; i <= nMinus1; ++i)
        {
            // Get the value from the stack
            uint32_t reg_value =
                devicesManager_.read(getRegister(REG_IDX_SP), wordSizeInBytes);

            printf("read reg:%d from %X: %X (%d)\n", i, getRegister(REG_IDX_SP),
                   reg_value, wordSizeInBytes);

            // Update the register
            setRegister(dst + i, reg_value);

            // Increment the stack pointer
            regInc(REG_IDX_SP, wordSizeInBytes);
        }
        regIncPc();
        break;

    default:
        assert(0);
        break;
    };
}

void MSP430::handleTypeExt18(Instruction *instr)
{
    runCoreMSP430Instruction(instr);
}

uint32_t MSP430::getInstructionMaskValue(InstructionOperand *operand)
{
    uint32_t mask = 0;

    switch (operand->wordSize)
    {
    case _20B_WORD:
        mask = 0x000FFFFF;
        break;
    case WORD:
        mask = 0xFFFF;
        break;
    case BYTE:
        mask = 0xFF;
        break;
    default:
        assert(0);
    };

    return mask;
}

uint32_t MSP430::getInstructionSignMask(InstructionOperand *operand)
{
    uint32_t mask = getInstructionMaskValue(operand);
    mask += 1;
    mask >>= 1;
    return mask;
}

void MSP430::instructionWrite(Instruction *instr, uint32_t value,
                              std::string str, bool alwaysIncPc)
{
    bool updatedRegPc = false;
    uint8_t nbBytes;
    InstructionOperand *dst = &(instr->destination);

    if (dst->addrMode == ADDR_MODE_REGISTER)
    {
        printf("write %X to reg %d\n", value, dst->reg);
        setRegister(dst->reg, value);
        if (dst->reg == REG_IDX_PC)
            updatedRegPc = true;
    }
    else
    {
        nbBytes = wordSizeToBytes(dst->wordSize);
        devicesManager_.write(dst->address, value, nbBytes);
    }

    printf("%s %X -> dest\n", str.c_str(), value);
    // if (updatedRegPc == false || alwaysIncPc)

    if (alwaysIncPc || (updatedRegPc == false))
    {
        regIncPc();
    }
}

void MSP430::runRrcInstruction(Instruction *instr)
{
    regStatus sr = getStatusRegister();
    uint32_t dstValue = getRegister(instr->destination.reg);

    if (instr->destination.wordSize == BYTE)
    {
        uint8_t oldCarry = sr.status.carry;
        sr.status.carry =
            dstValue & 0x01; // Update carry with LSB of the destination
        dstValue = (dstValue >> 1) | (oldCarry << 7); // 7 for BYTE
    }
    else
    {
        uint8_t oldCarry = sr.status.carry;
        sr.status.carry =
            dstValue & 0x0001; // Update carry with LSB of the destination
        dstValue = (dstValue >> 1) | (oldCarry << 15); // 15 for WORD
    }

    setRegister(instr->destination.reg, dstValue);
    setRegister(REG_IDX_SR, sr.value);
    regIncPc();
    printf("Rrc\n");
}

void MSP430::runRraInstruction(Instruction *instr)
{
    uint32_t dstValue = getRegister(instr->destination.reg);

    if (instr->destination.wordSize == BYTE)
    {
        uint32_t signBit = dstValue & 0x80; // Get the 8th bit for BYTE
        dstValue = (dstValue >> 1) | signBit;
    }
    else
    {
        uint32_t signBit = dstValue & 0x8000; // Get the MSB for WORD
        dstValue = (dstValue >> 1) | signBit;
    }

    setRegister(instr->destination.reg, dstValue);
    regIncPc();
    printf("Rra\n");
}

void MSP430::runSwpbInstruction(Instruction *instr)
{
    uint32_t dstValue = getRegister(instr->destination.reg);

    // Only applicable for word operations
    if (instr->destination.wordSize != BYTE)
    {
        dstValue = ((dstValue & 0xFF00) >> 8) |
                   ((dstValue & 0x00FF) << 8); // Swap bytes
        setRegister(instr->destination.reg, dstValue);
    }

    regIncPc();
    printf("Swp\n");
}

void MSP430::runSxtInstruction(Instruction *instr)
{
    uint32_t dstValue = getRegister(instr->destination.reg);

    if (instr->destination.wordSize == BYTE)
    {
        uint32_t signBit = dstValue & 0x40; // Get the 7th bit for BYTE
        dstValue = (dstValue & 0x7F) | (signBit << 1); // Sign extend
    }
    else
    {
        uint32_t signBit = dstValue & 0x0080; // Get the 8th bit for WORD
        dstValue =
            signBit ? (dstValue | 0xFF00) : (dstValue & 0x00FF); // Sign extend
    }

    setRegister(instr->destination.reg, dstValue);
    regIncPc();
    printf("Sxt\n");
}

bool MSP430::checkCondition(uint8_t opcode)
{
    regStatus sr = getStatusRegister();

    switch (opcode)
    {
    case MAJOR_OPCODE_JNE_JNZ:
        return !sr.status.zero;
    case MAJOR_OPCODE_JEQ_JZ:
        return sr.status.zero;
    case MAJOR_OPCODE_JNC:
        return !sr.status.carry;
    case MAJOR_OPCODE_JC:
        return sr.status.carry;
    case MAJOR_OPCODE_JN:
        return sr.status.negative;
    case MAJOR_OPCODE_JGE:
        // JGE is true if N == V, where N is the negative flag and V is the
        // overflow flag
        return sr.status.negative == sr.status.overflow;
    case MAJOR_OPCODE_JL:
        // JL is true if N != V
        return sr.status.negative != sr.status.overflow;
    case MAJOR_OPCODE_JMP:
        return true; // Always true for an unconditional jump
    default:
        return false;
    }
}

void MSP430::runJumpInstruction(Instruction *instr, uint8_t opcode)
{
    // InstructionOperand *dst = &(instr->destination);
    int offset;

    if (checkCondition(opcode))
    {
        // dst->value is a 10-bit value from the instruction, so we mask it
        // with 0x3FF to ensure it's indeed 10 bits
        uint16_t rawOffset = instr->rawInstruction[0] & 0x3FF;

        // Check if the most significant bit (bit 9) is set
        if (rawOffset & 0x200)
        {
            // This is a negative offset; convert it to a signed 10-bit integer
            offset = rawOffset - 0x400; // Equivalent to taking two's complement
        }
        else
        {
            // This is a positive offset
            offset = rawOffset;
        }

        // Convert the 10-bit offset to a byte offset and add 2
        offset = (offset * 2) + 2;
        printf("JUMP %X\n", offset);

        // Update the program counter
        setRegister(REG_IDX_PC, getRegister(REG_IDX_PC) + offset);
    }
    else
    {
        printf("JUMP continue\n");
        // Move to the next instruction (increment PC)
        regIncPc();
    }
}

/* MOV/MOV.B instruction */
void MSP430::runMovInstruction(Instruction *instr)
{
    if (instr->source.value == 0 && instr->destination.reg == REG_IDX_SR &&
        instr->source.addrMode == ADDR_MODE_IMMEDIATE &&
        instr->destination.addrMode == ADDR_MODE_REGISTER)
    {
        printf("NOP\n");
        return;
    }
    instructionWrite(instr, instr->source.value, "MOV", false);
}

void MSP430::runAddInstruction(Instruction *instr, bool isAddc)
{
    regStatus sr = getStatusRegister();

    uint32_t value = instr->destination.value + instr->source.value;

    if (isAddc)
    {
        value = value + sr.status.carry;
    }

    instructionWrite(instr, value, isAddc ? "ADDC" : "ADD");

    // Get the mask for the current word size
    uint32_t mask = getInstructionMaskValue(&instr->destination);
    uint32_t signBit = (mask + 1) >> 1;

    // Zero flag
    sr.status.zero = (value & mask) == 0;

    // Negative flag
    sr.status.negative = (value & signBit) != 0;

    // Carry flag (For addition, carry is set if an overflow occurs from the
    // most significant bit)
    sr.status.carry = (value > mask);

    // Overflow flag (For addition: sign of both operands is the same AND sign
    // of result is different from them)
    bool destSign = (instr->destination.value & signBit) != 0;
    bool srcSign = (instr->source.value & signBit) != 0;
    bool resultSign = (value & signBit) != 0;
    sr.status.overflow = (destSign == srcSign) && (destSign != resultSign);

    setRegister(REG_IDX_SR, sr.value);
    printf("ADD\n");
}

/**
 * Executes MSP430 RR/RL instructions.
 *
 * This function handles the execution of the RR/RL (Rotate Right/Left)
 * instructions for the MSP430 architecture. It covers RRCM, RRAM, RLAM,
 * and RRUM operations. The function fetches the source and destination
 * operands from the provided instruction structure, performs the
 * specified operation, and updates the destination register.
 *
 * @param instr Pointer to the instruction structure to be executed.
 */
void MSP430::runRrRlInstruction(Instruction *instr)
{
    regStatus sr = getStatusRegister();

    uint8_t instructionId =
        (instr->rawInstruction[0] >> 8) & 0x3; // instruction id

    // Determine the number of rotations or shifts
    uint8_t numRotations = instr->source.value + 1;

    // Get the current value from the destination register
    uint32_t mask = getInstructionMaskValue(&instr->destination);
    uint32_t dstValue = getRegister(instr->destination.reg) & mask;
    uint32_t carryBit = 0; // Placeholder for carry bit

    // Perform the operation based on the instruction ID
    switch (instructionId)
    {
    case 0: // RRCM
        // Rotate right through carry
        carryBit = (sr.status.carry) ? mask : 0;
        dstValue = (dstValue >> numRotations) | carryBit;
        break;

    case 1: // RRAM
        // Arithmetic right shift
        for (uint8_t i = 0; i < numRotations; i++)
        {
            dstValue = (dstValue >> 1) | (dstValue & mask); // Preserve sign bit
        }
        break;

    case 2: // RLAM
        // Shift left and handle carry
        for (uint8_t i = 0; i < numRotations; i++)
        {
            dstValue = (dstValue << 1) & mask;
            if (dstValue & (mask + 1)) // Check overflow
            {
                dstValue |= 1; // Set the lowest bit if overflow
            }
        }
        break;

    case 3: // RRUM
        // Logical right shift
        dstValue = dstValue >> numRotations;
        break;

    default:
        assert(0); // Invalid instruction ID
    }

    // Write the result back to the destination register
    setRegister(instr->destination.reg, dstValue);

    // Increment the program counter
    regIncPc();

    // Update status register
    uint32_t signMask = getInstructionSignMask(&instr->source);
    regSetNegative(sr, dstValue & signMask);
    regSetZero(sr, dstValue);

    // Carry is set from the bit that's about to be shifted out
    regSetCarry(sr,
                dstValue & (1 << (wordSizeInBits(instr->destination.wordSize) -
                                  numRotations)));

    // V flag is undefined for RLAM, but we reset it
    regSetOverflow(sr, false);

    setRegister(REG_IDX_SR, sr.value);
    printf("RlRr\n");
}

void MSP430::runSubInstruction(Instruction *instr, bool isSubc)
{
    // Get the mask for the current word size
    uint32_t mask = getInstructionMaskValue(&instr->destination);

    regStatus sr = getStatusRegister();
    uint32_t value = (instr->destination.value - instr->source.value) & mask;

    if (isSubc)
    {
        value = value - (1 - sr.status.carry);
    }
    else
    {
        printf("sub %X -%X\n", instr->destination.value, instr->source.value);
    }

    instructionWrite(instr, value, isSubc ? "SUBC" : "SUB");

    uint32_t signBit = (mask + 1) >> 1;

    // Zero flag
    sr.status.zero = (value & mask) == 0;

    // Negative flag
    sr.status.negative = (value & signBit) != 0;

    // Carry flag (For subtraction, carry is set if no borrow is needed)
    sr.status.carry = (instr->destination.value >= instr->source.value);

    // Overflow flag (For subtraction: sign of minuend != sign of subtrahend AND
    // sign of minuend != sign of result)
    bool destSign = (instr->destination.value & signBit) != 0;
    bool srcSign = (instr->source.value & signBit) != 0;
    bool resultSign = (value & signBit) != 0;
    sr.status.overflow = (destSign != srcSign) && (destSign != resultSign);

    setRegister(REG_IDX_SR, sr.value);
    printf("Sub");
}

void MSP430::runCmpInstruction(Instruction *instr)
{
    regStatus sr = getStatusRegister();
    uint32_t value = instr->destination.value - instr->source.value;

    // Get the mask for the current word size
    uint32_t mask = getInstructionMaskValue(&instr->destination);
    uint32_t signBit = (mask + 1) >> 1;

    // Zero flag
    sr.status.zero = (value & mask) == 0;

    // Negative flag
    sr.status.negative = (value & signBit) != 0;

    // Carry flag (For subtraction, carry is set if no borrow was required)
    sr.status.carry = (instr->destination.value >= instr->source.value);

    // Overflow flag (For subtraction: sign of both operands is different AND
    // sign of result is different from destination)
    bool destSign = (instr->destination.value & signBit) != 0;
    bool srcSign = (instr->source.value & signBit) != 0;
    bool resultSign = (value & signBit) != 0;
    sr.status.overflow = (destSign != srcSign) && (destSign != resultSign);

    setRegister(REG_IDX_SR, sr.value);
    printf("Cmp\n");

    // Note: We don't modify the destination operand for CMP instruction

    regIncPc();
}

uint32_t MSP430::addBCD(Instruction *instr)
{
    // Ensure both operands have the same size
    assert(instr->source.wordSize == instr->destination.wordSize);

    uint32_t mask = getInstructionMaskValue(&(instr->source));
    uint32_t value1 = instr->source.value;
    uint32_t value2 = instr->destination.value;
    uint32_t result = 0;

    for (uint32_t currentMask = 0x0F; currentMask <= mask; currentMask <<= 4)
    {
        uint32_t digit1 = value1 & currentMask;
        uint32_t digit2 = value2 & currentMask;

        uint32_t sum = digit1 + digit2 + (result & currentMask);
        if ((sum & currentMask) > 9)
        {
            sum += 6;
        }

        result |= sum;
    }

    return result & mask;
}

void MSP430::runDaddInstruction(Instruction *instr)
{
    regStatus sr = getStatusRegister();
    uint32_t value = addBCD(instr);

    // Get the mask for the current word size
    uint32_t mask = getInstructionMaskValue(&instr->destination);
    uint32_t signBit = (mask + 1) >> 1;

    // Zero flag
    sr.status.zero = (value & mask) == 0;

    // Negative flag (should not be valid for BCD addition, but setting it for
    // consistency)
    sr.status.negative = (value & signBit) != 0;

    // Carry flag (For DADD, carry is set if the result is greater than 9999 for
    // 16-bits, or 99 for 8-bits)
    sr.status.carry = (value > mask);

    // Overflow flag - not applicable for DADD.

    instructionWrite(instr, value, "DADD");

    setRegister(REG_IDX_SR, sr.value);
    printf("Dadd\n");
}

void MSP430::regUpdateStatusForLogicalOp(uint32_t result, uint32_t signMask,
                                         uint32_t operand1, uint32_t operand2,
                                         bool isXor)
{
    regStatus sr = getStatusRegister();

    printf("result=%X\n", result);
    regSetZero(sr, result);
    regSetNegative(sr, result & signMask);
    regSetCarry(sr, result != 0); // Carry is set if result is not zero

    // Overflow flag update for XOR operation
    if (isXor && (operand1 & 0x8000) &&
        (operand2 & 0x8000)) // Check if both operands have their MSB set
        regSetOverflow(sr, true);
    else
        regSetOverflow(sr, false);

    setRegister(REG_IDX_SR, sr.value);
}

// Function to execute a logical operation and (optionally) update the status
// register
void MSP430::executeLogicalOp(Instruction *instr,
                              std::function<uint32_t(uint32_t, uint32_t)> op,
                              bool updateStatus, uint32_t operand1,
                              uint32_t operand2, bool isXor)
{
    uint32_t signMask = getInstructionSignMask(&instr->source);
    uint32_t result = op(instr->destination.value, instr->source.value);
    instructionWrite(instr, result, "");

    if (updateStatus)
    {
        regUpdateStatusForLogicalOp(result, signMask, operand1, operand2,
                                    isXor);
    }
}

void MSP430::runBitInstruction(Instruction *instr)
{
    printf("Bit\n");
    executeLogicalOp(
        instr, [](uint32_t dst, uint32_t src) { return dst & src; }, true);
}

void MSP430::runBicInstruction(Instruction *instr)
{
    printf("Bic\n");
    executeLogicalOp(
        instr, [](uint32_t dst, uint32_t src) { return dst & ~src; }, false);
}

void MSP430::runBisInstruction(Instruction *instr)
{
    printf("Bis\n");
    executeLogicalOp(
        instr, [](uint32_t dst, uint32_t src) { return dst | src; }, false);
}

void MSP430::runXorInstruction(Instruction *instr)
{
    printf("Xor\n");
    executeLogicalOp(
        instr, [](uint32_t dst, uint32_t src) { return dst ^ src; }, true,
        instr->destination.value, instr->source.value,
        true); // The last 'true' indicates this is an XOR operation.
}

void MSP430::runAndInstruction(Instruction *instr)
{
    printf("And\n");
    executeLogicalOp(
        instr, [](uint32_t dst, uint32_t src) { return dst & src; }, true);
}

void MSP430::runCoreMSP430Instruction(Instruction *instr)
{
    /* On standard isntruction minorOpcode = majorOpcode.
     * We use here minorOpcode so extended command with 0x18XX can also use
     * this function. */
    switch (instr->minorOpcode)
    {
    case MAJOR_OPCODE_00:
        handleType00(instr);
        break;
    case MAJOR_OPCODE_10:
        handleType10(instr);
        break;

    case MAJOR_OPCODE_JNE_JNZ:
    case MAJOR_OPCODE_JEQ_JZ:
    case MAJOR_OPCODE_JNC:
    case MAJOR_OPCODE_JC:
    case MAJOR_OPCODE_JN:
    case MAJOR_OPCODE_JGE:
    case MAJOR_OPCODE_JL:
    case MAJOR_OPCODE_JMP:
        runJumpInstruction(instr, instr->minorOpcode);
        break;

    case MAJOR_OPCODE_MOV:
        runMovInstruction(instr);
        break;

    case MAJOR_OPCODE_ADD:
        runAddInstruction(instr, false);
        break;
    case MAJOR_OPCODE_ADDC:
        runAddInstruction(instr, true);
        break;
    case MAJOR_OPCODE_SUBC:
        runSubInstruction(instr, true);
        break;
    case MAJOR_OPCODE_SUB:
        runSubInstruction(instr, false);
        break;

    case MAJOR_OPCODE_CMP:
        runCmpInstruction(instr);
        break;
    case MAJOR_OPCODE_DADD:
        runDaddInstruction(instr);
        break;

    case MAJOR_OPCODE_BIT:
        runBitInstruction(instr);
        break;
    case MAJOR_OPCODE_BIC:
        runBicInstruction(instr);
        break;
    case MAJOR_OPCODE_BIS:
        runBisInstruction(instr);
        break;
    case MAJOR_OPCODE_XOR:
        runXorInstruction(instr);
        break;
    case MAJOR_OPCODE_AND:
        runAndInstruction(instr);
        break;

    default:
        assert(0);
        break;
    }
}

void MSP430::runExtendedMSP430Instruction(Instruction *instr)
{

    switch (instr->majorOpcode)
    {
    case MAJOR_OPCODE_00:
        handleTypeExt00(instr);
        break;
    case MAJOR_OPCODE_10:
        handleTypeExt10(instr);
        break;
    case MAJOR_OPCODE_14:
        runPushPopInstruction(instr);
        break;
    case MAJOR_OPCODE_18:
        handleTypeExt18(instr);
        break;
    default:
        assert(0);
        break;
    }
}

void MSP430::runOneInstruction(Instruction *instr)
{
    if (instr->extended)
    {
        runExtendedMSP430Instruction(instr);
    }
    else
    {
        runCoreMSP430Instruction(instr);
    }
}

const unsigned int SLEEP_DURATION = 100000; // 100ms

void MSP430::run()
{
    Instruction instr;

    // Initialize instr to zero
    memset(&instr, 0, sizeof(Instruction));

    while (true) // Run until the program is manually stopped
    {
        // Store initial PC for repetition
        uint32_t initialPC = getRegister(REG_IDX_PC);

        // Display debug information
        displayDebugInformation();

        // Reset repetition counter
        int currentRepetition = 0;

        do
        {
            // Decode the next instruction
            instr = decodeInstruction();

            // Debug output for repetition
            printf("Repetition %d/%d\n", currentRepetition, instr.repetition);

            // Execute the instruction
            runOneInstruction(&instr);

            // Restore PC for further repetitions
            if (currentRepetition < instr.repetition)
            {
                setRegister(REG_IDX_PC, initialPC);
            }

            // Sleep for a short duration
            usleep(SLEEP_DURATION);

            // Increment repetition counter
            currentRepetition++;

        } while (currentRepetition <= instr.repetition);
    }
}

/**
 * Displays debug information.
 */
void MSP430::displayDebugInformation()
{
    std::cout << "----------- PC: " << std::hex << getRegister(REG_IDX_PC)
              << " --------" << std::endl;
    regDump();
    devicesManager_.dump(getRegister(REG_IDX_SP) - 16, 32);
}
