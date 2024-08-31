
/* Emulation routines for Sonne 8 micro-controller Rev. Myth/LOX
   Author: mim@ok-schalter.de (Michael/Dosflange@github)
    */

#include <u.h>
#include <libc.h>

/* Struct for looking up opcodes for string literals
   such as numbers and mnemonics */
struct { uchar val; char *str; } opcode[] = {
{0x00, "NOP"},
{0x01, "SSI"}, {0x02, "SSO"}, {0x03, "SCL"}, {0x04, "SCH"}, 
{0x05, "RET"}, {0x06, "FAR"}, {0x07, "ORG"}, {0x08, "P4"}, 
{0x09, "P1"}, {0x0A, "P2"}, {0x0B, "P3"}, {0x0C, "N4"}, 
{0x0D, "N3"}, {0x0E, "N2"}, {0x0F, "N1"}, {0x10, "IDR"}, 
{0x11, "IDO"}, {0x12, "OCR"}, {0x13, "OCO"}, {0x14, "SLR"}, 
{0x15, "SLO"}, {0x16, "SRR"}, {0x17, "SRO"}, {0x18, "AND"}, 
{0x19, "IOR"}, {0x1A, "EOR"}, {0x1B, "ADD"}, {0x1C, "CAR"}, 
{0x1D, "RLO"}, {0x1E, "REO"}, {0x1F, "RGO"}, {0x20, "*0"}, 
{0x21, "*1"}, {0x22, "*2"}, {0x23, "*3"}, {0x24, "*4"}, 
{0x25, "*5"}, {0x26, "*6"}, {0x27, "*7"}, {0x28, "*8"}, 
{0x29, "*9"}, {0x2A, "*10"}, {0x2B, "*11"}, {0x2C, "*12"}, 
{0x2D, "*13"}, {0x2E, "*14"}, {0x2F, "*15"}, {0x30, "*16"}, 
{0x31, "*17"}, {0x32, "*18"}, {0x33, "*19"}, {0x34, "*20"}, 
{0x35, "*21"}, {0x36, "*22"}, {0x37, "*23"}, {0x38, "*24"}, 
{0x39, "*25"}, {0x3A, "*26"}, {0x3B, "*27"}, {0x3C, "*28"}, 
{0x3D, "*29"}, {0x3E, "*30"}, {0x3F, "*31"}, {0x40, "0r"}, 
{0x41, "1r"}, {0x42, "2r"}, {0x43, "3r"}, {0x44, "4r"}, 
{0x45, "5r"}, {0x46, "6r"}, {0x47, "7r"}, {0x48, "r0"}, 
{0x49, "r1"}, {0x4A, "r2"}, {0x4B, "r3"}, {0x4C, "r4"}, 
{0x4D, "r5"}, {0x4E, "r6"}, {0x4F, "r7"}, {0x50, "0o"}, 
{0x51, "1o"}, {0x52, "2o"}, {0x53, "3o"}, {0x54, "4o"}, 
{0x55, "5o"}, {0x56, "6o"}, {0x57, "7o"}, {0x58, "o0"}, 
{0x59, "o1"}, {0x5A, "o2"}, {0x5B, "o3"}, {0x5C, "o4"}, 
{0x5D, "o5"}, {0x5E, "o6"}, {0x5F, "o7"}, {0x60, "0d"}, 
{0x61, "1d"}, {0x62, "2d"}, {0x63, "3d"}, {0x64, "4d"}, 
{0x65, "5d"}, {0x66, "6d"}, {0x67, "7d"}, {0x68, "d0"}, 
{0x69, "d1"}, {0x6A, "d2"}, {0x6B, "d3"}, {0x6C, "d4"}, 
{0x6D, "d5"}, {0x6E, "d6"}, {0x6F, "d7"}, {0x70, "0g"}, 
{0x71, "1g"}, {0x72, "2g"}, {0x73, "3g"}, {0x74, "4g"}, 
{0x75, "5g"}, {0x76, "6g"}, {0x77, "7g"}, {0x78, "g0"}, 
{0x79, "g1"}, {0x7A, "g2"}, {0x7B, "g3"}, {0x7C, "g4"}, 
{0x7D, "g5"}, {0x7E, "g6"}, {0x7F, "g7"}, {0x80, "no"}, 
{0x81, "DEO"}, {0x82, "INO"}, {0x83, "ng"}, {0x84, "nr"}, 
{0x85, "ni"}, {0x86, "ns"}, {0x87, "np"}, {0x88, "ne"}, 
{0x89, "na"}, {0x8A, "nd"}, {0x8B, "nj"}, {0x8C, "nw"}, 
{0x8D, "nt"}, {0x8E, "nf"}, {0x8F, "nc"}, {0x90, "mo"}, 
{0x91, "---"}, {0x92, "---"}, {0x93, "mg"}, {0x94, "mr"}, 
{0x95, "mi"}, {0x96, "ms"}, {0x97, "mp"}, {0x98, "me"}, 
{0x99, "ma"}, {0x9A, "md"}, {0x9B, "mj"}, {0x9C, "mw"}, 
{0x9D, "mt"}, {0x9E, "mf"}, {0x9F, "mc"}, {0xA0, "lo"}, 
{0xA1, "---"}, {0xA2, "---"}, {0xA3, "lg"}, {0xA4, "lr"}, 
{0xA5, "li"}, {0xA6, "ls"}, {0xA7, "lp"}, {0xA8, "le"}, 
{0xA9, "la"}, {0xAA, "ld"}, {0xAB, "lj"}, {0xAC, "lw"}, 
{0xAD, "lt"}, {0xAE, "lf"}, {0xAF, "lc"}, {0xB0, "go"}, 
{0xB1, "gm"}, {0xB2, "gl"}, {0xB3, "---"}, {0xB4, "gr"}, 
{0xB5, "gi"}, {0xB6, "gs"}, {0xB7, "gp"}, {0xB8, "ge"}, 
{0xB9, "ga"}, {0xBA, "gd"}, {0xBB, "gj"}, {0xBC, "gw"}, 
{0xBD, "gt"}, {0xBE, "gf"}, {0xBF, "gc"}, {0xC0, "ro"}, 
{0xC1, "rm"}, {0xC2, "rl"}, {0xC3, "rg"}, {0xC4, "---"}, 
{0xC5, "ri"}, {0xC6, "rs"}, {0xC7, "rp"}, {0xC8, "re"}, 
{0xC9, "ra"}, {0xCA, "rd"}, {0xCB, "rj"}, {0xCC, "rw"}, 
{0xCD, "rt"}, {0xCE, "rf"}, {0xCF, "rc"}, {0xD0, "io"}, 
{0xD1, "im"}, {0xD2, "il"}, {0xD3, "ig"}, {0xD4, "ir"}, 
{0xD5, "---"}, {0xD6, "is"}, {0xD7, "ip"}, {0xD8, "ie"}, 
{0xD9, "ia"}, {0xDA, "id"}, {0xDB, "ij"}, {0xDC, "iw"}, 
{0xDD, "it"}, {0xDE, "if"}, {0xDF, "ic"}, {0xE0, "so"}, 
{0xE1, "sm"}, {0xE2, "sl"}, {0xE3, "sg"}, {0xE4, "sr"}, 
{0xE5, "si"}, {0xE6, "ss"}, {0xE7, "sp"}, {0xE8, "se"}, 
{0xE9, "sa"}, {0xEA, "sd"}, {0xEB, "sj"}, {0xEC, "sw"}, 
{0xED, "st"}, {0xEE, "sf"}, {0xEF, "sc"}, {0xF0, "po"}, 
{0xF1, "pm"}, {0xF2, "pl"}, {0xF3, "pg"}, {0xF4, "pr"}, 
{0xF5, "pi"}, {0xF6, "ps"}, {0xF7, "pp"}, {0xF8, "pe"}, 
{0xF9, "pa"}, {0xFA, "pd"}, {0xFB, "pj"}, {0xFC, "pw"}, 
{0xFD, "pt"}, {0xFE, "pf"}, {0xFF, "pc"}, 
 {0x00,"HALT"}
};

