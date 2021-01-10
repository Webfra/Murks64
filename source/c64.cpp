#include "c64.h"
#include "load.h"
#include "hex.h"

//========================================================================
#include <iostream>
#include <sstream>

//========================================================================
C64 c64;

//========================================================================
void show_help()
{
    std::cout << "-------------------------------------------------------------\n";
    std::cout << "Commands:\n";
    std::cout << " \"l\":      CPU status + asm listing at current PC.\n";
    std::cout << " <return>: Execute current line.\n";
    std::cout << "           JSRs will be executed until RTS.\n";
    std::cout << " \"i\":      Execute a single instruction.\n";
    std::cout << "           (JSRs will be followed.)\n";
    std::cout << " \"o\":      Execute until end of current subroutine.\n";
    std::cout << " \"run\":    Execute until breakpoint (BP) or timeout (5s).\n";
    std::cout << " \"reset\":  Clear RAM, jump to reset vector.\n";
    std::cout << " \"irq\":    Jump to IRQ vector.\n";
    std::cout << " \"nmi\":    Jump to NMI vector.\n";
    std::cout << " \"m HHHH\": Show memory dump from hex address HHHH.\n";
    std::cout << " \"w HHHH DD DD DD...\": Write hex data DD to address HHHH.\n";
    std::cout << " \"bp HHHH\": Set breakpoint to hex address HHHH.\n";
    std::cout << "-------------------------------------------------------------\n";
    std::cout << "Note: Stack is shown decreasing left to right. (Max. 8 bytes.))\n";
    std::cout << "      (Right-most byte is current stackpointer+1).\n";
    std::cout << "-------------------------------------------------------------\n";
}

//========================================================================
void C64::init()
{
    memset(&RAM[0], 0x00, 0x10000);
    memset(&ROM[0], 0x00, 0x10000);

#if defined(LOAD_BINARY_ROMS)
    load_all_roms();
#endif
    disasm = load_disasm();
}

//========================================================================
void C64::loop()
{
    c64.breakpoint = 0x0000;
    c64.cpu.clockticks = 0;
    while(!c64.quit)
    {
        std::string info = "";
        if(c64.step_out)
        {
            if( c64.cpu.pc == c64.step_out_point )
            {
                c64.pause = true;
            }
        }
        if( ((c64.cpu.clockticks - old_clock)>=5000000) || ( c64.cpu.clockticks<old_clock) )
        {
            info = "BREAK on timeout (5s)";
            old_clock = c64.cpu.clockticks;
            pause = true;
        }
        if(reset)
        {
            old_clock = 0;
            cpu.clockticks = 0;

            memset(&c64.RAM[0], 0x00, 0x10000);
            info = "RAM was cleared!";

            reset6502(&cpu);
            reset = false;
        }
        if( cpu.pc == breakpoint )
        {
            info = "Breakpoint reached:" + bin2hex4(breakpoint);
            pause = true;
        }
        if( pause )
        {
            if( editor(info) )
                continue;
        }
        if( mem_read(&cpu, cpu.pc) == 0x20 )
        {
            jsr_stack.push_back(cpu.pc);
        }
        if( mem_read(&cpu, cpu.pc) == 0x60 )
        {
            jsr_stack.pop_back();
        }
        step(&cpu);
        show_asm = true;
        // Simulate VIC-II rasterline register
        if( 0==(cpu.clockticks & 63) )
        {
           c64.RAM[0xD012]++;
        }
    }
    std::cout << cpu.clockticks << cpu.pc << "\n";
}

