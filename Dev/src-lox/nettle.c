/*
    Myth Project
    NETTLE Assembler for LOX Bootstrap Firmware
    Author: mim@ok-schalter.de (Michael/Dosflange@github)

    Requires a Plan 9 build environment:
    https://github.com/9fans/plan9port

    Build using:
    9c nettle.c
    9l nettle.o
    
    Run:
    ./a.out
*/

#include <u.h>
#include <libc.h>
#include "../src-myst/myth.h"
#include "../src-myst/myst.h"

char* fname = "../corestate.myst";
char* fnamesrc = "lox.asm";

p[0x00] = "        "
l[0x8000] = "        "; b[0x8000] = "00h       "; c[0x8000] = "                    "; 


void
main()
{
        struct myth_vm vm;
        load(&vm, fname);
        //myth_cycle(&vm);
        save(&vm, fname);
        exits("");
}
