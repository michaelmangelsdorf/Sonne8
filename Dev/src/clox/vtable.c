

/*
  Reference Implementation for Myst Emulators
  Author: mim@ok-schalter.de (Michael/Dosflange@github)

  The code uses a dispatch/computed jump table.
  Compile with GCC which has the required && operator
  for dereferencing LABELs as void*
*/

#include <stdint.h>

int
main()
{
    uint8_t RAM[256][256] __attribute__((aligned(8)));
    uint8_t E, SCLK, MISO, MOSI, SIR, SOR, PIR, POR;
    uint8_t R, O, I, PC, D, C, G, L;

/*
  Quick which-is-which:
  
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

  I is the inner counter register. It is a hardware
  loop counter and is used during function calls.

  PC is the program counter, which for this CPU means
  that it contains the offset within the current code page.

  D is the "dupe" register. It contains a copy of the
  code page index that is strategically updated.
  This register is maintained by the CPU only.

  C contains the code page index. Instruction fetch always
  occurs at RAM[C][PC].

  G is the global register. It contains the implied
  page index for memory operations using the M-prefix.

  L is the local register. It contains the implied
  page index for memory operations using the L-prefix,
  and this register is used as a stack frame pointer
  in conjunction with function calls. It is maintained
  by the CPU only.
*/
  
  
/* Each entry points to a routine handling 1 opcode */

    void* dispatch_table[256] = {

        &&SYS_NOP, &&SYS_SSI, &&SYS_SSO, &&SYS_SCL,
        &&SYS_SCH, &&SYS_RET, &&SYS_COR, &&SYS_OWN,

        &&FIX_P4, &&FIX_P1, &&FIX_P2, &&FIX_P3,
        &&FIX_M4, &&FIX_M3, &&FIX_M2, &&FIX_M1,
        
        &&ALU_CLR, &&ALU_IDO, &&ALU_OCR, &&ALU_OCO,
        &&ALU_SLR, &&ALU_SLO, &&ALU_SRR, &&ALU_SRO,
        &&ALU_AND, &&ALU_IOR, &&ALU_EOR, &&ALU_ADD,
        &&ALU_CAR, &&ALU_RLO, &&ALU_REO, &&ALU_RGO,
        
        &&TRAP_0, &&TRAP_1, &&TRAP_2, &&TRAP_3,
        &&TRAP_4, &&TRAP_5, &&TRAP_6, &&TRAP_7,
        &&TRAP_8, &&TRAP_9, &&TRAP_10, &&TRAP_11,
        &&TRAP_12, &&TRAP_13, &&TRAP_14, &&TRAP_15,
        &&TRAP_16, &&TRAP_17, &&TRAP_18, &&TRAP_19,
        &&TRAP_20, &&TRAP_21, &&TRAP_22, &&TRAP_23,
        &&TRAP_24, &&TRAP_25, &&TRAP_26, &&TRAP_27,
        &&TRAP_28, &&TRAP_29, &&TRAP_30, &&TRAP_31,

        &&GIRO_0G, &&GIRO_1G, &&GIRO_2G, &&GIRO_3G,
        &&GIRO_4G, &&GIRO_5G, &&GIRO_6G, &&GIRO_7G,
        &&GIRO_G0, &&GIRO_G1, &&GIRO_G2, &&GIRO_G3,
        &&GIRO_G4, &&GIRO_G5, &&GIRO_G6, &&GIRO_G7,
        &&GIRO_0I, &&GIRO_1I, &&GIRO_2I, &&GIRO_3I,
        &&GIRO_4I, &&GIRO_5I, &&GIRO_6I, &&GIRO_7I,
        &&GIRO_I0, &&GIRO_I1, &&GIRO_I2, &&GIRO_I3,
        &&GIRO_I4, &&GIRO_I5, &&GIRO_I6, &&GIRO_I7,
        &&GIRO_0R, &&GIRO_1R, &&GIRO_2R, &&GIRO_3R,
        &&GIRO_4R, &&GIRO_5R, &&GIRO_6R, &&GIRO_7R,
        &&GIRO_R0, &&GIRO_R1, &&GIRO_R2, &&GIRO_R3,
        &&GIRO_R4, &&GIRO_R5, &&GIRO_R6, &&GIRO_R7,
        &&GIRO_0O, &&GIRO_1O, &&GIRO_2O, &&GIRO_3O,
        &&GIRO_4O, &&GIRO_5O, &&GIRO_6O, &&GIRO_7O,
        &&GIRO_O0, &&GIRO_O1, &&GIRO_O2, &&GIRO_O3,
        &&GIRO_O4, &&GIRO_O5, &&GIRO_O6, &&GIRO_O7,

        &&PAIR_NO, &&SCROUNGE_NM, &&SCROUNGE_NL, &&PAIR_NG,
        &&PAIR_NR, &&PAIR_NI, &&PAIR_NS, &&PAIR_NP,
        &&PAIR_NE, &&PAIR_NA, &&PAIR_NB, &&PAIR_NJ,
        &&PAIR_NW, &&PAIR_NT, &&PAIR_NF, &&PAIR_NC,

        &&PAIR_MO, &&SCROUNGE_MM, &&SCROUNGE_ML, &&PAIR_MG,
        &&PAIR_MR, &&PAIR_MI, &&PAIR_MS, &&PAIR_MP,
        &&PAIR_ME, &&PAIR_MA, &&PAIR_MB, &&PAIR_MJ,
        &&PAIR_MW, &&PAIR_MT, &&PAIR_MF, &&PAIR_MC,
        
        &&PAIR_LO, &&SCROUNGE_LM, &&SCROUNGE_LL, &&PAIR_LG,
        &&PAIR_LR, &&PAIR_LI, &&PAIR_LS, &&PAIR_LP,
        &&PAIR_LE, &&PAIR_LA, &&PAIR_LB, &&PAIR_LJ,
        &&PAIR_LW, &&PAIR_LT, &&PAIR_LF, &&PAIR_LC,
        
        &&PAIR_GO, &&PAIR_GM, &&PAIR_GL, &&SCROUNGE_GG,
        &&PAIR_GR, &&PAIR_GI, &&PAIR_GS, &&PAIR_GP,
        &&PAIR_GE, &&PAIR_GA, &&PAIR_GB, &&PAIR_GJ,
        &&PAIR_GW, &&PAIR_GT, &&PAIR_GF, &&PAIR_GC,
        
        &&PAIR_RO, &&PAIR_RM, &&PAIR_RL, &&PAIR_RG,
        &&SCROUNGE_RR, &&PAIR_RI, &&PAIR_RS, &&PAIR_RP,
        &&PAIR_RE, &&PAIR_RA, &&PAIR_RB, &&PAIR_RJ,
        &&PAIR_RW, &&PAIR_RT, &&PAIR_RF, &&PAIR_RC,
        
        &&PAIR_IO, &&PAIR_IM, &&PAIR_IL, &&PAIR_IG,
        &&PAIR_IR, &&SCROUNGE_II, &&PAIR_IS, &&PAIR_IP,
        &&PAIR_IE, &&PAIR_IA, &&PAIR_IB, &&PAIR_IJ,
        &&PAIR_IW, &&PAIR_IT, &&PAIR_IF, &&PAIR_IC,
        
        &&PAIR_SO, &&PAIR_SM, &&PAIR_SL, &&PAIR_SG,
        &&PAIR_SR, &&PAIR_SI, &&PAIR_SS, &&PAIR_SP,
        &&PAIR_SE, &&PAIR_SA, &&PAIR_SB, &&PAIR_SJ,
        &&PAIR_SW, &&PAIR_ST, &&PAIR_SF, &&PAIR_SC,
        
        &&PAIR_PO, &&PAIR_PM, &&PAIR_PL, &&PAIR_PG,
        &&PAIR_PR, &&PAIR_PI, &&PAIR_PS, &&PAIR_PP,
        &&PAIR_PE, &&PAIR_PA, &&PAIR_PB, &&PAIR_PJ,
        &&PAIR_PW, &&PAIR_PT, &&PAIR_PF, &&PAIR_PC,
    };

    C = PC = 0;

    /*
      To do: Load a program into RAM[][] here!
    */

    /*
      Instruction pump loop:
      Fetch an opcode, run it, repeat.
    */

    NEXT: PC++;
    EXEC: goto *dispatch_table[RAM[C][PC]];
    END: return 0; /* Route one of the SCROUNGES here for example */

    /*
      The remaining lines define 256 instruction routines
      which either branch back to NEXT, EXEC, or END.
      Each one of them has a dispatch_table[] entry.
     */

    #define GIRO 0xF8 /*Local-page offset used by GIRO instructions*/

    unsigned TEMP;

    /* SYS */

    /*00h*/   SYS_NOP:  goto NEXT;
    /*01h*/   SYS_SSI:  SIR = (SIR << 1) + MISO; goto NEXT;
    /*02h*/   SYS_SSO:  MOSI = SOR & 0x80 ? 1 : 0; SOR <<= 1; goto NEXT;
    /*03h*/   SYS_SCL:  SCLK = 0; goto NEXT;
    /*04h*/   SYS_SCH:  SCLK = 1; goto NEXT;
    /*05h*/   SYS_RET:  C = RAM[L][GIRO+7]; PC = I; L++; goto NEXT;
    /*06h*/   SYS_COR:  C = G; PC = I; goto NEXT;
    /*07h*/   SYS_OWN:  RAM[L][GIRO+7] = D; goto NEXT;

    /* FIX */

    /*08h*/   FIX_P4:  R += 4; goto NEXT;
    /*09h*/   FIX_P1:  R += 1; goto NEXT;
    /*0Ah*/   FIX_P2:  R += 2; goto NEXT;
    /*0Bh*/   FIX_P3:  R += 3; goto NEXT;
    /*0Ch*/   FIX_M4:  R -= 4; goto NEXT;
    /*0Dh*/   FIX_M3:  R -= 3; goto NEXT;
    /*0Eh*/   FIX_M2:  R -= 2; goto NEXT;
    /*0Fh*/   FIX_M1:  R -= 1; goto NEXT;

    /* ALU */

    /*10h*/   ALU_CLR: R = 0; goto NEXT;
    /*11h*/   ALU_IDO: R = O; goto NEXT;
    /*12h*/   ALU_OCR: R = ~R; goto NEXT;
    /*13h*/   ALU_OCO: R = ~O; goto NEXT;
    /*14h*/   ALU_SLR: R <<= 1; goto NEXT;
    /*15h*/   ALU_SLO: R = O << 1; goto NEXT;
    /*16h*/   ALU_SRR: R >>= 1; goto NEXT;
    /*17h*/   ALU_SRO: R = O >> 1; goto NEXT;
    /*18h*/   ALU_AND: R &= O; goto NEXT;
    /*19h*/   ALU_IOR: R |= O; goto NEXT;
    /*1Ah*/   ALU_EOR: R ^= O; goto NEXT;
    /*1Bh*/   ALU_ADD: R += O; goto NEXT;
    /*1Ch*/   ALU_CAR: R = (uint8_t) R + (uint8_t) O > 255 ? 1 : 0; goto NEXT;
    /*1Dh*/   ALU_RLO: R = (R < O) ? 255 : 0; goto NEXT;
    /*1Eh*/   ALU_REO: R = (R == O) ? 255 : 0; goto NEXT;
    /*1Fh*/   ALU_RGO: R = (R > O) ? 255 : 0; goto NEXT;

    /* TRAP */

    /*20h*/   TRAP_0: I = PC; D = C; L--; PC = 0; C = 0; goto EXEC;
    /*21h*/   TRAP_1: I = PC; D = C; L--; PC = 0; C = 1; goto EXEC;
    /*22h*/   TRAP_2: I = PC; D = C; L--; PC = 0; C = 2; goto EXEC;
    /*23h*/   TRAP_3: I = PC; D = C; L--; PC = 0; C = 3; goto EXEC;
    /*24h*/   TRAP_4: I = PC; D = C; L--; PC = 0; C = 4; goto EXEC;
    /*25h*/   TRAP_5: I = PC; D = C; L--; PC = 0; C = 5; goto EXEC;
    /*26h*/   TRAP_6: I = PC; D = C; L--; PC = 0; C = 6; goto EXEC;
    /*27h*/   TRAP_7: I = PC; D = C; L--; PC = 0; C = 7; goto EXEC;
    /*28h*/  h TRAP_8: I = PC; D = C; L--; PC = 0; C = 8; goto EXEC;
    /*29h*/   TRAP_9: I = PC; D = C; L--; PC = 0; C = 9; goto EXEC;
    /*2Ah*/   TRAP_10: I = PC; D = C; L--; PC = 0; C = 10; goto EXEC;
    /*2Bh*/   TRAP_11: I = PC; D = C; L--; PC = 0; C = 11; goto EXEC;
    /*2Ch*/   TRAP_12: I = PC; D = C; L--; PC = 0; C = 12; goto EXEC;
    /*2Dh*/   TRAP_13: I = PC; D = C; L--; PC = 0; C = 13; goto EXEC;
    /*2Eh*/   TRAP_14: I = PC; D = C; L--; PC = 0; C = 14; goto EXEC;
    /*2Fh*/   TRAP_15: I = PC; D = C; L--; PC = 0; C = 15; goto EXEC;
    /*30h*/   TRAP_16: I = PC; D = C; L--; PC = 0; C = 16; goto EXEC;
    /*31h*/   TRAP_17: I = PC; D = C; L--; PC = 0; C = 17; goto EXEC;
    /*32h*/   TRAP_18: I = PC; D = C; L--; PC = 0; C = 18; goto EXEC;
    /*33h*/   TRAP_19: I = PC; D = C; L--; PC = 0; C = 19; goto EXEC;
    /*34h*/   TRAP_20: I = PC; D = C; L--; PC = 0; C = 20; goto EXEC;
    /*35h*/   TRAP_21: I = PC; D = C; L--; PC = 0; C = 21; goto EXEC;
    /*36h*/   TRAP_22: I = PC; D = C; L--; PC = 0; C = 22; goto EXEC;
    /*37h*/   TRAP_23: I = PC; D = C; L--; PC = 0; C = 23; goto EXEC;
    /*38h*/   TRAP_24: I = PC; D = C; L--; PC = 0; C = 24; goto EXEC;
    /*39h*/   TRAP_25: I = PC; D = C; L--; PC = 0; C = 25; goto EXEC;
    /*3Ah*/   TRAP_26: I = PC; D = C; L--; PC = 0; C = 26; goto EXEC;
    /*3Bh*/   TRAP_27: I = PC; D = C; L--; PC = 0; C = 27; goto EXEC;
    /*3Ch*/   TRAP_28: I = PC; D = C; L--; PC = 0; C = 28; goto EXEC;
    /*3Dh*/   TRAP_29: I = PC; D = C; L--; PC = 0; C = 29; goto EXEC;
    /*3Eh*/   TRAP_30: I = PC; D = C; L--; PC = 0; C = 30; goto EXEC;
    /*3Fh*/   TRAP_31: I = PC; D = C; L--; PC = 0; C = 31; goto EXEC;

    /* GIRO */

    /*40h*/   GIRO_0G: G = RAM[L][GIRO+0]; goto NEXT;
    /*41h*/   GIRO_1G: G = RAM[L][GIRO+1]; goto NEXT;
    /*42h*/   GIRO_2G: G = RAM[L][GIRO+2]; goto NEXT;
    /*43h*/   GIRO_3G: G = RAM[L][GIRO+3]; goto NEXT;
    /*44h*/   GIRO_4G: G = RAM[L][GIRO+4]; goto NEXT;
    /*45h*/   GIRO_5G: G = RAM[L][GIRO+5]; goto NEXT;
    /*46h*/   GIRO_6G: G = RAM[L][GIRO+6]; goto NEXT;
    /*47h*/   GIRO_7G: G = RAM[L][GIRO+7]; goto NEXT;
    /*48h*/   GIRO_G0: RAM[L][GIRO+0] = G; goto NEXT;
    /*49h*/   GIRO_G1: RAM[L][GIRO+1] = G; goto NEXT;
    /*4Ah*/   GIRO_G2: RAM[L][GIRO+2] = G; goto NEXT;
    /*4Bh*/   GIRO_G3: RAM[L][GIRO+3] = G; goto NEXT;
    /*4Ch*/   GIRO_G4: RAM[L][GIRO+4] = G; goto NEXT;
    /*4Dh*/   GIRO_G5: RAM[L][GIRO+5] = G; goto NEXT;
    /*4Eh*/   GIRO_G6: RAM[L][GIRO+6] = G; goto NEXT;
    /*4Fh*/   GIRO_G7: RAM[L][GIRO+7] = G; goto NEXT;

    /*50h*/   GIRO_0I: I = RAM[L][GIRO+0]; goto NEXT;
    /*51h*/   GIRO_1I: I = RAM[L][GIRO+1]; goto NEXT;
    /*52h*/   GIRO_2I: I = RAM[L][GIRO+2]; goto NEXT;
    /*53h*/   GIRO_3I: I = RAM[L][GIRO+3]; goto NEXT;
    /*54h*/   GIRO_4I: I = RAM[L][GIRO+4]; goto NEXT;
    /*55h*/   GIRO_5I: I = RAM[L][GIRO+5]; goto NEXT;
    /*56h*/   GIRO_6I: I = RAM[L][GIRO+6]; goto NEXT;
    /*57h*/   GIRO_7I: I = RAM[L][GIRO+7]; goto NEXT;
    /*58h*/   GIRO_I0: RAM[L][GIRO+0] = I; goto NEXT;
    /*59h*/   GIRO_I1: RAM[L][GIRO+1] = I; goto NEXT;
    /*5Ah*/   GIRO_I2: RAM[L][GIRO+2] = I; goto NEXT;
    /*5Bh*/   GIRO_I3: RAM[L][GIRO+3] = I; goto NEXT;
    /*5Ch*/   GIRO_I4: RAM[L][GIRO+4] = I; goto NEXT;
    /*5Dh*/   GIRO_I5: RAM[L][GIRO+5] = I; goto NEXT;
    /*5Eh*/   GIRO_I6: RAM[L][GIRO+6] = I; goto NEXT;
    /*5Fh*/   GIRO_I7: RAM[L][GIRO+7] = I; goto NEXT;

    /*60h*/   GIRO_0R: R = RAM[L][GIRO+0]; goto NEXT;
    /*61h*/   GIRO_1R: R = RAM[L][GIRO+1]; goto NEXT;
    /*62h*/   GIRO_2R: R = RAM[L][GIRO+2]; goto NEXT;
    /*63h*/   GIRO_3R: R = RAM[L][GIRO+3]; goto NEXT;
    /*64h*/   GIRO_4R: R = RAM[L][GIRO+4]; goto NEXT;
    /*65h*/   GIRO_5R: R = RAM[L][GIRO+5]; goto NEXT;
    /*66h*/   GIRO_6R: R = RAM[L][GIRO+6]; goto NEXT;
    /*67h*/   GIRO_7R: R = RAM[L][GIRO+7]; goto NEXT;
    /*68h*/   GIRO_R0: RAM[L][GIRO+0] = R; goto NEXT;
    /*69h*/   GIRO_R1: RAM[L][GIRO+1] = R; goto NEXT;
    /*6Ah*/   GIRO_R2: RAM[L][GIRO+2] = R; goto NEXT;
    /*6Bh*/   GIRO_R3: RAM[L][GIRO+3] = R; goto NEXT;
    /*6Ch*/   GIRO_R4: RAM[L][GIRO+4] = R; goto NEXT;
    /*6Dh*/   GIRO_R5: RAM[L][GIRO+5] = R; goto NEXT;
    /*6Eh*/   GIRO_R6: RAM[L][GIRO+6] = R; goto NEXT;
    /*6Fh*/   GIRO_R7: RAM[L][GIRO+7] = R; goto NEXT;

    /*70h*/   GIRO_0O: O = RAM[L][GIRO+0]; goto NEXT;
    /*71h*/   GIRO_1O: O = RAM[L][GIRO+1]; goto NEXT;
    /*72h*/   GIRO_2O: O = RAM[L][GIRO+2]; goto NEXT;
    /*73h*/   GIRO_3O: O = RAM[L][GIRO+3]; goto NEXT;
    /*74h*/   GIRO_4O: O = RAM[L][GIRO+4]; goto NEXT;
    /*75h*/   GIRO_5O: O = RAM[L][GIRO+5]; goto NEXT;
    /*76h*/   GIRO_6O: O = RAM[L][GIRO+6]; goto NEXT;
    /*77h*/   GIRO_7O: O = RAM[L][GIRO+7]; goto NEXT;
    /*78h*/   GIRO_O0: RAM[L][GIRO+0] = O; goto NEXT;
    /*79h*/   GIRO_O1: RAM[L][GIRO+1] = O; goto NEXT;
    /*7Ah*/   GIRO_O2: RAM[L][GIRO+2] = O; goto NEXT;
    /*7Bh*/   GIRO_O3: RAM[L][GIRO+3] = O; goto NEXT;
    /*7Ch*/   GIRO_O4: RAM[L][GIRO+4] = O; goto NEXT;
    /*7Dh*/   GIRO_O5: RAM[L][GIRO+5] = O; goto NEXT;
    /*7Eh*/   GIRO_O6: RAM[L][GIRO+6] = O; goto NEXT;
    /*7Fh*/   GIRO_O7: RAM[L][GIRO+7] = O; goto NEXT;

    /* PAIR */

    /*80h*/   PAIR_NO: O = RAM[C][PC]; goto NEXT;
    /*81h*/   SCROUNGE_NM: goto NEXT;
    /*82h*/   SCROUNGE_NL: goto NEXT;
    /*83h*/   PAIR_NG: G = RAM[C][PC]; goto NEXT;
    /*84h*/   PAIR_NR: R = RAM[C][PC]; goto NEXT;
    /*85h*/   PAIR_NI: I = RAM[C][PC]; goto NEXT;
    /*86h*/   PAIR_NS: SOR = RAM[C][PC]; goto NEXT;
    /*87h*/   PAIR_NP: POR = RAM[C][PC]; goto NEXT;
    /*88h*/   PAIR_NE: E = RAM[C][PC]; goto NEXT;
    /*89h*/   PAIR_NA: TEMP = O + RAM[C][PC]; O = (uint8_t)(TEMP & 0xFF); if (TEMP>0xFF) G++; goto NEXT;
    /*8Ah*/   PAIR_NB: PC += RAM[C][PC]; goto EXEC;
    /*8Bh*/   PAIR_NJ: PC = RAM[C][PC]; goto EXEC;
    /*8Ch*/   PAIR_NW: if (I--) {PC = RAM[C][PC]; goto EXEC;} else goto NEXT;
    /*8Dh*/   PAIR_NT: if (R) {PC = RAM[C][PC]; goto EXEC;} else goto NEXT;
    /*8Eh*/   PAIR_NF: if (!R) {PC = RAM[C][PC]; goto EXEC;} else goto NEXT;
    /*8Fh*/   PAIR_NC: I = PC; D = C; L--; PC = 0; C = RAM[C][PC]; goto EXEC;

    /*90h*/   PAIR_MO: O = RAM[G][O]; goto NEXT;
    /*91h*/   SCROUNGE_MM: goto NEXT;
    /*92h*/   SCROUNGE_ML: goto NEXT;
    /*93h*/   PAIR_MG: G = RAM[G][O]; goto NEXT;
    /*94h*/   PAIR_MR: R = RAM[G][O]; goto NEXT;
    /*95h*/   PAIR_MI: I = RAM[G][O]; goto NEXT;
    /*96h*/   PAIR_MS: SOR = RAM[G][O]; goto NEXT;
    /*97h*/   PAIR_MP: POR = RAM[G][O]; goto NEXT;
    /*98h*/   PAIR_ME: E = RAM[G][O]; goto NEXT;
    /*99h*/   PAIR_MA: TEMP = O + RAM[G][O]; O = (uint8_t)(TEMP & 0xFF); if (TEMP>0xFF) G++; goto NEXT;
    /*9Ah*/   PAIR_MB: PC += RAM[G][O]; goto EXEC;
    /*9Bh*/   PAIR_MJ: PC = RAM[G][O]; goto EXEC;
    /*9Ch*/   PAIR_MW: if (I--) {PC = RAM[G][O]; goto EXEC;} else goto NEXT;
    /*9Dh*/   PAIR_MT: if (R) {PC = RAM[G][O]; goto EXEC;} else goto NEXT;
    /*9Eh*/   PAIR_MF: if (!R) {PC = RAM[G][O]; goto EXEC;} else goto NEXT;
    /*9Fh*/   PAIR_MC: I = PC; D = C; L--; PC = 0; C = RAM[G][O]; goto EXEC;

    /*A0h*/   PAIR_LO: O = RAM[L][O]; goto NEXT;
    /*A1h*/   SCROUNGE_LM: goto NEXT;
    /*A2h*/   SCROUNGE_LL: goto NEXT;
    /*A3h*/   PAIR_LG: G = RAM[L][O]; goto NEXT;
    /*A4h*/   PAIR_LR: R = RAM[L][O]; goto NEXT;
    /*A5h*/   PAIR_LI: I = RAM[L][O]; goto NEXT;
    /*A6h*/   PAIR_LS: SOR = RAM[L][O]; goto NEXT;
    /*A7h*/   PAIR_LP: POR = RAM[L][O]; goto NEXT;
    /*A8h*/   PAIR_LE: E = RAM[L][O]; goto NEXT;
    /*A9h*/   PAIR_LA: TEMP = O + RAM[L][O]; O = (uint8_t)(TEMP & 0xFF); if (TEMP>0xFF) G++; goto NEXT;
    /*AAh*/   PAIR_LB: PC += RAM[L][O]; goto EXEC;
    /*ABh*/   PAIR_LJ: PC = RAM[L][O]; goto EXEC;
    /*ACh*/   PAIR_LW: if (I--) {PC = RAM[L][O]; goto EXEC;} else goto NEXT;
    /*ADh*/   PAIR_LT: if (R) {PC = RAM[L][O]; goto EXEC;} else goto NEXT;
    /*AEh*/   PAIR_LF: if (!R) {PC = RAM[L][O]; goto EXEC;} else goto NEXT;
    /*AFh*/   PAIR_LC: I = PC; D = C; L--; PC = 0; C = RAM[L][O]; goto EXEC;

    /*B0h*/   PAIR_GO: O = G; goto NEXT;
    /*B1h*/   PAIR_GM: RAM[G][O] = G; goto NEXT;
    /*B2h*/   PAIR_GL: RAM[L][O] = G; goto NEXT;
    /*B3h*/   SCROUNGE_GG: goto NEXT;
    /*B4h*/   PAIR_GR: R = G; goto NEXT;
    /*B5h*/   PAIR_GI: I = G; goto NEXT;
    /*B6h*/   PAIR_GS: SOR = G; goto NEXT;
    /*B7h*/   PAIR_GP: POR = G; goto NEXT;
    /*B8h*/   PAIR_GE: E = G; goto NEXT;
    /*B9h*/   PAIR_GA: TEMP = O + G; O = (uint8_t)(TEMP & 0xFF); if (TEMP>0xFF) G++; goto NEXT;
    /*BAh*/   PAIR_GB: PC += G; goto EXEC;
    /*BBh*/   PAIR_GJ: PC = G; goto EXEC;
    /*BCh*/   PAIR_GW: if (I--) {PC = G; goto EXEC;} else goto NEXT;
    /*BDh*/   PAIR_GT: if (R) {PC = G; goto EXEC;} else goto NEXT;
    /*BEh*/   PAIR_GF: if (!R) {PC = G; goto EXEC;} else goto NEXT;
    /*BFh*/   PAIR_GC: I = PC; D = C; L--; PC = 0; C = G; goto EXEC;

    /*C0h*/   PAIR_RO: O = R; goto NEXT;
    /*C1h*/   PAIR_RM: RAM[G][O] = R; goto NEXT;
    /*C2h*/   PAIR_RL: RAM[L][O] = R; goto NEXT;
    /*C3h*/   PAIR_RG: G = R; goto NEXT;
    /*C4h*/   SCROUNGE_RR: goto NEXT;
    /*C5h*/   PAIR_RI: I = R; goto NEXT;
    /*C6h*/   PAIR_RS: SOR = R; goto NEXT;
    /*C7h*/   PAIR_RP: POR = R; goto NEXT;
    /*C8h*/   PAIR_RE: E = R; goto NEXT;
    /*C9h*/   PAIR_RA: TEMP = O + R; O = (uint8_t)(TEMP & 0xFF); if (TEMP>0xFF) G++; goto NEXT;
    /*CAh*/   PAIR_RB: PC += R; goto EXEC;
    /*CBh*/   PAIR_RJ: PC = R; goto EXEC;
    /*CCh*/   PAIR_RW: if (I--) {PC = R; goto EXEC;} else goto NEXT;
    /*CDh*/   PAIR_RT: if (R) {PC = R; goto EXEC;} else goto NEXT;
    /*CEh*/   PAIR_RF: if (!R) {PC = R; goto EXEC;} else goto NEXT;
    /*CFh*/   PAIR_RC: I = PC; D = C; L--; PC = 0; C = R; goto EXEC;

    /*D0h*/   PAIR_IO: O = I; goto NEXT;
    /*D1h*/   PAIR_IM: RAM[G][O] = I; goto NEXT;
    /*D2h*/   PAIR_IL: RAM[L][O] = I; goto NEXT;
    /*D3h*/   PAIR_IG: G = I; goto NEXT;
    /*D4h*/   PAIR_IR: R = I; goto NEXT;
    /*D5h*/   SCROUNGE_II: goto NEXT;
    /*D6h*/   PAIR_IS: SOR = I; goto NEXT;
    /*D7h*/   PAIR_IP: POR = I; goto NEXT;
    /*D8h*/   PAIR_IE: E = I; goto NEXT;
    /*D9h*/   PAIR_IA: TEMP = O + I; O = (uint8_t)(TEMP & 0xFF); if (TEMP>0xFF) G++; goto NEXT;
    /*DAh*/   PAIR_IB: PC += I; goto EXEC;
    /*DBh*/   PAIR_IJ: PC = I; goto EXEC;
    /*DCh*/   PAIR_IW: if (I--) {PC = I; goto EXEC;} else goto NEXT;
    /*DDh*/   PAIR_IT: if (R) {PC = I; goto EXEC;} else goto NEXT;
    /*DEh*/   PAIR_IF: if (!R) {PC = I; goto EXEC;} else goto NEXT;
    /*DFh*/   PAIR_IC: I = PC; D = C; L--; PC = 0; C = I; goto EXEC;

    /*E0h*/   PAIR_SO: O = SIR; goto NEXT;
    /*E1h*/   PAIR_SM: RAM[G][O] = SIR; goto NEXT;
    /*E2h*/   PAIR_SL: RAM[L][O] = SIR; goto NEXT;
    /*E3h*/   PAIR_SG: G = SIR; goto NEXT;
    /*E4h*/   PAIR_SR: R = SIR; goto NEXT;
    /*E5h*/   PAIR_SI: I = SIR; goto NEXT;
    /*E6h*/   PAIR_SS: SOR = SIR; goto NEXT;
    /*E7h*/   PAIR_SP: POR = SIR; goto NEXT;
    /*E8h*/   PAIR_SE: E = SIR; goto NEXT;
    /*E9h*/   PAIR_SA: TEMP = O + SIR; O = (uint8_t)(TEMP & 0xFF); if (TEMP>0xFF) G++; goto NEXT;
    /*EAh*/   PAIR_SB: PC += SIR; goto EXEC;
    /*EBh*/   PAIR_SJ: PC = SIR; goto EXEC;
    /*ECh*/   PAIR_SW: if (I--) {PC = SIR; goto EXEC;} else goto NEXT;
    /*EDh*/   PAIR_ST: if (R) {PC = SIR; goto EXEC;} else goto NEXT;
    /*EEh*/   PAIR_SF: if (!R) {PC = SIR; goto EXEC;} else goto NEXT;
    /*EFh*/   PAIR_SC: I = PC; D = C; L--; PC = 0; C = SIR; goto EXEC;

    /*F0h*/   PAIR_PO: O = PIR; goto NEXT;
    /*F1h*/   PAIR_PM: RAM[G][O] = PIR; goto NEXT;
    /*F2h*/   PAIR_PL: RAM[L][O] = PIR; goto NEXT;
    /*F3h*/   PAIR_PG: G = PIR; goto NEXT;
    /*F4h*/   PAIR_PR: R = PIR; goto NEXT;
    /*F5h*/   PAIR_PI: I = PIR; goto NEXT;
    /*F6h*/   PAIR_PS: SOR = PIR; goto NEXT;
    /*F7h*/   PAIR_PP: POR = PIR; goto NEXT;
    /*F8h*/   PAIR_PE: E = PIR; goto NEXT;
    /*F9h*/   PAIR_PA: TEMP = O + PIR; O = (uint8_t)(TEMP & 0xFF); if (TEMP>0xFF) G++; goto NEXT;
    /*FAh*/   PAIR_PB: PC += PIR; goto EXEC;
    /*FBh*/   PAIR_PJ: PC = PIR; goto EXEC;
    /*FCh*/   PAIR_PW: if (I--) {PC = PIR; goto EXEC;} else goto NEXT;
    /*FDh*/   PAIR_PT: if (R) {PC = PIR; goto EXEC;} else goto NEXT;
    /*FEh*/   PAIR_PF: if (!R) {PC = PIR; goto EXEC;} else goto NEXT;
    /*FFh*/   PAIR_PC: I = PC; D = C; L--; PC = 0; C = PIR; goto EXEC;
}


