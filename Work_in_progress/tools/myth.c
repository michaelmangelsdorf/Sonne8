/*
    Single-stepper for the Myth micro-controller core.
    It expects (or creates) the file "machine_state" in the working directory.
    It runs one instruction cycle and updates the state file.

    The home for this project is:
    https://github.com/Dosflange/Myth
    Author: mim@ok-schalter.de (Michael)

    Requires a Plan 9 build environment:
    https://github.com/9fans/plan9port

    Build using:
    9c myth.c
    9l myth.o
*/

#include <u.h>
#include <libc.h>
#include "myth.h"

char* fname="machine_state";
int fdesc;
int i;

void
load(struct myth_vm *vm)
{
        fdesc=open(fname, OREAD);
        if(fdesc != -1)
                read(fdesc, vm, sizeof(struct myth_vm));
        else{
                print("Created missing input file '%s'\n", fname);
                create(fname, 0, 0666);
        }
        close(fdesc);
}

void
save(struct myth_vm *vm)
{
        fdesc=open(fname, OWRITE);
        if(fdesc != -1)
                write(fdesc, vm, sizeof(struct myth_vm));
        else
                print("File error on output file '%s'\n", fname);
        close(fdesc);
}


/*Load machine state, advance by one cycle, save*/

void
main()
{
        struct myth_vm vm;
        load(&vm);
        //myth_reset(&vm);
        myth_cycle(&vm);
        save(&vm);
        //print("myth\n");
        exits("");
}




