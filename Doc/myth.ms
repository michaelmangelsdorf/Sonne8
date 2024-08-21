

\# Create a PDF from this file:
\#  groff -Tpdf -tbl -ms -U  myth.ms > myth.pdf
\#  pdfinfo seems to be required for embedding PDFs

.TL
Myth Micro-Controller Manual
.AU
Michael Mangelsdorf
.BR
(mim@ok-schalter.de)

.LG
.LP
This document explains the design of a toy
8-bit micro-controller and its
programming environment.
.LP
The homepage for this project is:
.B
https://github.com/Dosflange/Myth
.R
.LP

.NH 1
Machine State
.LP
By way of introduction, we introduce the complete state of the micro-controller at any given time.

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
There are 4 memory transfer instructions and 2 implied memory read operations.
There are three page index registers, C (CODE), D (DATA),
and L (LOCAL).
There are two address offset registers, J (JUMP - Program Counter),
and O (ORIGIN).
.RE
.NH 1
Bus communication
.LP
This micro-controller has integrated circuitry for handing serial and
parallel communication. This is facilitated by dedicated hardware-registers
and instructions.
.NH 2
E (ENABLE) register
.RS
.LP
The 8-bit Enable register is used to control the enable state of devices
attached to the serial or parallel bus lines.
To this end, the register is divided into two four-bit groups for
device selection.
.LP
The
.B
low-order
.R
four-bit group (EL) drives a 4-to-16 line inverted
decoder, which maps the bit pattern in EL 1 of 16 possible,
mutually exclusive
.B
active-low
.R
select signals (SL0-15).
.LP
Select signal S0 is reserved to select a NULL device ("nothing").
SL1 corresponds to the internal POR register output enable signal (PAR_OE).
SL2-15 can be used freely for application hardware.
.LP
The
.B
high-order
.R
four-bit group (EH) drives a 4-to-16 line non-inverted decoder,
which maps the bit pattern in EH to 1 of 16 possible, mutually exclusive
.B
active-high
.R
select signals (SH0-15).
.LP
Select signal S0 is reserved to select a NULL device ("nothing").
SH1 corresponds to the internal PIR register copy select signal (PAR_CP).
SH2-15 can be used freely for application hardware.
.PDFPIC ereg.pdf
.RE
.bp
.NH 2
Communication Registers
.RS
.NH 3
SOR (Serial Output Register)
.QP
A write-only parallel-to-serial shift register for serialising an output byte,
modelled after a 74HC165 chip. 
Writing an output value for serialisation is done by writing the value
into register S. The value is clocked out/serialized by pulsing the SCLK
clock line. This happens when the SCL/SCH instructions are used repeatedly.
SCL-SCH-SCL gives a posititve clock edge. SCH-SCL-SCH generates an inverted clock.
Eight clock cycles are required to process a byte.
.NH 3
SIR (Serial Input Register)
.QP
A read-only serial-to-parallel shift register for de-serialising an incoming
bit stream into an input byte, modelled after a 74HC595 chip.
This is done by executing the SCL/SCH instructions
repeatedly.
SCL-SCH-SCL gives a positive clock edge. SCH-SCL-SCH generates an inverted clock.
Eight clock cycles are required to process a byte.
Reading a deserialised input byte is done by reading register S.
.PDFPIC sirsor.pdf
.bp
.NH 3
POR (Parallel Output Register)
.QP
A write-only tri-state register with 8-bit parallel output, modelled after
a 74HC574 chip.
Writing an output byte onto the parallel bus is a two step process.
First, the data byte must be latched into the register by writing it
into P. Then, the register output must be enabled by selecting PAR_OE
in the E register, as described above.
.NH 3
PIR (Parallel Input Register)
.QP
A read-only 8-bit parallel input register, modelled after a 74HC273 chip.
Latching the current 8-bit value of
the parallel bus into the register is done by selecting SH1 in E.
The latched data byte can then be read from P.
The bus is assumed to pull-down, so when all bus-devices are in
tri-state mode, a zero value is registered.
.PDFPIC pirpor.pdf
.RE
.bp
.NH 2
Communication Instructions
.RS
.QP
The following Signal instructions contained in the SYS group
operate on the communication registers:
.NH 3
SSI (Shift Serial In)
.QP
Shift in one bit from serial data input line into SIR (at LSB position)
.NH 3
SSO (Shift Serial Out)
.QP
Shift out one bit from SOR onto serial data output line (from MSB position)
.NH 3
SCH
.QP
Set serial clock line high
.NH 3
SCL
.QP
Set serial clock line low
.RE
.NH 2
Parallel Communication
.RS
.LP
The Myth controller interfaces to an external bidirectional
8-bit wide bus (IO bus).
.LP
It can communicate on this bus by writing a data byte
into P (POR register), and then enabling SL1 in the E register
by setting its lower nybble to 1.
Setting the bit to 1 switches the register from
tri-state output to active output, so that the byte value
is output on the bus lines.
.LP
While the output is active, other devices on the bus
can read the data byte by enabling the corresponding SHx signal
in E while keeping SL1 (the low-order nybble of E) set to 1.
.LP
Deselecting SL1 in E again (setting the low-order nybble to
a value different from 1) tristates the POR output,
so that other devices can put data bytes on the IO bus.
.LP
Enabling LH1 in E (setting the high-order nybble to 1)
latches a data byte into the PIR. This byte can then be
read from the P register.
.LP
Once a data byte has been read,
the PIR input should be deselected again in E by setting
the high-order nybble to a value different from 1.
.RE
.NH 2
Serial Communication
.RS
.LP
The Serial Peripheral Interface (SPI)
protocol can be implemented using the device enable register E,
serial registers SIR and SOR, and instructions SCL, SCH, SSI, and SSO.
.NH 2
Device Selection
.LP
Before communicating with a specific device connected to the serial bus,
the corresponding selector bit representing the device must be set in the E register.
.NH 2
Data Transmission
.LP
To transmit data to the selected device, the processor writes the dataus
to be sent into the SOR (Serial Output) register.
The SSO (Serial Shift Out) instruction is then used,
which clocks the serial output shift register and produces a data bit
on the MOSI line. Using the instruction sequence SCL SCH SCL (Serial Clock Low/High),
a positive edge clock pulse is generated.
As each bit is shifted out, it is sent to the selected device through the serial bus.
The passive device processes the transmitted bit and the cycle repeats.
.NH 2
Data Reception
.LP
To receive data from an external device, the SSI (Serial Shift In)
instruction is used. It clocks the serial input shift register, allowing the
processor to receive one bit of data at a time from the selected device via
the MISO line. The received data can then be read from the S register.
Clocking is done as for "Data Transmission".
.NH 2
CPOL (Clock Polarity)
.LP
The CPOL parameter determines the idle state of the clock signal.
The Myth controller provides signals SCL (Serial Clock Low) and SCH (Serial Clock High)
instructions which can be used to control the clock signal's state.
To configure CPOL=0 (clock idles low), execute SCL to set the clock signal low during
the idle state. To configure CPOL=1 (clock idles high), execute SCH to set the clock
signal high during the idle state.
.NH 2
CPHA (Clock Phase)
.LP
The CPHA parameter determines the edge of the clock signal where data is captured
or changed. The Myth controller provides instructions SSI (Serial shift in) and
SSO (Serial shift out) to control data transfer on each clock transition.
To configure CPHA=0 (data captured on the leading edge), execute SSI before the clock
transition to capture the incoming data. To configure CPHA=1 (data captured on the trailing edge),
execute SSI after the clock transition to capture the incoming data.
Similarly, to transmit data on the leading or trailing edge, execute SSO before or
after the clock transition, respectively.
.NH 2
Device Deselection
.LP
After data transmission is complete, the selected device needs to be deselected
to allow other devices to communicate on the bus. This is done by clearing the corresponding selector bit for the device in the E register.
.RE

