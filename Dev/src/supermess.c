
/* Emulation routines for Sonne 8 micro-controller Rev. Myth/LOX
   Author: mim@ok-schalter.de (Michael/Dosflange@github)
    */

#include <u.h>
#include <libc.h>

struct { uchar val; char *str; } strlits[] = {
/*SYS*/
{0x00, "NOP"}, 
{0x01, "SSI"}, {0x02, "SSO"}, {0x03, "SCL"}, {0x04, "SCH"}, 
{0x05, "RET"}, {0x06, "COR"}, {0x07, "OWN"},
/*FIX*/
{0x08, "P4"}, 
{0x09, "P1"}, {0x0A, "P2"}, {0x0B, "P3"}, {0x0C, "M4"}, 
{0x0D, "M3"}, {0x0E, "M2"}, {0x0F, "M1"},
/*ALU*/
{0x10, "CLR"}, 
{0x11, "IDO"}, {0x12, "OCR"}, {0x13, "OCO"}, {0x14, "SLR"}, 
{0x15, "SLO"}, {0x16, "SRR"}, {0x17, "SRO"}, {0x18, "AND"}, 
{0x19, "IOR"}, {0x1A, "EOR"}, {0x1B, "ADD"}, {0x1C, "CAR"}, 
{0x1D, "RLO"}, {0x1E, "REO"}, {0x1F, "RGO"},
/*TRAP*/
{0x20, "*0"}, 
{0x21, "*1"}, {0x22, "*2"}, {0x23, "*3"}, {0x24, "*4"}, 
{0x25, "*5"}, {0x26, "*6"}, {0x27, "*7"}, {0x28, "*8"}, 
{0x29, "*9"}, {0x2A, "*10"}, {0x2B, "*11"}, {0x2C, "*12"}, 
{0x2D, "*13"}, {0x2E, "*14"}, {0x2F, "*15"}, {0x30, "*16"}, 
{0x31, "*17"}, {0x32, "*18"}, {0x33, "*19"}, {0x34, "*20"}, 
{0x35, "*21"}, {0x36, "*22"}, {0x37, "*23"}, {0x38, "*24"}, 
{0x39, "*25"}, {0x3A, "*26"}, {0x3B, "*27"}, {0x3C, "*28"}, 
{0x3D, "*29"}, {0x3E, "*30"}, {0x3F, "*31"},
/*GIRO*/
{0x40, "0d"}, {0x41, "1d"}, {0x42, "2d"}, {0x43, "3d"}, {0x44, "4d"}, {0x45, "5d"}, {0x46, "6d"}, {0x47, "7d"},
{0x48, "d0"}, {0x49, "d1"}, {0x4A, "d2"}, {0x4B, "d3"}, {0x4C, "d4"}, {0x4D, "d5"}, {0x4E, "d6"}, {0x4F, "d7"},
{0x50, "0i"}, {0x51, "1i"}, {0x52, "2i"}, {0x53, "3i"}, {0x54, "4i"}, {0x55, "5i"}, {0x56, "6i"}, {0x57, "7i"},
{0x58, "i0"}, {0x59, "i1"}, {0x5A, "i2"}, {0x5B, "i3"}, {0x5C, "i4"}, {0x5D, "i5"}, {0x5E, "i6"}, {0x5F, "i7"},
{0x60, "0r"}, {0x61, "1r"}, {0x62, "2r"}, {0x63, "3r"}, {0x64, "4r"}, {0x65, "5r"}, {0x66, "6r"}, {0x67, "7r"},
{0x68, "r0"}, {0x69, "r1"}, {0x6A, "r2"}, {0x6B, "r3"}, {0x6C, "r4"}, {0x6D, "r5"}, {0x6E, "r6"}, {0x6F, "r7"},
{0x70, "0o"}, {0x71, "1o"}, {0x72, "2o"}, {0x73, "3o"}, {0x74, "4o"}, {0x75, "5o"}, {0x76, "6o"}, {0x77, "7o"},
{0x78, "o0"}, {0x79, "o1"}, {0x7A, "o2"}, {0x7B, "o3"}, {0x7C, "o4"}, {0x7D, "o5"}, {0x7E, "o6"}, {0x7F, "o7"},
/*PAIR*/
{0x80, "no"}, {0x81, "END"}, {0x82, "-"}, {0x83, "nd"}, {0x84, "nr"}, {0x85, "ni"}, {0x86, "ns"}, {0x87, "np"}, {0x88, "ne"}, {0x89, "na"}, {0x8A, "nb"}, {0x8B, "nj"}, {0x8C, "nw"}, {0x8D, "nt"}, {0x8E, "nf"}, {0x8F, "nc"},
{0x90, "mo"}, {0x91, "--"}, {0x92, "--"}, {0x93, "md"}, {0x94, "mr"}, {0x95, "mi"}, {0x96, "ms"}, {0x97, "mp"}, {0x98, "me"}, {0x99, "ma"}, {0x9A, "mb"}, {0x9B, "mj"}, {0x9C, "mw"}, {0x9D, "mt"}, {0x9E, "mf"}, {0x9F, "mc"},
{0xA0, "lo"}, {0xA1, "--"}, {0xA2, "--"}, {0xA3, "ld"}, {0xA4, "lr"}, {0xA5, "li"}, {0xA6, "ls"}, {0xA7, "lp"}, {0xA8, "le"}, {0xA9, "la"}, {0xAA, "lb"}, {0xAB, "lj"}, {0xAC, "lw"}, {0xAD, "lt"}, {0xAE, "lf"}, {0xAF, "lc"},
{0xB0, "do"}, {0xB1, "dm"}, {0xB2, "dl"}, {0xB3, "--"}, {0xB4, "dr"}, {0xB5, "di"}, {0xB6, "ds"}, {0xB7, "dp"}, {0xB8, "de"}, {0xB9, "da"}, {0xBA, "db"}, {0xBB, "dj"}, {0xBC, "dw"}, {0xBD, "dt"}, {0xBE, "df"}, {0xBF, "dc"},
{0xC0, "ro"}, {0xC1, "rm"}, {0xC2, "rl"}, {0xC3, "rd"}, {0xC4, "--"}, {0xC5, "ri"}, {0xC6, "rs"}, {0xC7, "rp"}, {0xC8, "re"}, {0xC9, "ra"}, {0xCA, "rb"}, {0xCB, "rj"}, {0xCC, "rw"}, {0xCD, "rt"}, {0xCE, "rf"}, {0xCF, "rc"},
{0xD0, "io"}, {0xD1, "im"}, {0xD2, "il"}, {0xD3, "id"}, {0xD4, "ir"}, {0xD5, "--"}, {0xD6, "is"}, {0xD7, "ip"}, {0xD8, "ie"}, {0xD9, "ia"}, {0xDA, "ib"}, {0xDB, "ij"}, {0xDC, "iw"}, {0xDD, "it"}, {0xDE, "if"}, {0xDF, "ic"},
{0xE0, "so"}, {0xE1, "sm"}, {0xE2, "sl"}, {0xE3, "sd"}, {0xE4, "sr"}, {0xE5, "si"}, {0xE6, "ss"}, {0xE7, "sp"}, {0xE8, "se"}, {0xE9, "sa"}, {0xEA, "sb"}, {0xEB, "sj"}, {0xEC, "sw"}, {0xED, "st"}, {0xEE, "sf"}, {0xEF, "sc"},
{0xF0, "po"}, {0xF1, "pm"}, {0xF2, "pl"}, {0xF3, "pd"}, {0xF4, "pr"}, {0xF5, "pi"}, {0xF6, "ps"}, {0xF7, "pp"}, {0xF8, "pe"}, {0xF9, "pa"}, {0xFA, "pb"}, {0xFB, "pj"}, {0xFC, "pw"}, {0xFD, "pt"}, {0xFE, "pf"}, {0xFF, "pc"}, 
{0x00,"*HALT*"}
};

