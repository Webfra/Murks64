#include "c64.h"
#include "load.h"
#include "hex.h"

#include <fstream>
#include <filesystem>
#include <string>
#include <iostream>

//========================================================================
extern const std::array<std::string, 9615> c64disasm_en;

//========================================================================
std::vector<std::string> load_disasm()
{
    std::vector<std::string> lines;

    int line_nr = 0;
    int expected_address = 0xA000;
    for( auto &line: c64disasm_en)
    {
        line_nr++;
        if( line[0] == '.' )
        {
            lines.push_back(line);

            uint16_t addr;
            std::string addr_str = line.substr(2,4);
            auto ok = hex2bin(addr_str, addr);
            if(!ok)
            {
                std::cout << "***ERROR in line #" << std::dec << line_nr;
                std::cout << ": not a hex address:\n";
                std::cout << line << "\n";
                std::cout << " -->" << addr_str << "<-- \n";
                exit(-1);
            }
            if( addr != expected_address )
            {
                std::cout << "***ERROR in line #" << std::dec << line_nr;
                std::cout << ": address mismatch:\n";
                std::cout << line << "\n";
                std::cout << "Address expected: " << bin2hex4(expected_address) << std::endl;
                exit(-1);
            }

            uint8_t bytes[8];
            std::string bytes_str[8];
            int num_bytes = 0;
            int ll = line.length();
            for( int pos=7; pos<30; pos++ )
            {
                if( line[pos] == ' ') continue;
                std::string test = line.substr(pos, std::min(3, ll-pos) );
                if( test.length() < 2)
                {
                    break;
                }
                if( test.length() > 2)
                    if( test[2] != ' ') break;
                test = test.substr(0,2);
                if(!hex2bin(test, bytes[num_bytes])) break;
                num_bytes++;
                pos++;
            }
            if( num_bytes<1 )
            {
                std::cout << "***ERROR in line #" << std::dec << line_nr;
                std::cout << ": No bytes listed:\n";
                std::cout << line << "\n";
                exit(-1);
            }
#if defined(LOAD_BINARY_ROMS)
            for( int i=0; i<num_bytes; i++ )
            {
                if( c64.ROM[addr+i] != bytes[i]) {
                    std::cout << "***ERROR in line #" << std::dec << line_nr;
                    std::cout << ": Disassembly doesn't match loaded ROM:\n";
                    std::cout << "Disassembly: " << line << "\n";
                    std::cout << "ROM:         . " << bin2hex4(addr) << ": ";
                    for( int i=0; i<num_bytes; i++ )
                    {
                        std::cout << bin2hex2(c64.ROM[addr+i]) << " ";
                    }
                    std::cout << std::endl;

                    exit(-1);
                }
            }
#else
            for( int i=0; i<num_bytes; i++ )
            {
                c64.ROM[addr+i] = bytes[i];
            }
#endif

            expected_address += num_bytes;
            if( expected_address == 0xc000 )
                expected_address = 0xe000;

        }
    }
    return lines;
}

//========================================================================
#if defined(LOAD_BINARY_ROMS)

//========================================================================
#if defined(__linux__)
    #include <unistd.h> // For "readlink()" on GNU/linux.
#endif
#if defined(_WIN32)
    #include <windows.h>
    #include <libloaderapi.h> // For GetModuleFileName() on Windows
#endif

//========================================================================
Path data_path()
{
    char exe_full[65536];
    memset(exe_full, 0, 65536);
#if defined(__linux__)
    auto size = readlink("/proc/self/exe", exe_path, 65535);
    std::cout << "readlink(\"/proc/self/exe\", exe_path, 65535); = " << size << std::endl;
#elif defined(_WIN32)
    (void)::GetModuleFileName(nullptr, exe_full, 65535);
#else
#error Operating system is not supported! Must be either Linux or Windows!
#endif
    Path exe { exe_full};
    Path roms { exe.parent_path() / "data" };
    return roms;
}

//========================================================================
uint16_t load_rom( std::string rom, uint16_t address, uint16_t size)
{
    std::ifstream file;
    uint16_t chksum = 0xffff;
    file.open( rom, std::ios::binary );
    if( file.is_open() )
    {
        char * location = (char *)(&c64.ROM[address]);
        file.read( location, size );
        file.close();

        for( uint16_t i=0; i<size; i++ )
        {
            chksum += c64.ROM[i+address];
        }
    }
    return chksum;
}

//========================================================================
bool load_all_roms()
{
    Path data = data_path();
    if( !std::filesystem::exists(data) )
    {
        std::cout << "***ERROR: Folder 'data' not found!\n";
        return false;
    }
    uint16_t chksum;
    chksum = load_rom( (data/"chargen").string(), 0xD000, 0x1000 );
    std::cout << "CHARGEN checksum: 0x" << bin2hex4(chksum) << "\n";
    chksum = load_rom( (data/"basic").string(),   0xA000, 0x2000 );
    std::cout << "BASIC   checksum: 0x" << bin2hex4(chksum) << "\n";
    chksum = load_rom( (data/"kernal").string(),  0xE000, 0x2000 );
    std::cout << "KERNAL  checksum: 0x" << bin2hex4(chksum) << "\n";

/*
 * BASIC checksum:   0x3d55
 * CHARGEN checksum: 0xf7f7
 * KERNAL checksum:  0xc709
*/

    return true;
}
#endif

//========================================================================
