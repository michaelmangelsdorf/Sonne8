
#ifndef __IO_H__
#define __IO_H__ 1

#include <u.h>
#include <libc.h>
#include "myth.h"

extern struct myth_vm vm;

struct {
        char data[256*256*256];
        uchar a0, a1, a2;
} smem;

uchar bus; /*Byte value on parallel bus, assume pull-down*/

int
isactive(uchar id)
{
        if(id>15) return (vm.e_new & 0xF0) == id ? 1:0;
        else return (vm.e_new & 0x0F) == id ? 1:0;
}

uchar
get_smemdata()
{
    long addr = (smem.a2 << 16) + (smem.a1 << 8) + smem.a0;
    return smem.data[addr];
}

void
set_smemdata(uchar byteval)
{
    long addr = (smem.a2 << 16) + (smem.a1 << 8) + smem.a0;
    smem.data[addr] = byteval;
}

void
falling_edge(uchar id)
{
        if (id<16) { /*Active-low on select*/
            switch(id){
                    case SL0_NULL: bus = 0; break; /*Tri-state pull-down*/
                    case SL1_PAROE: bus = vm.por; break;
                    case SL2_SMEMOE: bus = get_smemdata(); break;
                    case SL3_SMEMWE: set_smemdata(bus); break;
                    default:;
            }
        }
        else { /* Active-high on deselect*/
            switch(id>>4){
                    default:;
            }
        }
}

void
rising_edge(uchar id)
{
        if (id<16) { /*Active-low on deselect*/
            switch(id) {
                    case SL1_PAROE: bus = 0; /*Tri-state pull-down*/
                    default:;
            }
        }
        else { /*Active-high on select*/
            switch(id>>4){
                    case SH1_PARLE:
                            vm.pir = bus;
                            break;
                    case SH2_SMEMA0LE: smem.a0 = bus; break;
                    case SH3_SMEMA1LE: smem.a1 = bus; break;
                    case SH4_SMEMA2LE: smem.a2 = bus; break;
                    default:;
            }
        }
}

void
active_high(uchar id)
{
}

void
active_low(uchar id)
{
}

#endif
