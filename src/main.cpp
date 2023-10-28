#include <QtWidgets/QApplication>
#include <fstream>
#include <iostream>
#include <string>
#include <thread>

#include "MSP430.h"
#include "UserInterfaceIcu.h"

#include "Peripheral.h"
#include "Uart.h"

class Uart;

std::vector<std::shared_ptr<Peripheral>> peripherals = {
    std::make_shared<Uart>()};

void loadPeripherals(MSP430 &uC)
{
    DevicesManager &dm = uC.getDevicesManager();

    for (const auto &peripheral : peripherals)
    {
        assert(peripheral->port <= 8);

        TxCBType txCb = std::bind(&Peripheral::txCb, peripheral.get(),
                                  std::placeholders::_1);
        RxCBType rxCb = std::bind(&Peripheral::rxCb, peripheral.get());

        dm.registerPeripheral(peripheral->port - 1, rxCb, txCb);
    }
}

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    UIIcu ui;

    if (argc < 2)
    {
        std::cout << "Usage: emulator <rom_file> [<ui_flag>]" << std::endl;
        return 1;
    }

    std::string rom_file = argv[1];
    bool show_ui = (argc > 2) && (std::string(argv[2]) == "ui");

    // Create the MSP430F2618 microcontroller
    MSP430 uC;

    loadPeripherals(uC);

    // Load ROM
    if (!uC.loadROM(rom_file))
    {
        std::cerr << "Failed to load ROM" << std::endl;
        return 1;
    }

    show_ui = true;
    if (show_ui)
    {
        std::cout << "show UI" << std::endl;
        ui.show();
    }

    // Run the microcontroller on a separate thread
    std::thread uC_thread([&] { uC.run(); });

    // Start the UI event loop
    int ret = app.exec();

    // Wait for the microcontroller thread to finish
    uC_thread.join();

    return ret;
}
