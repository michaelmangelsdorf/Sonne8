
/* Sonne Microcontroller rev. Myth
   Virtual Machine
  Jan-2024 Michael Mangelsdorf
  Copyr. <mim@ok-schalter.de>
*/

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define LHS_N 0 /*Number - Fetch next opcode as literal*/
#define LHS_M 1 /*Memory - Load from implicit memory address (xMx)*/
#define LHS_R 2 /*Result*/
#define LHS_W 3 /*Working*/
#define LHS_D 4 /*Data*/
#define LHS_L 5 /*Local*/
#define LHS_S 6 /*Serial*/
#define LHS_P 7 /*Parallel*/

#define RHS_N 0 /*Number - Set loop counter*/
#define RHS_M 1 /*Memory - Store into implicit memory address (xMx)*/
#define RHS_R 2 /*Result*/
#define RHS_W 3 /*Working*/
#define RHS_D 4 /*Data*/
#define RHS_L 5 /*Local*/
#define RHS_S 6 /*Serial*/
#define RHS_P 7 /*Parallel*/
#define RHS_E 8 /*Enable - Device enable bits*/
#define RHS_J 9 /*Jump - Unconditional branch*/
#define RHS_I 10 /*Iterate - Branch if loop counter not zero*/
#define RHS_T 11 /*True - Branch if R+O not zero*/
#define RHS_F 12 /*False - Branch if R+0 zero*/
#define RHS_C 13 /*Call - Call subroutine*/
#define RHS_A 14 /*B*/
#define RHS_B 15 /*A*/

#define ALU_IDA 0 /*Identity A*/
#define ALU_IDB 1 /*Identity B*/
#define ALU_OCA 2 /*One's complement A*/
#define ALU_OCB 3 /*One's complement B*/
#define ALU_SLA 4 /*Shift left A*/
#define ALU_SLB 5 /*Shift left B*/
#define ALU_SRA 6 /*Shift right A*/
#define ALU_SRB 7 /*Shift right B*/
#define ALU_AND 8 /*A AND B*/
#define ALU_IOR 9 /*A OR B*/
#define ALU_EOR 10 /*A XOR B*/
#define ALU_ADD 11 /*A + B*/
#define ALU_CAR 12 /*Carry of A + B (0 or 1)*/
#define ALU_ALB 13 /*255 if A<B else 0*/
#define ALU_AEB 14 /*255 if A=B else 0*/
#define ALU_AGB 15 /*255 if A>B else 0*/

#define SYS_NOP 0 /*No operation*/
#define SYS_SSI 1 /*Serial Shift In*/
#define SYS_SSO 2 /*Serial Shift Out*/
#define SYS_SCL 3 /*Set serial Clock Low*/
#define SYS_SCH 4 /*Set serial Clock High*/
#define SYS_RDY 5 /*Ready/Tristate*/
#define SYS_NEW 6 /*Create stack frame*/
#define SYS_OLD 7 /*Resume stack frame*/

#define SCR_REA 1 /*Restore A*/
#define SCR_REB 2 /*Restore B*/
#define SCR_RER 3 /*Restore R*/

#define GLOBAL_PAGE 255

uint8_t quitf, loggingf;
char msg[81], tempStr80[81];

struct {
    uint8_t A, B, R, R_bias;
    uint8_t cache_A, cache_B, cache_R;
} acc;

struct {
    uint8_t pc;
    uint8_t I;
} aux;


struct {
    uint16_t xMx;
    uint8_t C, D, L;
    uint8_t W;
    uint8_t data[256*256];
} mem;

struct {
    uint8_t ser_ir; // 74HC595 shift register
    uint8_t ser_or; // 74HC165 shift register
    uint8_t par_ir, par_or;
    uint8_t clock, ready;
    uint8_t E;
} com;

char* binStr(unsigned char byte) {
    int i, pos=0;
    for (i = 7; i >= 0; i--) {
        tempStr80[pos++] = ((byte >> i) & 1) ? '1' : '0';
        if (pos==4) {
            tempStr80[pos] = '_';
            pos++;
        }
    }
    tempStr80[9] = '\0';
    return tempStr80;
}

uint8_t
get_R() { return acc.R + acc.R_bias; }

