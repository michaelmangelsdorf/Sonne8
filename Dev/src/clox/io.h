
#ifndef __IO_H__
#define __IO_H__ 1

/* Virtual IO routines for Sonne 8 micro-controller Rev. Myth/LOX
   Author: mim@ok-schalter.de (Michael/Dosflange@github)
    */

#include <u.h>
#include <libc.h>
#include "myth.h"

extern struct myth_vm vm;

static uchar lnybble_old, lnybble_new, hnybble_old, hnybble_new;

struct SMem {
        uchar data[256*256*256]; /*16MB*/
        uchar a0, /*Address select bits 0-7*/
              a1, /*Address select bits 8-15*/
              a2; /*Address select bits 16-23*/
};

struct SMem smem;

uchar bus; /*Byte value on parallel bus, assume pull-down*/


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
SL_enable(uchar id)
{
        switch(id){
                case SL0_NULL:;
                case SL1_PAROE: bus = vm.por; break;
                case SL2_SMEMOE: bus = get_smemdata(); break;
                case SL3_SMEMWE: set_smemdata(bus); break;
                default:;
        }
}

void
SL_disable(uchar id)
{
        switch(id){
                case SL1_PAROE: bus = 0; /*Tri-state pull-down*/
                default:;
        }
}


void
SH_enable(uchar id)
{
        switch(id){
                case SH0_NULL:;
                case SH1_PARLE: vm.pir = bus;             break;
                case SH2_SMEMA0LE: smem.a0 = bus; break;
                case SH3_SMEMA1LE: smem.a1 = bus; break;
                case SH4_SMEMA2LE: smem.a2 = bus; break;
                default:;
        }
}


void
SH_disable(uchar id)
{
        switch(id){
                default:;
        }
}

void
SL_active(uchar id)
{
        switch(id){
                default:;
        }
}

void
SH_active(uchar id)
{
        switch(id){
                default:;
        }
}

void
virtualio() /*Run this after each CPU step for device emulation*/
{
        /*Handle virtual IO operation*/
        
        lnybble_old = vm.e_old & 0xF;
        lnybble_new = vm.e_new & 0xF;
        hnybble_old = (vm.e_old >> 4) & 0xF;
        hnybble_new = (vm.e_new >> 4) & 0xF;

        /*SL device selection changed*/
        if (lnybble_new != lnybble_old){
                SL_disable(lnybble_old); // falling edge
                SL_enable(lnybble_new); // rising edge
        } /*When level triggered*/
        else
        if (lnybble_new) SL_active(lnybble_new);

        /*SH device selection changed*/
        if (hnybble_new != hnybble_old){
                SH_disable(hnybble_old); // falling edge
                SH_enable(hnybble_new); // rising edge
        } /*When level triggered*/
        if (hnybble_new) else SH_active(hnybble_new);
}


#endif
