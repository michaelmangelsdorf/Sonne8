
/* Sonne CPU core, rev. Daffodil
   Verilog Implementation
   Jan-2024 Michael Mangelsdorf
   <mim@ok-schalter.de>
*/

module daffo_core
(
  input rst,
  input clk,

  output reg[15:0] maddr,
  output reg[7:0] mdata_put,
  output reg mwren,
  input [7:0] mdata_get,
    
  /* SD/SPI */

  output reg sd_clk,
  output reg sd_mosi,
  output reg sd_cs,
  input      sd_miso,

  /* MISC */

  input  [31:0] gpio,
  output reg [31:0] gpio_out,

  output reg [6:0] seg7_1,
  output reg [6:0] seg7_2,
  output reg [6:0] seg7_3,
  output reg [6:0] seg7_4,
  output reg [6:0] seg7_5,
  output reg [6:0] seg7_6,
  
  output reg [9:0] led,
  input [13:0] keysw

);


localparam opc_NOP = 8'b00000000; /*00*/
localparam opc_RET = 8'b00000001; /*01*/
localparam opc_NH = 8'b00000010; /*02*/
localparam opc_NG = 8'b00000011; /*03*/
localparam opc_NL = 8'b00000100; /*04*/
localparam opc_NS = 8'b00000101; /*05*/
localparam opc_NP = 8'b00000110; /*06*/
localparam opc_NR = 8'b00000111; /*07*/
localparam opc_ND = 8'b00001000; /*08*/
localparam opc_NJ = 8'b00001001; /*09*/
localparam opc_NT = 8'b00001010; /*0A*/
localparam opc_NF = 8'b00001011; /*0B*/
localparam opc_NC = 8'b00001100; /*0C*/
localparam opc_R0P = 8'b00001101; /*0D*/
localparam opc_NA = 8'b00001110; /*0E*/
localparam opc_NB = 8'b00001111; /*0F*/
localparam opc_SSI = 8'b00010000; /*10*/
localparam opc_LID = 8'b00010001; /*11*/
localparam opc_MH = 8'b00010010; /*12*/
localparam opc_MG = 8'b00010011; /*13*/
localparam opc_ML = 8'b00010100; /*14*/
localparam opc_MS = 8'b00010101; /*15*/
localparam opc_MP = 8'b00010110; /*16*/
localparam opc_MR = 8'b00010111; /*17*/
localparam opc_MD = 8'b00011000; /*18*/
localparam opc_MJ = 8'b00011001; /*19*/
localparam opc_MT = 8'b00011010; /*1A*/
localparam opc_MF = 8'b00011011; /*1B*/
localparam opc_MC = 8'b00011100; /*1C*/
localparam opc_R1P = 8'b00011101; /*1D*/
localparam opc_MA = 8'b00011110; /*1E*/
localparam opc_MB = 8'b00011111; /*1F*/
localparam opc_SSO = 8'b00100000; /*20*/
localparam opc_HM = 8'b00100001; /*21*/
localparam opc_CLIPA = 8'b00100010; /*22*/
localparam opc_HG = 8'b00100011; /*23*/
localparam opc_HL = 8'b00100100; /*24*/
localparam opc_HS = 8'b00100101; /*25*/
localparam opc_HP = 8'b00100110; /*26*/
localparam opc_HR = 8'b00100111; /*27*/
localparam opc_HD = 8'b00101000; /*28*/
localparam opc_HJ = 8'b00101001; /*29*/
localparam opc_HT = 8'b00101010; /*2A*/
localparam opc_HF = 8'b00101011; /*2B*/
localparam opc_HC = 8'b00101100; /*2C*/
localparam opc_R2P = 8'b00101101; /*2D*/
localparam opc_HA = 8'b00101110; /*2E*/
localparam opc_HB = 8'b00101111; /*2F*/
localparam opc_SCL = 8'b00110000; /*30*/
localparam opc_GM = 8'b00110001; /*31*/
localparam opc_GH = 8'b00110010; /*32*/
localparam opc_CLIPB = 8'b00110011; /*33*/
localparam opc_GL = 8'b00110100; /*34*/
localparam opc_GS = 8'b00110101; /*35*/
localparam opc_GP = 8'b00110110; /*36*/
localparam opc_GR = 8'b00110111; /*37*/
localparam opc_GD = 8'b00111000; /*38*/
localparam opc_GJ = 8'b00111001; /*39*/
localparam opc_GT = 8'b00111010; /*3A*/
localparam opc_GF = 8'b00111011; /*3B*/
localparam opc_GC = 8'b00111100; /*3C*/
localparam opc_R3P = 8'b00111101; /*3D*/
localparam opc_GA = 8'b00111110; /*3E*/
localparam opc_GB = 8'b00111111; /*3F*/
localparam opc_SCH = 8'b01000000; /*40*/
localparam opc_LM = 8'b01000001; /*41*/
localparam opc_LH = 8'b01000010; /*42*/
localparam opc_LG = 8'b01000011; /*43*/
localparam opc_LL = 8'b01000100; /*44*/
localparam opc_LS = 8'b01000101; /*45*/
localparam opc_LP = 8'b01000110; /*46*/
localparam opc_LR = 8'b01000111; /*47*/
localparam opc_LD = 8'b01001000; /*48*/
localparam opc_LJ = 8'b01001001; /*49*/
localparam opc_LT = 8'b01001010; /*4A*/
localparam opc_LF = 8'b01001011; /*4B*/
localparam opc_LC = 8'b01001100; /*4C*/
localparam opc_R4M = 8'b01001101; /*4D*/
localparam opc_LA = 8'b01001110; /*4E*/
localparam opc_LB = 8'b01001111; /*4F*/
localparam opc_HIZ = 8'b01010000; /*50*/
localparam opc_SM = 8'b01010001; /*51*/
localparam opc_SH = 8'b01010010; /*52*/
localparam opc_SG = 8'b01010011; /*53*/
localparam opc_SL = 8'b01010100; /*54*/
localparam opc_SS = 8'b01010101; /*55*/
localparam opc_SP = 8'b01010110; /*56*/
localparam opc_SR = 8'b01010111; /*57*/
localparam opc_SD = 8'b01011000; /*58*/
localparam opc_SJ = 8'b01011001; /*59*/
localparam opc_ST = 8'b01011010; /*5A*/
localparam opc_SF = 8'b01011011; /*5B*/
localparam opc_SC = 8'b01011100; /*5C*/
localparam opc_R3M = 8'b01011101; /*5D*/
localparam opc_SA = 8'b01011110; /*5E*/
localparam opc_SB = 8'b01011111; /*5F*/
localparam opc_LEAVE = 8'b01100000; /*60*/
localparam opc_PM = 8'b01100001; /*61*/
localparam opc_PH = 8'b01100010; /*62*/
localparam opc_PG = 8'b01100011; /*63*/
localparam opc_PL = 8'b01100100; /*64*/
localparam opc_PS = 8'b01100101; /*65*/
localparam opc_PP = 8'b01100110; /*66*/
localparam opc_PR = 8'b01100111; /*67*/
localparam opc_PD = 8'b01101000; /*68*/
localparam opc_PJ = 8'b01101001; /*69*/
localparam opc_PT = 8'b01101010; /*6A*/
localparam opc_PF = 8'b01101011; /*6B*/
localparam opc_PC = 8'b01101100; /*6C*/
localparam opc_R2M = 8'b01101101; /*6D*/
localparam opc_PA = 8'b01101110; /*6E*/
localparam opc_PB = 8'b01101111; /*6F*/
localparam opc_ENTER = 8'b01110000; /*70*/
localparam opc_RM = 8'b01110001; /*71*/
localparam opc_RH = 8'b01110010; /*72*/
localparam opc_RG = 8'b01110011; /*73*/
localparam opc_RL = 8'b01110100; /*74*/
localparam opc_RS = 8'b01110101; /*75*/
localparam opc_RP = 8'b01110110; /*76*/
localparam opc_RR = 8'b01110111; /*77*/
localparam opc_RD = 8'b01111000; /*78*/
localparam opc_RJ = 8'b01111001; /*79*/
localparam opc_RT = 8'b01111010; /*7A*/
localparam opc_RF = 8'b01111011; /*7B*/
localparam opc_RC = 8'b01111100; /*7C*/
localparam opc_R1M = 8'b01111101; /*7D*/
localparam opc_RA = 8'b01111110; /*7E*/
localparam opc_RB = 8'b01111111; /*7F*/
localparam opc_G0A = 8'b11000000; /*C0*/
localparam opc_G1A = 8'b11000001; /*C1*/
localparam opc_G2A = 8'b11000010; /*C2*/
localparam opc_G3A = 8'b11000011; /*C3*/
localparam opc_L0A = 8'b11000100; /*C4*/
localparam opc_L1A = 8'b11000101; /*C5*/
localparam opc_L2A = 8'b11000110; /*C6*/
localparam opc_L3A = 8'b11000111; /*C7*/
localparam opc_AG0 = 8'b11001000; /*C8*/
localparam opc_AG1 = 8'b11001001; /*C9*/
localparam opc_AG2 = 8'b11001010; /*CA*/
localparam opc_AG3 = 8'b11001011; /*CB*/
localparam opc_AL0 = 8'b11001100; /*CC*/
localparam opc_AL1 = 8'b11001101; /*CD*/
localparam opc_AL2 = 8'b11001110; /*CE*/
localparam opc_AL3 = 8'b11001111; /*CF*/
localparam opc_G0B = 8'b11010000; /*D0*/
localparam opc_G1B = 8'b11010001; /*D1*/
localparam opc_G2B = 8'b11010010; /*D2*/
localparam opc_G3B = 8'b11010011; /*D3*/
localparam opc_L0B = 8'b11010100; /*D4*/
localparam opc_L1B = 8'b11010101; /*D5*/
localparam opc_L2B = 8'b11010110; /*D6*/
localparam opc_L3B = 8'b11010111; /*D7*/
localparam opc_BG0 = 8'b11011000; /*D8*/
localparam opc_BG1 = 8'b11011001; /*D9*/
localparam opc_BG2 = 8'b11011010; /*DA*/
localparam opc_BG3 = 8'b11011011; /*DB*/
localparam opc_BL0 = 8'b11011100; /*DC*/
localparam opc_BL1 = 8'b11011101; /*DD*/
localparam opc_BL2 = 8'b11011110; /*DE*/
localparam opc_BL3 = 8'b11011111; /*DF*/
localparam opc_G0R = 8'b11100000; /*E0*/
localparam opc_G1R = 8'b11100001; /*E1*/
localparam opc_G2R = 8'b11100010; /*E2*/
localparam opc_G3R = 8'b11100011; /*E3*/
localparam opc_L0R = 8'b11100100; /*E4*/
localparam opc_L1R = 8'b11100101; /*E5*/
localparam opc_L2R = 8'b11100110; /*E6*/
localparam opc_L3R = 8'b11100111; /*E7*/
localparam opc_RG0 = 8'b11101000; /*E8*/
localparam opc_RG1 = 8'b11101001; /*E9*/
localparam opc_RG2 = 8'b11101010; /*EA*/
localparam opc_RG3 = 8'b11101011; /*EB*/
localparam opc_RL0 = 8'b11101100; /*EC*/
localparam opc_RL1 = 8'b11101101; /*ED*/
localparam opc_RL2 = 8'b11101110; /*EE*/
localparam opc_RL3 = 8'b11101111; /*EF*/
localparam opc_IDA = 8'b11110000; /*F0*/
localparam opc_IDB = 8'b11110001; /*F1*/
localparam opc_OCA = 8'b11110010; /*F2*/
localparam opc_OCB = 8'b11110011; /*F3*/
localparam opc_SLA = 8'b11110100; /*F4*/
localparam opc_SLB = 8'b11110101; /*F5*/
localparam opc_SRA = 8'b11110110; /*F6*/
localparam opc_SRB = 8'b11110111; /*F7*/
localparam opc_AND = 8'b11111000; /*F8*/
localparam opc_IOR = 8'b11111001; /*F9*/
localparam opc_EOR = 8'b11111010; /*FA*/
localparam opc_ADD = 8'b11111011; /*FB*/
localparam opc_CAR = 8'b11111100; /*FC*/
localparam opc_ALB = 8'b11111101; /*FD*/
localparam opc_AEB = 8'b11111110; /*FE*/
localparam opc_AGB = 8'b11111111; /*FF*/


