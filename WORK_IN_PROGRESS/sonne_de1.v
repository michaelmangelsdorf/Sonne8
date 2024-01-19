
/* Sonne CPU core, rev. Daffodil
   Verilog Implementation
   Jan-2024 Michael Mangelsdorf
   Copyr. <mim@ok-schalter.de>

   This runs the mul8/div8 code
   on a Terasic DE1Soc board
   with output on 7-segment display.

   Resource usage on the DE1 was:
   (Quartus II)
   Logic ALM: 598
   Registers: 364
   Block memory bits: 524.288
   RAM blocks: 64
*/


module daffo_core
(
  input rst,
  input clk,

  output reg[16'hF:0] maddr,
  output reg[16'h7:0] mdata_put,
  output reg[16'h0:0] mwren,
  input wire[16'h7:0] mdata_get,
    
  /* SD/SPI */

  output reg sd_clk,
  output reg sd_mosi,
  output reg sd_cs,
  input      sd_miso,

  /* MISC */

  output reg [6:0] seg7_1,
  output reg [6:0] seg7_2,
  output reg [6:0] seg7_3,
  output reg [6:0] seg7_4,
  output reg [6:0] seg7_5,
  output reg [6:0] seg7_6,
  
  output reg [16'h9:0] led,
  input wire [16'hD:0] keysw
);


/* CPU - The binary layout of these opcodes is
   engineered and regular, see documention
   and discrete schematics.

  In short:
  Bit 7 clear: COPY, SCROUNGE, SIGNAL, DIAL
  Bit 7 set, bit 6 clear: TRAP
  Bit 7 set, bit 6 set: GETPUT, ALU
  
  Mnemonics:
  Prefix C => register-to-register COPY
  'R' + number + P(lus) M(inus) => DIAL
  G(lobal) L(ocal) + offset + A B R => PUT
  A B R + G(lobal) L(ocal) + offset => GET
*/

localparam opc_NOP = 8'b00000000; /*00*/
localparam opc_RET = 8'b00000001; /*01*/
localparam opc_CNH = 8'b00000010; /*02*/
localparam opc_CNG = 8'b00000011; /*03*/
localparam opc_CNL = 8'b00000100; /*04*/
localparam opc_CNS = 8'b00000101; /*05*/
localparam opc_CNP = 8'b00000110; /*06*/
localparam opc_CNR = 8'b00000111; /*07*/
localparam opc_CND = 8'b00001000; /*08*/
localparam opc_CNJ = 8'b00001001; /*09*/
localparam opc_CNT = 8'b00001010; /*0A*/
localparam opc_CNF = 8'b00001011; /*0B*/
localparam opc_CNC = 8'b00001100; /*0C*/
localparam opc_R0P = 8'b00001101; /*0D*/
localparam opc_CNA = 8'b00001110; /*0E*/
localparam opc_CNB = 8'b00001111; /*0F*/
localparam opc_SSI = 8'b00010000; /*10*/
localparam opc_LID = 8'b00010001; /*11*/
localparam opc_CMH = 8'b00010010; /*12*/
localparam opc_CMG = 8'b00010011; /*13*/
localparam opc_CML = 8'b00010100; /*14*/
localparam opc_CMS = 8'b00010101; /*15*/
localparam opc_CMP = 8'b00010110; /*16*/
localparam opc_CMR = 8'b00010111; /*17*/
localparam opc_CMD = 8'b00011000; /*18*/
localparam opc_CMJ = 8'b00011001; /*19*/
localparam opc_CMT = 8'b00011010; /*1A*/
localparam opc_CMF = 8'b00011011; /*1B*/
localparam opc_CMC = 8'b00011100; /*1C*/
localparam opc_R1P = 8'b00011101; /*1D*/
localparam opc_CMA = 8'b00011110; /*1E*/
localparam opc_CMB = 8'b00011111; /*1F*/
localparam opc_SSO = 8'b00100000; /*20*/
localparam opc_CHM = 8'b00100001; /*21*/
localparam opc_REA = 8'b00100010; /*22*/
localparam opc_CHG = 8'b00100011; /*23*/
localparam opc_CHL = 8'b00100100; /*24*/
localparam opc_CHS = 8'b00100101; /*25*/
localparam opc_CHP = 8'b00100110; /*26*/
localparam opc_CHR = 8'b00100111; /*27*/
localparam opc_CHD = 8'b00101000; /*28*/
localparam opc_CHJ = 8'b00101001; /*29*/
localparam opc_CHT = 8'b00101010; /*2A*/
localparam opc_CHF = 8'b00101011; /*2B*/
localparam opc_CHC = 8'b00101100; /*2C*/
localparam opc_R2P = 8'b00101101; /*2D*/
localparam opc_CHA = 8'b00101110; /*2E*/
localparam opc_CHB = 8'b00101111; /*2F*/
localparam opc_SCL = 8'b00110000; /*30*/
localparam opc_CGM = 8'b00110001; /*31*/
localparam opc_CGH = 8'b00110010; /*32*/
localparam opc_REB = 8'b00110011; /*33*/
localparam opc_CGL = 8'b00110100; /*34*/
localparam opc_CGS = 8'b00110101; /*35*/
localparam opc_CGP = 8'b00110110; /*36*/
localparam opc_CGR = 8'b00110111; /*37*/
localparam opc_CGD = 8'b00111000; /*38*/
localparam opc_CGJ = 8'b00111001; /*39*/
localparam opc_CGT = 8'b00111010; /*3A*/
localparam opc_CGF = 8'b00111011; /*3B*/
localparam opc_CGC = 8'b00111100; /*3C*/
localparam opc_R3P = 8'b00111101; /*3D*/
localparam opc_CGA = 8'b00111110; /*3E*/
localparam opc_CGB = 8'b00111111; /*3F*/
localparam opc_SCH = 8'b01000000; /*40*/
localparam opc_CLM = 8'b01000001; /*41*/
localparam opc_CLH = 8'b01000010; /*42*/
localparam opc_CLG = 8'b01000011; /*43*/
localparam opc_CLL = 8'b01000100; /*44*/
localparam opc_CLS = 8'b01000101; /*45*/
localparam opc_CLP = 8'b01000110; /*46*/
localparam opc_CLR = 8'b01000111; /*47*/
localparam opc_CLD = 8'b01001000; /*48*/
localparam opc_CLJ = 8'b01001001; /*49*/
localparam opc_CLT = 8'b01001010; /*4A*/
localparam opc_CLF = 8'b01001011; /*4B*/
localparam opc_CLC = 8'b01001100; /*4C*/
localparam opc_R4M = 8'b01001101; /*4D*/
localparam opc_CLA = 8'b01001110; /*4E*/
localparam opc_CLB = 8'b01001111; /*4F*/
localparam opc_HIZ = 8'b01010000; /*50*/
localparam opc_CSM = 8'b01010001; /*51*/
localparam opc_CSH = 8'b01010010; /*52*/
localparam opc_CSG = 8'b01010011; /*53*/
localparam opc_CSL = 8'b01010100; /*54*/
localparam opc_CSS = 8'b01010101; /*55*/
localparam opc_CSP = 8'b01010110; /*56*/
localparam opc_CSR = 8'b01010111; /*57*/
localparam opc_CSD = 8'b01011000; /*58*/
localparam opc_CSJ = 8'b01011001; /*59*/
localparam opc_CST = 8'b01011010; /*5A*/
localparam opc_CSF = 8'b01011011; /*5B*/
localparam opc_CSC = 8'b01011100; /*5C*/
localparam opc_R3M = 8'b01011101; /*5D*/
localparam opc_CSA = 8'b01011110; /*5E*/
localparam opc_CSB = 8'b01011111; /*5F*/
localparam opc_OLD = 8'b01100000; /*60*/
localparam opc_CPM = 8'b01100001; /*61*/
localparam opc_CPH = 8'b01100010; /*62*/
localparam opc_CPG = 8'b01100011; /*63*/
localparam opc_CPL = 8'b01100100; /*64*/
localparam opc_CPS = 8'b01100101; /*65*/
localparam opc_CPP = 8'b01100110; /*66*/
localparam opc_CPR = 8'b01100111; /*67*/
localparam opc_CPD = 8'b01101000; /*68*/
localparam opc_CPJ = 8'b01101001; /*69*/
localparam opc_CPT = 8'b01101010; /*6A*/
localparam opc_CPF = 8'b01101011; /*6B*/
localparam opc_CPC = 8'b01101100; /*6C*/
localparam opc_R2M = 8'b01101101; /*6D*/
localparam opc_CPA = 8'b01101110; /*6E*/
localparam opc_CPB = 8'b01101111; /*6F*/
localparam opc_NEW = 8'b01110000; /*70*/
localparam opc_CRM = 8'b01110001; /*71*/
localparam opc_CRH = 8'b01110010; /*72*/
localparam opc_CRG = 8'b01110011; /*73*/
localparam opc_CRL = 8'b01110100; /*74*/
localparam opc_CRS = 8'b01110101; /*75*/
localparam opc_CRP = 8'b01110110; /*76*/
localparam opc_CRR = 8'b01110111; /*77*/
localparam opc_CRD = 8'b01111000; /*78*/
localparam opc_CRJ = 8'b01111001; /*79*/
localparam opc_CRT = 8'b01111010; /*7A*/
localparam opc_CRF = 8'b01111011; /*7B*/
localparam opc_CRC = 8'b01111100; /*7C*/
localparam opc_R1M = 8'b01111101; /*7D*/
localparam opc_CRA = 8'b01111110; /*7E*/
localparam opc_CRB = 8'b01111111; /*7F*/
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
reg [7:0] par_ir; /* Connect to tri-state IO bus */
reg [7:0] par_or; /* Connect to tri-state IO bus */
reg [7:0] ser_ir;
reg [7:0] ser_or;
reg [0:0] par_hiz; /* Controls IO bus impedance */


/* MISC */

reg [7:0] opcode;
reg [7:0] source_val;
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
2: cpu_phase = 0;
endcase



/* Device Select Register D:
  Low order nybble encodes active low device select line,
  high order nybble encodes active high device select line.
  Value one: Empty device/NOP
*/

reg [15:0] io_devsel_AL;
reg [15:0] io_devsel_AH;
always@(posedge clk)
begin
case (reg_D[3:0])
0:  io_devsel_AL = 16'b0000_0000_0000_0001; /*Not connected*/
1:  io_devsel_AL = 16'b0000_0000_0000_0010; /*SPI bus*/
2:  io_devsel_AL = 16'b0000_0000_0000_0100;
3:  io_devsel_AL = 16'b0000_0000_0000_1000;
4:  io_devsel_AL = 16'b0000_0000_0001_0000;
5:  io_devsel_AL = 16'b0000_0000_0010_0000;
6:  io_devsel_AL = 16'b0000_0000_0100_0000;
7:  io_devsel_AL = 16'b0000_0000_1000_0000;
8:  io_devsel_AL = 16'b0000_0001_0000_0000;
9:  io_devsel_AL = 16'b0000_0010_0000_0000;
10: io_devsel_AL = 16'b0000_0100_0000_0000;
11: io_devsel_AL = 16'b0000_1000_0000_0000;
12: io_devsel_AL = 16'b0001_0000_0000_0000;
13: io_devsel_AL = 16'b0010_0000_0000_0000;
14: io_devsel_AL = 16'b0100_0000_0000_0000;
15: io_devsel_AL = 16'b1000_0000_0000_0000;
endcase
case (reg_D[7:4])
0:  io_devsel_AH = 16'b0000_0000_0000_0001; /*Not connected*/
1:  io_devsel_AH = 16'b0000_0000_0000_0010;
2:  io_devsel_AH = 16'b0000_0000_0000_0100;
3:  io_devsel_AH = 16'b0000_0000_0000_1000;
4:  io_devsel_AH = 16'b0000_0000_0001_0000;
5:  io_devsel_AH = 16'b0000_0000_0010_0000;
6:  io_devsel_AH = 16'b0000_0000_0100_0000;
7:  io_devsel_AH = 16'b0000_0000_1000_0000;
8:  io_devsel_AH = 16'b0000_0001_0000_0000;
9:  io_devsel_AH = 16'b0000_0010_0000_0000;
10: io_devsel_AH = 16'b0000_0100_0000_0000;
11: io_devsel_AH = 16'b0000_1000_0000_0000;
12: io_devsel_AH = 16'b0001_0000_0000_0000;
13: io_devsel_AH = 16'b0010_0000_0000_0000;
14: io_devsel_AH = 16'b0100_0000_0000_0000;
15: io_devsel_AH = 16'b1000_0000_0000_0000;
endcase
if (io_devsel_AL[1] == 1'd1) sd_cs = 1'd0;
else sd_cs = 1'd1;
end



always@(posedge clk)
case (cpu_phase)
      
    0: begin
       mwren = 0;
       maddr = {pc_high,pc_low};
       pc_low = pc_low + 8'd1;
       end
      
    1: begin
       opcode = mdata_get;

       /* -------------- READ PHASE ---------------------- */
  
       case (opcode)

           opc_NOP: ;
        
           opc_RET: /* Scrounge NM */
           begin
             pc_low =  reg_R;
             pc_high = reg_H;
           end
           
           opc_CNH, opc_CNG, opc_CNL, opc_CNS,
           opc_CNP, opc_CNR, opc_CND, opc_CNJ,
           opc_CNT, opc_CNF, opc_CNC, opc_CNA,
           opc_CNB:
           begin
             maddr = {pc_high,pc_low};
             pc_low = pc_low + 8'd1;
           end

           opc_R0P: reg_R_offs = 0;
        
           opc_SSI:
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
           
           opc_CMH, opc_CMG, opc_CML, opc_CMS,
           opc_CMP, opc_CMR, opc_CMD, opc_CMJ,
           opc_CMT, opc_CMF, opc_CMC, opc_CMA,
           opc_CMB:
           begin
              maddr = xMx;
           end

           opc_R1P: reg_R_offs = 1;
        
           opc_SSO:
           begin
             sd_mosi = ser_or[7];
             ser_or = ser_or << 1;       
           end
        
           opc_CHM:
           begin
             maddr = xMx;
             mdata_put = reg_H;
             mwren = 1;
           end
        
           opc_REA:/* Scrounge HH */
           begin
             reg_A = reg_CLIP;
             case (reg_A[7:6])
               3: xMx = {reg_L,reg_A};
               2: xMx = {reg_G,reg_A};
               default: xMx = {reg_H,reg_A};
             endcase 
           end
        
           opc_CHG: reg_G = reg_H;
           opc_CHL: reg_L = reg_H; 
           opc_CHS: ser_or = reg_H;
           opc_CHP:
           begin
             par_or = reg_H;
             par_hiz = 0;
           end
           
           opc_CHR:
           begin
             reg_R = reg_H;
             reg_R_offs = 0;
           end

           opc_CHD: reg_D = reg_H;
           opc_CHJ: pc_low = reg_H;
           opc_CHT: if (reg_R + reg_R_offs != 8'b0) pc_low = reg_H;
           opc_CHF: if (reg_R + reg_R_offs == 8'b0) pc_low = reg_H;
        
           opc_CHC:
           begin
             reg_R = pc_low;
             reg_R_offs = 0;
             pc_low = 0;
             pc_high = reg_H;
           end
           
           opc_R2P: reg_R_offs = 2;
        
           opc_CHA:
           begin
             reg_CLIP = reg_A;
             reg_A = reg_H;
             case (reg_A[7:6])
               3: xMx = {reg_L,reg_A};
               2: xMx = {reg_G,reg_A};
               default: xMx = {reg_H,reg_A};
             endcase 
           end
           
           opc_CHB:
           begin
             reg_CLIP = reg_B;
             reg_B = reg_H;
             case (reg_B[7:6])
               3: xMx = {reg_L,reg_B};
               2: xMx = {reg_G,reg_B};
               default: xMx = {reg_H,reg_B};
             endcase 
           end  

           opc_SCL: sd_clk = 0;
           
           opc_CGM:
           begin
             maddr = xMx;
             mdata_put = reg_G;
             mwren = 1;
           end
           
           opc_CGH: reg_H = reg_G;
               
           opc_REB: /* Scrounge GG */
           begin
             reg_B = reg_CLIP;
             case (reg_B[7:6])
               3: xMx = {reg_L,reg_B};
               2: xMx = {reg_G,reg_B};
               default: xMx = {reg_H,reg_B};
             endcase 
           end
               
           opc_CGL: reg_L = reg_G;
           opc_CGS: ser_ir = reg_G;
           opc_CGP:
           begin
             par_or = reg_G;
             par_hiz = 0;
           end

           opc_CGR:
           begin
             reg_R = reg_G;
             reg_R_offs = 0;
           end

           opc_CGD: reg_D = reg_G;
           opc_CGJ: pc_low = reg_G;
           opc_CGT: if (reg_R + reg_R_offs != 8'b0) pc_low = reg_G;
           opc_CGF: if (reg_R + reg_R_offs == 8'b0) pc_low = reg_G;

           opc_CGC:
           begin
             reg_R = pc_low;
             reg_R_offs = 0;
             pc_low = 0;
             reg_H = pc_high;
             pc_high = reg_G;
           end
        
           opc_R3P: reg_R_offs = 3;
           
           opc_CGA:
           begin
             reg_CLIP = reg_A;
             reg_A = reg_G;
             case (reg_A[7:6])
               3: xMx = {reg_L,reg_A};
               2: xMx = {reg_G,reg_A};
               default: xMx = {reg_H,reg_A};
             endcase 
           end
               
           opc_CGB:
           begin
             reg_CLIP = reg_B;
             reg_B = reg_G;
             case (reg_B[7:6])
               3: xMx = {reg_L,reg_B};
               2: xMx = {reg_G,reg_B};
               default: xMx = {reg_H,reg_B};
             endcase 
           end

           opc_SCH: sd_clk = 1;
        
           opc_CLM:
           begin
             maddr = xMx;
             mdata_put = reg_L;
             mwren = 1;
           end
           
           opc_CLH: reg_H = reg_L;
           opc_CLG: reg_G = reg_L;
           opc_CLL: ;
           opc_CLS: ser_ir = reg_L;
           opc_CLP: begin par_or = reg_L; par_hiz = 0; end

           opc_CLR:
           begin
             reg_R = reg_L;
             reg_R_offs = 0;
           end

           opc_CLD: reg_D = reg_L;
           opc_CLJ: pc_low = reg_L;
           opc_CLT: if (reg_R + reg_R_offs != 8'b0) pc_low = reg_L;
           opc_CLF: if (reg_R + reg_R_offs == 8'b0) pc_low = reg_L;

           opc_CLC:
           begin
             reg_R = pc_low;
             reg_R_offs = 0;
             pc_low = 0;
             reg_H = pc_high;
             pc_high = reg_L;
           end
        
           opc_R4M: reg_R_offs = (8'd4 ^ 8'hFFFF) + 8'd1;
           
           opc_CLA:
           begin
             reg_CLIP = reg_A;
             reg_A = reg_L;
             case (reg_A[7:6])
               3: xMx = {reg_L,reg_A};
               2: xMx = {reg_G,reg_A};
               default: xMx = {reg_H,reg_A};
             endcase 
           end
               
           opc_CLB:
           begin
             reg_CLIP = reg_B;
             reg_B = reg_L;
             case (reg_B[7:6])
               3: xMx = {reg_L,reg_B};
               2: xMx = {reg_G,reg_B};
               default: xMx = {reg_H,reg_B};
             endcase 
           end

           opc_HIZ:
           begin
             par_hiz = 1;
             seg7Byte2(reg_A); /*DBG print*/
             seg7Byte3(reg_B);
           end
  
           opc_CSM:
           begin
             maddr = xMx;
             mdata_put = ser_or;
             mwren = 1;
           end

           opc_CSH: reg_H = ser_or;
           opc_CSG: reg_G = ser_or;
           opc_CSL: reg_L = ser_or;
           opc_CSS: ;
           opc_CSP: begin par_or = ser_or; par_hiz = 0; end

           opc_CSR:
           begin
             reg_R = ser_or;
             reg_R_offs = 0;
           end

           opc_CSD: reg_D = ser_or;
           opc_CSJ: pc_low = ser_or;
           opc_CST: if (reg_R + reg_R_offs != 8'b0) pc_low = ser_or;
           opc_CSF: if (reg_R + reg_R_offs == 8'b0) pc_low = ser_or;

           opc_CSC:
           begin
             reg_R = pc_low;
             reg_R_offs = 0;
             pc_low = 0;
             reg_H = pc_high;
             pc_high = ser_or;
           end

           opc_R3M: reg_R_offs = (8'd3 ^ 8'hFFFF) + 8'd1;

           opc_CSA:
           begin
             reg_CLIP = reg_A;
             reg_A = ser_or;
             case (reg_A[7:6])
               3: xMx = {reg_L,reg_A};
               2: xMx = {reg_G,reg_A};
               default: xMx = {reg_H,reg_A};
             endcase 
           end
               
           opc_CSB:
           begin
             reg_CLIP = reg_B;
             reg_B = ser_or;
             case (reg_B[7:6])
               3: xMx = {reg_L,reg_B};
               2: xMx = {reg_G,reg_B};
               default: xMx = {reg_H,reg_B};
             endcase 
           end

           opc_OLD: maddr = {reg_L,8'hC4};

           opc_CPM:
           begin
             maddr = xMx;
             mdata_put = par_ir;
             mwren = 1;
           end
   
           opc_CPH: reg_H = par_ir;
           opc_CPG: reg_G = par_ir;
           opc_CPL: reg_L = par_ir;
           opc_CPS: ser_or = par_ir;
           opc_CPP: ;

           opc_CPR:
           begin
             reg_R = par_ir;
             reg_R_offs = 0;
           end

           opc_CPD: reg_D = par_ir;
           opc_CPJ: pc_low = par_ir;
           opc_CPT: if (reg_R + reg_R_offs != 8'b0) pc_low = par_ir;
           opc_CPF: if (reg_R + reg_R_offs == 8'b0) pc_low = par_ir;

           opc_CPC:
           begin
             reg_R = pc_low;
             reg_R_offs = 0;
             pc_low = 0;
             reg_H = pc_high;
             pc_high = par_ir;
           end
        
           opc_R2M: reg_R_offs = (8'd2 ^ 8'hFFFF) + 8'd1;
           
           opc_CPA:
           begin
             reg_CLIP = reg_A;
             reg_A = par_ir;
             case (reg_A[7:6])
               3: xMx = {reg_L,reg_A};
               2: xMx = {reg_G,reg_A};
               default: xMx = {reg_H,reg_A};
             endcase 
           end
               
           opc_CPB:
           begin
             reg_CLIP = reg_B;
             reg_B = par_ir;
             case (reg_B[7:6])
               3: xMx = {reg_L,reg_B};
               2: xMx = {reg_G,reg_B};
               default: xMx = {reg_H,reg_B};
             endcase 
           end

           opc_NEW:
           begin
             reg_L = reg_L - 8'd1;;
             maddr = {reg_L,8'hC4};
             mdata_put = reg_R + reg_R_offs;
             mwren = 1;
           end
  
           opc_CRM:
           begin
             maddr = xMx;
             mdata_put = reg_R + reg_R_offs;
             mwren = 1;
           end
   
           opc_CRH: reg_H = reg_R + reg_R_offs;
           opc_CRG: reg_G = reg_R + reg_R_offs;
           opc_CRL: reg_L = reg_R + reg_R_offs;
           opc_CRS: ser_ir = reg_R + reg_R_offs;
           opc_CRP: begin par_or = reg_R + reg_R_offs; par_hiz = 0; end
           opc_CRR: ;
           opc_CRD: reg_D = reg_R + reg_R_offs;
           opc_CRJ: pc_low = reg_R + reg_R_offs;
           opc_CRT: if (reg_R + reg_R_offs != 8'b0) pc_low = reg_R + reg_R_offs;
           opc_CRF: if (reg_R + reg_R_offs == 8'b0) pc_low = reg_R + reg_R_offs;

           opc_CRC:
           begin
             reg_H = pc_high;
             pc_high = reg_R + reg_R_offs;
             reg_R = pc_low;
             reg_R_offs = 0;
             pc_low = 0;
           end

           opc_R1M: reg_R_offs = (8'd1 ^ 8'hFFFF) + 8'd1;

           opc_CRA:
           begin
             reg_CLIP = reg_A;
             reg_A = reg_R + reg_R_offs;
             case (reg_A[7:6])
               3: xMx = {reg_L,reg_A};
               2: xMx = {reg_G,reg_A};
               default: xMx = {reg_H,reg_A};
             endcase
           end
        
           opc_CRB:
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
        end /*of READ PHASE*/
      
    /* -------------- WRITE PHASE ---------------------- */
    
      2: case (opcode)
                 
        opc_CNH, opc_CMH: reg_H = mdata_get;
        opc_CNG, opc_CMG: reg_G = mdata_get;
        opc_CNL, opc_CML: reg_L = mdata_get;
        opc_CNS, opc_CMS: ser_or = mdata_get;
        opc_CNP, opc_CMP: par_or = mdata_get;
        opc_CNR, opc_CMR: reg_R = mdata_get;
        opc_CND, opc_CMD: reg_D = mdata_get; 
        opc_CNJ, opc_CMJ: pc_low = mdata_get;
        opc_CNT, opc_CMT: if (reg_R + reg_R_offs != 8'b0) pc_low = mdata_get;
        opc_CNF, opc_CMF: if (reg_R + reg_R_offs == 8'b0) pc_low = mdata_get;    
             
        opc_CNC, opc_CMC:
        begin
          reg_R = pc_low + 8'd1;
          reg_R_offs = 0;
          pc_low = 0;
          reg_H = pc_high;
          pc_high = reg_R;
        end
        
        opc_CNA, opc_CMA:
        begin
          reg_CLIP = reg_A;
          reg_A = mdata_get;
          case (reg_A[7:6])
            3: xMx = {reg_L,reg_A};
            2: xMx = {reg_G,reg_A};
            default: xMx = {reg_H,reg_A};
          endcase 
        end
          
        opc_CNB, opc_CMB:
        begin
          reg_CLIP = reg_B;
          reg_B = mdata_get;
          case (reg_B[7:6])
            3: xMx = {reg_L,reg_B};
            2: xMx = {reg_G,reg_B};
            default: xMx = {reg_H,reg_B};
          endcase 
        end
    
        opc_OLD:
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


/* Use my sasm.c to assemble the following source
   and populate the RAM in Quartus with the resulting .mif file.

NA 44
NB 7
 *divmod8 HIZ ; HIZ instruction "hijacked" for 7-segment display output
              ; This outputs 2 (remainder) 6 (quotient)
stop@ NJ <stop

CLOSE

; ----------------------------- mul8 ----------------------------------------

@mul8 ; Multiply A * B, result in A and B 
ENTER

   AL1                    ; Initialize copy multiplicand (low order)
   BL0                    ; Save multiplier
   NA 0 AL2               ; Clear high-order
   NA 8 AL3               ; Initialize loop counter, 8 bits

loop@
   NB 1, L1A AND,
   NF >skip
   L0A, L2B ADD           ; Add multiplier if low order lsb set
   RL2

skip@
   NB 1, L2A AND RG0      ; Check if high order lsb set
   L1A SRA, RL1           ; Shift low order byte right
   L2A SRA, RL2           ; Shift high order byte right
   
   G0A IDA, NF >done
   NA 80h, L1B IOR RL1

done@
   L3R R1- RL3
   NT <loop
   L1A
   L2B

LEAVE
RET
CLOSE

; ----------------------------- divmod8 -------------------------------------

@divmod8 ; Divide A by B, division result in A, remainder in B
ENTER

AL0                       ; Dividend
BL1                       ; Divisor
NA 1, AL2                 ; Shift counter first 1 bit to MSB
NA 0, AL3                 ; Initialise quotient to zero

L1A IDA NF >ELOOP         ; Skip if divisor zero

NA 80h                    ; MSB mask
MSB_SHIFT@                ; Shift divisor left so that first 1 bit is at MSB
 L1B                      ; Load divisor
 AND NT >DIVIDE           ; Skip when MSB set
 SLB RL1                  ; Shift divisor left and update
 L2R R1+ RL2              ; Increment shift counter and update
 NJ <MSB_SHIFT

DIVIDE@
 L3B SLB RL3              ; Shift quotient left and update
 L1A OCA R1+ RA           ; Negate divisor
 L0B CAR                  ; Dividend check borrow bit
 NF >REP

 ADD RL0                  ; Accept subtraction, update dividend
 L3R R1+ RL3              ; Increment quotient

REP@
 L1A SRA RL1              ; Shift divisor right for next subtraction
 L2R R1- RL2              ; Decrement counter
 NT <DIVIDE               ; Branch back if not zero

ELOOP@ L3A, L0B

LEAVE RET
CLOSE

*/