char *
opcToMnemonic( uchar opc)
{
        int i=0;
        for(;;){
                if( opcode[i].val == opc)
                        return opcode[i].str;
                else
                if( !strcmp( opcode[i++].str, "HALT"))
                        break;
        }
        return NULL;
}

struct myth_vm
{
        uchar pagebyte[256][256];

        uchar e;    /*Device ENABLE register */

        uchar sclk; /*Serial clock state bit*/
        uchar miso; /*Serial input line state bit*/
        uchar mosi; /*Serial output line state bit*/
        uchar sir;  /*Serial input register, modelled after HC595*/
        uchar sor;  /*Serial output register, modelled after HC165*/

        uchar pir;  /*Parallel input register*/
        uchar por;  /*Parallel output register*/

        uchar pc;    /*JUMP register (program counter)*/
        uchar o;    /*ORIGIN*/
        uchar r;    /*RESULT*/

        uchar c;    /*CODE page register*/
        uchar d;    /*DATA page register*/
        uchar l;    /*LOCAL page register*/

        uchar g;    /*GLOBAL*/
        uchar i;    /*INNER*/
};

void myth_reset(struct myth_vm *vm);
uchar myth_fetch(struct myth_vm *vm, int);
void myth_step(struct myth_vm *vm);
void myth_exec_pair(struct myth_vm *vm, uchar opcode);
void myth_exec_gput(struct myth_vm *vm, uchar opcode);
void myth_exec_trap(struct myth_vm *vm, uchar opcode);
void myth_exec_alu(struct myth_vm *vm, uchar opcode);
void myth_exec_adj(struct myth_vm *vm, uchar opcode);
void myth_exec_sys(struct myth_vm *vm, uchar opcode);
void myth_call(struct myth_vm *vm, uchar dstpage);
void myth_ret(struct myth_vm *vm);