void
set_R( uint8_t val)
{
    acc.cache_R = get_R();
    acc.R = val;
    acc.R_bias = 0;
}

void
set_A( uint8_t val)
{
    acc.cache_A = acc.A;
    acc.A = val;
    mem.xMx = 256*mem.D + acc.A;
}

void
set_B( uint8_t val)
{
    acc.cache_A = acc.B;
    acc.B = val;
    mem.xMx = 256*mem.D + acc.B;;
}

void
set_W( uint8_t val)
{
    mem.W = val;
    mem.xMx = 256*mem.D + mem.W;
}

void
reset()
{
    quitf = 0;
    com.ready = 0;
    aux.pc = 0;
    mem.C = mem.D = 0;
    mem.L = GLOBAL_PAGE;
}

void
shift_in()
{
    uint8_t bit = 1; // Physical bit in
    com.ser_ir <<= 1; // MSB in first
    com.ser_ir |= bit;
}

void
shift_out()
{
    uint8_t bit = (com.ser_or & 128) >> 7; // MSB out first
    com.ser_or <<= 1;
}

void
aluRun( uint8_t func)
{
    switch(func & 15)
    {
        case ALU_IDA: set_R( acc.A ); break;
        case ALU_IDB: set_R( acc.B ); break;
        case ALU_OCA: set_R( ~acc.A ); break;
        case ALU_OCB: set_R( ~acc.B ); break;
        case ALU_SLA: set_R( acc.A << 1 ); break;
        case ALU_SLB: set_R( acc.B << 1 ); break;
        case ALU_SRA: set_R( acc.A >> 1 ); break;
        case ALU_SRB: set_R( acc.B >> 1 ); break;
        case ALU_AND: set_R( acc.A & acc.B ); break;
        case ALU_IOR: set_R( acc.A | acc.B ); break;
        case ALU_EOR: set_R( acc.A ^ acc.B ); break;
        case ALU_ADD: set_R( acc.A + acc.B ); break;
        case ALU_CAR: set_R( (int) acc.A + (int) acc.B > 255 ? 1 : 0 ); break;
        case ALU_ALB: set_R( (acc.A < acc.B) ? 255 : 0 ); break;
        case ALU_AEB: set_R( (acc.A == acc.B) ? 255 : 0 ); break;
        case ALU_AGB: set_R( (acc.A > acc.B) ? 255 : 0 ); break;
    }
}

void
dosys( uint8_t op)
{
      switch (op & 7)
      {
          case SYS_NOP: quitf = 1; break;
          case SYS_SSI: shift_in(); break;
          case SYS_SSO: shift_out(); break;
          case SYS_SCL: com.clock = 0; break;
          case SYS_SCH: com.clock = 1; break;
          case SYS_RDY: com.ready = 1; break;
          case SYS_NEW: mem.L -= 1; break;
          case SYS_OLD: mem.L += 1; break;
      }
}

void
exec_TRAP( uint8_t addr)
{
    set_W(aux.pc);
    aux.pc = 0;
    mem.D = mem.C;
    mem.C = addr;
}

uint8_t
fetch()
{
    uint16_t addr;
    addr = 256 * mem.C + aux.pc;
    aux.pc++;
    return mem.data[addr];
}

void
parallel_out(uint8_t val)
{
    com.par_or = val;
    com.ready=1;
    quitf = 1;
}

void
write_reg( uint8_t selector, uint8_t val)
{
    switch (selector & 15)
    {
        case RHS_N: aux.I = val; break;
        case RHS_M: mem.data[mem.xMx] = val; break;
        case RHS_R: set_R(val); break;
        case RHS_W: mem.W = val; break;
        case RHS_D: mem.D = val; break;
        case RHS_L: mem.L = val; break;
        case RHS_S: com.ser_or = val; break;
        case RHS_P: parallel_out(val); break;
        case RHS_E: com.E = val; break;
        case RHS_J: aux.pc = val; break;
        case RHS_I: aux.I--; if (aux.I) aux.pc = val; break;
        case RHS_T: if (get_R()) aux.pc = val; break;
        case RHS_F: if (!get_R()) aux.pc = val; break;
        case RHS_C: exec_TRAP(val); break;
        case RHS_A: set_A(val); break;
        case RHS_B: set_B(val); break;
    }
}

