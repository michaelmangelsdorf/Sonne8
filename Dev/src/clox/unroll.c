

/* Unroll the Myth emulator (myth.h) function per opcode,
   for various purposes.

   Sonne 8 micro-controller Rev. Myth/LOX Project
   Author: mim@ok-schalter.de (Michael/Dosflange@github)
    */

#include <u.h>
#include <libc.h>

struct myth_vm /*Complete machine state including all ram*/
{
        uchar ram[256][256]; /*MemoryByte[page][offset]*/

        uchar e_old; /*Device ENABLE register previous value - set by VM */
        uchar e_new; /*Device ENABLE register current value - set by VM */

        uchar irq;  /*Interrupt request bit - set by PERIPHERY*/
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

        uchar co;   /*Coroutine page index*/
        uchar c;    /*Code page index*/
        uchar g;    /*Global page index*/
        uchar l;    /*Local page index*/

        uchar scrounge; /*Set by VM if scrounge opcode executed, else zero*/
};

void myth_reset(struct myth_vm *vm);
void myth_step(struct myth_vm *vm, uchar opcode);

static char* srcval(struct myth_vm *vm, uchar srcreg);
static char*  scrounge(uchar opcode);
static void pair(struct myth_vm *vm, uchar opcode);
static void giro(struct myth_vm *vm, uchar opcode);
static void trap(struct myth_vm *vm, uchar opcode);
static void alu(struct myth_vm *vm, uchar opcode);
static void fix(struct myth_vm *vm, uchar opcode);
static void sys(struct myth_vm *vm, uchar opcode);


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
#define COR 6 /*Coroutine jump to D:I*/
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

        vm->e_old = 0; /*Clear signal edges*/
        vm->e_new = 0; /*Deselect any device*/

        vm->irq = 0;
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
        vm->g = 0;
        vm->l = 0;

        vm->scrounge = 0;
}


/* Single-step 1 instruction cycle
*/

void
myth_exec(struct myth_vm *vm, uchar opcode)
{

        /*Decode priority encoded opcode*/
        /*Execute decoded instruction*/

        if (opcode&0x80) pair(vm, opcode);
        else if (opcode&0x40) giro(vm, opcode);
        else if (opcode&0x20) trap(vm, opcode);
        else if (opcode&0x10) alu(vm, opcode);
        else if (opcode&0x08) fix(vm, opcode);
        else sys(vm, opcode);
}


/* TRAP has identical operation, but with an
   immediate destination page Offset
*/


void
trap(struct myth_vm *vm, uchar opcode)
{
        uchar dstpage = opcode & 31; /*Zero except low order 5 bits*/
        print("TRAP_%d: I = PC; CO = C; L--; PC = 0; C = %d; goto EXEC;\n", dstpage, dstpage);
}


/* First part of handling PAIR instructions,
   derives the source value
*/

char*
srcval(struct myth_vm *vm, uchar srcreg)
{
        switch(srcreg){
                case Nx: return "RAM[C][PC++]";
                case Mx: return "RAM[G][O]";
                case Lx: return "RAM[L][O]";
                case Gx: return "G";
                case Rx: return "R";
                case Ix: return "I";
                case Sx: return "SIR";
           default /*Px*/: return "PIR";
        }
}


/* SCROUNGING of certain PAIR instructions:
Remap undesirable opcodes to other,
application specific opcodes or CPU extensions etc.
Don't assume that these are generally NOPs!
*/

