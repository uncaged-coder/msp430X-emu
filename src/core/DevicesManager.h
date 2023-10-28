#pragma once

#include <map>
#include <memory>
#include <string>
#include <vector>

#include "Device.h"
//#include "MSP430.h"
#include "Memory.h"
#include "Port.h"

class DevicesManager
{
public:
    DevicesManager();
    ~DevicesManager();

    void registerDeviceRange(uint32_t startAddress, uint32_t endAddress,
                             Device *device);
    template <typename T>
    void instantiateAndRegisterDevice(uint32_t startAddress,
                                      uint32_t endAddress);

    uint8_t readByte(uint32_t address);
    uint16_t readWord(uint32_t address);
    uint32_t readDWord(uint32_t address);
    void writeByte(uint32_t address, uint8_t value);
    void writeWord(uint32_t address, uint16_t value);
    void writeDWord(uint32_t address, uint32_t value);

    uint32_t read(uint32_t address, uint32_t nbBytes);
    void write(uint32_t address, uint32_t value, uint32_t nbBytes);

    void updateAllDevices();

    std::shared_ptr<Memory> getMemoryDevice() const { return memoryDevice_; }

    void dump(uint32_t address, uint32_t len);

    void registerPeripheral(uint8_t port, RxCBType rxCb, TxCBType txCb);

private:
    void loadInternalDevices();
    void loadDevice(const std::string &deviceName);
    Device *getDeviceForAddress(uint32_t address);

    std::map<int, Device *> devicesMap_; // Map of address to device
    std::shared_ptr<Memory> memoryDevice_;
    std::array<std::shared_ptr<Port>, 8> ports_;
    // std::vector<std::shared_ptr<Device>> devices_; // External devices

    // Internal devices intrinsic to the microcontroller
    std::vector<std::shared_ptr<Device>> internalDevices_;
};
