
The design has changed considerably, without changing hardware complexity too much. The attached KiCad schematics haven't been tested, but should be faithful to the new design, with few changes required for producing a working board.
The Verilog has been tested on a Terasic DE1Soc board.

Sonne Microcontroller rev. Myth

This is a CPU design for an 8-bit microcontroller. Addresses are formed by concatenating an 8-bit "page" address prefix and an address "offset" byte. The resulting memory layout consists of 256 pages of 256 bytes each, thus 64 kilobytes of memory.

During fetch operations, the C (Code) register provides the page index. When in data context, the D register provides the page index.

Page 255 is called the Global page. Special instructions called GET-PUT involve another page index stored in the L (Local) register. These instructions allow register values A, B, R and W to be copied to and from a four-byte GET-PUT region in both the global and local page. Incrementing and decrementing the L register can be used to implement subroutine stack frames (each of one page).


Accumulator Registers

	There are three accumulator registers, A, B and R. A and B or the ALU input operands. R receives the result of all ALU instructions, and has an attached bias register to it. A and B each have a cache register attached to them, which holds their previous value when a new value is assigned. The previous value can be copied back into the respective register.


Address Registers

	Accumulator registers A and B, and the working register W are address registers. Whenever one of these three is written to, it becomes the active one. The active address register provides the implied address offset for every memory load/store operation.


Register M

	The value in the active address register is the implied offset for all memory transfers. Reading from the M register (instructions of type Mx) combines the implied page index and the implied offset into an effective address.

	The memory value of that address is then copied into the target register of the instruction. For instance the instruction "MA" transfers the memory content stored at the implied address into the A register. Writing to the M register (instructions of type xM) does as before, put replaces the memory content at the effective address with the value of the source register of the instruction. For instance "RM" stores the ALU result into the implied effective memory address.


Instruction Word Format

(Starting from the most significant bit of the instruction opcode)

	Pair Instructions

	If bit 7 of the opcode is 1, the opcode represents a Pair instruction. This name indicates that the remaining low-order bits are composed of two logical bit groups, called Source (bits 4-6) and Target (bits 0-3). These instructions all copy register values from the source register to the target register.
	("Scrounge" instructions re-purpose redundant or impractical combinations such as MM or NM. The instruction NM is scrounged (does not exist) and its opcode gives RET, MM is scrounged and gives REA, RR is scrounged and gives REB, WW gives RER.)

	GET-PUT Instructions

	If bits 7-6 of the opcode are 01, the opcode represents a GET-PUT instruction. Opcodes for Get-Put instructions encode the Register in bits 0-1, mapped as follows: 00=A, 01=B, 10=R, 11=W. Bit 3 selects one of two page bytes. If bit 3 is 0, the global page (255) is used. If bit 3 is 1, the page byte in the L register is used. Bit 2 encodes whether the register value is retrieved from the Get-Put location (GET, bit=0), or written into it (PUT, bit=1).
	Bits 5-4 encode a numeric offset which is added to the page address to obtain the effective Get-Put location.

	Trap Instructions

	If bits 7-5 of the opcode are 001, the opcode represents a Trap instruction.
	The remaining bits 0-4 directly encode the destination address for the corresponding Trap call. When a Trap instruction is encountered during program execution, it triggers a transparent subroutine call to a user-defined Trap handler routine. Since they are single-byte, these calls are indistinguishable from built-in instruction opcodes.

	ALU Instructions

	If bits 7-4 of the opcode are 0001, the opcode represents an ALU instruction.
	There are 16 Function instructions, corresponding to the possible values of bits 0-3, mapped as follows: 0000=IDA, 0001=IDB, 0010=OCA, 0011=OCB, 0100=SLA, 0101=SLB, 0110=SRA, 0111=SRB, 1000=AND, 1001=IOR, 1010=EOR, 1011=ADD, 1100=CYF, 1101=ALB, 1110=AEB, 1111=AGB.

	Bias Instructions

	If bits 7-3 of the opcode are 0000_1, the opcode represents a bias instruction. The instructions set the bias value associated with R. This is a 3-bit sign-extended two's complement byte. Every bias instruction has a mnemonic of the form: "r" + decimal value + number sign. For example: "r3+" meaning "bias the R register by 3", and "r1-" meaning "bias the R register by 1". The R register itself is not changed, but reading it will give the biased value.

	"SYSTEM" Instructions (SYS)

	If bits 7-2 of the  opcode are 0000_0, the opcode represents a SYS instruction. The SYS instructions are NOP, SSI, SSO, SCL, SCH, RDY, NEW, OLD.


Control Flow

In this architecture, calls and traps go to page heads (offset zero). Other branches (jumps) go to offsets within a page.

Branching

	Registers J, F, T, I

	These are pseudo-registers, that can only occur as target registers of transfer instructions. Transferring an offset value into J (Jump) does an unconditional branch to that offset within the current code page.
	F means FALSE: Transferring an offset value into F branches only if R is FALSE (zero).
	T means TRUE: Transferring an offset value into T branches only if R is TRUE (not zero).
	I means iterate: Transferring an offset value into I decrements the loop counter register, and then branches, if that register is zero.