struct myth_vm /*Complete machine state including all ram*/
{
        uchar ram[256][256];

        uchar e;    /*Device ENABLE register */

        uchar sclk; /*Serial clock state bit*/
        uchar miso; /*Serial input line state bit*/
        uchar mosi; /*Serial output line state bit*/
        uchar sir;  /*Serial input register, modelled after HC595*/
        uchar sor;  /*Serial output register, modelled after HC165*/

        uchar pir;  /*Parallel input register*/
        uchar por;  /*Parallel output register*/

        uchar r;    /*RESULT*/
        uchar o;    /*OFFSET*/

        uchar i;    /*INNER*/
        uchar pc;   /*PROGRAM Counter*/

        uchar co;   /*Coroutine register*/
        uchar c;    /*CODE page register*/
        uchar d;    /*DATA page register*/
        uchar l;    /*LOCAL page register*/

        /*Set by scrounge instruction stub.
          Not part of state!
        */
        uchar scrounge; /*An application specific opcode*/
};

void myth_reset(struct myth_vm *vm);
void myth_step(struct myth_vm *vm);

static uchar fetch(struct myth_vm *vm);
static uchar srcval(struct myth_vm *vm, uchar srcreg, uchar dst);
static int scrounge(uchar opcode);
static void pair(struct myth_vm *vm, uchar opcode);
static void diro(struct myth_vm *vm, uchar opcode);
static void trap(struct myth_vm *vm, uchar opcode);
static void alu(struct myth_vm *vm, uchar opcode);
static void fix(struct myth_vm *vm, uchar opcode);
static void sys(struct myth_vm *vm, uchar opcode);
static void call(struct myth_vm *vm, uchar dstpage);


