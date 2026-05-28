# Emulator-Z80

This is a simple ZX Spectrum emulator rendered using SDL2.

## Why am I doing this?

I'm fascinated by emulators and have planned on writing one for the Z80 for a while.

I started this project after I had a brainwave:

> An emulator is essentially a stateful disassembler with the state being stored in a representation of the CPU registers.

I'd already written a Z80 disassembler (in Z80) for [my homebrew Z80 computer](https://github.com/breakintoprogram/bsx) and one for [the Agon](https://github.com/breakintoprogram/agon-projects/tree/main/C/Disassembler) in C.

The core of the Agon disassembler was the ideal candidate for the basis of this project.

## Who is this for?

The code is targetting:

- Folk who would like to understand how to write a CPU emulator from first principles

## Etiquette

This software is a personal side project with many known flaws and omissions. Please do not submit any pull requests or issues at this point in time; they will be ignored.

## Building

The project currently builds unmodified on Ubuntu with the developer tools installed and the following dependencies:

- SDL development libraries: `sudo apt-get install libsdl2-dev`

I use the VSCode IDE with the following extensions installed:

- C/C++ by Microsoft
- C/C++ DevTools by Microsoft
- C/C++ Extension Pack by Microsoft
- C/C++ Themes by Microsoft
- CMake Tools by Microsoft
- CodeLLDB by Vadim Chugunov

## Useful links

The following sites helped me develop and test this:

- [Decoding Z80 Opcodes](http://www.z80.info/decoding.htm)
- [Z80 Information](https://jnz.dk/z80/)
- [The Complete Spectrum ROM Disassembly](https://skoolkid.github.io/rom/index.html)

## Additional Credits

Thank you to Amstrad and Sky Group who allow developers to use and distribute the original and unaltered ZX Spectrum ROM images for personal use in emulators.