Calling and TRAP mechanism

	Trap instructions jump to offset 0 of the page encoded in the opcode. They also save the return page index in D. The current address offset is saved into the W register, making it the active address register. The offset is then set to 0 and control is transferred to the new page. Storing a page index into pseudo register C has the same effect.

	The RET instruction ("Return") overwrites the current code page index by the value in D, and overwrites the current code offset by the value in W, conceptually reversing a preceding CALL or Trap instruction.

Arithmetic-Logic Unit (ALU)

The instructions compute mathematical functions using one or both of A and B as input arguments. The results are stored into R (Result). R can also be written using register transfers. R has an associated bias register. When reading R, the signed bias is transparently added to the result. The ALU instructions do the following, results are stored in R.

IDA Identity of A
IDB Identity of B
OCA One's complement of A
OCB One's complement of B
SLA Shift left A
SLB Shift left B
SRA Shift right logical A
SRB Shift right logical B
AND Bitwise AND between A and B
IOR Bitwise inclusive OR between A and B
EOR Bitwise exclusive OR between A and B
ADD 8-bit Addition result of A plus B
CAR Carry bit of addition result of A plus B, zero or one
ALB Logical flag (TRUE=255, FALSE=0) A less than B
AEB Logical flag (TRUE=255, FALSE=0) A equals B
AGB Logical flag (TRUE=255, FALSE=0) A greater than B



Review of Source and Target Registers

N (Number Literal as Nx, sets iteration register as xN)

Register N is a pseudo-register. When used as a source register, it copies then skips the following opcode into the target register. NM is scrounged, RET is executed instead. When used as a target register, the source register value is copied into the iteration register.

M (Memory)

Register M is a pseudo-register. It can be used as Source or Target of a Transfer instruction.
It is used to transfer values from/to an implied memory cell from/into a register.

D (Data page index)

It can be used as Source or Target of a Transfer instruction. During Trap and Call instructions, the current code page index is saved into D, and during RET instructions, the current code page index is restored from D.

L (Local page index)

It can be used as Source or Target of a Transfer instruction.
Register L contains the page index for GET-PUT instructions. The NEW instruction decrements L by 1. The OLD instruction increments L by 1.

S (Serial)

Register S is a pseudo-register. It can be used as Source or Target of a Transfer instruction.
Reading a deserialised input byte is done by reading register S as source.
Writing an output value for serialisation is done by writing to register S as target.

P (Parallel)

Register P is a pseudo-register. It can be used as Source or Target of a Transfer instruction.
Reading the current 8-bit value on the parallel bus is done by reading register P as source.
Writing an output byte onto the parallel bus is done by writing to register P as target.

R (Result)

It can be used as Source or Target of a Transfer instruction. R received the result of ALU instructions.
R has an associated bias register, which is transparently added or subtracted when reading R.

E (Enable)

It can only by the Source of a Transfer instruction.
This register is used for device selection (SPI "Slave Select", latch/output enable pins etc).

J (JUMP), T (TRUE), F (FALSE), I (Iterate)
These can only be Targets of Transfer instructions.
The semantics of these instructions are discussed above under "Control Flow".

C (CALL)
Like trap, but with a source register.

W (Working register)
A general purpose register which is also an address register. During call or trap instructions, the address offset of the next instruction to execute upon return is saved into W. During RET, the return offset is restored from W.

A, B (Accumulator Registers)
These can only be target registers. The ALU functions IDA (=A) and IDB (=B) as well as corresponding GET-PUT instructions can be used to read A and B.
The values in these registers are the implied arguments for ALU function instructions. Writing a new value into A, B or R copies the previous value into the cache register associated with the respective register. The instructions REA, REB and RER copy the previous value back into the corresponding register. 




Communication

The controller has a communication system for interfacing to serial/parallel devices.

Communication Registers

E

An 8-bit write-only device select register E (Enable). The bit pattern stored in this register is intended to be used for enabling/disabling devices attached to the serial/parallel buses. Writing a device select pattern is done by transferring values into E.

In the reference implementation, the device select byte logically consists of two four-bit halves (nybbles). Each nybble selects one of 16 enable signals via an attached 4-bit decoder. For instance, the lower nybble could generate an enable signal for an input (listening) device on the bus, while the higher nybble could generate a similar signal that enables the transmitting output device required for a particular data bus transfer.

By convention, the value 1 selects a null device (not connected), or some initialisation logic, since both nybbles of the D register will typically be zero (4-to-16 decoding to 1) upon power-up or reset.

SOR (Serial Output Register)

A write-only parallel-to-serial shift register for serialising an output byte.
Writing an output value for serialisation is done by writing to register S.

SIR (Serial Input Register)

A read-only serial-to-parallel shift register for deserialising an input byte.
Reading a deserialised input byte is done by reading register S.

POR (Parallel Output Register)

A write-only tri-state register with 8-bit parallel output.
Writing an output byte onto the parallel bus is done by writing to register P. If the parallel bus is in tri-state mode (HIZ instruction), a write-operation will end tri-state mode.