function [6:0] hex7;
input [3:0] nybble;
begin
  case (nybble)
  0: hex7 = 7'h3F;
  1: hex7 = 7'h06;
  2: hex7 = 7'h5B;
  3: hex7 = 7'h4F;
  4: hex7 = 7'h66;
  5: hex7 = 7'h6D;
  6: hex7 = 7'h7D;
  7: hex7 = 7'h07;
  8: hex7 = 7'h7F;
  9: hex7 = 7'h67;
  10: hex7 = 7'h77;
  11: hex7 = 7'h7C;
  12: hex7 = 7'h39;
  13: hex7 = 7'h5E;
  14: hex7 = 7'h79;
  15: hex7 = 7'h71;
  endcase
  hex7 = hex7 ^ 7'b1111111;
 end
endfunction

task seg7Byte1;
input [7:0] byt;
begin
 seg7_1 = hex7(byt[3:0]);
 seg7_2 = hex7(byt[7:4]);
end
endtask

task seg7Byte2;
input [7:0] byt;
begin
 seg7_3 = hex7(byt[3:0]);
 seg7_4 = hex7(byt[7:4]);
end
endtask

task seg7Byte3;
input [7:0] byt;
begin
 seg7_5 = hex7(byt[3:0]);
 seg7_6 = hex7(byt[7:4]);
