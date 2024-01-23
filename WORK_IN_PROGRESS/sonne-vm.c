
/* Sonne CPU Virtual Machine
    rev. Schneelein
  Jan-2024 Michael Mangelsdorf
  Copyr. <mim@ok-schalter.de>
*/


#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define LHS_N 0 /*Number - Fetch next opcode as literal*/
#define LHS_M 1 /*Memory - Load implicit memory address (xMx)*/
#define LHS_R 2 /*Result - Receives ALU result*/
#define LHS_H 3 /*Hold - Holds high-order return address*/
#define LHS_G 4 /*Global - Global bank register*/
#define LHS_L 5 /*Local - Local bank register*/
#define LHS_S 6 /*Serial - Serial input register*/
#define LHS_P 7 /*Parallel - Parallel input register*/

#define RHS_I 0 /*Iteration - Set inner loop counter*/
#define RHS_M 1 /*Memory - Write to implicit memory address (xMx)*/
#define RHS_R 2 /*Result*/
#define RHS_H 3 /*Hold*/
#define RHS_G 4 /*Global*/
#define RHS_L 5 /*Local*/
#define RHS_S 6 /*Serial*/
#define RHS_P 7 /*Parallel*/
#define RHS_E 8 /*Enable - Device enable bits*/
#define RHS_J 9 /*Jump - Unconditional branch*/
#define RHS_D 10 /*Decrement - Branch if iteration register not zero*/
#define RHS_T 11 /*True - Branch if R+O not zero*/
#define RHS_F 12 /*False - Branch if R+0 zero*/
#define RHS_C 13 /*Call - Call subroutine*/
#define RHS_B 14 /*Beta - Accumulator register*/
#define RHS_A 15 /*Alpha - Accumulator register*/

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

#define KICK_NOP 0 /*No operation*/
#define KICK_SSI 1 /*Serial input-shift-in*/
#define KICK_SSO 2 /*Serial output-shift-out*/
#define KICK_SCL 3 /*Set serial clock low*/
#define KICK_SCH 4 /*Set serial clock high*/
#define KICK_RDY 5 /*Parallel output state*/
#define KICK_LLF 6 /*Leave local frame*/
#define KICK_ELF 7 /*Enter local frame*/
#define KICK_REA 8 /*Reuse CLIP value in A*/
#define KICK_REB 9 /*Reuse CLIP value in B*/


uint8_t quitf, loggingf;
char msg[81], tempStr80[81];

struct {
    uint8_t A, B;
    uint8_t clip;
} acc;

struct {
    uint8_t low, high;
} pc;

struct {
    uint16_t xMx;
    uint8_t H, G, L;
    uint8_t data[256*256];
} mem;

struct {
    uint8_t R, O, I;
} ALU;

struct {
    uint8_t ser_ir; // 74HC595 shift register
    uint8_t ser_or; // 74HC165 shift register
    uint8_t par_ir, par_or;
    uint8_t clock, ready;
    uint8_t E;
} com;

void
logmsg()
{
    printf("%s",msg);
}

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

void
set_R( uint8_t val)
{
    ALU.R = val;
    ALU.O = 0;
}

