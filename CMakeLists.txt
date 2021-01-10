#========================================================================
cmake_minimum_required(VERSION 3.14)

#========================================================================
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

#========================================================================
set( target Murks64 )
project( ${target} C CXX )

#========================================================================
add_executable( ${target}
    source/main.cpp
    source/hex.h
    source/load.cpp
    source/load.h
    source/c64.h
    source/c64.cpp
    source/disasm.cpp

    fake6502/fake6502.h
    fake6502/fake6502.c
    )

#========================================================================
target_include_directories( ${target} PRIVATE fake6502 )

#========================================================================
set ( CPU_VARIANT   NMOS6502 CACHE STRING "Which CPU shall be emulated NMOS6502 or CMOS6502?" )
target_compile_definitions( ${target} PUBLIC -D${CPU_VARIANT} )

set ( LOAD_BINARY_ROMS OFF CACHE BOOL "Enable/Disable loading of ROM binaries from data folder." )
if( LOAD_BINARY_ROMS )
    target_compile_definitions( ${target} PUBLIC -DLOAD_BINARY_ROMS )
endif()

#========================================================================
# End of file.
#========================================================================
