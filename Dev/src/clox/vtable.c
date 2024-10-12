

/*
  Dispatch-table implementation of a Myst emulator
  Sonne 8 micro-controller Rev. Myth/LOX Project
  Author: mim@ok-schalter.de (Michael/Dosflange@github)

  Compile with GCC which has the required && operator
  for dereferencing LABELs as void*

  In spirit, this should be made interchangeable
  with the myst.h emulator myth_step() function.
  Some other day perhaps.
*/

#include <stdint.h>

int
main()
{
    uint8_t RAM[256][256] __attribute__((aligned(8)));
    uint8_t E, SCLK, MISO, MOSI, SIR, SOR, PIR, POR;
    uint8_t R, O, I, PC, D, C, G, L;

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

    #define GIRO 0xF8 /*Local-page offset used by DIRO instructions*/

    unsigned TEMP;

    /* SYS */
    SYS_NOP: goto NEXT;
    SYS_SSI: SIR = (SIR << 1) + MISO; goto NEXT;
    SYS_SSO: MOSI = SOR & 0x80 ? 1 : 0; SOR <<= 1; goto NEXT;
    SYS_SCL: SCLK = 0; goto NEXT;
    SYS_SCH: SCLK = 1; goto NEXT;
    SYS_RET: C = RAM[L][GIRO+7]; PC = I; L++; goto NEXT;
    SYS_COR: C = G; PC = I; goto NEXT;
    SYS_OWN: RAM[L][GIRO+7] = D; goto NEXT;

    /* FIX */
    FIX_P4: R += 4; goto NEXT;
    FIX_P1: R += 1; goto NEXT;
    FIX_P2: R += 2; goto NEXT;
    FIX_P3: R += 3; goto NEXT;
    FIX_M4: R -= 4; goto NEXT;
    FIX_M3: R -= 3; goto NEXT;
    FIX_M2: R -= 2; goto NEXT;
    FIX_M1: R -= 1; goto NEXT;

    /* ALU */
    ALU_CLR: R = 0; goto NEXT;
    ALU_IDO: R = O; goto NEXT;
    ALU_OCR: R = ~R; goto NEXT;
    ALU_OCO: R = ~O; goto NEXT;
    ALU_SLR: R <<= 1; goto NEXT;
    ALU_SLO: R = O << 1; goto NEXT;
    ALU_SRR: R >>= 1; goto NEXT;
    ALU_SRO: R = O >> 1; goto NEXT;
    ALU_AND: R &= O; goto NEXT;
    ALU_IOR: R |= O; goto NEXT;
    ALU_EOR: R ^= O; goto NEXT;
    ALU_ADD: R += O; goto NEXT;
    ALU_CAR: R = (uint8_t) R + (uint8_t) O > 255 ? 1 : 0; goto NEXT;
    ALU_RLO: R = (R < O) ? 255 : 0; goto NEXT;
    ALU_REO: R = (R == O) ? 255 : 0; goto NEXT;
    ALU_RGO: R = (R > O) ? 255 : 0; goto NEXT;

    /* TRAP */
    TRAP_0: I = PC; D = C; L--; PC = 0; C = 0; goto EXEC;
    TRAP_1: I = PC; D = C; L--; PC = 0; C = 1; goto EXEC;
    TRAP_2: I = PC; D = C; L--; PC = 0; C = 2; goto EXEC;
    TRAP_3: I = PC; D = C; L--; PC = 0; C = 3; goto EXEC;
    TRAP_4: I = PC; D = C; L--; PC = 0; C = 4; goto EXEC;
    TRAP_5: I = PC; D = C; L--; PC = 0; C = 5; goto EXEC;
    TRAP_6: I = PC; D = C; L--; PC = 0; C = 6; goto EXEC;
    TRAP_7: I = PC; D = C; L--; PC = 0; C = 7; goto EXEC;
    TRAP_8: I = PC; D = C; L--; PC = 0; C = 8; goto EXEC;
    TRAP_9: I = PC; D = C; L--; PC = 0; C = 9; goto EXEC;
    TRAP_10: I = PC; D = C; L--; PC = 0; C = 10; goto EXEC;
    TRAP_11: I = PC; D = C; L--; PC = 0; C = 11; goto EXEC;
    TRAP_12: I = PC; D = C; L--; PC = 0; C = 12; goto EXEC;
    TRAP_13: I = PC; D = C; L--; PC = 0; C = 13; goto EXEC;
    TRAP_14: I = PC; D = C; L--; PC = 0; C = 14; goto EXEC;
    TRAP_15: I = PC; D = C; L--; PC = 0; C = 15; goto EXEC;
    TRAP_16: I = PC; D = C; L--; PC = 0; C = 16; goto EXEC;
    TRAP_17: I = PC; D = C; L--; PC = 0; C = 17; goto EXEC;
    TRAP_18: I = PC; D = C; L--; PC = 0; C = 18; goto EXEC;
    TRAP_19: I = PC; D = C; L--; PC = 0; C = 19; goto EXEC;
    TRAP_20: I = PC; D = C; L--; PC = 0; C = 20; goto EXEC;
    TRAP_21: I = PC; D = C; L--; PC = 0; C = 21; goto EXEC;
    TRAP_22: I = PC; D = C; L--; PC = 0; C = 22; goto EXEC;
    TRAP_23: I = PC; D = C; L--; PC = 0; C = 23; goto EXEC;
    TRAP_24: I = PC; D = C; L--; PC = 0; C = 24; goto EXEC;
    TRAP_25: I = PC; D = C; L--; PC = 0; C = 25; goto EXEC;
    TRAP_26: I = PC; D = C; L--; PC = 0; C = 26; goto EXEC;
    TRAP_27: I = PC; D = C; L--; PC = 0; C = 27; goto EXEC;
    TRAP_28: I = PC; D = C; L--; PC = 0; C = 28; goto EXEC;
    TRAP_29: I = PC; D = C; L--; PC = 0; C = 29; goto EXEC;
    TRAP_30: I = PC; D = C; L--; PC = 0; C = 30; goto EXEC;
    TRAP_31: I = PC; D = C; L--; PC = 0; C = 31; goto EXEC;

    /* GIRO */
    GIRO_0G: G = RAM[L][GIRO+0]; goto NEXT;
    GIRO_1G: G = RAM[L][GIRO+1]; goto NEXT;
    GIRO_2G: G = RAM[L][GIRO+2]; goto NEXT;
    GIRO_3G: G = RAM[L][GIRO+3]; goto NEXT;
    GIRO_4G: G = RAM[L][GIRO+4]; goto NEXT;
    GIRO_5G: G = RAM[L][GIRO+5]; goto NEXT;
    GIRO_6G: G = RAM[L][GIRO+6]; goto NEXT;
    GIRO_7G: G = RAM[L][GIRO+7]; goto NEXT;
    GIRO_G0: RAM[L][GIRO+0] = G; goto NEXT;
    GIRO_G1: RAM[L][GIRO+1] = G; goto NEXT;
    GIRO_G2: RAM[L][GIRO+2] = G; goto NEXT;
    GIRO_G3: RAM[L][GIRO+3] = G; goto NEXT;
    GIRO_G4: RAM[L][GIRO+4] = G; goto NEXT;
    GIRO_G5: RAM[L][GIRO+5] = G; goto NEXT;
    GIRO_G6: RAM[L][GIRO+6] = G; goto NEXT;
    GIRO_G7: RAM[L][GIRO+7] = G; goto NEXT;
    GIRO_0I: I = RAM[L][GIRO+0]; goto NEXT;
    GIRO_1I: I = RAM[L][GIRO+1]; goto NEXT;
    GIRO_2I: I = RAM[L][GIRO+2]; goto NEXT;
    GIRO_3I: I = RAM[L][GIRO+3]; goto NEXT;
    GIRO_4I: I = RAM[L][GIRO+4]; goto NEXT;
    GIRO_5I: I = RAM[L][GIRO+5]; goto NEXT;
    GIRO_6I: I = RAM[L][GIRO+6]; goto NEXT;
    GIRO_7I: I = RAM[L][GIRO+7]; goto NEXT;
    GIRO_I0: RAM[L][GIRO+0] = I; goto NEXT;
    GIRO_I1: RAM[L][GIRO+1] = I; goto NEXT;
    GIRO_I2: RAM[L][GIRO+2] = I; goto NEXT;
    GIRO_I3: RAM[L][GIRO+3] = I; goto NEXT;
    GIRO_I4: RAM[L][GIRO+4] = I; goto NEXT;
    GIRO_I5: RAM[L][GIRO+5] = I; goto NEXT;
    GIRO_I6: RAM[L][GIRO+6] = I; goto NEXT;
    GIRO_I7: RAM[L][GIRO+7] = I; goto NEXT;
    GIRO_0R: R = RAM[L][GIRO+0]; goto NEXT;
    GIRO_1R: R = RAM[L][GIRO+1]; goto NEXT;
    GIRO_2R: R = RAM[L][GIRO+2]; goto NEXT;
    GIRO_3R: R = RAM[L][GIRO+3]; goto NEXT;
    GIRO_4R: R = RAM[L][GIRO+4]; goto NEXT;
    GIRO_5R: R = RAM[L][GIRO+5]; goto NEXT;
    GIRO_6R: R = RAM[L][GIRO+6]; goto NEXT;
    GIRO_7R: R = RAM[L][GIRO+7]; goto NEXT;
    GIRO_R0: RAM[L][GIRO+0] = R; goto NEXT;
    GIRO_R1: RAM[L][GIRO+1] = R; goto NEXT;
    GIRO_R2: RAM[L][GIRO+2] = R; goto NEXT;
    GIRO_R3: RAM[L][GIRO+3] = R; goto NEXT;
    GIRO_R4: RAM[L][GIRO+4] = R; goto NEXT;
    GIRO_R5: RAM[L][GIRO+5] = R; goto NEXT;
    GIRO_R6: RAM[L][GIRO+6] = R; goto NEXT;
    GIRO_R7: RAM[L][GIRO+7] = R; goto NEXT;
    GIRO_0O: O = RAM[L][GIRO+0]; goto NEXT;
    GIRO_1O: O = RAM[L][GIRO+1]; goto NEXT;
    GIRO_2O: O = RAM[L][GIRO+2]; goto NEXT;
    GIRO_3O: O = RAM[L][GIRO+3]; goto NEXT;
    GIRO_4O: O = RAM[L][GIRO+4]; goto NEXT;
    GIRO_5O: O = RAM[L][GIRO+5]; goto NEXT;
    GIRO_6O: O = RAM[L][GIRO+6]; goto NEXT;
    GIRO_7O: O = RAM[L][GIRO+7]; goto NEXT;
    GIRO_O0: RAM[L][GIRO+0] = O; goto NEXT;
    GIRO_O1: RAM[L][GIRO+1] = O; goto NEXT;
    GIRO_O2: RAM[L][GIRO+2] = O; goto NEXT;
    GIRO_O3: RAM[L][GIRO+3] = O; goto NEXT;
    GIRO_O4: RAM[L][GIRO+4] = O; goto NEXT;
    GIRO_O5: RAM[L][GIRO+5] = O; goto NEXT;
    GIRO_O6: RAM[L][GIRO+6] = O; goto NEXT;
    GIRO_O7: RAM[L][GIRO+7] = O; goto NEXT;

    /* PAIR */
    PAIR_NO: O = RAM[C][PC]; goto NEXT;
    SCROUNGE_NM: goto NEXT;
    SCROUNGE_NL: goto NEXT;
    PAIR_NG: G = RAM[C][PC]; goto NEXT;
    PAIR_NR: R = RAM[C][PC]; goto NEXT;
    PAIR_NI: I = RAM[C][PC]; goto NEXT;
    PAIR_NS: SOR = RAM[C][PC]; goto NEXT;
    PAIR_NP: POR = RAM[C][PC]; goto NEXT;
    PAIR_NE: E = RAM[C][PC]; goto NEXT;
    PAIR_NA: TEMP = O + RAM[C][PC]; O = (uint8_t)(TEMP & 0xFF); if (TEMP>0xFF) G++; goto NEXT;
    PAIR_NB: PC += RAM[C][PC]; goto EXEC;
    PAIR_NJ: PC = RAM[C][PC]; goto EXEC;
    PAIR_NW: if (I--) {PC = RAM[C][PC]; goto EXEC;} else goto NEXT;
    PAIR_NT: if (R) {PC = RAM[C][PC]; goto EXEC;} else goto NEXT;
    PAIR_NF: if (!R) {PC = RAM[C][PC]; goto EXEC;} else goto NEXT;
    PAIR_NC: I = PC; D = C; L--; PC = 0; C = RAM[C][PC]; goto EXEC;
    PAIR_MO: O = RAM[G][O]; goto NEXT;
    SCROUNGE_MM: goto NEXT;
    SCROUNGE_ML: goto NEXT;
    PAIR_MG: G = RAM[G][O]; goto NEXT;
    PAIR_MR: R = RAM[G][O]; goto NEXT;
    PAIR_MI: I = RAM[G][O]; goto NEXT;
    PAIR_MS: SOR = RAM[G][O]; goto NEXT;
    PAIR_MP: POR = RAM[G][O]; goto NEXT;
    PAIR_ME: E = RAM[G][O]; goto NEXT;
    PAIR_MA: TEMP = O + RAM[G][O]; O = (uint8_t)(TEMP & 0xFF); if (TEMP>0xFF) G++; goto NEXT;
    PAIR_MB: PC += RAM[G][O]; goto EXEC;
    PAIR_MJ: PC = RAM[G][O]; goto EXEC;
    PAIR_MW: if (I--) {PC = RAM[G][O]; goto EXEC;} else goto NEXT;
    PAIR_MT: if (R) {PC = RAM[G][O]; goto EXEC;} else goto NEXT;
    PAIR_MF: if (!R) {PC = RAM[G][O]; goto EXEC;} else goto NEXT;
    PAIR_MC: I = PC; D = C; L--; PC = 0; C = RAM[G][O]; goto EXEC;
    PAIR_LO: O = RAM[L][O]; goto NEXT;
    SCROUNGE_LM: goto NEXT;
    SCROUNGE_LL: goto NEXT;
    PAIR_LG: G = RAM[L][O]; goto NEXT;
    PAIR_LR: R = RAM[L][O]; goto NEXT;
    PAIR_LI: I = RAM[L][O]; goto NEXT;
    PAIR_LS: SOR = RAM[L][O]; goto NEXT;
    PAIR_LP: POR = RAM[L][O]; goto NEXT;
    PAIR_LE: E = RAM[L][O]; goto NEXT;
    PAIR_LA: TEMP = O + RAM[L][O]; O = (uint8_t)(TEMP & 0xFF); if (TEMP>0xFF) G++; goto NEXT;
    PAIR_LB: PC += RAM[L][O]; goto EXEC;
    PAIR_LJ: PC = RAM[L][O]; goto EXEC;
    PAIR_LW: if (I--) {PC = RAM[L][O]; goto EXEC;} else goto NEXT;
    PAIR_LT: if (R) {PC = RAM[L][O]; goto EXEC;} else goto NEXT;
    PAIR_LF: if (!R) {PC = RAM[L][O]; goto EXEC;} else goto NEXT;
    PAIR_LC: I = PC; D = C; L--; PC = 0; C = RAM[L][O]; goto EXEC;
    PAIR_GO: O = G; goto NEXT;
    PAIR_GM: RAM[G][O] = G; goto NEXT;
    PAIR_GL: RAM[L][O] = G; goto NEXT;
    SCROUNGE_GG: goto NEXT;
    PAIR_GR: R = G; goto NEXT;
    PAIR_GI: I = G; goto NEXT;
    PAIR_GS: SOR = G; goto NEXT;
    PAIR_GP: POR = G; goto NEXT;
    PAIR_GE: E = G; goto NEXT;
    PAIR_GA: TEMP = O + G; O = (uint8_t)(TEMP & 0xFF); if (TEMP>0xFF) G++; goto NEXT;
    PAIR_GB: PC += G; goto EXEC;
    PAIR_GJ: PC = G; goto EXEC;
    PAIR_GW: if (I--) {PC = G; goto EXEC;} else goto NEXT;
    PAIR_GT: if (R) {PC = G; goto EXEC;} else goto NEXT;
    PAIR_GF: if (!R) {PC = G; goto EXEC;} else goto NEXT;
    PAIR_GC: I = PC; D = C; L--; PC = 0; C = G; goto EXEC;
    PAIR_RO: O = R; goto NEXT;
    PAIR_RM: RAM[G][O] = R; goto NEXT;
    PAIR_RL: RAM[L][O] = R; goto NEXT;
    PAIR_RG: G = R; goto NEXT;
    SCROUNGE_RR: goto NEXT;
    PAIR_RI: I = R; goto NEXT;
    PAIR_RS: SOR = R; goto NEXT;
    PAIR_RP: POR = R; goto NEXT;
    PAIR_RE: E = R; goto NEXT;
    PAIR_RA: TEMP = O + R; O = (uint8_t)(TEMP & 0xFF); if (TEMP>0xFF) G++; goto NEXT;
    PAIR_RB: PC += R; goto EXEC;
    PAIR_RJ: PC = R; goto EXEC;
    PAIR_RW: if (I--) {PC = R; goto EXEC;} else goto NEXT;
    PAIR_RT: if (R) {PC = R; goto EXEC;} else goto NEXT;
    PAIR_RF: if (!R) {PC = R; goto EXEC;} else goto NEXT;
    PAIR_RC: I = PC; D = C; L--; PC = 0; C = R; goto EXEC;
    PAIR_IO: O = I; goto NEXT;
    PAIR_IM: RAM[G][O] = I; goto NEXT;
    PAIR_IL: RAM[L][O] = I; goto NEXT;
    PAIR_IG: G = I; goto NEXT;
    PAIR_IR: R = I; goto NEXT;
    SCROUNGE_II: goto NEXT;
    PAIR_IS: SOR = I; goto NEXT;
    PAIR_IP: POR = I; goto NEXT;
    PAIR_IE: E = I; goto NEXT;
    PAIR_IA: TEMP = O + I; O = (uint8_t)(TEMP & 0xFF); if (TEMP>0xFF) G++; goto NEXT;
    PAIR_IB: PC += I; goto EXEC;
    PAIR_IJ: PC = I; goto EXEC;
    PAIR_IW: if (I--) {PC = I; goto EXEC;} else goto NEXT;
    PAIR_IT: if (R) {PC = I; goto EXEC;} else goto NEXT;
    PAIR_IF: if (!R) {PC = I; goto EXEC;} else goto NEXT;
    PAIR_IC: I = PC; D = C; L--; PC = 0; C = I; goto EXEC;
    PAIR_SO: O = SIR; goto NEXT;
    PAIR_SM: RAM[G][O] = SIR; goto NEXT;
    PAIR_SL: RAM[L][O] = SIR; goto NEXT;
    PAIR_SG: G = SIR; goto NEXT;
    PAIR_SR: R = SIR; goto NEXT;
    PAIR_SI: I = SIR; goto NEXT;
    PAIR_SS: SOR = SIR; goto NEXT;
    PAIR_SP: POR = SIR; goto NEXT;
    PAIR_SE: E = SIR; goto NEXT;
    PAIR_SA: TEMP = O + SIR; O = (uint8_t)(TEMP & 0xFF); if (TEMP>0xFF) G++; goto NEXT;
    PAIR_SB: PC += SIR; goto EXEC;
    PAIR_SJ: PC = SIR; goto EXEC;
    PAIR_SW: if (I--) {PC = SIR; goto EXEC;} else goto NEXT;
    PAIR_ST: if (R) {PC = SIR; goto EXEC;} else goto NEXT;
    PAIR_SF: if (!R) {PC = SIR; goto EXEC;} else goto NEXT;
    PAIR_SC: I = PC; D = C; L--; PC = 0; C = SIR; goto EXEC;
    PAIR_PO: O = PIR; goto NEXT;
    PAIR_PM: RAM[G][O] = PIR; goto NEXT;
    PAIR_PL: RAM[L][O] = PIR; goto NEXT;
    PAIR_PG: G = PIR; goto NEXT;
    PAIR_PR: R = PIR; goto NEXT;
    PAIR_PI: I = PIR; goto NEXT;
    PAIR_PS: SOR = PIR; goto NEXT;
    PAIR_PP: POR = PIR; goto NEXT;
    PAIR_PE: E = PIR; goto NEXT;
    PAIR_PA: TEMP = O + PIR; O = (uint8_t)(TEMP & 0xFF); if (TEMP>0xFF) G++; goto NEXT;
    PAIR_PB: PC += PIR; goto EXEC;
    PAIR_PJ: PC = PIR; goto EXEC;
    PAIR_PW: if (I--) {PC = PIR; goto EXEC;} else goto NEXT;
    PAIR_PT: if (R) {PC = PIR; goto EXEC;} else goto NEXT;
    PAIR_PF: if (!R) {PC = PIR; goto EXEC;} else goto NEXT;
    PAIR_PC: I = PC; D = C; L--; PC = 0; C = PIR; goto EXEC;
}





