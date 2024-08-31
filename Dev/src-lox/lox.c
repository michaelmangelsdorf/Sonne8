/*
    LOX machine emulator for the Myth micro-controller core.
    It expects (or creates) the file "corestate.myst" in the working directory.

    Runs up to 10k machine cycles.
    0x7F00-0x7F7F will be displayed as text on return.
    0x7F80-0x7FFE receives arguments as concatened, spaced text.
    0x7FFF is a quit flag - if >0, exit and print output, and reset VM to c=0, j=0;

    Edit nettle, then run nettle, lox and regs in succession.

    Author: mim@ok-schalter.de (Michael/Dosflange@github)

    Requires a Plan 9 build environment:
    https://github.com/9fans/plan9port

    Build using:
    9c <progname>.c
    9l <progname>.o
    
    Run:
    ./a.out
*/

#include <u.h>
#include <libc.h>
#include "../src-myst/myth.h"
#include "../src-myst/myst.h"

char* fname="corestate.myst";
int i;

void
insertOrExitAt(int *offs)
{
        switch( *offs){
            case 0xFE:
            case 0xFF: {
                    print("truncated argline error");
                    exits("truncated args");
            }
            default: *offs = *offs + 1;
        }
}

void
main(int argc, char *argv[])
{
        int cyc, exitcode;
        int offs, chpos;
        char ch;

        print("LOX ");
        struct myth_vm vm;
        load(&vm, fname);


        /* Clear LOX arg buffer, output text buffer and return code
        */
        for( i=0x00; i<0x100; i++)
                vm.pagebyte[0x7F][i] = 0;

        /* Collect CLI parameters, concatenate at 0x7F80
        */
        offs = 0x80;
        for( i=0; i<argc; i++){
                if( i==0) continue;
                chpos = 0;
                while( (ch=argv[i][chpos++]) != 0){
                        insertOrExitAt( &offs);
                        vm.pagebyte[0x7F][offs] = ch;
                }
                insertOrExitAt( &offs);
                vm.pagebyte[0x7F][offs] = ' ';
        }
        vm.pagebyte[0x7F][offs] = 0; /* Replace final space */

        /* Cycle until 0x7FFF not equal to zero (return code)
           Max. 10.000 cycles
        */
        for( cyc=1; cyc<10000; cyc++){
                myth_cycle( &vm);
                exitcode = vm.pagebyte[0x7F][0xFF];
                if( exitcode != 0) break;
        }
        if( cyc==10000) print("elapsed (re-run to continue) ");
        else{
                print("%d", cyc);
                vm.c = 0;
                vm.pc = 0;
                vm.l++; /* Fix L */
        }
        print("/%.02Xh:", exitcode);

        /* Output 0x7F00 to 0x7F7F as zero terminated text
        */
        for( i=0x00; i<0x80; i++)
                print("%c", vm.pagebyte[0x7F][i]);

        print("\n");
        save(&vm, fname);
        exits("");
}