/* The 'REGx' notation means: REG into (something)
   i.e. REG is a source
*/

#define Nx 0 /*from code literal (NUMBER)*/
#define Mx 1 /*from MEMORY via DATA page index*/
#define Lx 2 /*from MEMORY via LOCAL page index*/
#define Dx 3 /*from GLOBAL register*/
#define Rx 4 /*from RESULT register*/
#define Ix 5 /*from DATA page register*/
#define Sx 6 /*from SERIAL input*/
#define Px 7 /*from PARALLEL input*/


/* The 'xREG' notation means: (something) into REG
   i.e. REG is a destination
*/

#define xO 0 /*to ORIGIN register*/
#define xM 1 /*to MEMORY via DATA page index*/
#define xL 2 /*to MEMORY via LOCAL page index*/
#define xD 3 /*to GLOBAL register*/
#define xR 4 /*to RESULT register*/
#define xI 5 /*to INNER register*/
#define xS 6 /*to SERIAL output*/
#define xP 7 /*to PARALLEL output*/

#define xE 8 /*to ENABLE register*/
#define xA 9 /*to O and D (ADD byte to 16-bit register pair)*/
#define xB 10 /*to DATA page register*/
#define xJ 11 /*write JUMP program counter*/
#define xW 12 /*write JUMP WHILE I not zero, decrement I*/
#define xT 13 /*write JUMP if R not zero*/
#define xF 14 /*write JUMP if R zero*/
#define xC 15 /*write CALL page index*/


/*ALU Instructions
*/

#define CLR 0 /*Clear (value 0)*/
#define IDO 1 /*Identity O*/
#define OCR 2 /*Ones' complement of R*/
#define OCO 3 /*Ones' complement of O*/
#define SLR 4 /*Shift left R*/
#define SLO 5 /*Shift left O*/
#define SRR 6 /*Shift right R*/
#define SRO 7 /*Shift right O*/
#define AND 8 /*R AND O*/
#define IOR 9 /*R OR O*/
#define EOR 10 /*R XOR O*/
#define ADD 11 /*R + O*/
#define CAR 12 /*Carry of R + O (0 or 1)*/
#define RLO 13 /*255 if R<O else 0*/
#define REO 14 /*255 if R=O else 0*/
#define RGO 15 /*255 if R>O else 0*/


/*SYS Instructions
*/

#define NOP 0 /*No Operation*/
#define SSI 1 /*Serial Shift In*/
#define SSO 2 /*Serial Shift Out*/
#define SCL 3 /*Set serial Clock Low*/
#define SCH 4 /*Set serial Clock High*/
#define RET 5 /*Return from nested call*/
#define COR 6 /*Pointer jump*/
#define OWN 7 /*Copy code page index*/


/*FIX Instructions
*/