/*  The 'REGx' notation means: REG into (something)
    i.e. REG is a source
*/

#define Nx 0 /*from code literal (NUMBER)*/
#define Mx 1 /*from MEMORY via DATA page index*/
#define Lx 2 /*from MEMORY via LOCAL page index*/
#define Gx 3 /*from GLOBAL register*/
#define Rx 4 /*from RESULT register*/
#define Ix 5 /*from INNER register*/
#define Sx 6 /*from SERIAL input*/
#define Px 7 /*from PARALLEL input*/

/*  The 'xREG' notation means: (something) into REG
    i.e. REG is a destination
*/

#define xO 0 /*to ORIGIN register*/
#define xM 1 /*to MEMORY via DATA page index*/
#define xL 2 /*to MEMORY via LOCAL page index*/
#define xG 3 /*to GLOBAL register*/
#define xR 4 /*to RESULT register*/
#define xI 5 /*to INNER register*/
#define xS 6 /*to SERIAL output*/
#define xP 7 /*to PARALLEL output*/

#define xE 8 /*to ENABLE register*/
#define xA 9 /*to GLOBAL register (ADD I)*/
#define xD 10 /*to DATA page register*/
#define xJ 11 /*write JUMP program counter*/
#define xW 12 /*write JUMP WHILE I not zero, decrement I*/
#define xT 13 /*write JUMP if R not zero*/
#define xF 14 /*write JUMP if R zero*/
#define xC 15 /*write CALL page index*/

#define IDR 0 /*Identity R*/
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

#define NOP 0 /*No Operation*/
#define SSI 1 /*Serial Shift In*/
#define SSO 2 /*Serial Shift Out*/
#define SCL 3 /*Set serial Clock Low*/
#define SCH 4 /*Set serial Clock High*/
#define RET 5 /*Wide Return*/
#define FAR 6 /*Wide Branch*/
#define SIP 7 /*Get wide PC*/

#define R4A 0
#define R1A 1
#define R2A 2
#define R3A 3
#define R4S 4
#define R3S 5
#define R2S 6
#define R1S 7



void
myth_reset(struct myth_vm *vm)
{
        print( "Reset\n");

        memset(vm->pagebyte, 0, 256*256);

        vm->e = 0; /* Deselect any device */

        vm->sclk = 0;
        vm->miso = 0;
        vm->mosi = 0;
        vm->sir = 0;
        vm->sor = 0;

        vm->pir = 0;
        vm->por = 0;

        vm->pc = 0;
        vm->o = 0;
        vm->r = 0;

        vm->c = 0; /* Clear page registers */
        vm->d = 0;
        vm->l = 0;

        vm->g = 0; /* Clear accumulator registers */
        vm->i = 0;
}


