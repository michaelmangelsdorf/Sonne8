/*
    Requires a Plan 9 build environment:
    https://github.com/9fans/plan9port

    Build using:
    9c myth.c
    9l myth.o
*/

#include <u.h>
#include <libc.h>
#include "myth.h"


void
main()
{
        struct myth_vm vm;
        myth_reset(&vm);
        myth_step(&vm);
        print("myth\n");
        exits("");
}