#define P4 0 /*R PLUS 4*/
#define P1 1
#define P2 2
#define P3 3
#define M4 4 /*R MINUS 4*/
#define M3 5
#define M2 6
#define M1 7

#define DIRO_BASE_OFFSET 0xF8 /*Local-page offset used by DIRO instructions*/

void
myth_reset(struct myth_vm *vm) /*Initialise machine state*/
{
        memset(vm->ram, 0, 256*256);

        vm->e = 0; /*Deselect any device*/

        vm->sclk = 0;
        vm->miso = 0;
        vm->mosi = 0;
        vm->sir = 0;
        vm->sor = 0;

        vm->pir = 0;
        vm->por = 0;

        vm->r = 0;
        vm->o = 0;
        vm->i = 0;
        vm->pc = 0;

        vm->co = 0;
        vm->c = 0;
        vm->d = 0;
        vm->l = 0;

        vm->scrounge = 0;
}


/* Single-step 1 instruction cycle
*/

void
myth_step(struct myth_vm *vm)
{
        vm->scrounge = 0;
        uchar opcode = fetch(vm);

        /*Decode priority encoded opcode*/
        /*Execute decoded instruction*/

        if (opcode&0x80) {print("PAIR\t"); pair(vm, opcode);}
        else if (opcode&0x40) {print("DIRO\t"); diro(vm, opcode);}
        else if (opcode&0x20) {print("TRAP\t"); trap(vm, opcode);}
        else if (opcode&0x10) {print("ALU\t"); alu(vm, opcode);}
        else if (opcode&0x08) {print("FIX\t"); fix(vm, opcode);}
        else {print("SYS\t"); sys(vm, opcode);}
}


/* Fetch next byte in CODE stream, then increment PC.
   Fetches either an instruction, or an instruction literal (Nx)
*/

uchar
fetch(struct myth_vm *vm)
{
        uchar val = vm->ram[ vm->c][ vm->pc];
        (vm->pc)++;
        return val;
}

char srcstr[80];

/* TRAP has identical operation, but with an
   immediate destination page operand
*/

void
call(struct myth_vm *vm, uchar dstpage)
{
        print("i=pc; pc=0; co=c; c=%s; l--",srcstr); 
    
        /*Save origin*/
        vm->i = vm->pc;
        vm->co = vm->c;

        /*Create stack frame*/
        vm->l--;

        /*Branch to page head*/
        vm->pc = 0;
        vm->c = dstpage;
}


void
trap(struct myth_vm *vm, uchar opcode)
{
        sprint(srcstr, "%d", opcode&31); 
        uchar dstpage = opcode & 31; /*Zero except low order 5 bits*/
        call(vm, dstpage);
}


/* First part of handling PAIR instructions,
   derives the source value
*/



uchar
srcval(struct myth_vm *vm, uchar srcreg, uchar dst)
{
        switch(srcreg){
                case Nx:
                    if (dst==xJ || dst==xW || dst==xF || dst==xT || dst==xB) strcpy(srcstr, "ram[c][pc]");
                    else if (dst==xC) strcpy(srcstr,"ram[c][pc]");
                    else strcpy(srcstr, "ram[c][pc]; pc++");
                    return fetch(vm); /*pseudo reg*/
                case Mx: strcpy(srcstr, "ram[d][o]"); return vm->ram[ vm->d][ vm->o]; /*pseudo reg*/
                case Lx: strcpy(srcstr, "ram[l][o]"); return vm->ram[ vm->l][ vm->o]; /*pseudo reg*/
                case Dx: strcpy(srcstr, "d"); return vm->d;
                case Rx: strcpy(srcstr, "r"); return vm->r;
                case Ix: strcpy(srcstr, "i"); return vm->i;
                case Sx: strcpy(srcstr, "sir"); return vm->sir;
           default /*Px*/: strcpy(srcstr, "pir"); return vm->pir;
        }
}


/* SCROUNGING of certain PAIR instructions:
Remap undesirable opcodes to other,
application specific opcodes or CPU extensions etc.
Don't assume that these are generally NOPs!
*/

