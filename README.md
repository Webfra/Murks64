# Murks64
Murks64 - a small and crude, bare-bones (and only partial) "C64 emulator" written in C++17 and running in a console.

## Features
- Emulates:
  - CPU NMOS6502 using "fake6502" (as submodule, added from https://github.com/omarandlorraine/fake6502.git)
  - 64K RAM, as well as Basic and Kernal ROM
    - Relies on a ROM listing from https://github.com/mist64/c64ref.git, which is "baked" into the source code.
    - Therefore, the executable can run on its own, without the need to load any files (ROMs etc.)
    - However, it is possible to load binary ROMs from a subfolder "data". 
  - "Emulates" The VIC-II rasterline register, which is required for BASIC to start up.
    - Emulation is probably too strong a word, the register is just inremented every few CPU clock cycles.
  - Nothing else! (At least for now.)
    - No VIC-II => No Graphics!
    - No SID, no Datasette or Floppy, no Cartriges...
- "Debugger"
  - Probably not worth calling it a debugger, but it allows looking at the CPU status, Stack and  RAM.
  - Provides Breakpoints, single-step and continuous execution.
  - Crude "Disassembly": It just uses the current PC address and looks up the correct line in the ROM listing.
  - NO Assembler! 
  - Want to see the screen? ...sorry!... Well, you can do a memory dump... (eg. "m 0400")
- Hobby project flair
  - Questionable future. 
  - Probably tons of bugs and missed opportunities.
  - 100% I will be just on a 4-month vacation when you have an urgent question.
  - Don't look at the code, if you don't want to catch a disease.
  - License situation completely unclear (to me...) Probably GPLv2? 

## Requirements
- OS: Windows or Linux (Maybe more, but what do I know...)
- Compiler: GCC (with C++-17, so probably 8.x minimum required.)
  - On Windows tested with MSYS2 / Mingw64
- CMake 3.14 (probably)
- Clone with submodules: git clone --recurse-submodules https://github.com/Webfra/Murks64.git

## Trivia
- "Murks" is german slang for "mess" or "botch"...
