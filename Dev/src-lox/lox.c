/*
    LOX machine emulator for the Myth micro-controller core.
    It expects (or creates) the file "corestate.myst" in the working directory.

    Runs up to 10k machine cycles.
    
     0x7F00-0x7F7F will be displayed as text on return.
    0x7F80-0x7FEF receives arguments as concatened, spaced text.
    0x7FF0- (Reserved)    
    0x7FF9 / 0x7FFA VOCAB top pointer (page/offset)
    0x7FFB / 0x7FFC  First point (page/offset)
    0x7FFD / 0x7FFE  Second point (page/offset) 
    0x7FFF  Return code. If >0, exit and print output,
            and reset VM to c=0, j=0;

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
#include "lox.h"

char* fname="corestate.myst";
int i;

void
insertOrExitAt(int *offs)
{
        if( *offs >= 0xF0){
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
        for( cyc=1; cyc<999*1000; cyc++){
                myth_cycle( &vm);
                if ( vm.scrounge == END) break;
        }
        if( cyc==999*1000) {
                 print( "Error:\n");
                 print( "99k cycles elapsed without output request (re-run to continue)\n!\n");
                 exits( "Elapsed");
        }
        else{
                print("requested end after %d cycles: ", cyc);
                vm.c = 0;
                vm.pc = 0;
                vm.l++; /* Fix L */
        }

        /* Output 0x7F00 to 0x7F7F as zero terminated text
        */
        for( i=0x00; i<0x80; i++)
                print("%c", vm.pagebyte[0x7F][i]);

        print("\n");
        save(&vm, fname);
        exits("");
}