char*
scrounge(uchar opcode)
{
        switch(opcode & 0x7F /*0111_1111 zero b7*/){
                case 16*Nx + xM: return "SCROUNGE_NM: goto NEXT;\n";
                case 16*Nx + xL: return "SCROUNGE NL: goto NEXT;\n";
                case 16*Mx + xM: return "SCROUNGE MM: goto NEXT;\n";
                case 16*Mx + xL: return "SCROUNGE ML: goto NEXT;\n";
                case 16*Lx + xM: return "SCROUNGE LM: goto NEXT;\n";
                case 16*Lx + xL: return "SCROUNGE LL: goto NEXT;\n";
                case 16*Gx + xG: return "SCROUNGE GG: goto NEXT;\n";
                case 16*Rx + xR: return "SCROUNGE RR: goto NEXT;\n";
                case 16*Ix + xI: return "SCROUNGE II: goto NEXT;\n";
        }
        return NULL;
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
                 print(scrounge(opcode));
                 return;
        }

        char str[8] = "NMLGRISP";
        char* v = srcval(vm, src);

        switch(dst){
                case xO:
                        print("PAIR_%cO: O = %s; goto NEXT;\n", str[src], v);
                        break;
                case xM:
                        print("PAIR_%cM: RAM[G][O] = %s; goto NEXT;\n", str[src], v);
                        break;
                case xL:
                        print("PAIR_%cL: RAM[L][O] = %s; goto NEXT;\n", str[src], v);
                        break;
                case xG:
                        print("PAIR_%cG: G = %s; goto NEXT;\n", str[src], v);
                        break;
                case xR:
                        print("PAIR_%cR: R = %s; goto NEXT;\n", str[src], v);
                        break;
                case xI:
                        print("PAIR_%cI: I = %s; goto NEXT;\n", str[src], v);
                        break;
                case xS:
                        print("PAIR_%cS: SOR = %s; goto NEXT;\n", str[src], v);
                        break;

                case xP:
                        print("PAIR_%cP: POR = %s; goto NEXT;\n", str[src], v);
                        break;
                case xE:
                        print("PAIR_%cE: E = %s; goto NEXT;\n", str[src], v);
                        break;
                case xA:
                        print("PAIR_%cA: (int)TEMP = O + %s; o = (uchar)(TEMP & 0xFF); if (TEMP>0xFF) G++; goto NEXT;\n", str[src], v);
                        break;
                case xB:
                        print("PAIR_%cB: PC += %s; goto EXEC;\n", str[src], v);
                        break;
                case xJ:
                        print("PAIR_%cJ: PC = %s; goto EXEC;\n", str[src], v);
                        break;
                case xW:
                        print("PAIR_%cW: if (I--) PC = %s; goto EXEC; else goto NEXT;\n", str[src], v);
                        break; 
                case xT:
                        print("PAIR_%cT: if (R) PC = %s; goto EXEC; else goto NEXT;\n", str[src], v);
                        break;
                case xF:
                        print("PAIR_%cF: if (!R) PC = %s; goto EXEC; else goto NEXT;\n", str[src], v);
                        break;
                case xC:
                        print("PAIR_%cC: I = PC; CO = C; L--; PC = 0; C = %s; goto EXEC;\n", str[src], v);
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

       // uchar index = BITS02;              
       // uchar *mptr = &(vm->ram[vm->l][GIRO_BASE_OFFSET + index]);

        if(opcode & BIT3)
                switch(BITS45){
                        case 0:
                                print("GIRO_G%d: RAM[L][GIRO+%d] = G; goto NEXT;\n", BITS02,BITS02);
                                 break;
                        case 1:
                                 print("GIRO_I%d: RAM[L][GIRO+%d] = I; goto NEXT\n", BITS02,BITS02);
                                 break;
                        case 2:
                                 print("GIRO_R%d: RAM[L][GIRO+%d] = R; goto NEXT;\n", BITS02,BITS02);
                                 break;
                        case 3:
                                 print("GIRO_O%d: RAM[L][GIRO+%d] = O; goto NEXT;\n", BITS02,BITS02);
                                 break;
                }
        else
        switch(BITS45){
                case 0:
                         print("GIRO_%dG: G = RAM[L][GIRO+%d]; goto NEXT;\n", BITS02,BITS02);
                         break;
                case 1:
                         print("GIRO_%dI: I = RAM[L][GIRO+%d]; goto NEXT;\n", BITS02,BITS02);
                         break;
                case 2:
                         print("GIRO_%dR: R = RAM[L][GIRO+%d]; goto NEXT;\n", BITS02,BITS02);
                         break;
                case 3:
                         print("GIRO_%dO: O = RAM[L][GIRO+%d]; goto NEXT;\n", BITS02,BITS02);
                         break;
        }
}


