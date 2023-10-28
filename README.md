# MSP430 Emulator Project

## Description

This project entails the development of a simple emulator for the Texas Instruments MSP430F2618 microcontroller. The MSP430F2618 is a 16-bit ultra-low-power microcontroller equipped with 116KB of flash memory, 8KB of RAM, and a multitude of peripherals. The primary objective of this emulator is to accurately simulate the operation of the microcontroller's CPU and memory, alongside a subset of its peripherals.

Currently, this project is in its alpha stage. The majority of instructions are expected to function correctly, although there may be occasional discrepancies.
I have developed a program that operates on a custom board provided by a customer. For testing purposes, I have managed to run the same program on my emulator up to the point of DCO (Digitally Controlled Oscillator) calibration.
The DCO calibration process necessitates the implementation of "internal devices", which has not been accomplished yet.
To verify the proper execution of your program for all instructions (excluding the access to internal devices or external peripherals), the tests/replay_ds_script/ directory can be utilized.
This project is complemented by tools/ccs_asm_extractor.js (refer to tools/README.md for more details). This is a code composer JavaScript automated debugger designed to execute the first 10,000 steps and monitor any updates in the stack or registers.
Subsequently, you can employ the tests/replay_ds_script/ test program to compare the behavior of the emulator with that of the actual hardware.

## Features

- Emulates the MSP430F2618's 16-bit CPU
- Supports loading of binary ROM images
- Emulates RAM

## Building

This project is written in C++ and uses the Qt library for its GUI. To build, you will need a C++ compiler and the Qt libraries installed on your system.

To compile the project:

```bash
$ rm -rf build/ && mkdir build && cd build && cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -G Ninja .. && ninja && cd ..
```

## Usage

To run the emulator, you will need a binary ROM image to load into the emulator's memory. The ROM image should be compiled for the MSP430F2618.

```bash
$ ./build/Emulator ./config/app.json roms/FW.hex config/params.json
```

To run unit tests:

```bash
$ ninja tests
```

## Code Formatting

The code adheres to LLVM-style formatting with some minor modifications. You can format the code using:

```bash
$ ninja format
```

## Future Plans

The outlined plans below indicate potential directions for enhancing the emulator. However, due to other engagements, my continued involvement in this project is uncertain. Users interested in advancing this project are encouraged to fork it and make their own contributions.

- Full support for all of the MSP430F2618's peripherals
- More comprehensive error checking and debugging capabilities and integration with gdb.
- Improved performance and efficiency

## Contributing

Contributions to this project are welcome. Please fork the project, make your changes, and submit a pull request. We ask that you make sure your changes are well-documented and include test cases where possible.

## License

This project is licensed under the MIT License - see the LICENSE file for details.

## About Me

I'm Abdel, also known as Uncaged-Coder. I'm passionate about embedded systems and have been working on this
MSP430 Emulator as a part of my ongoing journey in understanding low-level system architectures.

If you have any questions, issues, or if you're interested in contributing to this project, feel free to reach out.

You can reach me through Twitter or drop me an email at uncaged-coder@proton.me  

