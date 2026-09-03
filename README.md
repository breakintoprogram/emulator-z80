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

### For desktop

The project currently builds unmodified on Ubuntu with the developer tools installed and the following dependencies:

- SDL development libraries: `sudo apt-get install libsdl2-dev`

I use the VSCode IDE with the following extensions installed:

- C/C++ by Microsoft
- C/C++ DevTools by Microsoft
- C/C++ Extension Pack by Microsoft
- C/C++ Themes by Microsoft
- CMake Tools by Microsoft
- CodeLLDB by Vadim Chugunov

### For browser

The emulator can be compiled using Emscripten to run in a browser.

[Click here](https://emscripten.org/docs/getting_started/downloads.html) for Emscripten installation instructions.

## Starting the app

### On desktop

The emulator takes the following command line parameters:

- `s` or `scale` Set the emulator scale (default = 1)
- `l` or `load` Prepare a tape image for loading

Example: To run the emulator at 2x scale

```
bin/emulator-z80 s=2
```

Warnings and errors are piped to the console via cout.

### In browser

The arguments passed to the emulator are in the arguments property in var Module:

- scale=2
- load=games/stop_the_express.tzx

Modern browsers won't let you run the code directly, but you can run it from a local web server:

- In a terminal, run python3 -m http.server to launch a local web server
- In a browser, navigate to http://0.0.0.0:8000/docs/index.html to launch the app in a test page

You can also run the latest version by [clicking here](https://breakintoprogram.github.io/emulator-z80).

## Using the app

### Keys

Spectrum specific key combinations:

- `SHIFT` Caps Shift
- `ALT`, `CTRL` or `OPTION` Symbol Shift

Emulator specific key combinations:

- `F1` Turbo speed x 1
- `F2` Turbo speed x 2
- `F3` Turbo speed x 4
- `F4` Turbo speed x 8
- `F10` Start the tape
- `F11` Stop the tape
- `F12` Enter the debugger

Debugger specific key combinations:

- `ENTER` single-step the CPU after a breakpoint
- `g` Exit debugger, continue normal CPU exection
- `d` disable interrupts
- `e` enable interrupts
- `t` enable console debugging trace
- `o` output registers to console
- `r` reset the CPU

### Tape Interface

After inserting the tape file on launch from the command line, for example:

```
bin/emulator-z80 l=games/stop_the_express.tap
```

Do the following to load the file:

- `LOAD ""` from BASIC
- `F10` to start the tape file playing

It should start loading. For games that have a multipart load, i.e. Gauntlet, press `F11` to stop the tape as directed by the game.

NB: The tape interface only supports TAP and a subset of TZX files at the moment.

## Testing

I've included Raxoft's excellent [Zilog Z80 CPU Test Suite](https://github.com/raxoft/z80test)
in the tests folder as TAP files. For more details on the tests please go to their GitHub page
or read the [readme.txt](tests/readme.txt) in the tests folder.

At present time, the emulator passes the z80doc test, and most of the z80full test.

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

Stop the Express was published by Sinclair Research Ltd (UK) and created by Hudson Soft (Japan).

Vectron was originally published by Insight Software and developed by Mike Follin, Mark Wilson, Peter Gough and Tim Follin. 

I sourced these from the Spectrum Computing website (links below) and are provided in this repo for testing purposes only.

- [Stop the Express](https://spectrumcomputing.co.uk/entry/4916/ZX-Spectrum/Stop_the_Express)
- [Vectron](https://spectrumcomputing.co.uk/entry/5548/ZX-Spectrum/Vectron)