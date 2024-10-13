/*
    LOX machine emulator for the Myth micro-controller core.
    It expects (or creates) the file "corestate.myst" in the working directory.

    Runs a batch of machine cycles.

    The firmware image communicates with LOX using the
    following buffers and variables:
    
    0x7F00-0x7F7F will be displayed as text on return.
    0x7F80-0x7FEF receives command line arguments (null-separated).
    0x7FF0 - 0x7FFF (System variables, see lox.h #defines)

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
#include "lox.h"
#include "io.h"


void load( struct myth_vm*, char *);
void save( struct myth_vm*, char *);
void loadsmem( char* fname);
void savesmem( char* fname);



struct myth_vm vm;
char* fname_vm = "corestate.myst";
int i,n;


void
save(struct myth_vm *vm, char *fname_vm)
{   
        int fdesc;
        create(fname_vm, 0, 0666);
        fdesc=open(fname_vm, OWRITE);
        if (fdesc != -1) {
             write(fdesc, vm, sizeof(struct myth_vm));
             close(fdesc);
        }
        else print("Write error\n");
}

void
load(struct myth_vm *vm, char *fname_vm)
{
        int fdesc;
        fdesc=open(fname_vm, OREAD);
        if(fdesc != -1) {
                read(fdesc, vm, sizeof(struct myth_vm));
                close(fdesc);
        }
        else save(vm, "corestate.myst");
}

void
savesmem(char* fname)
{
        int fdesc;
        create(fname, 0, 0666);
        fdesc=open(fname, OWRITE);
        if (fdesc != -1) {
             write(fdesc, smem.data, sizeof(smem.data));
             close(fdesc);
       }
        else print("Write error\n");
}

void
loadsmem(char* fname)
{
        int fdesc;
        fdesc=open(fname, OREAD);
        if(fdesc != -1) {
                read(fdesc, smem.data, sizeof(smem.data));
                close(fdesc);
        }
        else {
                print("Created LOX ramdisk file\n");
                savesmem("ramdisk.myth");
        }
}


/* This increments a string pointer, checking an upper bound
*/
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
greet()
{
        print("CLI Lox C version Dosflange/2409\n");
}

void
usage()
{
        print("Usage:\n");
        print("Single step\t-s\n");
        print("Print regs\t-r\n");
        print("Run with args\t[-f file] <args>\n\n");
        exits("Show usage completed");
}

void
singlestep()
{
        //myth_reset(&vm);
        myth_step(&vm);
        save(&vm, fname_vm);
        //print("myth\n");
        exits("Single step completed");
}

void
printregs()
{
        print( "LOX regs: ", vm.l);

        print( "r:%.02Xh(%d)(%b) ", vm.r, vm.r, vm.r);
        print( "o:%.02Xh(%d)(%b) ", vm.o, vm.o, vm.o);
        print( "c:%.02Xh(%d)(%b) ", vm.c, vm.c, vm.c);
        print( "d:%.02Xh(%d)(%b) ", vm.d, vm.d, vm.d);
        print( "pc:%.02Xh(%d)(%b) ", vm.pc, vm.pc, vm.pc);
        print( "i:%.02Xh(%d)(%b)\n", vm.i, vm.i, vm.i);
        print( "g:%.02Xh(%d)(%b) ", vm.g, vm.g, vm.g);

        print( "\te:%.02Xh(%d)(%b) ", vm.e_new, vm.e_new, vm.e_new);
        print( "sclk:%d ", vm.sclk ? 1:0);
        print( "miso:%d ", vm.miso ? 1:0);
        print( "mosi:%d ", vm.mosi ? 1:0);
        print( "sir:%.02Xh(%d)(%b) ", vm.sir, vm.sir, vm.sir);
        print( "sor:%.02Xh(%d)(%b) ", vm.sor, vm.sor, vm.sor);
        print( "pir:%.02Xh(%d)(%b) ", vm.pir, vm.pir, vm.pir);
        print( "por:%.02Xh(%d)(%b)\n", vm.por, vm.por, vm.por);

        print( "Locals @l%.02X: ", vm.l);
        for( i=0; i<8; i++){
                n = vm.ram[vm.l][GIRO_BASE_OFFSET +i];
                print( "L%d:%X(%d)(%b) ", i, n, n, n);
        }

        print( "\n");
        exits( "Register display completed");
}


void
main(int argc, char *argv[])
{
        int cyc;
        int offs, chpos;
        char ch;
        int withfile;

        withfile = 0;
        if (argc==1) usage();

        load(&vm, fname_vm);
        if (argc==2 && !strcmp("-s", argv[1])) singlestep();
        if (argc==2 && !strcmp("-r", argv[1])) printregs();
        if (!strcmp("-f", argv[1])){
        
                if (argc>3){
                        withfile = 1;
                        loadsmem(argv[2]);
                }
                else{
                        print("Too few arguments...\n");
                        usage();
                }
        }

        /* Clear LOX arg buffer, output text buffer and return code
        */
        for( i=0x00; i<0xF0; i++)
                vm.ram[0x7F][i] = 0;

        /* Collect CLI parameters, concatenate at 0x7F80
        */
        offs = 0x80;
        for( i = withfile ? 3:1; i<argc; i++){
                chpos = 0;
                while( (ch=argv[i][chpos++]) != 0){
                        vm.ram[0x7F][offs] = ch;
                        insertOrExitAt( &offs);
                }
                insertOrExitAt( &offs);
                vm.ram[0x7F][offs] = 0;
        }

        /* Cycle until VM executes END,
           given max. number of cycles
        */
        for( cyc=1; cyc<999*1000; cyc++){

                myth_step( &vm);
                if (vm.scrounge == END) break;
                else virtualio();
        }

        if( cyc==999*1000) {
                 print( "Error:\n");
                 print( "999k cycles elapsed without END (re-run?)\n!\n");
                 exits( "Elapsed");
        }
        else{
                print("END after %d cycles: ", cyc);

                /* Reset Program Counter for next run
                   Reset pointers to arg buffer and output text buffer
                   Reset ECODE
                   Reset L
                */
                
                vm.c = 0;
                vm.pc = 0;
                vm.l = 0;
                vm.ram[0x7F][POS] = 0;
                vm.ram[0x7F][ARG] = 0x80;
                vm.ram[0x7F][ECODE] = 0;
        }

        /* Output 0x7F00 to 0x7F7F as zero-terminated string
        */
        for( i=0x00; i<0x80; i++){
                ch = vm.ram[0x7F][i];
                if( !ch) break;
                print("%c", ch);
        }

        print("\n");
        save(&vm, fname_vm);
        if (withfile) savesmem(argv[2]);
        exits("Run completed");
}


