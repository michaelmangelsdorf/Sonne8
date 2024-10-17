
#ifndef __MYTH_H__
#define __MYTH_H__ 1


/* Emulation routines for Sonne 8 micro-controller Rev. Myth/LOX
   Author: mim@ok-schalter.de (Michael/Dosflange@github)
    */

#include <u.h>
#include <libc.h>

struct myth_vm /*Complete machine state including all RAM*/
{
        uchar ram[256][256]; /*MemoryByte[page][offset]*/

        uchar irq;  /*Interrupt request bit - set by PERIPHERY*/

        uchar e_old; /*Device ENABLE register previous value - set by VM */
        uchar e_new; /*Device ENABLE register current value - set by VM */

        uchar sclk; /*Serial clock state bit - set by VM*/
        uchar miso; /*Serial input line state bit - set by PERIPHERY*/
        uchar mosi; /*Serial output line state bit - set by VM*/
        uchar sir;  /*Serial input register - set by VM*/
        uchar sor;  /*Serial output register - set by VM*/

        uchar pir;  /*Parallel input register - set by PERIPHERY*/
        uchar por;  /*Parallel output register - set by VM*/

        uchar r;    /*Result*/
        uchar o;    /*Offset*/

        uchar i;    /*Inner Counter*/
        uchar pc;   /*Program Counter*/

        uchar d;    /*Dupe latch*/
        uchar c;    /*Code page index*/
        uchar g;    /*Global page index*/
        uchar l;    /*Local page index*/

        uchar scrounge; /*Set by VM if scrounge opcode executed, else zero*/
};

/*
  RAM[][] is organised as [page][offset];
  
  E is the enable register. The low-order nybble is called
  the low-order selector, and it encodes device select
  signals LS0-15. Device LS0 is the null device.
  The high-order nybble is called the high-order selector,
  and it encodes device select signals HS0-15.
  Device HS0 is the null device.

  SCLK is the CPU driven serial clock state bit.
  MISO is the device driven serial data input bit.
  MOSI is the CPU driven serial data output bit.
  SIR is the serialisation byte input register.
  SOR is the serialisation byte output register.
  PIR is the parallel bus input register.
  POR is the parallel bus output register.
  
  R is the result register receiving ALU results and
  this register is also one of the two ALU input operands.
  
  O is the implied offset register for all memory operations
  outside of instruction or literal fetching, and this
  register is also one of the two ALU input operands.

  I is the inner counter register. It can function as
  a hardware loop counter but is also used for storing
  the return address during function calls.

  PC is the program counter, which for this CPU means
  that it contains the offset within the current code page.
  This register is hidden.

  D is the "dupe" register. It contains a copy of the
  code page index that is strategically updated.
  This register is hidden.

  C contains the code page index. Instruction fetch always
  occurs at RAM[C][PC].

  G is the global page index register.
  It contains the implied
  page index for memory operations using the M-prefix.

  L is the local page index register.
  It contains the implied
  page index for memory operations using the L-prefix.
  This page index represents the stack frame during
  function calls. This register is hidden.
*/


void myth_reset(struct myth_vm *vm);
void myth_step(struct myth_vm *vm);

static uchar fetch(struct myth_vm *vm);
static uchar srcval(struct myth_vm *vm, uchar srcreg);
static int scrounge(uchar opcode);
static void pair(struct myth_vm *vm, uchar opcode);
static void giro(struct myth_vm *vm, uchar opcode);
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
#define Gx 3 /*from GLOBAL register*/
#define Rx 4 /*from RESULT register*/
#define Ix 5 /*from DATA page register*/
#define Sx 6 /*from SERIAL input*/
#define Px 7 /*from PARALLEL input*/


/* The 'xREG' notation means: (something) into REG
   i.e. REG is a destination
*/

#define xO 0 /*to OFFSET register*/
#define xM 1 /*to MEMORY via DATA page index*/
#define xL 2 /*to MEMORY via LOCAL page index*/
#define xG 3 /*to GLOBAL register*/
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
#define RET 5 /*Return to L7:I from nested call*/
#define COR 6 /*Coroutine jump to G:I*/
#define OWN 7 /*Save code page index in L7*/


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

#define GIRO_BASE_OFFSET 0xF8 /*Local-page offset used by DIRO instructions*/