.bp
.NH 1
Assembler syntax
.RS
.LP
.NH 2
Comments
.LP
Comments are introduced by a semicolon (";"). Everything on the same line after the semicolon is ignored.
.NH 2
Phrasing
.LP
Commas ("," and ".") can be used for grouping "phrases" of instructions that logically belong together. They don't generate code and are just for visual clarity.
.NH 2
Closing
.LP
The assembler directive CLOSE "closes" the current page for object code placement. Subsequent code is placed from offset zero of the following page.
.NH 2
Number Literals
.LP
Decimal numbers from 0-255 can be included the source text as literals, and be prefixed by an optional minus sign. Hexadecimal numbers must be in two uppercase digits and marked with the suffix "h", for instance: "80h" for 128. Binary numbers must be formatted as two 4-bit groups separated by an underscore ("_") and have the suffix "b", for instance: "0010_0000b" for 32.
.NH 2
Page Labels
.LP
Page labels are alphanumeric identifiers, preceded with an at-sign ("@xyz"). Page labels mark the beginning of pages (offset=0), and only one page label is allowed per page. Only page labels can be targets of CALL and TRAP instructions.
.NH 2
Offset Labels
.LP
Offset labels are identifiers *ending* with an at-sign ("xyz@"). Offset labels can be targets of xJ, xI, xT, xF instructions, or be used to reference data locations within a page.
.NH 2
Trap Labels
.LP
An asterisk placed before a page label encodes a trap call to that address.
Such label references will generate an opcode that will trigger a TRAP and call the handler function referenced by the label.
.NH 2
Label References
.LP
Bank and offset labels are referenced by prefixing their identifier with "<" (for backward references, i.e. the label is defined earlier in the source code than the reference to it), or with ">" (forward reference, i.e. the label is defined later than the reference to it in the source code). If a label is not unique, the reference goes to the nearest occurrence of it in the given direction. A label reference is just a numerical value and can be used as such, too.
.NH 2
GETPUT
.LP
GETPUT instructions are written similar to x[LO]x memory transfers,
replacing O by the required offset (0-7), and prefixing or suffixing
by either R, O, D or G.
.LP
Examples: r[L0] ("PUT r into local var 0"), [L4]o ("GET o from local var 4")
.NH 2
Shortcut Notations
.LP
Instructions of type Nx (fetch literal value and place in register) have an alternative syntax:
9A place 9 into A, generates NA 9
<LABEL:J generates NJ <LABEL
33hR and 33h:R both generate NR 33h
.RE