void
reset()
{
    quitf = 0;
    com.ready = 0;
    pc.low = 0;
    pc.high = 0;
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

uint8_t
get_R() { return ALU.R + ALU.O; }

void
aluRun( uint8_t func)
{
    switch(func & 15)
    {
        case ALU_IDA:
        set_R( acc.A );
        sprintf(msg, "IDA ");
        break;

        case ALU_IDB:
        set_R( acc.B );
        sprintf(msg, "IDB ");
        break;

        case ALU_OCA:
        set_R( ~acc.A );
        sprintf(msg, "OCA ");
        break;

        case ALU_OCB:
        set_R( ~acc.B );
        sprintf(msg, "OCB ");
        break;

        case ALU_SLA:
        set_R( acc.A << 1 );
        sprintf(msg, "SLA ");
        break;

        case ALU_SLB:
        set_R( acc.B << 1 );
        sprintf(msg, "SLB ");
        break;

        case ALU_SRA:
        set_R( acc.A >> 1 );
        sprintf(msg, "SRA ");
        break;

        case ALU_SRB:
        set_R( acc.B >> 1 );
        sprintf(msg, "SRB ");
        break;

        case ALU_AND:
        set_R( acc.A & acc.B );
        sprintf(msg, "AND ");
        break;

        case ALU_IOR:
        set_R( acc.A | acc.B );
        sprintf(msg, "IOR ");
        break;

        case ALU_EOR:
        set_R( acc.A ^ acc.B );
        sprintf(msg, "EOR ");
        break;

        case ALU_ADD:
        set_R( acc.A + acc.B );
        sprintf(msg, "ADD ");
        break;

        case ALU_CAR:
        set_R( (int) acc.A + (int) acc.B > 255 ? 1 : 0 );
        sprintf(msg, "CAR ");
        break;

        case ALU_ALB:
        set_R( (acc.A < acc.B) ? 255 : 0 );
        sprintf(msg, "ALB ");
        break;

        case ALU_AEB:
        set_R( (acc.A == acc.B) ? 255 : 0 );
        sprintf(msg, "AEB ");
        break;

        case ALU_AGB:
        set_R( (acc.A > acc.B) ? 255 : 0 );
        sprintf(msg, "AGB ");
        break;
    }
    if (loggingf) {
        sprintf(msg, "%s%02Xh(%dd,%sb)\n", msg, ALU.R, ALU.R, binStr(ALU.R));
        printf("%s",msg);
    }
}

#define SAVEPOS 256 * mem.L + 0xC4
void
kick( uint8_t op)
{
      sprintf(msg,"");
      switch (op & 15)
      {
          case KICK_NOP:
          sprintf(msg, "NOP\n");
          quitf = 1;
          break;
          
          case KICK_SSI:
          shift_in();
          break;

          case KICK_SSO:
          shift_out();
          break;

          case KICK_SCL:
          com.clock = 0;
          sprintf(msg, "SCL\n");
          break;
          
          case KICK_SCH:
          com.clock = 1;
          sprintf(msg, "SCH\n");
          break;
          
          case KICK_RDY:
          com.ready = 0;
          sprintf(msg, "RDY\n");
          break;

          case KICK_LLF:
          set_R(mem.data[SAVEPOS]);
          mem.L++;
          sprintf(msg, "LLF R=%02X frame %02X\n", mem.data[SAVEPOS], mem.L);
          break;
          
          case KICK_ELF:
          mem.L--;
          mem.data[SAVEPOS] = get_R();
          sprintf(msg, "ELF R=%02X frame %02X\n", mem.data[SAVEPOS], mem.L);
          break;

          case KICK_REA:
          acc.A = acc.clip;
          sprintf(msg, "REA %02X\n", acc.A);
          break;
          
          case KICK_REB:
          acc.B = acc.clip;
          sprintf(msg, "REA %02X\n", acc.B);
          break;
          
          default:
          break;
      }
      if (strlen(msg) && loggingf) printf("%s",msg);
}

uint16_t
effective( uint8_t offs)
{
    uint8_t addr_high;
    if ( offs < 128) addr_high = mem.H;
    else if ( offs < 192) addr_high = mem.G;
    else addr_high = mem.L;
    return 256 * addr_high + offs;
}

void
exec_TRAP( uint8_t addr)
{
    set_R(pc.low);
    pc.low = 0;
    mem.H = pc.high;
    pc.high = addr & 31;
    
    sprintf(msg, "Trap/Call to %02X00h\n", pc.high);
    if (loggingf) printf("%s",msg);
}


uint8_t
fetch()
{
    uint16_t addr;
    if (pc.low < 128) addr = 256 * pc.high + pc.low;
    else if (pc.low < 192) addr = 256 * mem.G + pc.low;
    else addr = 256 * mem.L + pc.low;

    uint8_t f = mem.data[addr];
    
    if (loggingf){
     sprintf(msg, "Fetch %02Xh(%sb) @%04Xh\n", f, binStr(f), addr);
     logmsg();
    }
    
    pc.low++;
    return f;
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
        case RHS_I:
        sprintf(msg, "Set I\n");
        ALU.I = val;
        break;

        case RHS_M:
        sprintf(msg, "Set M@%02Xh\n", mem.xMx);
        mem.data[mem.xMx] = val;
        break;

        case RHS_R:
        sprintf(msg, "Set R\n");
        set_R(val);
        break;

        case RHS_H:
        sprintf(msg, "Set H\n");
        mem.H = val;
        break;

        case RHS_G:
        sprintf(msg, "Set G\n");
        mem.G = val;
        break;

        case RHS_L:
        sprintf(msg, "Set L\n");
        mem.L = val;
        break;

        case RHS_S:
        sprintf(msg, "Set SOR\n");
        com.ser_or = val;
        break;

        case RHS_P:
        sprintf(msg, "Set POR\n");
        parallel_out(val);
        break;

        case RHS_E:
        sprintf(msg, "Set E\n");
        com.E = val;
        break;

        case RHS_J:
        sprintf(msg, "Jump to %02X%02Xh\n", pc.high, pc.low);
        pc.low = val;
        break;

        case RHS_D:
        ALU.I--;
        sprintf(msg, "Iterations left=%d\n", ALU.I);
        if (ALU.I) {
            pc.low = val;
            sprintf(msg, "%sJump to %02X%02Xh\n", msg, pc.high, pc.low);
        }
        else sprintf(msg, "%s(Falling through)\n", msg);
        break;

        case RHS_T:
        sprintf(msg, "Test R=TRUE\n");
        if (get_R()){
            sprintf(msg, "%sJump to %02X%02Xh\n", msg, pc.high, pc.low);
            pc.low = val;
        }
        break;

        case RHS_F:
        sprintf(msg, "Test R=FALSE\n");
        if (!get_R()) {
            sprintf(msg, "%sJump to %02X%02Xh\n", msg, pc.high, pc.low);
            pc.low = val;
        }
        break;

        case RHS_C:
        exec_TRAP(val);
        sprintf(msg, "Call\n");
        break;

        case RHS_B:
        sprintf(msg, "Set B\n");
        acc.clip = acc.B;
        acc.B = val;
        mem.xMx = effective(acc.B);
        break;

        case RHS_A:
        sprintf(msg, "Set A\n");
        acc.clip = acc.A;
        acc.A = val;
        mem.xMx = effective(acc.A);
        break;
   }
   if (loggingf) printf("%s",msg);
}

