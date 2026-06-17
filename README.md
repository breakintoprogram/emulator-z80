# Speculation

Speculation is a simple ZX Spectrum emulator written in C++ and rendered using SDL2.

## Why am I doing this?

I'm fascinated by emulators and have planned on writing one for the Z80 for a while.

I started this project after I had a brainwave:

> An emulator is essentially a stateful disassembler with the state being stored in a representation of the CPU registers.

I'd already written a Z80 disassembler (in Z80) for [my homebrew Z80 computer](https://github.com/breakintoprogram/bsx) and one for [the Agon](https://github.com/breakintoprogram/agon-projects/tree/main/C/Disassembler) in C.

The core of the Agon disassembler was the ideal candidate for the basis of this project.

## Who is this for?

The code is targetting:

- Folk who would like to understand how to write a CPU emulator from first principles.
- Folk who would like to understand how to write an accurate(ish) emulator for a retro computer.

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

## Running

The emulator takes the following command line parameters:

- `s` or `scale` Set the emulator scale (default = 1)
- `l` or `load` Prepare a tape image for loading

Example: To run the emulator at 2x scale

```
bin/emulator-z80 s=2
```

Warnings and errors are piped to the console via cout.

### Keys

The emulator maps the left shift to CAPS SHIFT and right shift to SYMBOL SHIFT. There are a handful of emulator specific key combinations I'm using whilst testing the code:

- `F1` Turbu speed x 1
- `F2` Turbu speed x 2
- `F3` Turbu speed x 4
- `F4` Turbu speed x 8
- `F12` Enter the debugger

When in the debugger:

- `ENTER` single-step the CPU after a breakpoint
- `g` Exit debugger, continue normal CPU exection
- `d` disable interrupts
- `e` enable interrupts
- `t` enable console debugging trace
- `o` output registers to console
- `r` reset the CPU
- `p` play the loaded tape image

### Tape Interface

After inserting the tape file on launch from the command line, for example:

```
bin/emulator-z80 l=games/stop_the_express.tap
```

Do the following to load the file:

- `LOAD ""` from BASIC
- `F12` then `p` to start the tape file playing
- `G` to restart the Spectrum

It should start loading.

NB: The tape interface only supports TAP and a subset of TZX files at the moment.

## Testing

I've included Raxoft's excellent [Zilog Z80 CPU Test Suite](https://github.com/raxoft/z80test)
in the tests folder as TAP files. For more details on the tests please go to their GitHub page
or read the [readme.txt](tests/readme.txt) in the tests folder.

At present time, the emulator passes the z80doc test, with the exception of LD A,R.

Follow the instructions for loading tape files to run them.

## Useful links

The following sites helped me develop and test this:

- [Decoding Z80 Opcodes](http://www.z80.info/decoding.htm)
- [Z80 Information](https://jnz.dk/z80)
- [The Complete Spectrum ROM Disassembly](https://skoolkid.github.io/rom/index.html)
- [Raxoft Z80 Test Suite](https://github.com/raxoft/z80test)
- [Remy's ZX Spectrum Tools](https://zx.remysharp.com)
- [Spectrum Computing](https://spectrumcomputing.co.uk)

## Additional Credits

Thank you to Amstrad and Sky Group for allowing developers to use and distribute the original and unaltered ZX Spectrum ROM images for personal use in their emulators.

Patrik Rak (Raxoft) for his excellent Zilog Z80 CPU test suite (link above).

Stop the Express was published by Sinclair Research Ltd (UK) and created by Hudson Soft (Japan). I downloaded the TAP and TZX files from the [Spectrum Computing website](https://spectrumcomputing.co.uk/entry/4916/ZX-Spectrum/Stop_the_Express) and have provided them here for testing purposes only.