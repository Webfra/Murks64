#ifndef LOAD_H
#define LOAD_H

#include "c64.h"

#include <string>
#include <vector>
#include <cstdint>

#if defined(LOAD_BINARY_ROMS)
#include <filesystem>

using Path = std::filesystem::path;

Path data_path();

uint16_t load_rom( std::string rom, uint16_t address, uint16_t size);
bool load_all_roms();
#endif

std::vector<std::string> load_disasm();

#endif // LOAD_H
