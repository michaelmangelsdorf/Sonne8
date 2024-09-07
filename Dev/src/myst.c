/*
    Single-stepper (MyST) for the Myth micro-controller core.
    It expects (or creates) the file "corestate.myst" in the working directory.
    It runs one instruction cycle and updates the state file.

    Author: mim@ok-schalter.de (Michael/Dosflange@github)

    Requires a Plan 9 build environment:
    https://github.com/9fans/plan9port

    Build using:
    9c myst.c
    9l myst.o
    
    Run:
    ./a.out
*/

#include <u.h>
#include <libc.h>
#include "myth.h"
#include "myst.h"

char* fname="corestate.myst";
int fdesc;
int i;

/*Load machine state, advance by one cycle, save*/

void
main()
{
        struct myth_vm vm;
        load(&vm, fname);
        //myth_reset(&vm);
        myth_step(&vm);
        save(&vm, fname);
        //print("myth\n");
        exits("");
}




