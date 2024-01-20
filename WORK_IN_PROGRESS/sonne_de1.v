
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
   regular, see documention and discrete schematics.

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

localparam opc_NOP = 8'h00;
localparam opc_RET = 8'h01;
localparam opc_NH  = 8'h02;
localparam opc_NG  = 8'h03;
localparam opc_NL  = 8'h04;
localparam opc_NS  = 8'h05;
localparam opc_NP  = 8'h06;
localparam opc_NR  = 8'h07;
localparam opc_ND  = 8'h08;
localparam opc_NJ  = 8'h09;
localparam opc_NT  = 8'h0A;
localparam opc_NF  = 8'h0B;
localparam opc_NC  = 8'h0C;
localparam opc_R0P = 8'h0D;
localparam opc_NA  = 8'h0E;
localparam opc_NB  = 8'h0F;
localparam opc_ISI = 8'h10;
localparam opc_LID = 8'h11;
localparam opc_MH  = 8'h12;
localparam opc_MG  = 8'h13;
localparam opc_ML  = 8'h14;
localparam opc_MS  = 8'h15;
localparam opc_MP  = 8'h16;
localparam opc_MR  = 8'h17;
localparam opc_MD  = 8'h18;
localparam opc_MJ  = 8'h19;
localparam opc_MT  = 8'h1A;
localparam opc_MF  = 8'h1B;
localparam opc_MC  = 8'h1C;
localparam opc_R1P = 8'h1D;
localparam opc_MA  = 8'h1E;
localparam opc_MB  = 8'h1F;
localparam opc_OSO = 8'h20;
localparam opc_HM =  8'h21;
localparam opc_REA = 8'h22;
localparam opc_HG  = 8'h23;
localparam opc_HL  = 8'h24;
localparam opc_HS  = 8'h25;
localparam opc_HP  = 8'h26;
localparam opc_HR  = 8'h27;
localparam opc_HD  = 8'h28;
localparam opc_HJ  = 8'h29;
localparam opc_HT  = 8'h2A;
localparam opc_HF  = 8'h2B;
localparam opc_HC  = 8'h2C;
localparam opc_R2P = 8'h2D;
localparam opc_HA  = 8'h2E;
localparam opc_HB  = 8'h2F;
localparam opc_SCL = 8'h30;
localparam opc_GM  = 8'h31;
localparam opc_GH  = 8'h32;
localparam opc_REB = 8'h33;
localparam opc_GL  = 8'h34;
localparam opc_GS  = 8'h35;
localparam opc_GP  = 8'h36;
localparam opc_GR  = 8'h37;
localparam opc_GD  = 8'h38;
localparam opc_GJ  = 8'h39;
localparam opc_GT  = 8'h3A;
localparam opc_GF  = 8'h3B;
localparam opc_GC  = 8'h3C;
localparam opc_R3P = 8'h3D;
localparam opc_GA  = 8'h3E;
localparam opc_GB  = 8'h3F;
localparam opc_SCH = 8'h40;
localparam opc_LM  = 8'h41;
localparam opc_LH  = 8'h42;
localparam opc_LG  = 8'h43;
localparam opc_LL  = 8'h44;
localparam opc_LS  = 8'h45;
localparam opc_LP  = 8'h46;
localparam opc_LR  = 8'h47;
localparam opc_LD  = 8'h48;
localparam opc_LJ  = 8'h49;
localparam opc_LT  = 8'h4A;
localparam opc_LF  = 8'h4B;
localparam opc_LC  = 8'h4C;
localparam opc_R4M = 8'h4D;
localparam opc_LA  = 8'h4E;
localparam opc_LB  = 8'h4F;
localparam opc_HIZ = 8'h50;
localparam opc_SM  = 8'h51;
localparam opc_SH  = 8'h52;
localparam opc_SG  = 8'h53;
localparam opc_SL  = 8'h54;
localparam opc_SS  = 8'h55;
localparam opc_SP  = 8'h56;
localparam opc_SR  = 8'h57;
localparam opc_SD  = 8'h58;
localparam opc_SJ  = 8'h59;
localparam opc_ST  = 8'h5A;
localparam opc_SF  = 8'h5B;
localparam opc_SC  = 8'h5C;
localparam opc_R3M = 8'h5D;
localparam opc_SA  = 8'h5E;
localparam opc_SB  = 8'h5F;
localparam opc_LLF = 8'h60;
localparam opc_PM  = 8'h61;
localparam opc_PH  = 8'h62;
localparam opc_PG  = 8'h63;
localparam opc_PL  = 8'h64;
localparam opc_PS  = 8'h65;
localparam opc_PP  = 8'h66;
localparam opc_PR  = 8'h67;
localparam opc_PD  = 8'h68;
localparam opc_PJ  = 8'h69;
localparam opc_PT  = 8'h6A;
localparam opc_PF  = 8'h6B;
localparam opc_PC  = 8'h6C;
localparam opc_R2M = 8'h6D;
localparam opc_PA  = 8'h6E;
localparam opc_PB  = 8'h6F;
localparam opc_ELF = 8'h70;
localparam opc_RM  = 8'h71;
localparam opc_RH  = 8'h72;
localparam opc_RG  = 8'h73;
localparam opc_RL  = 8'h74;
localparam opc_RS  = 8'h75;
localparam opc_RP  = 8'h76;
localparam opc_RR  = 8'h77;
localparam opc_RD  = 8'h78;
localparam opc_RJ  = 8'h79;
localparam opc_RT  = 8'h7A;
localparam opc_RF  = 8'h7B;
localparam opc_RC  = 8'h7C;
localparam opc_R1M = 8'h7D;
localparam opc_RA  = 8'h7E;
localparam opc_RB  = 8'h7F;
localparam opc_G0A = 8'hC0;
localparam opc_G1A = 8'hC1;
localparam opc_G2A = 8'hC2;
localparam opc_G3A = 8'hC3;
localparam opc_L0A = 8'hC4;
localparam opc_L1A = 8'hC5;
localparam opc_L2A = 8'hC6;
localparam opc_L3A = 8'hC7;
localparam opc_AG0 = 8'hC8;
localparam opc_AG1 = 8'hC9;
localparam opc_AG2 = 8'hCA;
localparam opc_AG3 = 8'hCB;
localparam opc_AL0 = 8'hCC;
localparam opc_AL1 = 8'hCD;
localparam opc_AL2 = 8'hCE;
localparam opc_AL3 = 8'hCF;
localparam opc_G0B = 8'hD0;
localparam opc_G1B = 8'hD1;
localparam opc_G2B = 8'hD2;
localparam opc_G3B = 8'hD3;
localparam opc_L0B = 8'hD4;
localparam opc_L1B = 8'hD5;
localparam opc_L2B = 8'hD6;
localparam opc_L3B = 8'hD7;
localparam opc_BG0 = 8'hD8;
localparam opc_BG1 = 8'hD9;
localparam opc_BG2 = 8'hDA;
localparam opc_BG3 = 8'hDB;
localparam opc_BL0 = 8'hDC;
localparam opc_BL1 = 8'hDD;
localparam opc_BL2 = 8'hDE;
localparam opc_BL3 = 8'hDF;
localparam opc_G0R = 8'hE0;
localparam opc_G1R = 8'hE1;
localparam opc_G2R = 8'hE2;
localparam opc_G3R = 8'hE3;
localparam opc_L0R = 8'hE4;
localparam opc_L1R = 8'hE5;
localparam opc_L2R = 8'hE6;
localparam opc_L3R = 8'hE7;
localparam opc_RG0 = 8'hE8;
localparam opc_RG1 = 8'hE9;
localparam opc_RG2 = 8'hEA;
localparam opc_RG3 = 8'hEB;
localparam opc_RL0 = 8'hEC;
localparam opc_RL1 = 8'hED;
localparam opc_RL2 = 8'hEE;
localparam opc_RL3 = 8'hEF;
localparam opc_IDA = 8'hF0;
localparam opc_IDB = 8'hF1;
localparam opc_OCA = 8'hF2;
localparam opc_OCB = 8'hF3;
localparam opc_SLA = 8'hF4;
localparam opc_SLB = 8'hF5;
localparam opc_SRA = 8'hF6;
localparam opc_SRB = 8'hF7;
localparam opc_AND = 8'hF8;
localparam opc_IOR = 8'hF9;
localparam opc_EOR = 8'hFA;
localparam opc_ADD = 8'hFB;
localparam opc_CAR = 8'hFC;
localparam opc_ALB = 8'hFD;
localparam opc_AEB = 8'hFE;
localparam opc_AGB = 8'hFF;


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
  High order nybble encodes 'active high' device select line,
  low order nybble encodes 'active low' device select line.
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
           
           opc_NH, opc_NG, opc_NL, opc_NS,
           opc_NP, opc_NR, opc_ND, opc_NJ,
           opc_NT, opc_NF, opc_NC, opc_NA,
           opc_NB:
           begin
             maddr = {pc_high,pc_low};
             pc_low = pc_low + 8'd1;
           end

           opc_R0P: reg_R_offs = 0;
        
           opc_ISI:
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
        
           opc_OSO:
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
        
           opc_HG: reg_G = reg_H;
           opc_HL: reg_L = reg_H; 
           opc_HS: ser_or = reg_H;
           opc_HP:
           begin
             par_or = reg_H;
             par_hiz = 0;
           end
           
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

           opc_SCL: sd_clk = 0;
           
           opc_GM:
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
               
           opc_GL: reg_L = reg_G;
           opc_GS: ser_ir = reg_G;
           opc_GP:
           begin
             par_or = reg_G;
             par_hiz = 0;
           end

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

           opc_SCH: sd_clk = 1;
        
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

           opc_HIZ:
           begin
             par_hiz = 1;
             seg7Byte2(reg_A); /*DBG print*/
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

           opc_OLD: maddr = {reg_L,8'hC4};

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

           opc_NEW:
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
        end /*of READ PHASE*/
      
    /* -------------- WRITE PHASE ---------------------- */
    
      2: case (opcode)
                 
        opc_NH, opc_MH: reg_H = mdata_get;
        opc_NG, opc_MG: reg_G = mdata_get;
        opc_NL, opc_ML: reg_L = mdata_get;
        opc_NS, opc_MS: ser_or = mdata_get;
        opc_NP, opc_MP: par_or = mdata_get;
        opc_NR, opc_MR: reg_R = mdata_get;
        opc_ND, opc_MD: reg_D = mdata_get; 
        opc_NJ, opc_MJ: pc_low = mdata_get;
        opc_NT, opc_MT: if (reg_R + reg_R_offs != 8'b0) pc_low = mdata_get;
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
ELF
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

LLF RET
CLOSE

; ----------------------------- divmod8 -------------------------------------

@divmod8 ; Divide A by B, division result in A, remainder in B
ELF

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

LLF RET
CLOSE

; ----------------------------- divmod8 -------------------------------------

SPI_RDBYTE@ ; Read eight bits, output SOR byte
    SCH SCL ; Clock pulse
    ISI OSO, SCH SCL
    ISI OSO, SCH SCL
    ISI OSO, SCH SCL
    ISI OSO, SCH SCL
    ISI OSO, SCH SCL
    ISI OSO, SCH SCL
    ISI OSO, SCH SCL
    ISI OSO
   RET

SPI_WRBYTE@ ; Write eight bits, discard SIR byte
    SCH SCL ; Clock pulse
    OSO, SCH SCL
    OSO, SCH SCL
    OSO, SCH SCL
    OSO, SCH SCL
    OSO, SCH SCL
    OSO, SCH SCL
    OSO, SCH SCL
   RET

*/

