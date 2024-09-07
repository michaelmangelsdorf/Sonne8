/*
    LOX machine emulator for the Myth micro-controller core.
    It expects (or creates) the file "corestate.myst" in the working directory.

    Runs a batch of machine cycles.

    The firmware image communicates with LOX using the
    following buffers and variables:
    
    0x7F00-0x7F7F will be displayed as text on return.
    0x7F80-0x7FEF receives command line arguments (null-separated).
    0x7FF0 - 
    0x7FFF (See lox.h #defines)

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
#include "myth.h"
#include "myst.h"
#include "lox.h"

char* fname = "corestate.myst";
int i;

void
insertOrExitAt(int *offs)
{
        if( *offs >= 0xEF){
                print("truncated argline error");
                exits("truncated args");
        }
        else *offs = *offs + 1;
}

void
main(int argc, char *argv[])
{
        int cyc;
        int offs, chpos;
        char ch;

        print("LOX ");
        struct myth_vm vm;
        load(&vm, fname);


        /* Clear LOX arg buffer, output text buffer and return code
        */
        for( i=0x00; i<0xF0; i++)
                vm.ram[0x7F][i] = 0;

        /* Collect CLI parameters, concatenate at 0x7F80
        */
        offs = 0x80;
        for( i=0; i<argc; i++){
                if( i==0) continue;
                chpos = 0;
                while( (ch=argv[i][chpos++]) != 0){
                        vm.ram[0x7F][offs] = ch;
                        insertOrExitAt( &offs);
                }
                insertOrExitAt( &offs);
                vm.ram[0x7F][offs] = 0; /*Double zero*/
        }

        /* Cycle until VM executes END,
           Max. 10.000 cycles
        */
        for( cyc=1; cyc<999*1000; cyc++){
                myth_step( &vm);
                if ( vm.scrounge == END) break;
        }
        if( cyc==999*1000) {
                 print( "Error:\n");
                 print( "999k cycles elapsed without output request (re-run to continue)\n!\n");
                 exits( "Elapsed");
        }
        else{
                print("END after %d cycles: ", cyc);
                vm.c = 0;
                vm.pc = 0;
                vm.ram[0x7F][POS] = 0;
                // vm.l++; /* Fix L */
        }

        /* Output 0x7F00 to 0x7F7F as zero-terminated string
        */
        for( i=0x00; i<0x80; i++){
                ch = vm.ram[0x7F][i];
                if( !ch) break;
                print("%c", ch);
        }

        print("\n");
        save(&vm, fname);
        exits("");
}