void
alu(struct myth_vm *vm, uchar opcode)
{
        switch(opcode & 15){ /* Zero except low order 4 bits*/
                case CLR:
                        print("ALU_CLR: R = 0; goto NEXT;\n");
                        break;
                case IDO:
                        print("ALU_IDO: R = O; goto NEXT;\n");
                        break;
                case OCR:
                        print("ALU_OCR: R = ~R; goto NEXT;\n");
                        break;
                case OCO:
                        print("ALU_OCO: R = ~O; goto NEXT;\n");
                        break;
                case SLR:
                        print("ALU_SLR: R <<= 1; goto NEXT;\n");
                        break;
                case SLO:
                        print("ALU_SLO: R = O << 1; goto NEXT;\n");
                        break;
                case SRR:
                        print("ALU_SRR: R >>= 1; goto NEXT;\n");
                        break;
                case SRO:
                        print("ALU_SRO: R = O >> 1; goto NEXT;\n");
                        break;
                case AND:
                        print("ALU_AND: R &= O; goto NEXT;\n");
                        break;
                case IOR:
                        print("ALU_IOR: R |= O; goto NEXT;\n");
                        break;
                case EOR:
                        print("ALU_EOR: R ^= O; goto NEXT;\n");
                        break;
                case ADD:
                        print("ALU_ADD: R += O; goto NEXT;\n");
                        break;
                case CAR:
                        print("ALU_CAR: R = (uint) R + (uint) O > 255 ? 1 : 0; goto NEXT;\n");
                        break;
                case RLO:
                        print ("ALU_RLO: R = (R < O) ? 255 : 0; goto NEXT;\n");
                        break;
                case REO:
                        print ("ALU_REO: R = (R == O) ? 255 : 0; goto NEXT;\n");
                        break;
                case RGO:
                        print ("ALU_RGO: R = (R > O) ? 255 : 0; goto NEXT;\n");
                        break;
        }
}


void /*Add sign-extended number to R*/
fix(struct myth_vm *vm, uchar opcode)
{
        switch(opcode & 7){ /*Zero except low order 3 bits*/
                case P4: print("FIX_P4: R += 4; goto NEXT;\n");
                        break;
                case P1: print("FIX_P1: R += 1; goto NEXT;\n");
                        break;
                case P2: print("FIX_P2: R += 2; goto NEXT;\n");
                        break;
                case P3: print("FIX_P3: R += 3; goto NEXT;\n");
                        break;
                case M4: print("FIX_M4: R -= 4; goto NEXT;\n");
                        break;
                case M3: print("FIX_M3: R -= 3; goto NEXT;\n");
                        break;
                case M2: print("FIX_M2: R -= 2; goto NEXT;\n");
                        break;
                case M1: print("FIX_M1: R -= 1; goto NEXT;\n");
                        break;
        }
}


void
sys(struct myth_vm *vm, uchar opcode)
{
        switch(opcode & 7){ /*Zero except low order 3 bits*/
                
                case NOP: print("SYS_NOP: goto NEXT;\n");
                          break;
                case SSI:
                        /*Clocks in MISO line bit into LSB*/
                        print("SYS_SSI: SIR = SIR << 1 + MISO; goto NEXT;\n");
                        break;
                case SSO:
                        /*Clocks out MSB first*/
                        print("SYS_SSO: MOSI = SOR & 0x80 ? 1 : 0; SOR <<= 1; goto NEXT;\n");
                        break;
                case SCL:
                        print("SYS_SCL: SCLK = 0; goto NEXT;\n");
                        break;

                case SCH:
                        print("SYS_SCH: SCLK = 1; goto NEXT;\n");
                        break;

                case RET:
                        print("SYS_RET: C = RAM[L][GIRO+7]; PC = I; L++; goto NEXT;\n");
                        break;

                case COR:
                        print("SYS_COR: C = G; PC = I; goto NEXT;\n");
                        break;

                case OWN:
                        print("SYS_OWN: ram[L][GIRO+7] = CO; goto NEXT;\n");
                        break;
        }
}


void
main()
{
        struct myth_vm vm;
        for (int i=0; i<=255; i++){
                if (i==0) print("\n/* SYS */\n");
                else if (i==8) print("\n/* FIX */\n");
                else if (i==16) print("\n/* ALU */\n");
                else if (i==32) print("\n/* TRAP */\n");
                else if (i==64) print("\n/* GIRO */\n");
                else if (i==128) print("\n/* PAIR */\n");
                myth_exec(&vm, i);
        }
}




