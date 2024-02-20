
/* Sonne Microcontroller rev. Myth
    Verilog Implementation
  Feb-2024 Michael Mangelsdorf
  Copyr. <mim@ok-schalter.de>

   Run the mul8/div8 code
   on a Terasic DE1Soc board
   result on 7-segment display.

   Around 548 ALMs, 366 regs
   Block memory bits:64k*8
   50MHz clock
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


/* CPU - The binary layout of these opcodes is follows
   regular, see documention and discrete schematics.
   (There are six priority-encoded instruction groups.)
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

/*ADJUST*/

localparam opc_P4 = 8'h08;
localparam opc_P1 = 8'h09;
localparam opc_P2 = 8'h0A;
localparam opc_P3 = 8'h0B;
localparam opc_M4 = 8'h0C;
localparam opc_M3 = 8'h0D;
localparam opc_M2 = 8'h0E;
localparam opc_M1 = 8'h0F;

/*ALU*/

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
localparam opc_G0I = 8'h43;
localparam opc_L0A = 8'h44;
localparam opc_L0B = 8'h45;
localparam opc_L0R = 8'h46;
localparam opc_L0I = 8'h47;
localparam opc_AG0 = 8'h48;
localparam opc_BG0 = 8'h49;
localparam opc_RG0 = 8'h4A;
localparam opc_IG0 = 8'h4B;
localparam opc_AL0 = 8'h4C;
localparam opc_BL0 = 8'h4D;
localparam opc_RL0 = 8'h4E;
localparam opc_IL0 = 8'h4F;
localparam opc_G1A = 8'h50;
localparam opc_G1B = 8'h51;
localparam opc_G1R = 8'h52;
localparam opc_G1I = 8'h53;
localparam opc_L1A = 8'h54;
localparam opc_L1B = 8'h55;
localparam opc_L1R = 8'h56;
localparam opc_L1I = 8'h57;
localparam opc_AG1 = 8'h58;
localparam opc_BG1 = 8'h59;
localparam opc_RG1 = 8'h5A;
localparam opc_IG1 = 8'h5B;
localparam opc_AL1 = 8'h5C;
localparam opc_BL1 = 8'h5D;
localparam opc_RL1 = 8'h5E;
localparam opc_IL1 = 8'h5F;
localparam opc_G2A = 8'h60;
localparam opc_G2B = 8'h61;
localparam opc_G2R = 8'h62;
localparam opc_G2I = 8'h63;
localparam opc_L2A = 8'h64;
localparam opc_L2B = 8'h65;
localparam opc_L2R = 8'h66;
localparam opc_L2I = 8'h67;
localparam opc_AG2 = 8'h68;
localparam opc_BG2 = 8'h69;
localparam opc_RG2 = 8'h6A;
localparam opc_IG2 = 8'h6B;
localparam opc_AL2 = 8'h6C;
localparam opc_BL2 = 8'h6D;
localparam opc_RL2 = 8'h6E;
localparam opc_IL2 = 8'h6F;
localparam opc_G3A = 8'h70;
localparam opc_G3B = 8'h71;
localparam opc_G3R = 8'h72;
localparam opc_G3I = 8'h73;
localparam opc_L3A = 8'h74;
localparam opc_L3B = 8'h75;
localparam opc_L3R = 8'h76;
localparam opc_L3I = 8'h77;
localparam opc_AG3 = 8'h78;
localparam opc_BG3 = 8'h79;
localparam opc_RG3 = 8'h7A;
localparam opc_IG3 = 8'h7B;
localparam opc_AL3 = 8'h7C;
localparam opc_BL3 = 8'h7D;
localparam opc_RL3 = 8'h7E;
localparam opc_IL3 = 8'h7F;

/*PAIR*/

