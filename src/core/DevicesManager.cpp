#include <cstdint>
#include <functional>
#include <iostream>
#include <memory>

#include "DevicesManager.h"
#include "MSP430Watchdog.h"
#include "Memory.h"
#include "Port.h"
#include "Port1.h"
#include "Port2.h"
#include "Port3.h"
#include "Port4.h"
#include "Port5.h"
#include "Port6.h"
#include "Port7.h"
#include "Port8.h"

using DeviceFactory = std::function<std::shared_ptr<Device>()>;

DevicesManager::DevicesManager() { loadInternalDevices(); }

void DevicesManager::loadInternalDevices()
{
    int portIndex = 0;

    // Memory has to be the last one
    std::vector<DeviceFactory> deviceFactories = {
        []() -> std::shared_ptr<Device>
        { return std::make_shared<MSP430Watchdog>(); },
        []() -> std::shared_ptr<Device> { return std::make_shared<Port1>(); },
        []() -> std::shared_ptr<Device> { return std::make_shared<Port2>(); },
        []() -> std::shared_ptr<Device> { return std::make_shared<Port3>(); },
        []() -> std::shared_ptr<Device> { return std::make_shared<Port4>(); },
        []() -> std::shared_ptr<Device> { return std::make_shared<Port5>(); },
        []() -> std::shared_ptr<Device> { return std::make_shared<Port6>(); },
        []() -> std::shared_ptr<Device> { return std::make_shared<Port7>(); },
        []() -> std::shared_ptr<Device> { return std::make_shared<Port8>(); },
        []() -> std::shared_ptr<Device> { return std::make_shared<Memory>(); },
    };

    for (const auto &factory : deviceFactories)
    {
        auto device = factory();
        // device->init();
        internalDevices_.push_back(device);

        // Register each address range for the device
        for (const auto &range : device->addressRanges_)
        {
            registerDeviceRange(range.startAddr, range.endAddr, device.get());
        }

        // Check if this device is the Memory device and, if so, set
        // memoryDevice_
        if (auto memoryDevice = std::dynamic_pointer_cast<Memory>(device))
        {
            memoryDevice_ = memoryDevice;
        }
        else if (auto port = std::dynamic_pointer_cast<Port>(device))
        {
            ports_[portIndex] = port;
            portIndex++;
        }
    }
}

DevicesManager::~DevicesManager()
{
    // No manual deletion needed as std::shared_ptr will handle it
}

void DevicesManager::dump(uint32_t address, uint32_t len)
{
    uint8_t byte;

    if (len % 16 != 0)
    {
        printf("\n%X :", address + len);
    }

    while (len)
    {
        if (len % 16 == 0)
        {
            printf("\n%02X: ", address + len);
        }
        else if (len % 16 == 0)
        {
            printf(", ");
        }

        byte = readByte(address + len);
        printf("%X ", byte);
        len--;
    };

    printf("\n");
}

void DevicesManager::registerPeripheral(uint8_t port, RxCBType rxCb,
                                        TxCBType txCb)
{
    ports_[port - 1]->registerPeripheral(rxCb, txCb);
}

void DevicesManager::registerDeviceRange(uint32_t startAddress,
                                         uint32_t endAddress, Device *device)
{
    std::cout << "register device " << device->name_ << std::hex << startAddress
              << " to " << endAddress << std::endl;
    for (uint32_t address = startAddress; address <= endAddress; ++address)
    {
        if (devicesMap_.find(address) == devicesMap_.end())
        {
            devicesMap_[address] = device;
        }
    }
}

template <typename T>
void DevicesManager::instantiateAndRegisterDevice(uint32_t startAddress,
                                                  uint32_t endAddress)
{
    T *device = new T();
    device->init();
    registerDeviceRange(startAddress, endAddress, device);
}

Device *DevicesManager::getDeviceForAddress(uint32_t address)
{
    // Check for specific devices first
    if (devicesMap_.count(address))
    {
        return devicesMap_[address];
    }
    // Otherwise, return memory
    return memoryDevice_.get();
}

uint8_t DevicesManager::readByte(uint32_t address)
{
    Device *device = getDeviceForAddress(address);
    // std::cout << "dm read byte from dev " << device->name_ <<  " **** " <<
    // std::endl;
    return device->readByte(address);
}

uint16_t DevicesManager::readWord(uint32_t address)
{
    Device *device = getDeviceForAddress(address);
    std::cout << "dm read word **** " << device->name_ << " @" << std::hex
              << address << std::endl;
    return device->readWord(address);
}

uint32_t DevicesManager::readDWord(uint32_t address)
{
    std::cout << "dm read Dword **** " << std::hex << address << std::endl;
    Device *device = getDeviceForAddress(address);
    return device->readDWord(address);
}

void DevicesManager::writeByte(uint32_t address, uint8_t value)
{
    Device *device = getDeviceForAddress(address);
    device->writeByte(address, value);
}

void DevicesManager::writeWord(uint32_t address, uint16_t value)
{
    Device *device = getDeviceForAddress(address);
    device->writeWord(address, value);
}

void DevicesManager::writeDWord(uint32_t address, uint32_t value)
{
    Device *device = getDeviceForAddress(address);
    device->writeDWord(address, value);
}

uint32_t DevicesManager::read(uint32_t address, uint32_t nbBytes)
{
    switch (nbBytes)
    {
    case 1:
        return readByte(address);
    case 2:
        return readWord(address);
    case 4:
        return readDWord(address);
    default:
        assert(0);
        return 0xFFFFFFFF;
    }
}

void DevicesManager::write(uint32_t address, uint32_t value, uint32_t nbBytes)
{
    switch (nbBytes)
    {
    case 1:
        writeByte(address, value);
        break;
    case 2:
        writeWord(address, value);
        break;
    case 4:
        writeDWord(address, value);
        break;
    default:
        assert(0);
    }
}

void DevicesManager::updateAllDevices()
{
    for (const auto &entry : devicesMap_)
    {
        entry.second->update();
    }
}