void
myth_reset(struct myth_vm *vm) /*Initialise machine state*/
{
        memset(vm->ram, 0, 256*256);

        vm->irq = 0;

        vm->e_old = 0; /*Clear signal edges*/
        vm->e_new = 0; /*Deselect all devices*/

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

        vm->d = 0;
        vm->c = 0;
        vm->g = 0;
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

        if (vm->irq && (vm->c < 32)) trap(vm, 32); /*to page zero if FREE*/
        else
        if (opcode&0x80) pair(vm, opcode);
        else if (opcode&0x40) giro(vm, opcode);
        else if (opcode&0x20) trap(vm, opcode);
        else if (opcode&0x10) alu(vm, opcode);
        else if (opcode&0x08) fix(vm, opcode);
        else sys(vm, opcode);
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


/* TRAP has identical operation, but with an
   immediate destination page Offset
*/

void
call(struct myth_vm *vm, uchar dstpage)
{
        /*Save origin*/
        vm->i = vm->pc;
        vm->d = vm->c;

        /*Create stack frame*/
        vm->l--;

        /*Branch to page head*/
        vm->pc = 0;
        vm->c = dstpage;
}


void
trap(struct myth_vm *vm, uchar opcode)
{
        uchar dstpage = opcode & 31; /*Zero except low order 5 bits*/
        call(vm, dstpage);
}


/* First part of handling PAIR instructions,
   derives the source value
*/

uchar
srcval(struct myth_vm *vm, uchar srcreg)
{
        switch(srcreg){
                case Nx: return fetch(vm); /*pseudo reg*/
                case Mx: return vm->ram[ vm->g][ vm->o]; /*pseudo reg*/
                case Lx: return vm->ram[ vm->l][ vm->o]; /*pseudo reg*/
                case Gx: return vm->g;
                case Rx: return vm->r;
                case Ix: return vm->i;
                case Sx: return vm->sir;
           default /*Px*/: return vm->pir;
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
                case 16*Nx + xM: return opcode; /*NM => NOP (reserved)*/
                case 16*Nx + xL: return opcode; /*NL => NOP (reserved)*/
                case 16*Mx + xM: return opcode; /*MM => NOP (reserved)*/
                case 16*Mx + xL: return opcode; /*ML => NOP (reserved)*/
                case 16*Lx + xM: return opcode; /*LM => NOP (reserved)*/
                case 16*Lx + xL: return opcode; /*LL => NOP (reserved)*/
                case 16*Gx + xG: return opcode; /*GG => NOP (reserved)*/
                case 16*Rx + xR: return opcode; /*RR => NOP (reserved)*/
                case 16*Ix + xI: return opcode; /*II => NOP (reserved)*/
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

        uchar v = srcval(vm, src);
        int temp;
        switch(dst){
                case xO: vm->o = v; break;
                case xM: vm->ram[ vm->g][ vm->o] = v; break;
                case xL: vm->ram[ vm->l][ vm->o] = v; break;
                case xG: vm->g = v; break;
                case xR: vm->r = v; break;
                case xI: vm->i = v; break;
                case xS: vm->sor = v; break;
                case xP: vm->por = v; break;
                case xE: vm->e_old = vm->e_new;
                         vm->e_new = v;
                         break;
                case xA:
                        temp = vm->o + v;
                        vm->o = (uchar) (temp & 0xFF);
                        if (temp>255) vm->g += 1;
                        break;
                case xB: vm->pc = vm->pc + v; break;
                case xJ: vm->pc = v; break;
                case xW:
                        if (vm->i) vm->pc = v;
                        (vm->i)--; /*Post decrement, either case!*/
                        break; 
                case xT: if (vm->r) vm->pc = v; break;
                case xF: if (!vm->r) vm->pc = v; break;
                case xC: call(vm, v); break;
        }
}

void
giro(struct myth_vm *vm, uchar opcode) /*Execute GIRO instruction*/
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
        uchar *mptr = &(vm->ram[vm->l][GIRO_BASE_OFFSET + index]);

        if(opcode & BIT3)
                switch(BITS45){
                        case 0: *mptr = vm->g; break;
                        case 1: *mptr = vm->i; break;
                        case 2: *mptr = vm->r; break;
                        case 3: *mptr = vm->o; break;
                }
        else
        switch(BITS45){
                case 0: vm->g = *mptr; break;
                case 1: vm->i = *mptr; break;
                case 2: vm->r = *mptr; break;
                case 3: vm->o = *mptr; break;
        }
}


void
alu(struct myth_vm *vm, uchar opcode)
{
        switch(opcode & 15){/* Zero except low order 4 bits*/
                case CLR: vm->r = 0; break;
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
                        vm->r = (uint) vm->r + (uint) vm->o > 255 ? 1 : 0;
                        break;
                case RLO: vm->r = (vm->r < vm->o) ? 255 : 0; break;
                case REO: vm->r = (vm->r == vm->o) ? 255 : 0; break;
                case RGO: vm->r = (vm->r > vm->o) ? 255 : 0; break;
        }
}


void /*Add sign-extended number to R*/
fix(struct myth_vm *vm, uchar opcode)
{
        switch(opcode & 7){ /*Zero except low order 3 bits*/
                case P4: vm->r += 4; break;
                case P1: vm->r += 1; break;
                case P2: vm->r += 2; break;
                case P3: vm->r += 3; break;
                case M4: vm->r -= 4; break;
                case M3: vm->r -= 3; break;
                case M2: vm->r -= 2; break;
                case M1: vm->r -= 1; break;
        }
}


void
sys(struct myth_vm *vm, uchar opcode)
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

                #define L7 (vm->ram[vm->l][GIRO_BASE_OFFSET +7])

                case RET:
                        vm->c = L7;
                        vm->pc = vm->i;
                        vm->l++;
                        break;

                case COR:
                        vm->c = vm->r;
                        vm->pc = vm->i;
                        break;

                case OWN: L7 = vm->d; break;
        }
}

#endif