localparam opc_NG  = 8'h80;
localparam opc_RET = 8'h81;
localparam opc_ND  = 8'h82;
localparam opc_NO  = 8'h83;
localparam opc_NR  = 8'h84;
localparam opc_NI  = 8'h85;
localparam opc_NS  = 8'h86;
localparam opc_NP  = 8'h87;
localparam opc_NE  = 8'h88;
localparam opc_NX  = 8'h89;
localparam opc_NJ  = 8'h8A;
localparam opc_NT  = 8'h8B;
localparam opc_NF  = 8'h8C;
localparam opc_NC  = 8'h8D;
localparam opc_NA  = 8'h8E;
localparam opc_NB  = 8'h8F;
localparam opc_MG  = 8'h90;
localparam opc_CLR = 8'h91;
localparam opc_MD  = 8'h92;
localparam opc_MO  = 8'h93;
localparam opc_MR  = 8'h94;
localparam opc_MI  = 8'h95;
localparam opc_MS  = 8'h96;
localparam opc_MP  = 8'h97;
localparam opc_ME  = 8'h98;
localparam opc_MX  = 8'h99;
localparam opc_MJ  = 8'h9A;
localparam opc_MT  = 8'h9B;
localparam opc_MF  = 8'h9C;
localparam opc_MC  = 8'h9D;
localparam opc_MA  = 8'h9E;
localparam opc_MB  = 8'h9F;
localparam opc_DG  = 8'hA0;
localparam opc_DM  = 8'hA1;
localparam opc_DD  = 8'hA2;
localparam opc_DO  = 8'hA3;
localparam opc_DR  = 8'hA4;
localparam opc_DI  = 8'hA5;
localparam opc_DS  = 8'hA6;
localparam opc_DP  = 8'hA7;
localparam opc_DE  = 8'hA8;
localparam opc_DX  = 8'hA9;
localparam opc_DJ  = 8'hAA;
localparam opc_DT  = 8'hAB;
localparam opc_DF  = 8'hAC;
localparam opc_LJO = 8'hAD;
localparam opc_DA  = 8'hAE;
localparam opc_DB  = 8'hAF;
localparam opc_OG  = 8'hB0;
localparam opc_OM  = 8'hB1;
localparam opc_OD  = 8'hB2;
localparam opc_OO  = 8'hB3;
localparam opc_OR  = 8'hB4;
localparam opc_OI  = 8'hB5;
localparam opc_OS  = 8'hB6;
localparam opc_OP  = 8'hB7;
localparam opc_OE  = 8'hB8;
localparam opc_OX  = 8'hB9;
localparam opc_OJ  = 8'hBA;
localparam opc_OT  = 8'hBB;
localparam opc_OF  = 8'hBC;
localparam opc_CPI = 8'hBD;
localparam opc_OA  = 8'hBE;
localparam opc_OB  = 8'hBF;
localparam opc_RG  = 8'hC0;
localparam opc_RM  = 8'hC1;
localparam opc_RD  = 8'hC2;
localparam opc_RO  = 8'hC3;
localparam opc_RR  = 8'hC4;
localparam opc_RI  = 8'hC5;
localparam opc_RS  = 8'hC6;
localparam opc_RP  = 8'hC7;
localparam opc_RE  = 8'hC8;
localparam opc_RX  = 8'hC9;
localparam opc_RJ  = 8'hCA;
localparam opc_RT  = 8'hCB;
localparam opc_RF  = 8'hCC;
localparam opc_RC  = 8'hCD;
localparam opc_RA  = 8'hCE;
localparam opc_RB  = 8'hCF;
localparam opc_IG  = 8'hD0;
localparam opc_IM  = 8'hD1;
localparam opc_ID  = 8'hD2;
localparam opc_IO  = 8'hD3;
localparam opc_IR  = 8'hD4;
localparam opc_II  = 8'hD5;
localparam opc_IS  = 8'hD6;
localparam opc_IP  = 8'hD7;
localparam opc_IE  = 8'hD8;
localparam opc_IX  = 8'hD9;
localparam opc_IJ  = 8'hDA;
localparam opc_IT  = 8'hDB;
localparam opc_IF  = 8'hDC;
localparam opc_IC  = 8'hDD;
localparam opc_IA  = 8'hDE;
localparam opc_IB  = 8'hDF;
localparam opc_SG  = 8'hE0;
localparam opc_SM  = 8'hE1;
localparam opc_SD  = 8'hE2;
localparam opc_SO  = 8'hE3;
localparam opc_SR  = 8'hE4;
localparam opc_SI  = 8'hE5;
localparam opc_SS  = 8'hE6;
localparam opc_SP  = 8'hE7;
localparam opc_SE  = 8'hE8;
localparam opc_SX  = 8'hE9;
localparam opc_SJ  = 8'hEA;
localparam opc_ST  = 8'hEB;
localparam opc_SF  = 8'hEC;
localparam opc_SC  = 8'hED;
localparam opc_SA  = 8'hEE;
localparam opc_SB  = 8'hEF;
localparam opc_PG  = 8'hF0;
localparam opc_PM  = 8'hF1;
localparam opc_PD  = 8'hF2;
localparam opc_PO  = 8'hF3;
localparam opc_PR  = 8'hF4;
localparam opc_PI  = 8'hF5;
localparam opc_PS  = 8'hF6;
localparam opc_PP  = 8'hF7;
localparam opc_PE  = 8'hF8;
localparam opc_PX  = 8'hF9;
localparam opc_PJ  = 8'hFA;
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