.bp
.NH 1
Control Flow
.LP
In this architecture, calls and traps go to page heads (offset zero). Other branches (jumps) go to offsets within a page.
.RS
.NH 2
Relative in-page Branching
.NH 3
Registers J, W, F, T
.LP
These are pseudo-registers, that can only occur as target registers of transfer instructions.
.LP
Transferring an offset value into J (Jump) does an unconditional branch to that offset within the current code page.
.LP
F means FALSE: Transferring an offset value into F branches only if R is FALSE (zero).
.LP
T means TRUE: Transferring an offset value into T branches only if R is TRUE (not zero).
.LP
I means iterate: Transferring an offset value into I decrements the loop counter register, and then branches, if that register is zero.
.PDFPIC jumps-decode.pdf
.bp
.NH 2
Page-head Calls and Traps
.LP
Transfering a page index byte into C executes a call to offset 0 of that page ("Page head").
This is equivalent to executing a TRAP instruction encoding the same page index, as described below.
.PDFPIC traps.pdf
.LP
Writing a page index into the xC receptor
executes a function call. The call goes to
the first byte of the page.
The effect is the same as if a TRAP
instruction to that page had occurred.
The current page index (register C)
is saved into the D register, overwriting
its value.
The current instruction offset (register J)
is saved into the O register,
overwriting its value.
.RE
.bp
.NH 1
Operation Codes and Decoding
.RS
.NH 2
Instruction Categories
.LP
Each instruction is encoded in an 8-bit operation code ("opcode")
and belongs to one of five categories.
The binary layout of the opcodes is such that a priority encoder
can be used to decode the instruction category of each opcode.
.LP
If bit 7 of the opcode is set, the opcode is a PAIR instruction.
.LP
ELSE
.LP
If bit 6 of the opcode is set, the opcode is a GETPUT instruction.
.LP
ELSE
.LP
If bit 5 of the opcode is set, the opcode is a TRAP instruction.
.LP
ELSE
.LP
If bit 4 of the opcode is set, the opcode is an ALU instruction.
.LP
ELSE
.LP
If bit 3 of the opcode is set, the opcode is an ADJUST instruction.
.LP
ELSE
.LP
The opcode is a SYS instruction.

.PDFPIC decoder.pdf
.bp
.NH 3
SYS Instructions
.LP
SYS opcodes encode instructions that control system functions.
Bits 0-2 of the opcode hold an index into the following
table of instructions.
.LP
NOP SSI SSO SCL SCH RET FAR ORG
.NH 3
ADJUST Instructions
.LP
ADJUST opcodes encode instructions which add a small signed
number to the R register.
.PDFPIC adjust.pdf
.NH 3
ALU Instructions (Arithmetic-Logic Unit)
.LP
ALU opcodes encode instructions which compute a function
between operand registers R and O, and then place the
result into the R register.
There are sixteen such instructions. The function index
is encoded in the low-order four bits of the opcode.
.TS
tab(@); lb l .
IDR@Identity R
IDO@Identity O
OCR@One's complement of R
OCO@One's complement of O
SLR@Shift left R
SLO@Shift left O
SRR@Shift right (logical) R
SRO@Shift right (logical) O
AND@Bitwise AND between R and O
IOR@Bitwise inclusive OR between R and O
EOR@Bitwise exclusive OR between R and O
ADD@8-bit Addition result of R plus O
CAR@Carry bit of addition result of R plus O, zero or one
RLO@Logical flag (TRUE=255, FALSE=0) R less than O
REO@Logical flag (TRUE=255, FALSE=0) R equals O
RGO@Logical flag (TRUE=255, FALSE=0) R greater than O
.TE
.NH 3
TRAP Instructions
.LP
TRAP opcodes encode instructions which
execute a function call to a given
page index.
The index is encoded in the low-order
five bits of the opcode.
This is equivalent to writing the
page index into the C (CALL)
receptacle. After the call, execution
begins at byte offset 0 of the new
page.
.NH 3
GETPUT Instructions
.LP
GETPUT opcodes encode instructions which
transfer values between registers R, O, G, and D
and the 8 top-most byte offsets of the
local page.
The assembler notation for these instructions
is \*[CODE]p[L0-7]g\*[CODE X].
.NH 3
Assignment PAIR instructions
.LP
PAIR opcodes encode instructions which
transfer a byte value from a source object to a
receptor object.
The source is encoded in the four low-order bits
0-3, and the destination is encoded in the three
high-order bits 4-6.
.LP
Sources and receptors can be physical registers,
but in some cases are used to control more
complex functionality.
.LP
This manual uses the notation, that sources
have an "x" suffixed to their letter
(example: Gx "G into"), and receptors are
prefixed with an "x" (example: xG "into G").
Some objects can occur as both source and
receptor.
.SH 4
Registers and Receptors
.LP
Nx xO xGx xRx xIx xMLx xMDx xSx xPx
xE xB xD  xJ  xW  xT  xF  xC


.RE