uint8_t
read_reg( uint8_t selector)
{
    uint8_t val;
    switch (selector & 7)
    {
        case LHS_N:
        sprintf(msg, "Copy N ");
        if (loggingf) printf("%s",msg);
        return fetch();

        case LHS_M:
        val = mem.data[mem.xMx];
        sprintf(msg, "Copy M@%04Xh (%02Xh) ", mem.xMx, val);
        if (loggingf) printf("%s",msg);
        return val;

        case LHS_R:
        sprintf(msg, "Copy R (%02Xh) ", get_R());
        if (loggingf) printf("%s",msg);
        return get_R();

        case LHS_H:
        sprintf(msg, "Copy H (%02Xh) ", mem.H);
        if (loggingf) printf("%s",msg);
        return mem.H;

        case LHS_G:
        sprintf(msg, "Copy G (%02Xh) ", mem.G);
        if (loggingf) printf("%s",msg);
        return mem.G;

        case LHS_L:
        sprintf(msg, "Copy L (%02Xh) ", mem.L);
        if (loggingf) printf("%s",msg);
        return mem.L; 

        case LHS_S:
        sprintf(msg, "Copy SIR (%02Xh) ", com.ser_ir);
        if (loggingf) printf("%s",msg);
        return com.ser_ir;

        case LHS_P:
        sprintf(msg, "Copy PIR (%02Xh) ", com.par_ir);
        if (loggingf) printf("%s",msg);
        return com.par_ir;

        default: return 0; // Warning dummy
    }
}