/* CPU registers */

reg ['h7:0] pc;
reg ['h7:0] reg_C;
reg ['h7:0] reg_I;
reg ['h7:0] reg_R;
reg ['h7:0] reg_D;
reg ['h7:0] reg_L = GLOBAL_PAGE;
reg ['h7:0] reg_G = GLOBAL_PAGE;
reg ['h7:0] reg_E;
reg ['h7:0] par_ir; /* Connect to tri-state IO bus */
reg ['h7:0] par_or; /* Connect to tri-state IO bus */
reg ['h7:0] ser_ir;
reg ['h7:0] ser_or;
reg ['h0:0] par_rdy; /* Controls POR output impedance */
reg ['h7:0] reg_A;
reg ['h7:0] reg_B;
reg ['h7:0] reg_O;

/* MISC */

reg [7:0] opcode;
reg [7:0] source_val;
reg [2:0] cpu_phase;

reg ['h7:0] ljo;
reg ['h7:0] xMx;
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


task do_CALL;
input [7:0]src;
begin
   reg_D = reg_C;
   reg_C = src;
   reg_O = pc;
   pc = 0;
end
endtask


always@(posedge clk)
case (cpu_phase)
      
    0: begin
       mwren = 0;
       maddr = { pc&128 ? 255 : reg_C, pc[6:0]};
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

           /*ADJUST*/
           
           opc_P4: reg_R = reg_R + 8'd4;
           opc_P1: reg_R = reg_R + 8'd1;
           opc_P2: reg_R = reg_R + 8'd2;
           opc_P3: reg_R = reg_R + 8'd3;
           opc_M4: reg_R = reg_R + (8'd4 ^ 8'hFF) + 8'd1;
           opc_M3: reg_R = reg_R + (8'd3 ^ 8'hFF) + 8'd1;
           opc_M2: reg_R = reg_R + (8'd2 ^ 8'hFF) + 8'd1;
           opc_M1: reg_R = reg_R + (8'd1 ^ 8'hFF) + 8'd1;

           /*SCROUNGES*/
           
           opc_RET: /* Scrounge NM */
           begin
             pc =  reg_O;
             reg_C = reg_D;
           end

           opc_CPI: reg_R = reg_C; /* Scrounge OC */
           opc_LJO: reg_R = ljo;   /* Scrounge DC */
           opc_CLR: reg_L = 0;     /* Scrounge MM */

           opc_DD:;
           opc_OO:;
           opc_II:;
           opc_SS:;
           opc_PP:;


           /*PAIRS*/

  /*N*/    opc_NG,         opc_NR, opc_NO,
           opc_ND, opc_NI, opc_NS, opc_NP,
           opc_NE, opc_NJ, opc_NX, opc_NT,
           opc_NF, opc_NC, opc_NA, opc_NB:
           begin
             maddr = { pc&128 ? 255 : reg_C, pc[6:0] };
             pc = pc + 8'd1;
           end


  /*M*/    opc_MG,         opc_MR, opc_MO,
           opc_MD, opc_MI, opc_MS, opc_MP,
           opc_ME, opc_MJ, opc_MX, opc_MT,
           opc_MF, opc_MC, opc_MA, opc_MB: maddr = { xMx&128 ? reg_L : reg_D, xMx[6:0] };


  /*R*/    opc_RG: reg_G = reg_R;

           opc_RM:
           begin
             maddr = { xMx&128 ? reg_L : reg_D, xMx[6:0] };
             mdata_put = reg_R;
             mwren = 1;
           end

           opc_RO: reg_O = reg_R; 
           opc_RD: reg_D = reg_R;
           opc_RI: reg_I = reg_R;
           opc_RS: ser_ir = reg_R;
           opc_RP: begin par_or = reg_R; par_rdy = 0; end
           opc_RE: reg_E = reg_R;
           opc_RJ: pc = reg_R;
           opc_RX: begin reg_I = reg_I - 8'd1; if (reg_I) pc = reg_R; end
           opc_RT: if (reg_R != 8'b0) pc = reg_R;
           opc_RF: if (reg_R == 8'b0) pc = reg_R;

           opc_RC: do_CALL(reg_R);

           opc_RA:
           begin
             reg_A = reg_R;
             xMx = reg_A;
           end
        
           opc_RB:
           begin
             reg_B = reg_R;
             xMx = reg_B;
           end


  /*O*/    opc_OG: reg_G = reg_O;

           opc_OM:
           begin
             maddr = { xMx&128 ? reg_L : reg_D, xMx[6:0] };
             mdata_put = reg_O;
             mwren = 1;
           end
           
           opc_OD: reg_D = reg_O;
           opc_OI: reg_I = reg_O;
           opc_OS: ser_ir = reg_O;
           
           opc_OP:
           begin
             par_or = reg_O;
             par_rdy = 0;
           end

           opc_OR: reg_R = reg_O;

           opc_OE: reg_E = reg_O;
           opc_OJ: pc = reg_O;
           opc_OX: begin reg_I = reg_I - 8'd1; if (reg_I) pc = reg_O; end
           opc_OT: if (reg_R != 8'b0) pc = reg_O;
           opc_OF: if (reg_R == 8'b0) pc = reg_O;
         
           opc_OA:
           begin
             reg_A = reg_O;
             xMx = reg_A;
           end
               
           opc_OB:
           begin
             reg_B = reg_O;
             xMx = reg_B;
           end


  /*D*/    opc_DG: reg_G = reg_D; 

           opc_DM:
           begin
             maddr = { xMx&128 ? reg_L : reg_D, xMx[6:0] };
             mdata_put = reg_D;
             mwren = 1;
           end      

           opc_DR: reg_R = reg_D;

           opc_DO: reg_O = reg_D;
           opc_DI: reg_I = reg_D; 
           opc_DS: ser_or = reg_D;

           opc_DP:
           begin
             par_or = reg_D;
             par_rdy = 0;
           end

           opc_DE: reg_E = reg_D;
           opc_DJ: pc = reg_D;
           opc_DX: begin reg_I = reg_I - 8'd1; if (reg_I) pc = reg_D; end
           opc_DT: if (reg_R != 8'b0) pc = reg_D;
           opc_DF: if (reg_R == 8'b0) pc = reg_D;
        
           opc_DA:
           begin
             reg_A = reg_D;
             xMx = reg_A;
           end
           
           opc_DB:
           begin
             reg_B = reg_D;
             xMx = reg_B;
           end  

    
  /*I*/    opc_IG: reg_G = reg_I;

           opc_IM:
           begin
             maddr = { xMx&128 ? reg_L : reg_D, xMx[6:0] };
             mdata_put = reg_I;
             mwren = 1;
           end

           opc_IR: reg_R = reg_I;
           opc_IO: reg_O = reg_I;
           opc_ID: reg_D = reg_I;
           opc_IS: ser_ir = reg_I;
           opc_IP: begin par_or = reg_I; par_rdy = 0; end

           opc_IE: reg_E = reg_I;
           opc_IJ: pc = reg_I;
           opc_IX: begin reg_I = reg_I - 8'd1; if (reg_I) pc = reg_I; end
           opc_IT: if (reg_R != 8'b0) pc = reg_I;
           opc_IF: if (reg_R == 8'b0) pc = reg_I;

           opc_IC: do_CALL(reg_I);
        
           opc_IA:
           begin
             reg_A = reg_I;
             xMx = reg_A;
           end
               
           opc_IB:
           begin
             reg_B = reg_I;
             xMx = reg_B;
           end


  /*S*/    opc_SG: reg_I = ser_ir;  /*DBG, TODO: All these should tristate POR*/

           opc_SM:
           begin
             maddr = { xMx&128 ? reg_L : reg_D, xMx[6:0] };
             mdata_put = ser_ir;
             mwren = 1;
           end

           opc_SR: reg_R = ser_ir;

           opc_SO: reg_O = ser_ir;
           opc_SD: reg_D = ser_ir;
           opc_SI: reg_I = ser_ir;
           opc_SP: begin par_or = ser_ir; par_rdy = 0; end

           opc_SE: reg_E = ser_ir;
           opc_SJ: pc = ser_ir;
           opc_SX: begin reg_I =reg_I - 8'd1; if (reg_I) pc = ser_ir; end
           opc_ST: if (reg_R != 8'b0) pc = ser_ir;
           opc_SF: if (reg_R == 8'b0) pc = ser_ir;

           opc_SC: do_CALL(ser_ir);

           opc_SA:
           begin
             reg_A = ser_ir;
             xMx = reg_A;
           end
               
           opc_SB:
           begin
             reg_B = ser_ir;
             xMx = reg_B;
           end


  /*P*/    opc_PG: reg_I = par_ir;

           opc_PM:
           begin
             maddr = { xMx&128 ? reg_L : reg_D, xMx[6:0] };
             mdata_put = par_ir;
             mwren = 1;
           end

           opc_PR: reg_R = par_ir;

           opc_PO: reg_O = par_ir;
           opc_PD: reg_D = par_ir;
           opc_PI: reg_I = par_ir;
           opc_PS: ser_or = par_ir;

           opc_PE: reg_E = par_ir;
           opc_PJ: pc = par_ir;
           opc_PX: begin reg_I =reg_I - 8'd1; if (reg_I) pc = par_ir; end
           opc_PT: if (reg_R != 8'b0) pc = par_ir;
           opc_PF: if (reg_R == 8'b0) pc = par_ir;

           opc_PC: do_CALL(par_ir);
           
           opc_PA:
           begin
             reg_A = par_ir;
             xMx = reg_A;
           end
               
           opc_PB:
           begin
             reg_B = par_ir;
             xMx = reg_B;
           end


           /*Traps, 32 opcodes*/ 
           
           default:
           begin
             reg_O = pc;
             pc = 0;
             reg_D = reg_C;
             reg_C = opcode[4:0];
           end

           /*GETPUT*/
        
           opc_G0A, opc_G0B, opc_G0I, opc_G0R: maddr = {GLOBAL_PAGE, 8'h7C};
           opc_G1A, opc_G1B, opc_G1I, opc_G1R: maddr = {GLOBAL_PAGE, 8'h7D};
           opc_G2A, opc_G2B, opc_G2I, opc_G2R: maddr = {GLOBAL_PAGE, 8'h7E};
           opc_G3A, opc_G3B, opc_G3I, opc_G3R: maddr = {GLOBAL_PAGE, 8'h7F};
           opc_L0A, opc_L0B, opc_L0I, opc_L0R: maddr = {reg_L, 8'h7C};
           opc_L1A, opc_L1B, opc_L1I, opc_L1R: maddr = {reg_L, 8'h7D};
           opc_L2A, opc_L2B, opc_L2I, opc_L2R: maddr = {reg_L, 8'h7E};
           opc_L3A, opc_L3B, opc_L3I, opc_L3R: maddr = {reg_L, 8'h7F};
           opc_AG0, opc_BG0, opc_IG0, opc_RG0: maddr = {GLOBAL_PAGE, 8'h7C};
           opc_AG1, opc_BG1, opc_IG1, opc_RG1: maddr = {GLOBAL_PAGE, 8'h7D};
           opc_AG2, opc_BG2, opc_IG2, opc_RG2: maddr = {GLOBAL_PAGE, 8'h7E};
           opc_AG3, opc_BG3, opc_IG3, opc_RG3: maddr = {GLOBAL_PAGE, 8'h7F};
           opc_AL0, opc_BL0, opc_IL0, opc_RL0: maddr = {reg_L, 8'h7C};
           opc_AL1, opc_BL1, opc_IL1, opc_RL1: maddr = {reg_L, 8'h7D};
           opc_AL2, opc_BL2, opc_IL2, opc_RL2: maddr = {reg_L, 8'h7E};
           opc_AL3, opc_BL3, opc_IL3, opc_RL3: maddr = {reg_L, 8'h7F};
    

          /*ALU*/
    
           opc_IDA: reg_R = reg_A;
           opc_IDB: reg_R = reg_B;
           opc_OCA: reg_R = reg_A ^ 8'hFF;
           opc_OCB: reg_R = reg_B ^ 8'hFF;
           opc_SLA: reg_R = reg_A << 1;
           opc_SLB: reg_R = reg_B << 1;
           opc_SRA: reg_R = reg_A >> 1;
           opc_SRB: reg_R = reg_B >> 1;
           opc_AND: reg_R = reg_A & reg_B;
           opc_IOR: reg_R = reg_A | reg_B;
           opc_EOR: reg_R = reg_A ^ reg_B;
           opc_ADD: reg_R = reg_A + reg_B;
           
           opc_CAR:
           begin
            temp9bits = reg_A + reg_B;
            reg_R = temp9bits[8] ? 1'b1 : 1'b0;
           end
           
           opc_ALB:
            reg_R = (reg_A < reg_B) ? 8'd255:8'd0;
           
           opc_AEB:
            reg_R = (reg_A == reg_B) ? 8'd255:8'd0;
           
           opc_AGB:
            reg_R = (reg_A > reg_B) ? 8'd255:8'd0;
        
        endcase
        end /*of READ PHASE*/
      
    /* -------------- WRITE PHASE ---------------------- */
    
      2: case (opcode)
    
        opc_NG, opc_MG: reg_G = mdata_get;
        /*NM Scrounge RET*/

        opc_NR, opc_MR:
          reg_R = mdata_get;

        opc_ND, opc_MD: reg_D = mdata_get;
        opc_NI, opc_MI: reg_I = mdata_get;
        opc_NS, opc_MS: ser_or = mdata_get;
        opc_NP, opc_MP: par_or = mdata_get;
        opc_NE, opc_ME: reg_E = mdata_get; 
        opc_NJ, opc_MJ: begin pc = mdata_get; seg7Byte1(pc); end /*dbg*/
        opc_NX, opc_MX: begin reg_I = reg_I - 8'b1; if (reg_I) pc = mdata_get; end
        opc_NT, opc_MT: if (reg_R != 8'b0) pc = mdata_get;
        opc_NF, opc_MF: if (reg_R == 8'b0) pc = mdata_get;
             
        opc_NC, opc_MC: do_CALL(mdata_get);
        
        opc_NA, opc_MA:
        begin
          reg_A = mdata_get;
          xMx = reg_A;
        end
          
        opc_NB, opc_MB:
        begin
          reg_B = mdata_get;
          xMx = reg_B;
        end

        opc_NO, opc_MO: reg_O = mdata_get;

         /*Traps, 32 opcodes*/ 
        default:;


        /*GETPUT*/
        
        opc_G0A, opc_G1A, opc_G2A, opc_G3A, /*A*/
        opc_L0A, opc_L1A, opc_L2A, opc_L3A:
        begin
          reg_A = mdata_get;
          xMx = reg_A;
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
          reg_B = mdata_get;
          xMx = reg_B;
        end
        
        opc_BG0, opc_BG1, opc_BG2, opc_BG3,
        opc_BL0, opc_BL1, opc_BL2, opc_BL3:
        begin
          mdata_put = reg_B;
          mwren = 1;
        end

        opc_G0I, opc_G1I, opc_G2I, opc_G3I, /*I*/
        opc_L0I, opc_L1I, opc_L2I, opc_L3I:
          reg_O = mdata_get;
        
        opc_IG0, opc_IG1, opc_IG2, opc_IG3,
        opc_IL0, opc_IL1, opc_IL2, opc_IL3:
        begin
          mdata_put = reg_O;
          mwren = 1;
        end

        opc_G0R, opc_G1R, opc_G2R, opc_G3R,  /*R*/
        opc_L0R, opc_L1R, opc_L2R, opc_L3R:
          reg_R = mdata_get;
        
        opc_RG0, opc_RG1, opc_RG2, opc_RG3,
        opc_RL0, opc_RL1, opc_RL2, opc_RL3:
        begin
          mdata_put = reg_R;
          mwren = 1;
        end
        
        endcase
    
endcase
endmodule


/* Use my sasm.c to assemble the following source
   and populate the RAM in Quartus with the resulting .mif file.
   This outputs 07 05 07 on the DE1's display.
   (7 is the remainder, 5 is the quotient of 77/14.)


77A, 14B *divmod8, NOP RDY
noend@ NJ <noend
CLOSE

@divmod8 ; Divide A by B, division result in A, remainder in B
NEW

aL0                       ; Dividend
bL1                       ; Divisor
1A, aL2                   ; Shift counter first 1 bit to MSB
0A, aL3                   ; Initialise quotient to zero

L1r NF >ELOOP             ; Skip if divisor zero

80h:A                     ; MSB mask
MSB_SHIFT@                ; Shift divisor left so that first 1 bit is at MSB
 L1b                      ; Load divisor
 AND NT >DIVIDE           ; Skip when MSB set
 SLB rL1                  ; Shift divisor left and update
 L2r P1 rL2               ; Increment shift counter and update
 NJ <MSB_SHIFT

DIVIDE@
 L3b SLB rL3              ; Shift quotient left and update
 L1a OCA P1 RA            ; Negate divisor
 L0b CAR                  ; Dividend check borrow bit
 NF >REP

 ADD rL0                  ; Accept subtraction, update dividend
 L3r P1 rL3               ; Increment quotient
 
REP@
 L1a SRA rL1              ; Shift divisor right for next subtraction
 L2r M1 rL2               ; Decrement counter
 NT <DIVIDE               ; Branch back if not zero

ELOOP@ L3a, L0b

OLD RET
CLOSE


*/