//========================================================================
bool C64::editor(std::string & info)
{
    bool no_step = true;

    if( show_asm )
    {
        if( info != "" )
            std::cout << info << std::endl;
        show_status();
        old_clock = cpu.clockticks;
        show_asm = false;
    }

    std::cout << "> ";
    std::string cmd;
    std::getline(std::cin, cmd);

    if(cmd == "l")
    {
        show_asm = true;
        no_step = true;
    }
    else if(cmd == "run")
    {
        pause = false;
        no_step = false;
    }
    else if(cmd == "reset")
    {
        reset = true;
        no_step = true;
    }
    else if(cmd == "irq")
    {
        irq6502(&cpu);
        no_step = true;
    }
    else if(cmd == "nmi")
    {
        nmi6502(&cpu);
        no_step = true;
    }
    else if( cmd.substr(0,3) == "bp " )
    {
        hex2bin(cmd.substr(3), breakpoint, true );
        no_step = true;
    }
    else if( cmd == "" and mem_read(&cpu,cpu.pc) == 0x20 )
    {
        // do "go over" on jsr
        step_out = true;
        step_out_point = cpu.pc+3;
        pause = false;
        no_step = false;
    }
    else if( cmd == "" )
    {
        // Do one step
        no_step = false;
    }
    else if( cmd == "i" )
    {
        // Do one step
        no_step = false;
    }
    else if( cmd == "o" && jsr_stack.size() > 0)
    {
        // run until jumping out from current JSR.
        step_out = true;
        step_out_point = jsr_stack.back() + 3;
        pause = false;
        no_step = false;
    }
    else if( cmd.substr(0,2) == "m " )
    {
        uint16_t mem;
        bool ok = hex2bin(cmd.substr(2), mem, true);
        if(ok)
        {
            for( int row =0; row<10; row++ )
            {
                std::cout << bin2hex4( mem+8*row ) << ": ";
                for( int x=0; x <8; x++)
                {
                    std::cout << bin2hex2( mem_read(&cpu, mem+8*row+x) ) << " ";
                }
                std::cout << std::endl;
            }
        }
        no_step = true;
    }
    else if( cmd.substr(0,2) == "w " )
    {
        int16_t addr;
        int pos = 2;
        int nc = hex2bin( cmd.substr(pos), addr, true, true);
        if(nc > 0)
        {
            pos += nc;
            while( pos < int(cmd.size()) )
            {
                uint8_t data;
                nc = hex2bin( cmd.substr(pos), data, true, true );
                if( nc > 0 )
                {
                    c64.RAM[addr++] = data;
                    pos += nc;
                }
                else
                {
                    pos++;
                }
            }
        }
        no_step = true;
    }
    else if( cmd=="h" or cmd=="help" )
    {
        show_help();
        no_step = true;
    }
    else
    {
        // Command not understood. Do nothing.
        no_step = true;
    }

    return no_step;
}

//========================================================================
void C64::show_status()
{
    const char *g = "cCzZiIdDbB-+vVnN";
    uint8_t f = cpu.flags;

    std::stringstream ss;
    ss << bin2hex4(cpu.pc);
    std::string addr = ss.str();
    std::string mark = ".,";
    for( auto &c: addr ) mark += (char)toupper(c);

    uint16_t top = std::min( 0xff, cpu.s+8);




    std::cout << "S:01" << bin2hex2(cpu.s)
              << " [" << bin2hex4(0x100+top) << ":";
    for( uint16_t s = top, max=0; (s>cpu.s)&&(max<8); s--, max++ )
        std::cout << " " << bin2hex2(RAM[0x100 + s]);
    std::cout << "] " << jsr_stack.size() << " JSRs active."
              << std::endl;

    std::cout << "PC: " << bin2hex4(cpu.pc)
              << " A:" << bin2hex2(cpu.a)
              << " X:" << bin2hex2(cpu.x)
              << " Y:" << bin2hex2(cpu.y)
              << " flags:" << bin2hex2(cpu.flags) << " [" << g[14+((f>>7)&1)] << g[12+((f>>6)&1)] << g[10+((f>>5)&1)] << g[ 8+((f>>4)&1)] << g[ 6+((f>>3)&1)] << g[ 4+((f>>2)&1)] << g[ 2+((f>>1)&1)] << g[ 0+((f>>0)&1)] << "]"
              << " (BP:" << bin2hex4(breakpoint) << ")"
              << " Ticks: " << std::dec
              << cpu.clockticks
              << " (+" << cpu.clockticks-old_clock << ")"
              << std::endl;

    int followup = -1;
    for( auto &s: disasm )
    {
        if( followup > 0)
        {
            std::cout << s << std::endl;
            followup--;
            if(followup==0)
                break;
        }
        if( s.compare(0,6,mark) == 0 )
        {
            followup = 3;
            std::cout << s << std::endl;
        }
    }
}

//========================================================================
void mem_write(context_t *, uint16_t address, uint8_t value)
{
    c64.RAM[address] = value;
}

//========================================================================
uint8_t mem_read(context_t *, uint16_t address)
{
    uint8_t value;
    if( (address>=0xA000) && (address<=0xBFFF) )
    {
        value = c64.ROM[address];
    }
    /* Leave out char rom, as it usually is hidden behind I/O area. */
    else if( (address>=0xE000) && (address<=0xFFFF) )
    {
        value = c64.ROM[address];
    }
    else
    {
        value = c64.RAM[address];
    }
    return value;
}

//========================================================================
