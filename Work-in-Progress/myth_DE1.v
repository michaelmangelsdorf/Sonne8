
/* Sonne Microcontroller rev. Myth
    Verilog Implementation
  Jan-2024 Michael Mangelsdorf
  Copyr. <mim@ok-schalter.de>

   Run the mul8/div8 code
   on a Terasic DE1Soc board
   result on 7-segment display.

   Around 548 ALMs, 366 regs
   Block memory bits: 32k*8
*/


module myth_core
(
  input rst,
  input clk,

  output reg[15'h14:0] maddr,
  output reg[8'h7:0] mdata_put,
  output reg[1'h0:0] mwren,
  input wire[8'h7:0] mdata_get,
    
  /*SD/SPI*/

  output reg sd_clk,
  output reg sd_mosi,
  output reg sd_cs,
  input      sd_miso,

  /*MISC*/

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
*/

/*SYS*/

localparam opc_NOP = 8'h00;
localparam opc_SSI = 8'h01;
localparam opc_SSO = 8'h02;
localparam opc_SCL = 8'h03;
localparam opc_SCH = 8'h04;
localparam opc_RDY = 8'h05;
localparam opc_NEW = 8'h06;
localparam opc_OLD = 8'h07;

/*BIAS*/

localparam opc_R0P = 8'h08;
localparam opc_R1P = 8'h09;
localparam opc_R2P = 8'h0A;
localparam opc_R3P = 8'h0B;
localparam opc_R4M = 8'h0C;
localparam opc_R3M = 8'h0D;
localparam opc_R2M = 8'h0E;
localparam opc_R1M = 8'h0F;
localparam opc_IDA = 8'h10;
localparam opc_IDB = 8'h11;
localparam opc_OCA = 8'h12;
localparam opc_OCB = 8'h13;
localparam opc_SLA = 8'h14;
localparam opc_SLB = 8'h15;
localparam opc_SRA = 8'h16;
localparam opc_SRB = 8'h17;
localparam opc_AND = 8'h18;
localparam opc_IOR = 8'h19;
localparam opc_EOR = 8'h1A;
localparam opc_ADD = 8'h1B;
localparam opc_CAR = 8'h1C;
localparam opc_ALB = 8'h1D;
localparam opc_AEB = 8'h1E;
localparam opc_AGB = 8'h1F;


/*TRAPS
 (32 opcodes omitted)*/


/*GET-PUT*/

localparam opc_G0A = 8'h40;
localparam opc_G0B = 8'h41;
localparam opc_G0R = 8'h42;
localparam opc_G0W = 8'h43;
localparam opc_L0A = 8'h44;
localparam opc_L0B = 8'h45;
localparam opc_L0R = 8'h46;
localparam opc_L0W = 8'h47;
localparam opc_AG0 = 8'h48;
localparam opc_BG0 = 8'h49;
localparam opc_RG0 = 8'h4A;
localparam opc_WG0 = 8'h4B;
localparam opc_AL0 = 8'h4C;
localparam opc_BL0 = 8'h4D;
localparam opc_RL0 = 8'h4E;
localparam opc_WL0 = 8'h4F;
localparam opc_G1A = 8'h50;
localparam opc_G1B = 8'h51;
localparam opc_G1R = 8'h52;
localparam opc_G1W = 8'h53;
localparam opc_L1A = 8'h54;
localparam opc_L1B = 8'h55;
localparam opc_L1R = 8'h56;
localparam opc_L1W = 8'h57;
localparam opc_AG1 = 8'h58;
localparam opc_BG1 = 8'h59;
localparam opc_RG1 = 8'h5A;
localparam opc_WG1 = 8'h5B;
localparam opc_AL1 = 8'h5C;
localparam opc_BL1 = 8'h5D;
localparam opc_RL1 = 8'h5E;
localparam opc_WL1 = 8'h5F;
localparam opc_G2A = 8'h60;
localparam opc_G2B = 8'h61;
localparam opc_G2R = 8'h62;
localparam opc_G2W = 8'h63;
localparam opc_L2A = 8'h64;
localparam opc_L2B = 8'h65;
localparam opc_L2R = 8'h66;
localparam opc_L2W = 8'h67;
localparam opc_AG2 = 8'h68;
localparam opc_BG2 = 8'h69;
localparam opc_RG2 = 8'h6A;
localparam opc_WG2 = 8'h6B;
localparam opc_AL2 = 8'h6C;
localparam opc_BL2 = 8'h6D;
localparam opc_RL2 = 8'h6E;
localparam opc_WL2 = 8'h6F;
localparam opc_G3A = 8'h70;
localparam opc_G3B = 8'h71;
localparam opc_G3R = 8'h72;
localparam opc_G3W = 8'h73;
localparam opc_L3A = 8'h74;
localparam opc_L3B = 8'h75;
localparam opc_L3R = 8'h76;
localparam opc_L3W = 8'h77;
localparam opc_AG3 = 8'h78;
localparam opc_BG3 = 8'h79;
localparam opc_RG3 = 8'h7A;
localparam opc_WG3 = 8'h7B;
localparam opc_AL3 = 8'h7C;
localparam opc_BL3 = 8'h7D;
localparam opc_RL3 = 8'h7E;
localparam opc_WL3 = 8'h7F;

/*PAIR*/

localparam opc_NN  = 8'h80;
localparam opc_RET = 8'h81;
localparam opc_NR  = 8'h82;
localparam opc_NW  = 8'h83;
localparam opc_ND  = 8'h84;
localparam opc_NL  = 8'h85;
localparam opc_NS  = 8'h86;
localparam opc_NP  = 8'h87;
localparam opc_NE  = 8'h88;
localparam opc_NJ  = 8'h89;
localparam opc_NI  = 8'h8A;
localparam opc_NT  = 8'h8B;
localparam opc_NF  = 8'h8C;
localparam opc_NC  = 8'h8D;
localparam opc_NA  = 8'h8E;
localparam opc_NB  = 8'h8F;
localparam opc_MN  = 8'h90;
localparam opc_REA = 8'h91;
localparam opc_MR  = 8'h92;
localparam opc_MW  = 8'h93;
localparam opc_MD  = 8'h94;
localparam opc_ML  = 8'h95;
localparam opc_MS  = 8'h96;
localparam opc_MP  = 8'h97;
localparam opc_ME  = 8'h98;
localparam opc_MJ  = 8'h99;
localparam opc_MI  = 8'h9A;
localparam opc_MT  = 8'h9B;
localparam opc_MF  = 8'h9C;
localparam opc_MC  = 8'h9D;
localparam opc_MA  = 8'h9E;
localparam opc_MB  = 8'h9F;
localparam opc_RN  = 8'hA0;
localparam opc_RM  = 8'hA1;
localparam opc_REB = 8'hA2;
localparam opc_RW  = 8'hA3;
localparam opc_RD  = 8'hA4;
localparam opc_RL  = 8'hA5;
localparam opc_RS  = 8'hA6;
localparam opc_RP  = 8'hA7;
localparam opc_RE  = 8'hA8;
localparam opc_RJ  = 8'hA9;
localparam opc_RI  = 8'hAA;
localparam opc_RT  = 8'hAB;
localparam opc_RF  = 8'hAC;
localparam opc_RC  = 8'hAD;
localparam opc_RA  = 8'hAE;
localparam opc_RB  = 8'hAF;
localparam opc_WN  = 8'hB0;
localparam opc_WM  = 8'hB1;
localparam opc_WR  = 8'hB2;
localparam opc_RER = 8'hB3;
localparam opc_WD  = 8'hB4;
localparam opc_WL  = 8'hB5;
localparam opc_WS  = 8'hB6;
localparam opc_WP  = 8'hB7;
localparam opc_WE  = 8'hB8;
localparam opc_WJ  = 8'hB9;
localparam opc_WI  = 8'hBA;
localparam opc_WT  = 8'hBB;
localparam opc_WF  = 8'hBC;
localparam opc_WC  = 8'hBD;
localparam opc_WA  = 8'hBE;
localparam opc_WB  = 8'hBF;
localparam opc_DN  = 8'hC0;
localparam opc_DM  = 8'hC1;
localparam opc_DR  = 8'hC2;
localparam opc_DW  = 8'hC3;
localparam opc_DD  = 8'hC4;
localparam opc_DL  = 8'hC5;
localparam opc_DS  = 8'hC6;
localparam opc_DP  = 8'hC7;
localparam opc_DE  = 8'hC8;
localparam opc_DJ  = 8'hC9;
localparam opc_DI  = 8'hCA;
localparam opc_DT  = 8'hCB;
localparam opc_DF  = 8'hCC;
localparam opc_DC  = 8'hCD;
localparam opc_DA  = 8'hCE;
localparam opc_DB  = 8'hCF;
localparam opc_LN  = 8'hD0;
localparam opc_LM  = 8'hD1;
localparam opc_LR  = 8'hD2;
localparam opc_LW  = 8'hD3;
localparam opc_LD  = 8'hD4;
localparam opc_LL  = 8'hD5;
localparam opc_LS  = 8'hD6;
localparam opc_LP  = 8'hD7;
localparam opc_LE  = 8'hD8;
localparam opc_LJ  = 8'hD9;
localparam opc_LI  = 8'hDA;
localparam opc_LT  = 8'hDB;
localparam opc_LF  = 8'hDC;
localparam opc_LC  = 8'hDD;
localparam opc_LA  = 8'hDE;
localparam opc_LB  = 8'hDF;
localparam opc_SN  = 8'hE0;
localparam opc_SM  = 8'hE1;
localparam opc_SR  = 8'hE2;
localparam opc_SW  = 8'hE3;
localparam opc_SD  = 8'hE4;
localparam opc_SL  = 8'hE5;
localparam opc_SS  = 8'hE6;
localparam opc_SP  = 8'hE7;
localparam opc_SE  = 8'hE8;
localparam opc_SJ  = 8'hE9;
localparam opc_SI  = 8'hEA;
localparam opc_ST  = 8'hEB;
localparam opc_SF  = 8'hEC;
localparam opc_SC  = 8'hED;
localparam opc_SA  = 8'hEE;
localparam opc_SB  = 8'hEF;
localparam opc_PN  = 8'hF0;
localparam opc_PM  = 8'hF1;
localparam opc_PR  = 8'hF2;
localparam opc_PW  = 8'hF3;
localparam opc_PD  = 8'hF4;
localparam opc_PL  = 8'hF5;
localparam opc_PS  = 8'hF6;
localparam opc_PP  = 8'hF7;
localparam opc_PE  = 8'hF8;
localparam opc_PJ  = 8'hF9;
localparam opc_PI  = 8'hFA;
localparam opc_PT  = 8'hFB;
localparam opc_PF  = 8'hFC;
localparam opc_PC  = 8'hFD;
localparam opc_PA  = 8'hFE;
localparam opc_PB  = 8'hFF;

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

localparam GLOBAL_PAGE = 8'd255;
localparam LOCAL_PAGE = 8'd254;

/* CPU registers */

reg ['h7:0] pc;
reg ['h7:0] reg_C;
reg ['h7:0] reg_I;
reg ['h7:0] reg_R;
reg ['h7:0] reg_R_bias;
reg ['h7:0] reg_D;
reg ['h7:0] reg_L = LOCAL_PAGE;
reg ['h7:0] reg_E;
reg ['h7:0] par_ir; /* Connect to tri-state IO bus */
reg ['h7:0] par_or; /* Connect to tri-state IO bus */
reg ['h7:0] ser_ir;
reg ['h7:0] ser_or;
reg ['h0:0] par_rdy; /* Controls IO bus impedance */
reg ['h7:0] reg_A;
reg ['h7:0] reg_B;
reg ['h7:0] reg_W;

/* MISC */

reg [7:0] opcode;
reg [7:0] source_val;
reg [2:0] cpu_phase;

reg ['h14:0] xMx;
reg ['h7:0] reg_CLIP_A;
reg ['h7:0] reg_CLIP_B;
reg ['h7:0] reg_CLIP_R;
reg ['h8:0] temp9bits;



always @(posedge clk)
case (cpu_phase)
0: cpu_phase = 1;
1: cpu_phase = 2;
2: cpu_phase = 0;
endcase



/* Device Select Register E:
  High order nybble encodes 'active high' device select line,
  low order nybble encodes 'active low' device select line.
  Value one: Empty device/NOP
*/

reg [15:0] io_devsel_AL;
reg [15:0] io_devsel_AH;
always@(posedge clk)
begin
case (reg_E[3:0])
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
case (reg_E[7:4])
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


task do_CALL;   // DBG: What happens at pc=7F, FF?
input [7:0]src;
begin
   reg_D = pc ? GLOBAL_PAGE : reg_C;
   reg_C = src;
   reg_W = pc;
   xMx = { pc[7] ?GLOBAL_PAGE:reg_C, pc[6:0]};
   pc = 0;
end
endtask


task set_xMx;
input [7:0] offs_reg;
begin
  xMx = {offs_reg[7] ?GLOBAL_PAGE:reg_D, offs_reg[6:0]};
end
endtask


always@(posedge clk)
case (cpu_phase)
      
    0: begin
       mwren = 0;
       maddr = pc[7] ? {GLOBAL_PAGE, pc[6:0]} : {reg_C,pc[6:0]};
       pc = pc + 8'd1;
       end
      
    1: begin
       opcode = mdata_get;

       /* -------------- READ PHASE ---------------------- */
  
       case (opcode)

           /*SYS Instructions*/
           
           opc_NOP:;

           opc_NEW: reg_L = reg_L - 8'd1;

           opc_OLD: reg_L = reg_L + 8'd1;

           opc_SSI:
           begin
             ser_ir = ser_ir << 1;
             ser_ir = ser_ir | sd_miso;
           end
        
           opc_SSO:
           begin
             sd_mosi = ser_or[7];
             ser_or = ser_or << 1;
           end

           opc_SCL: sd_clk = 0;
           
           opc_SCH: sd_clk = 1;
           
           opc_RDY:
           begin
             par_rdy = 1;
             seg7Byte2(reg_A); /*DBG print*/
             seg7Byte3(reg_B);
           end

           /*BIAS*/
           
           opc_R0P: reg_R_bias = 0;
           opc_R1P: reg_R_bias = 1;
           opc_R2P: reg_R_bias = 2;
           opc_R3P: reg_R_bias = 3;
           opc_R4M: reg_R_bias = (8'd4 ^ 8'hFF) + 8'd1;
           opc_R3M: reg_R_bias = (8'd3 ^ 8'hFF) + 8'd1;
           opc_R2M: reg_R_bias = (8'd2 ^ 8'hFF) + 8'd1;
           opc_R1M: reg_R_bias = (8'd1 ^ 8'hFF) + 8'd1;

           /*SCROUNGES*/
           
           opc_RET: /* Scrounge NM */
           begin
             pc =  reg_W;
             reg_C = pc[7] ? GLOBAL_PAGE : reg_D; 
           end

           opc_REA:
           begin
             reg_A = reg_CLIP_A;
             set_xMx(reg_A);
           end

           opc_REB:
           begin
             reg_B = reg_CLIP_B;
             set_xMx(reg_B);
           end

           opc_RER:
           begin
             reg_R = reg_CLIP_R;
             reg_R_bias = 0;
           end

           opc_DD:;
           opc_LL:;
           opc_SS:;
           opc_PP:;


           /*PAIRS*/

  /*N*/    opc_NN,         opc_NR, opc_NW,
           opc_ND, opc_NL, opc_NS, opc_NP,
           opc_NE, opc_NJ, opc_NI, opc_NT,
           opc_NF, opc_NC, opc_NA, opc_NB:
           begin
             maddr = {pc[7] ? GLOBAL_PAGE : reg_C, pc[6:0]};
             pc = pc + 8'd1;
           end


  /*M*/    opc_MN,         opc_MR, opc_MW,
           opc_MD, opc_ML, opc_MS, opc_MP,
           opc_ME, opc_MJ, opc_MI, opc_MT,
           opc_MF, opc_MC, opc_MA, opc_MB: maddr = xMx;


  /*R*/    opc_RN: reg_I = reg_R + reg_R_bias;

           opc_RM:
           begin
             maddr = xMx;
             mdata_put = reg_R + reg_R_bias;
             mwren = 1;
           end

           opc_RW:
           begin
             reg_W = reg_R + reg_R_bias;
             set_xMx(reg_W);
           end
   
           opc_RD: reg_D = reg_R + reg_R_bias;
           opc_RL: reg_L = reg_R + reg_R_bias;
           opc_RS: ser_ir = reg_R + reg_R_bias;
           opc_RP: begin par_or = reg_R + reg_R_bias; par_rdy = 0; end
           opc_RE: reg_E = reg_R + reg_R_bias;
           opc_RJ: pc = reg_R + reg_R_bias;
           opc_RI: begin reg_I = reg_I - 8'd1; if (reg_I) pc = mdata_get; end
           opc_RT: if (reg_R + reg_R_bias != 8'b0) pc = reg_R + reg_R_bias;
           opc_RF: if (reg_R + reg_R_bias == 8'b0) pc = reg_R + reg_R_bias;

           opc_RC: do_CALL(reg_R + reg_R_bias);

           opc_RA:
           begin
             reg_CLIP_A = reg_A;
             reg_A = reg_R + reg_R_bias;
             set_xMx(reg_A);
           end
        
           opc_RB:
           begin
             reg_CLIP_B = reg_B;
             reg_B = reg_R + reg_R_bias;
             set_xMx(reg_B);
           end


  /*W*/    opc_WN: reg_I = reg_W;

           opc_WM:
           begin
             maddr = xMx;
             mdata_put = reg_W;
             mwren = 1;
           end
           
           opc_WD: reg_D = reg_W;
           opc_WL: reg_L = reg_W;
           opc_WS: ser_ir = reg_W;
           
           opc_WP:
           begin
             par_or = reg_W;
             par_rdy = 0;
           end

           opc_WR:
           begin
             reg_CLIP_R = reg_R + reg_R_bias;
             reg_R = reg_W;
             reg_R_bias = 0;
           end

           opc_WE: reg_E = reg_W;
           opc_WJ: pc = reg_W;
           opc_WI: begin reg_I = reg_I - 8'd1; if (reg_I) pc = reg_W; end
           opc_WT: if (reg_R + reg_R_bias != 8'b0) pc = reg_W;
           opc_WF: if (reg_R + reg_R_bias == 8'b0) pc = reg_W;
 
           opc_WC: do_CALL(reg_W);
        
           opc_WA:
           begin
             reg_CLIP_A = reg_A;
             reg_A = reg_W;
             set_xMx(reg_A);
           end
               
           opc_WB:
           begin
             reg_CLIP_B = reg_B;
             reg_B = reg_W;
             set_xMx(reg_B);
           end


  /*D*/    opc_DN: reg_I = reg_D; 

           opc_DM:
           begin
             maddr = xMx;
             mdata_put = reg_D;
             mwren = 1;
           end      

           opc_DR:
           begin
             reg_CLIP_R = reg_R + reg_R_bias;
             reg_R = reg_D;
             reg_R_bias = 0;
           end

           opc_DW:
           begin
             reg_W = reg_D;
             set_xMx(reg_W);
           end

           opc_DL: reg_L = reg_D; 
           opc_DS: ser_or = reg_D;

           opc_DP:
           begin
             par_or = reg_D;
             par_rdy = 0;
           end

           opc_DE: reg_E = reg_D;
           opc_DJ: pc = reg_D;
           opc_DI: begin reg_I = reg_I - 8'd1; if (reg_I) pc = reg_D; end
           opc_DT: if (reg_R + reg_R_bias != 8'b0) pc = reg_D;
           opc_DF: if (reg_R + reg_R_bias == 8'b0) pc = reg_D;
        
           opc_DC:
           begin
             temp9bits = reg_D;
             reg_D = pc ? GLOBAL_PAGE : reg_C;
             reg_C = temp9bits[7:0];
             reg_W = pc;
             xMx = {pc[7] ?GLOBAL_PAGE:reg_D, pc[6:0]};
             pc = 0;
           end

           opc_DA:
           begin
             reg_CLIP_A = reg_A;
             reg_A = reg_D;
             set_xMx(reg_A);
           end
           
           opc_DB:
           begin
             reg_CLIP_B = reg_B;
             reg_B = reg_D;
             set_xMx(reg_B);
           end  

    
  /*L*/    opc_LN: reg_I = reg_L;

           opc_LM:
           begin
             maddr = xMx;
             mdata_put = reg_L;
             mwren = 1;
           end

           opc_LR:
           begin
             reg_CLIP_R = reg_R + reg_R_bias;
             reg_R = reg_L;
             reg_R_bias = 0;
           end

           opc_LW:
           begin
             reg_W = reg_L;
             set_xMx(reg_W);
           end  
           
           opc_LD: reg_D = reg_L;
           opc_LS: ser_ir = reg_L;
           opc_LP: begin par_or = reg_L; par_rdy = 0; end

           opc_LE: reg_E = reg_L;
           opc_LJ: pc = reg_L;
           opc_LI: begin reg_I = reg_I - 8'd1; if (reg_I) pc = reg_L; end
           opc_LT: if (reg_R + reg_R_bias != 8'b0) pc = reg_L;
           opc_LF: if (reg_R + reg_R_bias == 8'b0) pc = reg_L;

           opc_LC: do_CALL(reg_L);
        
           opc_LA:
           begin
             reg_CLIP_A = reg_A;
             reg_A = reg_L;
             set_xMx(reg_A);
           end
               
           opc_LB:
           begin
             reg_CLIP_B = reg_B;
             reg_B = reg_L;
             set_xMx(reg_B);
           end


  /*S*/    opc_SN: reg_I = ser_ir;  /*DBG, TODO: All these should tristate POR*/

           opc_SM:
           begin
             maddr = xMx;
             mdata_put = ser_ir;
             mwren = 1;
           end

           opc_SR:
           begin
             reg_CLIP_R = reg_R + reg_R_bias;
             reg_R = ser_ir;
             reg_R_bias = 0;
           end

           opc_SW:
           begin
             reg_W = ser_ir;
             set_xMx(reg_W);
           end  

           opc_SD: reg_D = ser_ir;
           opc_SL: reg_L = ser_ir;
           opc_SP: begin par_or = ser_ir; par_rdy = 0; end

           opc_SE: reg_E = ser_ir;
           opc_SJ: pc = ser_ir;
           opc_SI: begin reg_I =reg_I - 8'd1; if (reg_I) pc = ser_ir; end
           opc_ST: if (reg_R + reg_R_bias != 8'b0) pc = ser_ir;
           opc_SF: if (reg_R + reg_R_bias == 8'b0) pc = ser_ir;

           opc_SC: do_CALL(ser_ir);

           opc_SA:
           begin
             reg_CLIP_A = reg_A;
             reg_A = ser_ir;
             set_xMx(reg_A);
           end
               
           opc_SB:
           begin
             reg_CLIP_B = reg_B;
             reg_B = ser_ir;
             set_xMx(reg_B);
           end


  /*P*/    opc_PN: reg_I = par_ir;

           opc_PM:
           begin
             maddr = xMx;
             mdata_put = par_ir;
             mwren = 1;
           end

           opc_PR:
           begin
             reg_CLIP_R = reg_R + reg_R_bias;
             reg_R = par_ir;
             reg_R_bias = 0;
           end

           opc_PW:
           begin
             reg_W = par_ir;
             set_xMx(reg_W);
           end  

           opc_PD: reg_D = par_ir;
           opc_PL: reg_L = par_ir;
           opc_PS: ser_or = par_ir;

           opc_PE: reg_E = par_ir;
           opc_PJ: pc = par_ir;
           opc_PI: begin reg_I =reg_I - 8'd1; if (reg_I) pc = par_ir; end
           opc_PT: if (reg_R + reg_R_bias != 8'b0) pc = par_ir;
           opc_PF: if (reg_R + reg_R_bias == 8'b0) pc = par_ir;

           opc_PC: do_CALL(par_ir);
           
           opc_PA:
           begin
             reg_CLIP_A = reg_A;
             reg_A = par_ir;
             set_xMx(reg_A);
           end
               
           opc_PB:
           begin
             reg_CLIP_B = reg_B;
             reg_B = par_ir;
             set_xMx(reg_B);
           end


           /*Traps, 32 opcodes*/ 
           
           default:
           begin
             reg_W = pc;
             reg_D = pc[7] ? GLOBAL_PAGE : reg_C;
             pc = 0;
             reg_C = opcode[4:0];
           end

           /*GETPUT*/
        
           opc_G0A, opc_G0B, opc_G0W, opc_G0R: maddr = {GLOBAL_PAGE, 7'h7C};
           opc_G1A, opc_G1B, opc_G1W, opc_G1R: maddr = {GLOBAL_PAGE, 7'h7D};
           opc_G2A, opc_G2B, opc_G2W, opc_G2R: maddr = {GLOBAL_PAGE, 7'h7E};
           opc_G3A, opc_G3B, opc_G3W, opc_G3R: maddr = {GLOBAL_PAGE, 7'h7F};
           opc_L0A, opc_L0B, opc_L0W, opc_L0R: maddr = {reg_L, 7'h7C};
           opc_L1A, opc_L1B, opc_L1W, opc_L1R: maddr = {reg_L, 7'h7D};
           opc_L2A, opc_L2B, opc_L2W, opc_L2R: maddr = {reg_L, 7'h7E};
           opc_L3A, opc_L3B, opc_L3W, opc_L3R: maddr = {reg_L, 7'h7F};
           opc_AG0, opc_BG0, opc_WG0, opc_RG0: maddr = {GLOBAL_PAGE, 7'h7C};
           opc_AG1, opc_BG1, opc_WG1, opc_RG1: maddr = {GLOBAL_PAGE, 7'h7D};
           opc_AG2, opc_BG2, opc_WG2, opc_RG2: maddr = {GLOBAL_PAGE, 7'h7E};
           opc_AG3, opc_BG3, opc_WG3, opc_RG3: maddr = {GLOBAL_PAGE, 7'h7F};
           opc_AL0, opc_BL0, opc_WL0, opc_RL0: maddr = {reg_L, 7'h7C};
           opc_AL1, opc_BL1, opc_WL1, opc_RL1: maddr = {reg_L, 7'h7D};
           opc_AL2, opc_BL2, opc_WL2, opc_RL2: maddr = {reg_L, 7'h7E};
           opc_AL3, opc_BL3, opc_WL3, opc_RL3: maddr = {reg_L, 7'h7F};
    

          /*ALU*/
    
           opc_IDA:
           begin
             reg_R = reg_A;
             reg_R_bias = 0;
           end

           opc_IDB:
           begin
             reg_R = reg_B;
             reg_R_bias = 0;
           end

           opc_OCA:
           begin
             reg_R = reg_A ^ 8'hFF;
             reg_R_bias = 0;
           end

           opc_OCB:
           begin
             reg_R = reg_B ^ 8'hFF;
             reg_R_bias = 0;
           end

           opc_SLA:
           begin
             reg_R = reg_A << 1;
             reg_R_bias = 0;
           end
        
           opc_SLB:
           begin
             reg_R = reg_B << 1;
             reg_R_bias = 0;
           end

          opc_SRA:
          begin
            reg_R = reg_A >> 1;
            reg_R_bias = 0;
          end

          opc_SRB:
          begin
            reg_R = reg_B >> 1;
            reg_R_bias = 0;
          end

          opc_AND:
          begin
            reg_R = reg_A & reg_B;
            reg_R_bias = 0;
          end

          opc_IOR:
          begin
            reg_R = reg_A | reg_B;
            reg_R_bias = 0;
          end

          opc_EOR:
          begin
            reg_R = reg_A ^ reg_B;
            reg_R_bias = 0;
          end

          opc_ADD:
          begin
            reg_R = reg_A + reg_B;
            reg_R_bias = 0;
          end

          opc_CAR:
          begin
            temp9bits = reg_A + reg_B;
            reg_R = temp9bits[8] ? 1'b1 : 1'b0;
            reg_R_bias = 0;
          end
        
          opc_ALB:
          begin
            reg_R = (reg_A < reg_B) ? 8'd255:8'd0;
            reg_R_bias = 0;
          end
        
          opc_AEB:
          begin
            reg_R = (reg_A == reg_B) ? 8'd255:8'd0;
            reg_R_bias = 0;
          end

          opc_AGB:
          begin
            reg_R = (reg_A > reg_B) ? 8'd255:8'd0;
            reg_R_bias = 0;
          end
        
        endcase
        end /*of READ PHASE*/
      
    /* -------------- WRITE PHASE ---------------------- */
    
      2: case (opcode)
    
        opc_NN, opc_MN: reg_I = mdata_get;
        /*NM Scrounged RET*/

        opc_NR, opc_MR:
        begin
          reg_CLIP_R = reg_R + reg_R_bias;
          reg_R = mdata_get;
          reg_R_bias = 0;
        end

        opc_ND, opc_MD: reg_D = mdata_get;
        opc_NL, opc_ML: reg_L = mdata_get;
        opc_NS, opc_MS: ser_or = mdata_get;
        opc_NP, opc_MP: par_or = mdata_get;
        opc_NE, opc_ME: reg_E = mdata_get; 
        opc_NJ, opc_MJ: begin pc = mdata_get; seg7Byte1(pc); end /*dbg*/
        opc_NI, opc_MI: begin reg_I = reg_I - 8'b1; if (reg_I) pc = mdata_get; end
        opc_NT, opc_MT: if (reg_R + reg_R_bias != 8'b0) pc = mdata_get;
        opc_NF, opc_MF: if (reg_R + reg_R_bias == 8'b0) pc = mdata_get;
             
        opc_NC, opc_MC: do_CALL(mdata_get);
        
        opc_NA, opc_MA:
        begin
          reg_CLIP_A = reg_A;
          reg_A = mdata_get;
          set_xMx(reg_A);
        end
          
        opc_NB, opc_MB:
        begin
          reg_CLIP_B = reg_B;
          reg_B = mdata_get;
          set_xMx(reg_B);
        end

        opc_NW, opc_MW:
        begin
          reg_W = mdata_get;
          set_xMx(reg_W);
        end


         /*Traps, 32 opcodes*/ 
        default:;


        /*GETPUT*/
        
        opc_G0A, opc_G1A, opc_G2A, opc_G3A, /*A*/
        opc_L0A, opc_L1A, opc_L2A, opc_L3A:
        begin
          reg_CLIP_A = reg_A;
          reg_A = mdata_get;
          set_xMx(reg_A);
        end
           
        opc_AG0, opc_AG1, opc_AG2, opc_AG3,
        opc_AL0, opc_AL1, opc_AL2, opc_AL3:
        begin
          mdata_put = reg_A;
          mwren = 1;
        end

        opc_G0B, opc_G1B, opc_G2B, opc_G3B, /*B*/
        opc_L0B, opc_L1B, opc_L2B, opc_L3B:
        begin
          reg_CLIP_B = reg_B;
          reg_B = mdata_get;
          set_xMx(reg_B);
        end
        
        opc_BG0, opc_BG1, opc_BG2, opc_BG3,
        opc_BL0, opc_BL1, opc_BL2, opc_BL3:
        begin
          mdata_put = reg_B;
          mwren = 1;
        end

        opc_G0W, opc_G1W, opc_G2W, opc_G3W, /*W*/
        opc_L0W, opc_L1W, opc_L2W, opc_L3W:
        begin
          reg_W = mdata_get;
          set_xMx(reg_W);
        end
        
        opc_WG0, opc_WG1, opc_WG2, opc_WG3,
        opc_WL0, opc_WL1, opc_WL2, opc_WL3:
        begin
          mdata_put = reg_W;
          mwren = 1;
        end

        opc_G0R, opc_G1R, opc_G2R, opc_G3R,  /*R*/
        opc_L0R, opc_L1R, opc_L2R, opc_L3R:
        begin
          reg_CLIP_R = reg_R + reg_R_bias;
          reg_R = mdata_get;
          reg_R_bias = 0;
        end
        
        opc_RG0, opc_RG1, opc_RG2, opc_RG3,
        opc_RL0, opc_RL1, opc_RL2, opc_RL3:
        begin
          mdata_put = reg_R + reg_R_bias;
          mwren = 1;
        end
        
        endcase
    
endcase
endmodule


/* Use my sasm.c to assemble the following source
   and populate the RAM in Quartus with the resulting .mif file.


77A 14B *divmod8 RDY ; RDY instruction "hijacked" for 7-segment display output
stop@ <stop:J   ; This outputs 07 05 (remainder, quotient of 5*14+7=77)
CLOSE


77A, 14B *divmod8 RDY  ;= 57h
endless@ NJ <endless
CLOSE

@divmod8 ; Divide A by B, division result in A, remainder in B
NEW wG0

aL0                       ; Dividend
bL1                       ; Divisor
NA 1, aL2                 ; Shift counter first 1 bit to MSB
NA 0, aL3                 ; Initialise quotient to zero

L1a IDA NF >ELOOP         ; Skip if divisor zero

NA 80h                    ; MSB mask
MSB_SHIFT@                ; Shift divisor left so that first 1 bit is at MSB
 L1b                      ; Load divisor
 AND NT >DIVIDE           ; Skip when MSB set
 SLB rL1                  ; Shift divisor left and update
 L2r R1+ rL2              ; Increment shift counter and update
 NJ <MSB_SHIFT

DIVIDE@
 L3b SLB rL3              ; Shift quotient left and update
 L1a OCA R1+ RA           ; Negate divisor
 L0b CAR                  ; Dividend check borrow bit
 NF >REP

 ADD rL0                  ; Accept subtraction, update dividend
 L3r R1+ rL3              ; Increment quotient

REP@
 L1a SRA rL1              ; Shift divisor right for next subtraction
 L2r R1- rL2              ; Decrement counter
 NT <DIVIDE               ; Branch back if not zero

ELOOP@ L3a, L0b

G0w OLD
RET
CLOSE

*/
