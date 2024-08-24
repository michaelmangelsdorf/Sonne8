.NH 1
Machine State
.LP
The following table shows the complete machine state at any given time.

.RS
.TS
allbox tab(@); r l.
State bits @ sclk miso mosi
Byte registers @ e SIR SOR PIR POR j o r c d l g i
Byte array @ Memory[256][256]
.TE
.LP
\~
.NH 2
State bits
.LP
SCLK is the state of the serial clock line, which the Myth CPU controls ("Master").
Devices listening on the serial bus read this signal to process clock transitions and coordinate data transfer.
MISO is the data bit on the incoming data line from a serial device.
MOSI is the data bit on the outgoing data line to a serial device.
.NH 2
Byte registers
.LP
.TS
tab(@); lb l l .
E@Enable @ Enable/Disable device select lines
SIR@Serial Input @ Deserializes SPI bus input
SOR@Serial Output @ Serializes SPI bus output
PIR@Parallel Input @ Samples byte on IO bus
POR@Parallel Output @ Outputs byte to IO bus
J@Jump (Program Counter low-order) @ Set for branching
O@Offset @ Multi purpose register, ALU operand
R@Result @ Multi purpose register, ALU operand and result
C@Code Page (Program Counter high-order)@ Set for subroutine calls
D@Data Page @ High order address byte ("Page") used for data access 
L@Local Page @ High order address byte ("Page") used for local access
G@Global @ Multi purpose register
I@Iterator @ Hardware loop-counter
.TE
.NH 2
Memory byte array
.LP
The controller can directly address up to 256 pages of 256 bytes each (64K bytes).
.LP
A 16-bit effective memory address consists of an 8-bit page index (high-order byte),
and an 8-bit offset (low-order byte).
.LP
There are three page index registers, C (CODE), D (DATA),
and L (LOCAL).
There are two address offset registers, J (JUMP - Program Counter),
and O (ORIGIN).
.RE