PIR (Parallel Input Register)

A read-only 8-bit parallel input register.
Reading the current 8-bit value on the parallel bus is done by reading register P.

Communication Instructions

The following Signal instructions operate on the communication registers:

SSI (Shift Serial In): Shift in one bit from serial data input line into SIR (at LSB position)

SSO (Shift Serial Out): Shift out one bit from SOR onto serial data output line (from MSB position)

SCL: Set serial clock line high

SCH: Set serial clock line low

RDY: Put POR into tristate mode (writing to SOR deactivates tristate mode, reading from SIR is equivalent to RDY).


Serial IO and implementing SPI

The Serial Peripheral Interface (SPI) protocol can be implemented using the device select register E, serial register S, and instructions SCL, SCH, SSI, and SSO.

Device Selection: Before communicating with a specific device connected to the serial bus, the corresponding bit pattern representing the device must be set in the E register.

Data Transmission: To transmit data to the selected device, the processor loads the data to be sent into a SOR register.
The SSO (Serial Shift Out) instruction is then executed repeatedly, which clocks the serial output shift register. As each bit is shifted out, it is sent to the selected device through the serial bus.

Device Deselection: After data transmission is complete, the selected device needs to be deselected to allow other devices to communicate on the bus. This is done by clearing the bit pattern corresponding to the device in the D register.

Data Reception: To receive data from an external device, the SSI (Serial Shift In) instruction is used. It clocks the serial input shift register, allowing the processor to receive one bit of data at a time from the selected device. The received data can then be read from the S register.

CPOL (Clock Polarity): The CPOL parameter determines the idle state of the clock signal.
In Sonne, the SCL (Serial Clock Low) and SCH (Serial Clock High) instructions can be used to control the clock signal's state. To configure CPOL=0 (clock idles low), execute SCL to set the clock signal low during the idle state. To configure CPOL=1 (clock idles high), execute SCH to set the clock signal high during the idle state.

CPHA (Clock Phase): The CPHA parameter determines the edge of the clock signal where data is captured or changed. In Sonne, the SSI (Serial shift in) and SSO (Serial shift out) instructions can be used to control data transfer on each clock transition. To configure CPHA=0 (data captured on the leading edge), execute SSI before the clock transition to capture the incoming data. To configure CPHA=1 (data captured on the trailing edge), execute SSI after the clock transition to capture the incoming data. Similarly, to transmit data on the leading or trailing edge, execute SSO before or after the clock transition, respectively.




Assembler syntax

Comments are introduced by a semicolon (";"). Everything on the same line after the semicolon is ignored.

Commas ("," and ".") can be used for grouping "phrases" of instructions that logically belong together. They don't generate code and are just for visual clarity.

The assembler directive CLOSE "closes" the current page for object code placement. Subsequent code is placed from offset zero of the following page.

Number Literals

Decimal numbers from 0-255 can be included the source text as literals, and be prefixed by an optional minus sign. Hexadecimal numbers must be in two uppercase digits and marked with the suffix "h", for instance: "80h" for 128. Binary numbers must be formatted as two 4-bit groups separated by an underscore ("_") and have the suffix "b", for instance: "0010_0000b" for 32.

Page Labels

Page labels are alphanumeric identifiers, preceded with an at-sign ("@xyz"). Page labels mark the beginning of pages (offset=0), and only one page label is allowed per page. Only page labels can be targets of CALL and TRAP instructions.

Offset Labels

Offset labels are identifiers *ending* with an at-sign ("xyz@"). Offset labels can be targets of xJ, xI, xT, xF instructions, or be used to reference data locations within a page.

Asterisk

An asterisk placed before a page label encodes a trap call to that address.
Such label references will generate an opcode that will trigger a TRAP and call the handler function referenced by the label.

Label References

Bank and offset labels are referenced by prefixing their identifier with "<" (for backward references, i.e. the label is defined earlier in the source code than the reference to it), or with ">" (forward reference, i.e. the label is defined later than the reference to it in the source code). If a label is not unique, the reference goes to the nearest occurrence of it.
A label reference is just a numerical value and can be used as such, too.

Instruction Mnemonics

Mnemonics for transfer pair instructions are two-letter uppercase words such as "RA" (take R into A). Mnemonics of the form ("Nx") for transferring a literal embedded in the code stream into a target register, must be followed by that number, reference or other value to be transferred. Example: "na 80h" transfers the number 128 into register A.

GET-PUT Mnemonics

Get operation: Precede the lower case target register name by the upper case Get-Put location, like so: G3a (get A register from location G3).
Put operation: Precede the upper case Get-Put location with the lower case source register name, like so: rG0 (put R register into location G3).

R-Bias Mnemonics

R2+ is the mnemonic for the opcode that sets the bias register of R to 2. R1- is the mnemonic for the opcode that sets the bias register to -1.

Instructions of type Nx (fetch literal value and place in register) have an alternative syntax:
9A place 9 into A, generates NA 9
<LABEL:J generates NJ <LABEL
33hR and 33h:R both generate NR 33h
