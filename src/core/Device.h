#pragma once

#include <cassert>
#include <iostream>
#include <stdint.h>
#include <string>
#include <vector>

struct AddressRange
{
    uint32_t startAddr;
    uint32_t endAddr;
};

class Device
{
public:
    Device(const std::vector<AddressRange> &addressRanges,
           const std::string &name)
        : addressRanges_(addressRanges), name_(name)
    {
    }
    Device(uint32_t startAddr, uint32_t endAddr, const std::string &name)
        : addressRanges_({{startAddr, endAddr}}), name_(name)
    {
    }
    virtual ~Device() = default;

    virtual void init() = 0;
    virtual void destroy() = 0;
    virtual void update() = 0;

    // Function that must be implemented by derived class
    virtual uint16_t readWord(uint32_t address) = 0;
    virtual void writeWord(uint32_t address, uint16_t value) = 0;

    // Reading methods
    virtual uint8_t readByte(uint32_t address)
    {
        assert(false); // not implemented by default
        return 0;      // Just to avoid compiler warning
    }

    virtual uint32_t readDWord(uint32_t address)
    {
        assert(false);
        return 0;
    }

    virtual void writeByte(uint32_t address, uint8_t value) { assert(false); }
    virtual void writeDWord(uint32_t address, uint32_t value) { assert(false); }
    virtual void dump(uint32_t address, uint32_t len) { assert(false); }

    const std::vector<AddressRange> addressRanges_;
    const std::string name_;
};