end
endtask



/* CPU registers */

reg [7:0] pc_low;
reg [7:0] pc_high;
reg [7:0] reg_H;
reg [7:0] reg_G;
reg [7:0] reg_L;
reg [7:0] reg_D;
reg [7:0] par_ir;
reg [7:0] par_or;
reg [7:0] ser_ir;
reg [7:0] ser_or;
reg [0:0] par_hiz;


/* MISC */

reg [7:0] opcode;
reg [7:0] source_val;
reg [0:0] stop;
reg [2:0] cpu_phase;

reg ['hF:0] xMx;
reg ['h7:0] reg_R;
reg ['h7:0] reg_R_offs;
reg ['h7:0] reg_A;
reg ['h7:0] reg_B;
reg ['h7:0] reg_CLIP;
reg ['h8:0] temp9bits;
    


always @(posedge clk)
case (cpu_phase)
0: cpu_phase = 1;
1: cpu_phase = 2;
2: cpu_phase = 3;
3: cpu_phase = 0;
endcase


 



always@(posedge clk)
//if (!stop)
case (cpu_phase)
      
    0: begin
       mwren = 0;
      maddr = {pc_high,pc_low};
      pc_low = pc_low + 8'd1;
      end
      
    1: begin
       opcode = mdata_get;
      /*DBG*/ // if (pc_high==1) begin reg_A = pc_low; pc_low = 15; pc_high=0; end
       end

    /* -------------- READ PHASE ---------------------- */
  
    2: case (opcode)

           opc_NOP: ;
        
           opc_RET: /* Scrounge NM */
        begin
          pc_low =  reg_R;
         pc_high = reg_H;
        end
           
        opc_NH, opc_NG, opc_NL, opc_NS,
           opc_NP, opc_NR, opc_ND, opc_NJ,
           opc_NT, opc_NF, opc_NC, opc_NA,
           opc_NB:
        begin
          maddr = {pc_high,pc_low};
         pc_low = pc_low + 8'd1;
        end

           opc_R0P: reg_R_offs = 0;
        
   /**/    opc_SSI:
        begin
          ser_ir = ser_ir << 1;
          ser_ir = ser_ir | sd_miso;
        end
        
           opc_LID: /* Scrounge MM */
        begin
          pc_low = 0;
         pc_high = pc_high + 8'd1;
         reg_H = pc_high;
        end
           
        opc_MH, opc_MG, opc_ML, opc_MS,
           opc_MP, opc_MR, opc_MD, opc_MJ,
           opc_MT, opc_MF, opc_MC, opc_MA,
           opc_MB:
        begin
          maddr = xMx;
        end

           opc_R1P: reg_R_offs = 1;
        
   /**/    opc_SSO:
        begin
          sd_mosi = ser_or[7];
          ser_or = ser_or << 1;       
        end
        
           opc_HM:
        begin
          maddr = xMx;
         mdata_put = reg_H;
         mwren = 1;
        end
        
           opc_CLIPA:/* Scrounge HH */
        begin
         reg_A = reg_CLIP;
         case (reg_A[7:6])
          3: xMx = {reg_L,reg_A};
          2: xMx = {reg_G,reg_A};
          default: xMx = {reg_H,reg_A};
         endcase 
        end
        
           opc_HG: reg_G = reg_H;
           opc_HL: reg_L = reg_H; 
           opc_HS: ser_or = reg_H;
           opc_HP: begin par_or = reg_H; par_hiz = 0; end
           
        opc_HR:
        begin
             reg_R = reg_H;
            reg_R_offs = 0;
        end

           opc_HD: reg_D = reg_H;
           opc_HJ: pc_low = reg_H;
          opc_HT: if (reg_R + reg_R_offs != 8'b0) pc_low = reg_H;
        opc_HF: if (reg_R + reg_R_offs == 8'b0) pc_low = reg_H;
        
           opc_HC:
        begin
             reg_R = pc_low;
            reg_R_offs = 0;
          pc_low = 0;
         pc_high = reg_H;
        end
           
        opc_R2P: reg_R_offs = 2;
        
           opc_HA:
        begin
         reg_CLIP = reg_A;
         reg_A = reg_H;
         case (reg_A[7:6])
          3: xMx = {reg_L,reg_A};
          2: xMx = {reg_G,reg_A};
          default: xMx = {reg_H,reg_A};
         endcase 
        end
           
        opc_HB:
        begin
         reg_CLIP = reg_B;
         reg_B = reg_H;
         case (reg_B[7:6])
          3: xMx = {reg_L,reg_B};
          2: xMx = {reg_G,reg_B};
          default: xMx = {reg_H,reg_B};
         endcase 
        end  

   /**/    opc_SCL: sd_clk = 0;
           
        opc_GM:
        begin
          maddr = xMx;
         mdata_put = reg_G;
         mwren = 1;
        end
           
        opc_GH: reg_H = reg_G;
           
        opc_CLIPB: /* Scrounge GG */
        begin
         reg_B = reg_CLIP;
         case (reg_B[7:6])
          3: xMx = {reg_L,reg_B};
          2: xMx = {reg_G,reg_B};
          default: xMx = {reg_H,reg_B};
         endcase 
        end
           
        opc_GL: reg_L = reg_G;
           opc_GS: ser_ir = reg_G;
           opc_GP: begin par_or = reg_G; par_hiz = 0; end

           opc_GR:
        begin
             reg_R = reg_G;
            reg_R_offs = 0;
        end

           opc_GD: reg_D = reg_G;
           opc_GJ: pc_low = reg_G;
          opc_GT: if (reg_R + reg_R_offs != 8'b0) pc_low = reg_G;
        opc_GF: if (reg_R + reg_R_offs == 8'b0) pc_low = reg_G;

           opc_GC:
        begin
             reg_R = pc_low;
            reg_R_offs = 0;
          pc_low = 0;
         reg_H = pc_high;
         pc_high = reg_G;
        end
        
           opc_R3P: reg_R_offs = 3;
           
        opc_GA:
        begin
         reg_CLIP = reg_A;
         reg_A = reg_G;
         case (reg_A[7:6])
          3: xMx = {reg_L,reg_A};
          2: xMx = {reg_G,reg_A};
          default: xMx = {reg_H,reg_A};
         endcase 
        end
           
        opc_GB:
        begin
         reg_CLIP = reg_B;
         reg_B = reg_G;
         case (reg_B[7:6])
          3: xMx = {reg_L,reg_B};
          2: xMx = {reg_G,reg_B};
          default: xMx = {reg_H,reg_B};
         endcase 
        end

   /**/    opc_SCH: sd_clk = 1;
        
           opc_LM:
        begin
          maddr = xMx;
         mdata_put = reg_L;
         mwren = 1;
        end
           
        opc_LH: reg_H = reg_L;
           opc_LG: reg_G = reg_L;
           opc_LL: ;
           opc_LS: ser_ir = reg_L;
           opc_LP: begin par_or = reg_L; par_hiz = 0; end

           opc_LR:
        begin
             reg_R = reg_L;
            reg_R_offs = 0;
        end

           opc_LD: reg_D = reg_L;
           opc_LJ: pc_low = reg_L;
          opc_LT: if (reg_R + reg_R_offs != 8'b0) pc_low = reg_L;
        opc_LF: if (reg_R + reg_R_offs == 8'b0) pc_low = reg_L;

           opc_LC:
        begin
             reg_R = pc_low;
            reg_R_offs = 0;
          pc_low = 0;
         reg_H = pc_high;
         pc_high = reg_L;
        end
        
           opc_R4M: reg_R_offs = (8'd4 ^ 8'hFFFF) + 8'd1;

           
        opc_LA:
        begin
         reg_CLIP = reg_A;
         reg_A = reg_L;
         case (reg_A[7:6])
          3: xMx = {reg_L,reg_A};
          2: xMx = {reg_G,reg_A};
          default: xMx = {reg_H,reg_A};
         endcase 
        end
           
        opc_LB:
        begin
         reg_CLIP = reg_B;
         reg_B = reg_L;
         case (reg_B[7:6])
          3: xMx = {reg_L,reg_B};
          2: xMx = {reg_G,reg_B};
          default: xMx = {reg_H,reg_B};
         endcase 
        end

   /**/    opc_HIZ:
           begin
        par_hiz = 1;
        seg7Byte2(reg_A);
        seg7Byte3(reg_B);
        end
  
           opc_SM:
        begin
          maddr = xMx;
         mdata_put = ser_or;
         mwren = 1;
        end

           opc_SH: reg_H = ser_or;
           opc_SG: reg_G = ser_or;
           opc_SL: reg_L = ser_or;
           opc_SS: ;
           opc_SP: begin par_or = ser_or; par_hiz = 0; end

           opc_SR:
        begin
             reg_R = ser_or;
            reg_R_offs = 0;
        end

           opc_SD: reg_D = ser_or;
           opc_SJ: pc_low = ser_or;
          opc_ST: if (reg_R + reg_R_offs != 8'b0) pc_low = ser_or;
        opc_SF: if (reg_R + reg_R_offs == 8'b0) pc_low = ser_or;

           opc_SC:
        begin
             reg_R = pc_low;
            reg_R_offs = 0;
          pc_low = 0;
         reg_H = pc_high;
         pc_high = ser_or;
        end

           opc_R3M: reg_R_offs = (8'd3 ^ 8'hFFFF) + 8'd1;

        opc_SA:
        begin
         reg_CLIP = reg_A;
         reg_A = ser_or;
         case (reg_A[7:6])
          3: xMx = {reg_L,reg_A};
          2: xMx = {reg_G,reg_A};
          default: xMx = {reg_H,reg_A};
         endcase 
        end
           
        opc_SB:
        begin
         reg_CLIP = reg_B;
         reg_B = ser_or;
         case (reg_B[7:6])
          3: xMx = {reg_L,reg_B};
          2: xMx = {reg_G,reg_B};
          default: xMx = {reg_H,reg_B};
         endcase 
        end

   /**/    opc_LEAVE: maddr = {reg_L,8'hC4};

           opc_PM:
        begin
          maddr = xMx;
         mdata_put = par_ir;
         mwren = 1;
        end
   
           opc_PH: reg_H = par_ir;
           opc_PG: reg_G = par_ir;
           opc_PL: reg_L = par_ir;
           opc_PS: ser_or = par_ir;
           opc_PP: ;

           opc_PR:
        begin
             reg_R = par_ir;
            reg_R_offs = 0;
        end

           opc_PD: reg_D = par_ir;
           opc_PJ: pc_low = par_ir;
          opc_PT: if (reg_R + reg_R_offs != 8'b0) pc_low = par_ir;
        opc_PF: if (reg_R + reg_R_offs == 8'b0) pc_low = par_ir;

           opc_PC:
        begin
             reg_R = pc_low;
            reg_R_offs = 0;
          pc_low = 0;
         reg_H = pc_high;
         pc_high = par_ir;
        end
        
           opc_R2M: reg_R_offs = (8'd2 ^ 8'hFFFF) + 8'd1;
           
        opc_PA:
        begin
         reg_CLIP = reg_A;
         reg_A = par_ir;
         case (reg_A[7:6])
          3: xMx = {reg_L,reg_A};
          2: xMx = {reg_G,reg_A};
          default: xMx = {reg_H,reg_A};
         endcase 
        end
           
        opc_PB:
        begin
         reg_CLIP = reg_B;
         reg_B = par_ir;
         case (reg_B[7:6])
          3: xMx = {reg_L,reg_B};
          2: xMx = {reg_G,reg_B};
          default: xMx = {reg_H,reg_B};
         endcase 
        end

   /**/    opc_ENTER:
        begin
          reg_L = reg_L - 8'd1;;
          maddr = {reg_L,8'hC4};
             mdata_put = reg_R + reg_R_offs;
         mwren = 1;
        end
  
           opc_RM:
        begin
          maddr = xMx;
         mdata_put = reg_R + reg_R_offs;
         mwren = 1;
        end
   
           opc_RH: reg_H = reg_R + reg_R_offs;
           opc_RG: reg_G = reg_R + reg_R_offs;
           opc_RL: reg_L = reg_R + reg_R_offs;
           opc_RS: ser_ir = reg_R + reg_R_offs;
           opc_RP: begin par_or = reg_R + reg_R_offs; par_hiz = 0; end
           opc_RR: ;
           opc_RD: reg_D = reg_R + reg_R_offs;
           opc_RJ: pc_low = reg_R + reg_R_offs;
          opc_RT: if (reg_R + reg_R_offs != 8'b0) pc_low = reg_R + reg_R_offs;
        opc_RF: if (reg_R + reg_R_offs == 8'b0) pc_low = reg_R + reg_R_offs;

           opc_RC:
        begin
         reg_H = pc_high;
         pc_high = reg_R + reg_R_offs;
             reg_R = pc_low;
            reg_R_offs = 0;
          pc_low = 0;
        end

           opc_R1M: reg_R_offs = (8'd1 ^ 8'hFFFF) + 8'd1;

           opc_RA:
        begin
         reg_CLIP = reg_A;
         reg_A = reg_R + reg_R_offs;
         case (reg_A[7:6])
          3: xMx = {reg_L,reg_A};
          2: xMx = {reg_G,reg_A};
          default: xMx = {reg_H,reg_A};
         endcase
           //if (reg_A == 0) begin pc_low = 15; seg7Byte1(reg_A == 0 ? 1:0); end /*DBG*/
        end
        
           opc_RB:
        begin
         reg_CLIP = reg_B;
         reg_B = reg_R + reg_R_offs;
         case (reg_B[7:6])
          3: xMx = {reg_L,reg_B};
          2: xMx = {reg_G,reg_B};
          default: xMx = {reg_H,reg_B};
         endcase 
        end

         /*Traps, 64 opcodes from 80h-bFh*/ 
           
        default:
        begin
             reg_R = pc_low;
            reg_R_offs = 0;
         pc_low = 0;
         reg_H = pc_high;
         pc_high = opcode[5:0];       
        end

        /*GETPUT*/
        
           opc_G0A, opc_G0B, opc_G0R: maddr = {reg_G, 8'h80};
        opc_G1A, opc_G1B, opc_G1R: maddr = {reg_G, 8'h81};
           opc_G2A, opc_G2B, opc_G2R: maddr = {reg_G, 8'h82};
           opc_G3A, opc_G3B, opc_G3R: maddr = {reg_G, 8'h83};
           opc_L0A, opc_L0B, opc_L0R: maddr = {reg_L, 8'hC0};
           opc_L1A, opc_L1B, opc_L1R: maddr = {reg_L, 8'hC1};
           opc_L2A, opc_L2B, opc_L2R: maddr = {reg_L, 8'hC2};
           opc_L3A, opc_L3B, opc_L3R: maddr = {reg_L, 8'hC3};
           opc_AG0, opc_BG0, opc_RG0: maddr = {reg_G, 8'h80};
           opc_AG1, opc_BG1, opc_RG1: maddr = {reg_G, 8'h81};
           opc_AG2, opc_BG2, opc_RG2: maddr = {reg_G, 8'h82};
           opc_AG3, opc_BG3, opc_RG3: maddr = {reg_G, 8'h83};
           opc_AL0, opc_BL0, opc_RL0: maddr = {reg_L, 8'hC0};
           opc_AL1, opc_BL1, opc_RL1: maddr = {reg_L, 8'hC1};
           opc_AL2, opc_BL2, opc_RL2: maddr = {reg_L, 8'hC2};
           opc_AL3, opc_BL3, opc_RL3: maddr = {reg_L, 8'hC3};
    
          /*ALU*/
    
           opc_IDA:
        begin
             reg_R = reg_A;
            reg_R_offs = 0;
           end

           opc_IDB:
        begin
             reg_R = reg_B;
            reg_R_offs = 0;
           end

           opc_OCA:
        begin
             reg_R = reg_A ^ 8'hFF;
            reg_R_offs = 0;
           end

        opc_OCB:
        begin
             reg_R = reg_B ^ 8'hFF;
            reg_R_offs = 0;
           end

        opc_SLA:
        begin
             reg_R = reg_A << 1;
            reg_R_offs = 0;
           end
        
        opc_SLB:
        begin
             reg_R = reg_B << 1;
            reg_R_offs = 0;
           end

          opc_SRA:
        begin
             reg_R = reg_A >> 1;
            reg_R_offs = 0;
           end

          opc_SRB:
        begin
             reg_R = reg_B >> 1;
            reg_R_offs = 0;
           end

          opc_AND:
        begin
             reg_R = reg_A & reg_B;
            reg_R_offs = 0;
           end

          opc_IOR:
        begin
             reg_R = reg_A | reg_B;
            reg_R_offs = 0;
           end

          opc_EOR:
        begin
             reg_R = reg_A ^ reg_B;
            reg_R_offs = 0;
           end

          opc_ADD:
        begin
             reg_R = reg_A + reg_B;
            reg_R_offs = 0;
           end

           opc_CAR:
           begin
             temp9bits = reg_A + reg_B;   
         reg_R = temp9bits[8] ? 1'b1 : 1'b0;
            reg_R_offs = 0;
         end
        
          opc_ALB:
        begin
             reg_R = (reg_A < reg_B) ? 8'd255:8'd0;
            reg_R_offs = 0;
           end
        
          opc_AEB:
        begin
             reg_R = (reg_A == reg_B) ? 8'd255:8'd0;
            reg_R_offs = 0;
           end

          opc_AGB:
        begin
             reg_R = (reg_A > reg_B) ? 8'd255:8'd0;
            reg_R_offs = 0;
           end
        
         endcase
      
      
    /* -------------- WRITE PHASE ---------------------- */
    
      3: case (opcode)
                 
        opc_NH, opc_MH: reg_H = mdata_get;
        opc_NG, opc_MG: reg_G = mdata_get;
        opc_NL, opc_ML: reg_L = mdata_get;
        opc_NS, opc_MS: ser_or = mdata_get;
           opc_NP, opc_MP: par_or = mdata_get;
        opc_NR, opc_MR: reg_R = mdata_get;
        opc_ND, opc_MD: reg_D = mdata_get; 
        opc_NJ, opc_MJ: begin pc_low = mdata_get; seg7Byte1(pc_low); end
           opc_NT, opc_MT: if (reg_R + reg_R_offs != 8'b0) pc_low = mdata_get;
         ///*DBG*/ if (reg_A == 0) pc_low = 15;
        opc_NF, opc_MF: if (reg_R + reg_R_offs == 8'b0) pc_low = mdata_get;    
             
        opc_NC, opc_MC:
        begin
             reg_R = pc_low + 8'd1;
            reg_R_offs = 0;
          pc_low = 0;
         reg_H = pc_high;
         pc_high = reg_R;
        end
        
        opc_NA, opc_MA:
        begin
         reg_CLIP = reg_A;
         reg_A = mdata_get;
         case (reg_A[7:6])
          3: xMx = {reg_L,reg_A};
          2: xMx = {reg_G,reg_A};
          default: xMx = {reg_H,reg_A};
         endcase 
        end
          
        opc_NB, opc_MB:
        begin
         reg_CLIP = reg_B;
         reg_B = mdata_get;
         case (reg_B[7:6])
          3: xMx = {reg_L,reg_B};
          2: xMx = {reg_G,reg_B};
          default: xMx = {reg_H,reg_B};
         endcase 
        end                                           
    
           opc_LEAVE:
        begin
          reg_L = reg_L + 8'd1;
             reg_R = mdata_get;
            reg_R_offs = 0;         
        end

         /*Traps, 64 opcodes from 80h-bFh*/ 
           
        default:;

        /*GETPUT*/
        
           opc_G0A, opc_G1A, opc_G2A, opc_G3A,
           opc_L0A, opc_L1A, opc_L2A, opc_L3A:
        begin
         reg_CLIP = reg_A;
         reg_A = mdata_get;
         case (reg_A[7:6])
          3: xMx = {reg_L,reg_A};
          2: xMx = {reg_G,reg_A};
          default: xMx = {reg_H,reg_A};
         endcase 
        end
           
           opc_AG0, opc_AG1, opc_AG2, opc_AG3,
           opc_AL0, opc_AL1, opc_AL2, opc_AL3:
        begin
          mdata_put = reg_A;
         mwren = 1;
        end

           opc_G0B, opc_G1B, opc_G2B, opc_G3B,
           opc_L0B, opc_L1B, opc_L2B, opc_L3B:
        begin
         reg_CLIP = reg_B;
         reg_B = mdata_get;
         case (reg_B[7:6])
          3: xMx = {reg_L,reg_B};
          2: xMx = {reg_G,reg_B};
          default: xMx = {reg_H,reg_B};
         endcase 
        end     
        
           opc_BG0, opc_BG1, opc_BG2, opc_BG3,
           opc_BL0, opc_BL1, opc_BL2, opc_BL3:
        begin
          mdata_put = reg_B;
         mwren = 1;
        end     
        
           opc_G0R, opc_G1R, opc_G2R, opc_G3R,
           opc_L0R, opc_L1R, opc_L2R, opc_L3R:
        begin
             reg_R = mdata_get;
            reg_R_offs = 0;
        end       
        
           opc_RG0, opc_RG1, opc_RG2, opc_RG3,
           opc_RL0, opc_RL1, opc_RL2, opc_RL3:
        begin
          mdata_put = reg_R + reg_R_offs;
         mwren = 1;
        end
        
         endcase
    
endcase
endmodule