int
scrounge(uchar opcode)
{
        switch(opcode & 0x7F /*0111_1111 zero b7*/){
                case 16*Nx + xM: print("NM scrounge"); return opcode; /*NM => NOP (reserved)*/
                case 16*Nx + xL: print("NL scrounge"); return opcode; /*NL => NOP (reserved)*/
                case 16*Mx + xM: print("MM scrounge"); return opcode; /*MM => NOP (reserved)*/
                case 16*Mx + xL: print("ML scrounge"); return opcode; /*ML => NOP (reserved)*/
                case 16*Lx + xM: print("LM scrounge"); return opcode; /*LM => NOP (reserved)*/
                case 16*Lx + xL: print("LL scrounge"); return opcode; /*LL => NOP (reserved)*/
                case 16*Dx + xD: print("DD scrounge"); return opcode; /*DD => NOP (reserved)*/
                case 16*Rx + xR: print("RR scrounge"); return opcode; /*RR => NOP (reserved)*/
                case 16*Ix + xI: print("II scrounge"); return opcode; /*II => NOP (reserved)*/
        }
        return 0;
}


/* Second part of handling PAIR instructions,
   stores source value into destination
*/

void
pair(struct myth_vm *vm, uchar opcode)
{
        uchar src = (opcode >> 4) & 7; /*Zero except bits 4-6 at LSB*/
        uchar dst = opcode & 15; /* Zero except bits 0-3 at LSB*/

        if (scrounge(opcode)){
                 vm->scrounge = opcode;
                 return;
        }

        uchar v = srcval(vm, src, dst);
        int temp;
        switch(dst){
                case xO: vm->o = v; print("o=%s",srcstr); break;
                case xM: vm->ram[ vm->d][ vm->o] = v; print("ram[d][o]=%s",srcstr); break;
                case xL: vm->ram[ vm->l][ vm->o] = v; print("ram[l][o]=%s",srcstr); break;
                case xD: vm->d = v; print("d=%s",srcstr); break;
                case xR: vm->r = v; print("r=%s",srcstr); break;
                case xI: vm->i = v; print("i=%s",srcstr); break;
                case xS: vm->sor = v; print("sor=%s",srcstr); break;
                case xP: vm->por = v; print("por=%s",srcstr); break;
                case xE: vm->e = v; print("e=%s",srcstr); break;
                case xA:
                        print("if (o+%s)>255 then d+=1",srcstr); 
                        temp = vm->o + v;
                        vm->o = (uchar) (temp & 0xFF);
                        if (temp>255) vm->d += 1;
                        break;
                case xB: vm->pc = vm->pc + v; print("pc+=%s",srcstr); break;
                case xJ: vm->pc = v; print("pc=%s",srcstr); break;
                case xW:
                        print("if i!=0 then pc=%s; i--",srcstr); 
                        if (vm->i) vm->pc = v;
                        (vm->i)--; /*Post decrement, either case!*/
                        break; 
                case xT: if (vm->r) vm->pc = v; print("if r!=0 then pc=%s",srcstr); break;
                case xF: if (!vm->r) vm->pc = v; print("if r==0 then pc=%s",srcstr); break;
                case xC: call(vm, v); break;
        }
}

void
diro(struct myth_vm *vm, uchar opcode) /*Execute DIRO instruction*/
{
        /* OPCODE
            BITS 0-2 encode byte address offset in local page (from F8)
            BIT 3 encodes GET/PUT mode
            BITS 4-5 encode register index (DIRO)
        */

        #define BIT3 8
        #define BITS45 (opcode >> 4) & 3
        #define BITS02 opcode & 7

        uchar index = BITS02;              
        uchar *mptr = &(vm->ram[vm->l][DIRO_BASE_OFFSET + index]);

        if(opcode & BIT3)
                switch(BITS45){
                        case 0: *mptr = vm->d; print("L%d = d",index); break;
                        case 1: *mptr = vm->i; print("L%d = i",index); break;
                        case 2: *mptr = vm->r; print("L%d = r",index); break;
                        case 3: *mptr = vm->o; print("L%d = o",index); break;
                }
        else
        switch(BITS45){
                case 0: vm->d = *mptr; print("d=L%d",index); break;
                case 1: vm->i = *mptr; print("i=L%d",index); break;
                case 2: vm->r = *mptr; print("r=L%d",index); break;
                case 3: vm->o = *mptr; print("o=L%d",index); break;
        }
}