uchar
myth_fetch(struct myth_vm *vm, int mode) /*Fetch next byte in CODE stream, increment PC*/
{
        uchar val = vm->pagebyte[ vm->c][ vm->pc];
        if ( mode)
                print( "Fetch instruction @%.02X_%.02X@ is %s(%.02Xh)\n",
                        vm->c, vm->pc, opcToMnemonic(val), val );
        else 
                print( "Fetch literal @%.02X_%.02X@ is %.02Xh(%d)(%b)\n",
                        vm->c, vm->pc, val, val, val );
        (vm->pc)++;
        return val;
}


void
myth_cycle(struct myth_vm *vm)
{

        uchar opcode = myth_fetch(vm, 1);

                /*Decode priority encoded opcode*/
                /*Execute decoded instruction*/

                if (opcode&0x80) myth_exec_pair(vm, opcode);
                else if (opcode&0x40) myth_exec_gput(vm, opcode);
                else if (opcode&0x20) myth_exec_trap(vm, opcode);
                else if (opcode&0x10) myth_exec_alu(vm, opcode);
                else if (opcode&0x08) myth_exec_adj(vm, opcode);
                else myth_exec_sys(vm, opcode);
}


void
myth_sip(struct myth_vm *vm) /* Save Instruction Pointer */
{
        vm->o = vm->pc;
        vm->d = vm->c;
}

void
myth_call(struct myth_vm *vm, uchar dstpage)
{
        print( " @%.02X_%.02X@ to @%.02X_%.02X@\n",
         vm->c, vm->pc, dstpage);

        myth_sip(vm);
        vm->pc = 0;
        vm->c = dstpage;
        vm->l -= 1;
}


uchar
myth_exec_pair_srcval(struct myth_vm *vm, uchar srcreg)
{
        switch(srcreg){
                case Nx: return myth_fetch(vm, 0); /*pseudo reg*/
                case Mx: return vm->pagebyte[ vm->d][ vm->o]; /*pseudo reg*/
                case Lx: return vm->pagebyte[ vm->l][ vm->o]; /*pseudo reg*/
                case Gx: return vm->g;
                case Rx: return vm->r;
                case Ix: return vm->i;
                case Sx: return vm->sir;
                default /*Px*/: return vm->pir;
        }
}


void
myth_exec_pair(struct myth_vm *vm, uchar opcode)
{
                uchar src = (opcode >> 4) & 7; /*Zero except bits 4-6 at LSB*/
                uchar dst = opcode & 15; /* Zero except bits 0-3 at LSB*/

                /* SCROUNGING
                   Remap ("scrounge") opcodes to other instructions
                   NL => O1A
                   NM => O1S
                   LL => NOP (reserved)
                   LM => NOP (reserved)
                   ML => NOP (reserved)
                   MM => NOP (reserved)
                   GG => NOP (reserved)
                   RR => NOP (reserved)
                   II => NOP (reserved)
                    */

                if(src==Nx)
                        switch(dst){
                                case xL: vm->o = vm->o + 1; return; /*O1A*/
                                case xM: vm->o = vm->o - 1; return; /*O1S*/
                        }

                if( (src==Mx || src==Lx) && (dst==xM || dst==xL) )
                        return; /*NOP (reserved)*/

        uchar srcval = myth_exec_pair_srcval(vm, src);
        switch(dst){
                case xO: vm->o = srcval; break;
                case xM: /*pseudo reg*/
                        vm->pagebyte[ vm->d][ vm->o] = srcval;
                        break;
                case xL: /*pseudo reg*/
                        vm->pagebyte[ vm->l][ vm->o] = srcval;
                        break;
                case xG: vm->g = srcval; break;
                case xR: vm->r = srcval; break;
                case xI: vm->i = srcval; break;
                case xS: vm->sor = srcval; break;
                case xP: vm->por = srcval; break;

                case xE: vm->e = srcval; break;
                case xA: vm->g = srcval + vm->i; break;
                case xD: vm->d = srcval; break;
                case xJ: /*pseudo reg*/
                        vm->pc = srcval;
                        break;
                case xW: /*pseudo reg*/
                        if (vm->i) vm->pc = srcval;
                        (vm->i)--; /*Post decrement, either case!*/
                        break; 
                case xT: /*pseudo reg*/
                        if (vm->r) vm->pc = srcval;
                        break;
                case xF: /*pseudo reg*/
                        if (!vm->r) vm->pc = srcval;
                        break;
                case xC:
                        print( "Page CALL");
                        myth_call(vm, srcval); break; /*pseudo reg*/
        }
}


