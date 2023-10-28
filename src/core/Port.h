#pragma once

#include <functional>
#include <stdint.h>

#include "Device.h"

typedef std::function<void(uint8_t value)> TxCBType;
typedef std::function<uint8_t()> RxCBType;

class Port : public Device
{
public:
    Port(std::string name, uint32_t ren, uint32_t in, uint32_t out,
         uint32_t dir, uint32_t sel, uint32_t ifg = 0, uint32_t ies = 0,
         uint32_t ie = 0)
        : Device(setAddresses({ren, in, out, dir, sel, ifg, ies, ie}), name),
          value_(0), ren_(0), dir_(0), sel_(0), ies_(0), ifg_(0), ie_(0),
          addrRen_(ren), addrIn_(in), addrOut_(out), addrDir_(dir),
          addrSel_(sel), addrIfg_(ifg), addrIes_(ies), addrIe_(ie)
    {
    }

    ~Port(){};

    // Override functions from Device interface
    void init() override;
    void destroy() override;
    void update() override;

    void registerPeripheral(RxCBType rxCb, TxCBType txCb);

    uint16_t readWord(uint32_t address) override;
    uint8_t readByte(uint32_t address) override;
    void writeWord(uint32_t address, uint16_t value) override;
    void writeByte(uint32_t address, uint8_t value) override;

protected:
    std::vector<RxCBType> rxCbs_;
    std::vector<TxCBType> txCbs_;
    std::vector<AddressRange>

    setAddresses(const std::initializer_list<uint32_t> &addresses);
    void invokeTxCbs(uint8_t value);

    uint8_t value_;
    uint8_t ren_;
    uint8_t dir_;
    uint8_t sel_;
    uint8_t out_;
    uint8_t ies_;
    uint8_t ifg_;
    uint8_t ie_;

    uint32_t addrRen_;
    uint32_t addrIn_;
    uint32_t addrOut_;
    uint32_t addrDir_;
    uint32_t addrSel_;
    uint32_t addrIfg_;
    uint32_t addrIes_;
    uint32_t addrIe_;
};