void
alu(struct myth_vm *vm, uchar opcode)
{
        switch(opcode & 15){/* Zero except low order 4 bits*/
                case CLR: vm->r = 0; print("r=0"); break;
                case IDO: vm->r = vm->o; print("r=o"); break;
                case OCR: vm->r = ~vm->r; print("r=~r"); break;
                case OCO: vm->r = ~vm->o; print("r=~o"); break;
                case SLR: vm->r = vm->r << 1; print("r=r<<1"); break;
                case SLO: vm->r = vm->o << 1; print("r=o<<1"); break;
                case SRR: vm->r = vm->r >> 1; print("r=r>>1"); break;
                case SRO: vm->r = vm->o >> 1; print("r=o>>1"); break;
                case AND: vm->r = vm->r & vm->o; print("r=r&o"); break;
                case IOR: vm->r = vm->r | vm->o; print("r=r|o"); break;
                case EOR: vm->r = vm->r ^ vm->o; print("r=r^o"); break;
                case ADD: vm->r = vm->r + vm->o; print("r=r+o"); break;
                case CAR:
                        print("if (r+o)>255 then r=1 else r=0"); 
                        vm->r = (uint) vm->r + (uint) vm->o > 255 ? 1 : 0;
                        break;
                case RLO: vm->r = (vm->r < vm->o) ? 255 : 0; print("if r<o then r=255 else r=0"); break;
                case REO: vm->r = (vm->r == vm->o) ? 255 : 0; print("if r=o then r=255 else r=0"); break;
                case RGO: vm->r = (vm->r > vm->o) ? 255 : 0; print("if r>o then r=255 else r=0"); break;
        }
}


void /*Add sign-extended number to R*/
fix(struct myth_vm *vm, uchar opcode)
{
        switch(opcode & 7){ /*Zero except low order 3 bits*/
                case P4: vm->r += 4; print("r+=4"); break;
                case P1: vm->r += 1; print("r+=1"); break;
                case P2: vm->r += 2; print("r+=2"); break;
                case P3: vm->r += 3; print("r+=3"); break;
                case M4: vm->r -= 4; print("r-=4"); break;
                case M3: vm->r -= 3; print("r-=3"); break;
                case M2: vm->r -= 2; print("r-=2"); break;
                case M1: vm->r -= 1; print("r-=1"); break;
        }
}


void
sys(struct myth_vm *vm, uchar opcode)
{
        switch(opcode & 7){ /*Zero except low order 3 bits*/
                
                case NOP: print("no operation"); break;
                case SSI:
                        /*Clocks in MISO line bit into LSB*/
                        print("sir=(sir<<1)+miso");
                        vm->sir = ((vm->sir)<<1) + vm->miso;
                        break;
                case SSO:
                        /*Clocks out MSB first*/
                        print("mosi=sor&0x80 ? 1:0; sor<<=1");
                        vm->mosi = (vm->sor)&0x80 ? 1:0;
                        vm->sor <<= 1;
                        break;
                case SCL: vm->sclk = 0; print("sclk=0"); break;
                case SCH: vm->sclk = 1; print("sclk=1"); break;

                #define L7 (vm->ram[vm->l][DIRO_BASE_OFFSET +7])

                case RET:
                        print("c=L7; pc=i; l++;");
                        vm->c = L7;
                        vm->pc = vm->i;
                        vm->l++;
                        break;

                case COR:
                        print("c=d; pc=i");
                        vm->c = vm->d;
                        vm->pc = vm->i;
                        break;

                case OWN: print("L7=co");
                 L7 = vm->co; break;
        }
}



void
main()
{
    struct myth_vm vm;
    myth_reset(&vm);
    for( int i=0; i<=255; i++){
            vm.ram[0][0] = i;
            vm.c = 0;
            vm.pc = 0;
            print( "%d\t", i);
            print( "%s\t", strlits[i].str);
            myth_step(&vm);
            print("\n");
    }
}