void
myth_exec_gput(struct myth_vm *vm, uchar opcode) /*Execute GETPUT instruction*/
{
        /* OPCODE
            BITS 0-2 encode byte address offset in local page (from F8)
            BIT 3 encodes GET/PUT mode
            BITS 4-5 encode register index (RODG)
        */

        #define BIT3 8

        uchar *mptr;
        uchar offs = opcode & 7; /*Zero except low order 3 bits*/
        
        mptr = &(vm->pagebyte[vm->l][0xF8 + offs]);
        if( opcode & BIT3)
                switch( (opcode>>4) & 3){ /*Zero except bits 4-5 at LSB*/
                        case 0: *mptr = vm->r; break;
                        case 1: *mptr = vm->o; break;
                        case 2: *mptr = vm->d; break;
                        case 3: *mptr = vm->g; break;
                }
        else
                switch( (opcode>>4) & 3){ /*Zero except bits 4-5 at LSB*/
                        case 0: vm->r = *mptr; break;
                        case 1: vm->o = *mptr; break;
                        case 2: vm->d = *mptr; break;
                        case 3: vm->g = *mptr; break;
                }
}

void
myth_exec_trap(struct myth_vm *vm, uchar opcode)
{
        uchar dstpage = opcode & 31; /*Zero except low order 5 bits*/
        print( "TRAP");
        myth_call(vm, dstpage);
}


void
myth_exec_alu(struct myth_vm *vm, uchar opcode)
{
        switch(opcode & 15){/* Zero except low order 4 bits*/
                case IDR: vm->r = vm->r; break;
                case IDO: vm->r = vm->o; break;
                case OCR: vm->r = ~vm->r; break;
                case OCO: vm->r = ~vm->o; break;
                case SLR: vm->r = vm->r << 1; break;
                case SLO: vm->r = vm->o << 1; break;
                case SRR: vm->r = vm->r >> 1; break;
                case SRO: vm->r = vm->o >> 1; break;
                case AND: vm->r = vm->r & vm->o; break;
                case IOR: vm->r = vm->r | vm->o; break;
                case EOR: vm->r = vm->r ^ vm->o; break;
                case ADD: vm->r = vm->r + vm->o; break;
                case CAR:
                        vm->r = (int) vm->r + (int) vm->o > 255 ? 1 : 0;
                        break;
                case RLO: vm->r = (vm->r < vm->o) ? 255 : 0; break;
                case REO: vm->r = (vm->r == vm->o) ? 255 : 0; break;
                case RGO: vm->r = (vm->r > vm->o) ? 255 : 0; break;
        }
}


void /*Adjust R by sign-extended offset*/
myth_exec_adj(struct myth_vm *vm, uchar opcode)
{
        switch(opcode & 7){ /*Zero except low order 3 bits*/
                case R4A: vm->r += 4; break;
                case R1A: vm->r += 1; break;
                case R2A: vm->r += 2; break;
                case R3A: vm->r += 3; break;
                case R4S: vm->r -= 4; break;
                case R3S: vm->r -= 3; break;
                case R2S: vm->r -= 2; break;
                case R1S: vm->r -= 1; break;
        }
}


void
myth_exec_sys(struct myth_vm *vm, uchar opcode)
{
        switch(opcode & 7){ /*Zero except low order 3 bits*/
                
                case NOP: break;
                case SSI:
                        /*Clocks in MISO line bit into LSB*/
                        vm->sir = ((vm->sir)<<1) + vm->miso;
                        break;
                case SSO:
                        /*Clocks out MSB first*/
                        vm->mosi = (vm->sor)&0x80 ? 1:0;
                        vm->sor <<= 1;
                        break;
                case SCL: vm->sclk = 0; break;
                case SCH: vm->sclk = 1; break;
                
                case RET: 
                        print( "RET ");
                        vm->l += 1; /*FALL THROUGH*/
                case FAR:
                        print( "FAR @%.02X_%.02X@ to @%.02X_%.02X@\n",
                        vm->c, vm->pc, vm->d, vm->o);
                        vm->c = vm->d;
                        vm->pc = vm->o;
                        break;
                
                case SIP: myth_sip(vm); break;
        }
}


