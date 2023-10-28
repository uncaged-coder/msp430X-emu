#include <assert.h>
#include <iostream>

#include "Port.h"

void Port::init()
{
    // Initialize the Watchdog Timer Control Register
    value_ = 0x90;
}

void Port::destroy()
{
    // Cleanup if any is required when destroying the device
}

void Port::update()
{
    // Update the device state, if required
}

void Port::registerPeripheral(RxCBType rxCb, TxCBType txCb)
{
    rxCbs_.push_back(rxCb);
    txCbs_.push_back(txCb);
}

std::vector<AddressRange>
Port::setAddresses(const std::initializer_list<uint32_t> &addresses)
{
    std::vector<AddressRange> ranges;
    for (auto address : addresses)
    {
        if (address != 0)
        {
            ranges.push_back({address, address});
        }
    }
    return ranges;
}

void Port::invokeTxCbs(uint8_t value)
{
    for (auto cb : txCbs_)
    {
        cb(value);
    }
}

uint8_t Port::readByte(uint32_t address)
{
    std::cout << "read " << name_ << " = " << std::hex << " @addr:" << address
              << std::endl;

    // Address check is needed to ensure we're reading the correct register
    if (address == addrIn_) // The address of WDTCTL based on your map file
    {
        std::cout << "read from " << name_ << ": " << std::hex << value_ + 0
                  << std::endl;
        return (value_ & ren_ & (!dir_));
    }
    else if (address == addrOut_)
    {
        return (out_);
    }
    else if (address == addrDir_)
    {
        return (dir_);
    }
    else if (address == addrSel_)
    {
        return (sel_);
    }
    else if (address == addrRen_)
    {
        return (ren_);
    }
    else if (address == addrIes_)
    {
        return (ies_);
    }
    else if (address == addrIfg_)
    {
        return (ifg_);
    }
    else if (address == addrIe_)
    {
        return (ie_);
    }

    std::cout << "read ooops " << name_ << " = " << std::hex
              << " @addr:" << address << " vs " << std::hex << addrOut_
              << std::endl;

    // If the address doesn't match any known register, trigger an error
    assert(false);
    return 0;
}

void Port::writeByte(uint32_t address, uint8_t value)
{
    std::cout << "write " << name_ << " = " << std::hex << value + 0
              << " @addr:" << address << std::endl;

    // Address check is needed to ensure we're writing to the correct register
    if (address == addrOut_)
    {
        // hack
        value_ = (value & ren_ & dir_);
        if (addrOut_ == 0x21)
            value_ = value_ | 0x90;
        else if (addrOut_ == 0x29)
            value_ = value_ | 0x1;
        else if (addrOut_ == 0x1d)
            value_ = value_ | 0xe0;
        else if (addrOut_ == 0x35) // p6
            value_ = value_ | 0x80;
        else if (addrOut_ == 0x3b) // p8
            value_ = 0x010;        // value_ |0x80;
        else
            value_ = value_ | 0x1;
        out_ = value_;

        invokeTxCbs(value_);
    }
    else if (address == addrRen_)
    {
        ren_ = value & 0xFF;
    }
    else if (address == addrDir_)
    {
        dir_ = value & 0xFF;
    }
    else if (address == addrSel_)
    {
        sel_ = value & 0xFF;
    }
    else if (address == addrIfg_)
    {
        /* FIXME */
    }
    else if (address == addrIes_)
    {
        /* FIXME */
    }
    else if (address == addrIe_)
    {
        /* FIXME */
    }
    else if (address == addrIn_)
    {
        std::cout << "***** FW error ? trying to write into input port"
                  << std::endl;
    }
    else
    {
        // If the address doesn't match any known register, trigger an error
        assert(false);
    }
}

uint16_t Port::readWord(uint32_t address)
{
    assert(false);
    return 0;
}

void Port::writeWord(uint32_t address, uint16_t value) { assert(false); }