uint8_t
read_reg( uint8_t selector)
{
    uint8_t val;
    switch (selector & 7)
    {
        case LHS_N: return fetch();
        case LHS_M: val = mem.data[mem.xMx]; return val;
        case LHS_R: return get_R();
        case LHS_W: return mem.W;
        case LHS_D: return mem.D;
        case LHS_L: return mem.L;
        case LHS_S: com.ready = 1; return com.ser_ir; 
        case LHS_P: return com.par_ir;
        default: return 0; // Warning dummy
    }
}

void
exec_PAIR( uint8_t dst, uint8_t src)
{
    src &= 3;
    dst &= 15;
    if ((src == LHS_N) && (dst == RHS_M)) { // Scrounge RET
        aux.pc = mem.W;
        mem.C = mem.D;
    }
    else if (src!=0 && src==dst)
    switch (src) {
        case SCR_REA: set_A(acc.cache_A); break;
        case SCR_REB: set_B(acc.cache_B); break;
        case SCR_RER: set_R(acc.cache_R); break;
        default:;
    }
    else write_reg( dst, read_reg( src));
}


void
exec_GETPUT( uint8_t bits)
{
    uint8_t guide = bits & 3;
    uint8_t gp_offs = (bits >> 4) & 3;
    uint16_t addr;

    #define PUTBIT bits & 8
    #define LOCALBIT bits & 4

    // GETPUT locations located at end of page
    if (LOCALBIT) addr = 256*(mem.L+1) - 4 + gp_offs; // Local page
    else addr = 256*(GLOBAL_PAGE+1) - 4 + gp_offs; // Global page

    switch(guide) {
        case 0: if (PUTBIT) mem.data[addr] = acc.A;
                else set_A(mem.data[addr]);
                break;

        case 1: if (PUTBIT) mem.data[addr] = acc.B;
                else set_B(mem.data[addr]);
                break;

        case 2: if (PUTBIT) mem.data[addr] = get_R();
                else set_R(mem.data[addr]);
                break;

        case 3: if (PUTBIT) mem.data[addr] = mem.W;
                else set_W(mem.data[addr]);
                break;
    }
}

void
decode()
{
    uint8_t instr = fetch();
    uint8_t src = instr & 15;
    uint8_t dst = (instr >> 4) & 3;

    if (instr & 128) { exec_PAIR(src, dst); return; }

    if (instr & 64) { exec_GETPUT(instr & 63); return; }

    if (instr & 32) { exec_TRAP(instr & 31); return; }

    if (instr & 16) { aluRun(src); return; }

    if (instr & 8) { acc.R_bias = (src&7) < 4 ? (src&7) : (uint8_t) (0xF8 | (src&7)); return; }

    else dosys(instr & 7);
}

int
rdimg(char *fname)
{
    FILE *f;
    f = fopen(fname, "r");
    if (f == NULL) return -1;
    fread(mem.data, 1, 256*256, f);
    fclose(f);
    return 0;
}

int
wrimg(char *fname)
{
    FILE *f;
    f = fopen(fname, "rb+");
    if (f == NULL) return -1;
    fwrite(mem.data, 1, 256*256, f);
    fclose(f);
    return 0;
}

#define MAX_CYCLES 1000
int
main( int argc, char **argv)
{
    unsigned i;
    char *fname;
    reset();
    loggingf = 1;

    printf("\n/* Sonne Microcontroller rev. Myth\n");
    printf("   Virtual Machine");
    printf("  Jan-2024 Michael Mangelsdorf\n");
    printf("  Copyr. <mim@ok-schalter.de>\n");
    printf("*/\n\n");

    if (argc<2){
        printf("Missing work file name, using default\n");
        fname = "watking.obj";
        //exit(-1);
    } else fname = argv[1];
    
    if (rdimg(fname)){
       printf("File '%s' input error\n", argv[1]);
       exit(-1);
    }
    else for (i=0; i<MAX_CYCLES; i++){
        decode();
        if (quitf) break;
    }
    printf("Ax%02X Bx%02X Rx%02X\n", acc.A, acc.B, get_R());
    
    if (wrimg(fname)){
       printf("File '%s' output error\n", argv[1]);
       exit(-1);
    } else printf("Saved after %d cycles\n", i);
}