void
exec_PAIR( uint8_t dst, uint8_t src)
{
    src &= 7;
    dst &= 15;

    if ((src == LHS_N) && (dst == RHS_M)) { // Scrounge RET
        pc.low = get_R();
        pc.high = mem.H;
        sprintf(msg, "RET (%02X%02Xh) ", pc.high, pc.low);
        if (loggingf) printf("%s",msg);
    }
    else if ((src == LHS_M) && (dst == RHS_M)){  // Scrounge LID
        pc.high++;
        pc.low=0;
        mem.H = pc.high;
        sprintf(msg, "LID (%02X%02Xh) ", pc.high, pc.low);
        if (loggingf) printf("%s",msg);
   }
   else write_reg( dst, read_reg( src));
}

void
exec_GETPUT( uint8_t bits)
{
    uint8_t *loc = NULL;
    uint8_t gp_offs = bits & 3;
    uint8_t guide = (bits >> 4) & 3;

    if (!guide) kick(bits & 15); /*KICKS*/
    else /*GETPUT*/
    {
        switch(guide) {
        case 1: acc.clip = acc.A; loc = &acc.A; sprintf(msg, "A"); break;
        case 2: acc.clip = acc.B; loc = &acc.B; sprintf(msg, "B"); break;
        case 3: loc = &ALU.R; sprintf(msg, "R"); break;
        }

        uint16_t addr;
        if (bits & 4) {
            addr = 256 * mem.L + 192 + gp_offs; // Local bank
            sprintf(msg, "%s L%d", msg, gp_offs);
        }
        else {
            addr = 256 * mem.G + 128 + gp_offs; // Global bank
            sprintf(msg, "%s G%d", msg, gp_offs);
        }
        
        if (bits & 8) // PUT
        {
             if (loc == &ALU.R){
                 mem.data[addr] = get_R();
                 sprintf(msg, "%s PUT %d\n", msg, get_R());
             }
             else {
                 mem.data[addr] = *loc;
                 sprintf(msg, "%s PUT %d\n", msg, *loc);
             }
        }
        else // GET
        {
            sprintf(msg, "%s GET %d\n", msg, mem.data[addr]);
            if (loc == &ALU.R) set_R(mem.data[addr]);
            else {
                if (loc == &acc.A) {
                    acc.clip = acc.A;
                    acc.A = mem.data[addr];
                    mem.xMx = effective(acc.A);
                }
                else {
                    acc.clip = acc.B;
                    acc.B = mem.data[addr];
                    mem.xMx = effective(acc.B);
                }
            }
        }

        if (loggingf) printf("%s", msg);
    }
}

void
decode()
{
    uint8_t instr = fetch();
    uint8_t src = instr & 15;
    uint8_t dst = (instr >> 4) & 7;

    if (instr & 128) { exec_PAIR(src, dst); return; }

    if (!(instr & 64)) { exec_GETPUT(instr & 63); return; }

    if (!(instr & 32)) { exec_TRAP(instr & 31); return; }

    if (instr & 16) aluRun(src);
    else ALU.O = src < 8 ? src : (uint8_t) (0xF8 | src);
}

int
rdimg(char *fname)
{
    FILE *f;
    f = fopen(fname, "r");
    if (f == NULL) return -1;
    fread(mem.data, 1, 65535, f);
    fclose(f);
    return 0;
}


int
wrimg(char *fname)
{
    FILE *f;
    f = fopen(fname, "rb+");
    if (f == NULL) return -1;
    fwrite(mem.data, 1, 65535, f);
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
    loggingf = 0;

//    printf("\n/* Sonne CPU Virtual Machine\n");
//    printf("  Jan-2024 Michael Mangelsdorf\n");
//    printf("  Copyr. <mim@ok-schalter.de>\n");
//    printf("*/\n\n");

    if (argc<2){
        printf("Missing work file name, using default\n");
        fname = "daffodil.obj";
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






