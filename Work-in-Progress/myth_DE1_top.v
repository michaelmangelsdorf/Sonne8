
/* Sonne Microcontroller rev. Myth
    Verilog Implementation
    TOP level file for Quartus
    (Terasic DE1Soc board)
  Jan-2024 Michael Mangelsdorf
  Copyr. <mim@ok-schalter.de>
*/

module paver
(
  input CLOCK_50,

  /* 7 SEGMENT */
  output [6:0] HEX0,
  output [6:0] HEX1,
  output [6:0] HEX2,
  output [6:0] HEX3,
  output [6:0] HEX4, 
  output [6:0] HEX5,

  /* Keypress */
  input [3:0] KEY,

  /* LEDs */
   output [9:0] LEDR,

  /* Switches */
   input [9:0] SW,

  /* PS/2 */
  input PS2_CLK,
  inout PS2_CLK2,
  input PS2_DAT,
  inout PS2_DAT2,

  /* VGA */
  output VGA_BLANK_N,
  output [7:0] VGA_B,
  output VGA_CLK,
  output [7:0] VGA_G,
  output VGA_HS,
  output [7:0] VGA_R,
  output VGA_SYNC_N,
  output VGA_VS,

  /* GPIO */
  input [31:0] GPIO,
  output [31:0] GPIO_OUT,

  /* SPI */
  input SDMISO,
  output SDCLK,
  output SDCS,
  output SDMOSI
);

wire [7:0] mdata_put;
wire[7:0] mdata_get;
wire[15:0] maddr;
wire mwren;

memory memory_inst (
	.address ( maddr ),
	.clock ( ~CLOCK_50 ),
	.data ( mdata_put ),
	.wren ( mwren ),
	.q ( mdata_get )
	);

/* CPU */

myth_core 
cpu1
(
  .rst (rst),
  .clk (CLOCK_50),

  .maddr ( maddr ),
  .mdata_put ( mdata_put ),
  .mwren ( mwren ),
  .mdata_get ( mdata_get ),
  
  .seg7_1 (HEX0),
  .seg7_2 (HEX1),
  .seg7_3 (HEX2),
  .seg7_4 (HEX3),
  .seg7_5 (HEX4),
  .seg7_6 (HEX5),
  
  .led (LEDR),

  .keysw ({KEY,SW}),
 
  .sd_clk (SDCLK),
  .sd_mosi (SDMOSI),
  .sd_cs (SDCS),
  .sd_miso (SDMISO)
);


endmodule

