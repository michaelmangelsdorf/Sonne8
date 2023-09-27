
Welcome to this project!

This repo contains all files you need to build and program an 8-bit CPU that I have designed und built.

It's not even micro-coded, so it's very simple, although I wanted it to be sane and fun to code for instead of minimal. It's easy to create custom instructions, it's got single-cycle trap calls that don't appear different from inbuilt opcodes.

You would need to have the two PCBs fabricated: Firstly, there's the CPU board itself.
It is actually a micro-controller, in that this board implements RAM and an IO interface.
Then secondly, there is an IO expansion board, that fits on top of the CPU board via Arduino type stackable headers.

What can it do? Well, the demo program computes 7 x 13 and shows the result (5B in hexadecimal) on a 7-segment display (see the mul16 movie in the repo)... Behind the scenes, it reads the code for doing that by bit-banging the SPI interface of a serial EEPROM on the IO board. So it's a micro-controller alright.

The two KiCad boards work fine!, I've had them manufactured by JLCPCB. The project files in the repo correspond to what I sent them. The screenshots (jpeg) of the boards show details. I've soldered sockets for all chips to be safe. The quartz oscillator I used is 8MHz, which corresponds to roughly 1 Million instructions per second.

The CPU board looks like this (this is an older version, notice the wire patch):

![CPU board](https://github.com/Dosflange/Sonne-THT-74HC/blob/main/board_and_screws.jpg)

These plastic spacers go between the CPU board and the IO board. The distance/length of the spacer part (without the protruding thread) is 2 cm.

![CPU board](https://github.com/Dosflange/Sonne-THT-74HC/blob/main/board_screws.jpg)

Both modules go together like so:

![CPU board](https://github.com/Dosflange/Sonne-THT-74HC/blob/main/board_sandwich.jpg)

The assembled development kit i.e. hardware portion of this project:

![CPU board](https://github.com/Dosflange/Sonne-THT-74HC/blob/main/board_ready.jpg)

The entire lower right quarter of the IO board doesn't really need to be populated, but you can of course. The large chip socket on the right
is just a place-holder that connects straight through to the identical EEPROM socket just below it on the CPU board.
That EEPROM is actually the ALU of the CPU. It does via simple look-up tables, what the discrete version on the IO board does with logic chips.

## Address space

The address space of this CPU is unusual. Eight bits give you 256 address places. The first 128 places form a segment, which can be switched to point to another memory segment ("bank switching"). The next 64 places after that are fixed (for global variables). The remaining 64 places form a segment again, which can be switched out to point to another memory segment (for local variables).

### Global segment

This 64 byte segment serves as a global (i.e. persistent during subroutine calls) data repository.
The first eight addresses (128-135) of it are referred to as G0-G7 (G for global) and there are special single-cycle instructions for transfering these values to and from the two accumulator registers. These locations are essentially additional registers.

### Local segment

This 64 byte segment serves as a stack frame, which can be swapped out by the following two instructions.
The first eight addresses (192-199) of it are referred to as L0-L7 (L for local) and there are special single-cycle instructions for transfering these values to and from the two accumulator registers. These locations are essentially additional registers local to the current subroutine!

#### Leave and Enter

The LEAVE signal increments an internal address prefix. This causes the final 64-bytes ("local" segment) of the address space to point to the previous stack frame. The ENTER signal decrements the internal address prefix, causing a new stack frame to appear in the "local" segment.

## Binary Instruction Format

There are four types of instructions. The format is completely regular and you would have no problem hand-coding these opcodes!

SIGNAL
If bit 7 is clear, and bits 0-3 are all zero, the instruction is of type SIGNAL. There are sixteen such instructions, corresponding to each combination of bits 4-6. These instructions trigger internal signals.

RCOPY
If bit 7 is clear, and bits 0-3 are *not* all zero, the instruction is of type RCOPY. In this case, bits 0-3 encode one of eight possible source registers, and bits 4-6 encode one of sixteen possible target registers. Each instruction of type RCOPY copies the value of its source register into its target register.

TRAP
If bit 7 is set, but bit 6 is clear, the instruction is of type TRAP, and bits 0-5 encode the memory bank address of a trap handler function. Each TRAP instruction performs a function call to the first byte of its trap handler bank.

GETPUT
If bits 6-7 are both set, the instruction is of type GETPUT. These instructions copy values between the first eight bytes (G0-G7) of the global segment or the first eight bytes (L0-L7) of the local segment and registers A or B.
Bits 0-5 encode the operation of each such instruction in the following way.
Bit 5 encodes the source or destination register: 0=A, 1=B
Bit 4 encodes the segment: 0=Global, 1=Local
Bit 3 encodes the operation: 0=Read (Get), 1=Write (Put)
Bits 0-2 encode the byte offset within the segment
Example:
The mnemonic "aG3g" ("a G3 get") would mean:
"a": Register A is the target of the operation. "G": Global memory is being used. "3": The third location in global memory is being accessed. "g": Get operation, the value from the specified memory location is being read and loaded into register A. So, "aG3g" instruction gets the value from the third location of global memory and loads it into register A.

### Signal instructions

These are used to trigger specific operations that don't require explicit arguments, such as IO operations or creating/destroying stack frames.

### Register-to-register transfers ("RCOPY")

These instructions copy the content of one register into another. GA for instance copies the value of G into A. Some registers are pseudo registers, such as B (Branch) -- copying a source register into B branches to the location stored in the source register. See the descriptions of individual registers below. 

#### Scrounge instructions

Instructions such as MM or SS that would essentially do nothing ("NOP"), and instructions such as LM which would conflict with the timing requirements and the complexity of the hardware required, are detected (preselected) by a logic module called the scrounger, and they are treated as entirely different instructions -- their opcode is "repurposed". For instance, the opcode with regular mnemonic LM is scrounged and repurposed as opcode for the RET instruction.

Not all such "NOP" opcodes are "scrounged" and could potentially be used as CPU instruction set extensions in future versions.


### Trap calls ("TRAP")

When a TRAP instruction is encountered during program execution, it triggers a transparent subroutine call to a user-defined TRAP handler routine. This handler routine acts as the implementation of the custom instruction, enabling the execution of specialized operations that go beyond the capabilities of the standard built-in instructions.

User-defined instructions implemented through trap handlers are indistinguishable from pre-existing built-in instructions. They allow developers to create new instructions that appear and behave just like native instructions to the processor.

Flexible Byte Lengths:
Custom instructions defined through trap calls can have variable byte lengths. The PULL instruction facilitates this. It allows the custom instruction's handler routine to access subsequent bytes in the instruction stream, enabling the retrieval of data or parameters specific to the functionality of the custom instruction. This flexibility opens up possibilities for instructions with different byte lengths tailored to specific requirements, such as immediate addressing.

In this way, the TRAP mechanism in Sonne architecture allows you to effectively "extend" the instruction set with your own custom operations. By strategically writing and calling these trap handlers, you can enhance the functionality of the processor in ways that go beyond the capabilities of the built-in instructions. 

### Memory-Register-Memory transfers ("GETPUT")

These instructions copy values between the 8 bytes G0-G7 of the global segment or the 8 bytes L0-L7 of the local segment and registers A or Q.

#### Examples

The mnemonic "aG3g" ("a G3 get") would mean:

"a": Register A is the target of the operation.
"G": Global memory is being used.
"3": The third location in global memory is being accessed.
"g": Get operation, the value from the specified memory location is being read and loaded into register A.
So, "aG3g" instruction gets the value from the third location of global memory and loads it into register A.

"aL7p": This instruction would put the value from register A into the seventh location of local memory.
q: Indicates that the data (Q) register is the source or target of the operation.
"qG2g": This instruction would get the value from the second location of global memory and load it into register Q.
"qL5p": This instruction would put the value from register Q into the fifth location of local memory.
Remember that in these mnemonics, the first character ("a" or "q") denotes the register being operated upon, the second character ("G" or "L") specifies the scope of the memory being accessed (Global or Local), the third character (a digit from 0-7) specifies the location in the specified memory, and the last character ("g" or "p") specifies the operation (Get or Put).


## Fetch mechanism

The X register holds the byte offset for instruction fetches. It represents the byte offset within the code branch (frame) from which the next instruction will be fetched.
The X register is incremented by 1 after each instruction, so that the next instruction is pointed to.
Jump instructions reset the X register to 0 (!), jumps are made to first bytes (offset 0) of a target bank.

The Y register holds the pending code branch (high order portion or "bank"). The value stored in the Y register represents the code branch that will be resumed upon executing a RET instruction.

The LID instruction (LID stands for the English word "lid") resets the byte offset for instruction fetches to 0, and increments the current frame by 1.
LID is called "LID" because it effectively closes ("puts a lid on") the currently executed frame.

In the assembler code I provided, a "." assembles to a LID opcode. So a dot in an assembler listing closes the current frame and begins at the next frame, byte offset 0. It is just a shorthand for explicitly writing LID.

## Groups of registers

### L

L stands for literal. It's a pseudo-register that can only be a source in register-register transfers. The value transfered is the next byte in the instruction stream, which is then skipped.

### A, B and M

M is a pseudo-register. Reading or writing it transfers between memory and a register operand.
The last recently used ALU register - A or B - is the implied offset address register that combines with a bank register to give the effective address.

### R

The R register holds the bank prefix of the 128 byte "row" region from 0-127 of the address space. Changing R switches all 128 bytes simultaneously by address mapping.

### J, E, T

These are pseudo-registers, that can only occur as register-targets in transfers. Transfering a value to B does an unconditional jump to that "branch". You can't jump to a byte offset, only to the first byte offset (0) of a branch. E means Else - the branch is taken when the ALU result is zero. T means Then - the branch is taken when the ALU result is not zero.

### C, X, Y

The C register is a pseudo register, that can only occur as a register-target in transfers. Transfering a value to C executes a subroutine call. The "branch" get's stored in Y, the byte offset for instruction fetches within that "branch" gets stored in X. By writing values into X and Y, the return address can be redirected. The RET signal returns from the subroutine, by restoring the "branch" and byte offset from X and Y. The PULL signal transfers the current X into A, and the current Y into R.

### A, B, F

A and B are two accumulator registers. These are the two input operands connected to the ALU (Arithmetic-Logic Unit). F stands for Function. Writing an ALU opcode to F computes the respective function. Reading from F yields the result.

A and B are not readable directly. The intended way is to execute one of the identity functions of the ALU, IDA or IDB.

To prepare ALU input values, program the ALU, and retrieve the result, the following steps are typically involved:

Load Operand Values: The input values for the ALU operation need to be loaded into the A and B registers. This can be done by transferring values from memory or other registers to the A and B registers using appropriate transfer instructions.
Set ALU Opcode: Determine the specific ALU operation you want to perform and set the corresponding ALU opcode. The ALU opcode is written to the F register, which programs the ALU to perform the desired operation.
Execute ALU Operation: Trigger the execution of the ALU operation by performing an ALU instruction that uses the A and B registers as source operands and the F register as the target. The ALU will perform the specified operation on the input operands and store the result in the F register.
Retrieve Result: After the ALU operation is executed, the result will be stored in the F register. Retrieve the result from the F register for further processing or storing in memory if needed.
These steps ensure that the input values are properly prepared, the ALU is programmed with the desired operation, the ALU operation is executed, and the result is retrieved for subsequent use or storage.

In Sonne, the F register functions differently depending on whether it is used as an input register or when it is read from. This distinction is important and can be easily overlooked.

When the F register is used as an input register, it serves as a parameter to program the ALU's operation. The value written to the F register determines the specific ALU opcode, which in turn determines the arithmetic or logical operation to be performed. It essentially configures the ALU's behavior for the subsequent operation.

On the other hand, when the F register is read from, it holds the most recent result produced by the ALU. This result reflects the outcome of the operation executed using the previously programmed ALU opcode. In this context, the F register acts as a storage location for the ALU result, allowing it to be accessed by subsequent instructions or transferred to other registers.

This distinction highlights the dual role of the F register in Sonne: as a configuration parameter for the ALU when written to, and as a storage location for the ALU result when read from. It is important to understand this behavior to properly utilize the ALU and interpret the effects of instructions involving the F register.

### ALU instructions

ALU instruction words (to be written to the F register) are bytes. The low order 4 bits encode the following 16 instructions:

```
|--- | --- | --- |--- |
| 0 |   IDA | Identity A | F := A |
| 1 |   IDB | Identity B | F := B |
| 2 |   OCA | Ones Complement A | F := ~A |
| 3 |  OCB | Ones Complement B | F := ~B |
| 4 |   SLA | Shift left A | F := A<<1 |
| 5 |   SLB | Shift left B | F := B<<1 |
| 6 |   SRA | Shift right A | F := A>>1 |
| 7 |   SRB | Shift right B | F := B>>1 |
| 8 |   AND | Logical A AND B | F := A&B |
| 9 |   IOR | Logical A OR B | F := A\|B |
| 10 |  EOR | Logical A XOR B | F := A\^B |
| 11 |  ADD | Bits 0-7 of A+B | F := (A+B)&255 (ignore carry) |
| 12 |  CYB | Carry bit of A+B | F := A+B; F := (F>255)?1:0 |
| 13 |  ALB | A less than B | F := (A<B)? 0xFF:0 |
| 14 |  AEB |A equals B | F := (A==B)? 0xFF:0 |
| 15 |  AGB |A greater than B | F:= (A>B)? 0xFF:0 |
```

The high order 4 bits hold a signed 3 bit offset which is added to the ALU result. In the assembly language, these mnemonics can be followed by an optional number term, such as IDA+2, SLB+1 etc. The default ALU operation on reset is "IDA+0".
Read ALU results from the F register.


## IO

### D, S, P

The D register selects IO bus devices in an implementation specific way (for example SPI device select etc). Writing to P (Parallel) transfers a byte onto the tri-state IO bus. Reading from P transfers a byte from the IO-bus. Use the OFF signal to tri-state the bus. Writing to P also removes the tri-state.

Writing to the S (Serial) register puts that byte into a shift-register for serialization. The CSO signal ("clock serial out") shifts out one bit on the MOSI line. Reading from S yields the currently deserialized byte in the shift-register. The CSI signal ("clock serial in") shifts in one bit from the MISO line. The SCH and SCL signals respectively toggle the serial master clock high or low.

The serial interface facilitates serial communication between the CPU and external devices.
The S (Serial) register acts as a shift register, allowing data to be transmitted or received bit by bit.
The CSI (Clock Serial In) instruction clocks the serial input shift register, reading in data.
The CSO (Clock Serial Out) instruction clocks the serial output shift register, sending out data.
Device Selection:
The D register is involved in device selection for serial or parallel communication.
By setting specific bit patterns in the D register, the CPU can select specific devices connected to the bus for communication.

### Parallel IO

Parallel Interface:
The parallel interface enables communication between the CPU and external devices using a parallel bus.
The P (Parallel) register is used for data transfer. Reading from the P register retrieves a byte value from the parallel bus, and writing to it sends a byte value onto the bus.
The OFF instruction can be used to tristate the parallel bus, disabling its output when necessary.

### Serial IO

The Serial Peripheral Interface (SPI) protocol can be implemented using the Sonne processor's instructions, including SCL, SCH, CSI, and CSO, along with proper configuration of control signals. Here's a description of how the SPI protocol can be implemented with the Sonne instructions:

In addition to the serial input and output shift registers, the Sonne processor utilizes the U and V registers for device selection and deselection in serial communication.

Here's an expanded summary of the serial communication capabilities of Sonne, including the roles of U and V:

Device Selection: Before communicating with a specific device connected to the serial bus, the corresponding bit pattern representing the device must be set in either the D register. This selection process ensures that the desired device is enabled for communication.
Data Transmission: To transmit data to the selected device, the processor loads the data to be sent into a register or memory location. The CSO (Clock Serial Out) instruction is then executed, which clocks the serial output shift register. As each bit is shifted out, it is sent to the selected device through the serial bus.
Device Deselection: After data transmission is complete, the selected device needs to be deselected to allow other devices to communicate on the bus. This is done by clearing the bit pattern in the D register corresponding to the device. Deselecting the device prevents further communication with it.
Data Reception: To receive data from an external device, the CSI (Clock Serial In) instruction is used. It clocks the serial input shift register, allowing the processor to receive one bit of data at a time from the selected device. The received data can then be processed or stored in registers or memory.
By utilizing the D registers for device selection and deselection, the Sonne processor can communicate with different devices connected to the serial bus in a controlled and synchronized manner. It allows for flexible communication with multiple devices using a serial interface such as SPI, ensuring proper device selection and enabling reliable data transmission and reception.

CPOL (Clock Polarity):
The CPOL parameter determines the idle state of the clock signal.
In Sonne, the SCL (Set Clock Low) and SCH (Set Clock High) instructions can be used to control the clock signal's state.
To configure CPOL=0 (clock idles low), execute SCL to set the clock signal low during the idle state.
To configure CPOL=1 (clock idles high), execute SCH to set the clock signal high during the idle state.
CPHA (Clock Phase):
The CPHA parameter determines the edge of the clock signal where data is captured or changed.
In Sonne, the CSI (Clock Serial In) and CSO (Clock Serial Out) instructions can be used to control data transfer on each clock transition.
To configure CPHA=0 (data captured on the leading edge), execute CSI before the clock transition to capture the incoming data.
To configure CPHA=1 (data captured on the trailing edge), execute CSI after the clock transition to capture the incoming data.
Similarly, to transmit data on the leading or trailing edge, execute CSO before or after the clock transition, respectively.
Slave Select (SS) Signal:
In SPI, a Slave Select signal is used to enable/disable specific devices on the bus during communication.
In Sonne, the U and V registers can be utilized to control the Slave Select (SS) signal.
Before initiating communication with a specific device, set the corresponding bit pattern in the D register to select the device.
After communication, clear the bit pattern in the D register to deselect the device.
By combining the SCL, SCH, CSI, and CSO instructions along with appropriate configuration of the control signals, the Sonne processor can effectively implement the SPI protocol. This allows for synchronized and controlled data transfer with SPI-compatible devices, including the flexibility to configure CPOL, CPHA, and SS signals to meet the specific requirements of the SPI interface.

### Parallel Interface
The P (Parallel) Register: Writing to the P register sends a byte of data onto the parallel bus, and reading from the P register receives a byte of data from the parallel bus.
The OFF instruction: This is used to tristate the parallel bus, disabling its output when necessary.

### Serial Interface
The S (Serial) Register: Writing to this register puts a byte into a shift-register for serialization. Reading from this register yields the currently deserialized byte in the shift register.
The CSO Instruction (Clock Serial Out): This instruction shifts out one bit on the MOSI (Master Out Slave In) line from the byte currently held in the shift register.
The CSI Instruction (Clock Serial In): This instruction shifts in one bit from the MISO (Master In Slave Out) line to the shift register.
The SCH and SCL Instructions: These are used to toggle the serial master clock high or low respectively.

### Device Selection
The D register: Each bit in this register selects a specific I/O device, like an SPI device select. This allows for specific device communication during parallel or serial transmission.
 
### Implementing SPI protocol
Through the appropriate use of the S Register and the CSO, CSI, SCH and SCL instructions, the Sonne CPU can implement Serial Peripheral Interface (SPI) communication with external devices.



# Alternative Write-up



## Sonne Programmer's Overview
## Register Model and Instruction Set


## Preliminaries

Bits are numbered starting from 0 (least significant bit) to 7 (most significant bit). Clearing a bit means setting it to zero. Setting a bit (without giving the explicit value 0) means setting it to 1. A byte is an 8-bit number. A nybble is a 4-bit number. A signed binary number means a number in twos-complement notation, its most significant bit representing the numerical sign.

### Assembler Notation

Comments are introduced by a semicolon (";"). Everything on the same line after the semicolon is ignored.

Commas (",") can be used for grouping "phrases" of instructions that logically belong together. They don't generate code and are just for visual clarity.

A dot (".") is shorthand notation for and produces the same output as the "LID" instruction. The assembler will insert the LID-count at offset 127 of the current bank, and place subsequent code into the next bank at offset 0. The dot can be added after other elements without separating space.

#### Number Literals

Decimal numbers from 0-255 can be included the source text as literals, and be prefixed by an optional minus sign. Hexadecimal numbers must be in two uppercase digits and marked with the suffix "h", for instance: "80h" for 128. Binary numbers must be formatted as two 4-bit groups separated by a dot (".") and have the suffix "b", for instance: "0010.0000b" for 32.

#### Code Labels

Code labels are alphanumeric identifiers, preceded with an at-sign ("@") instead of the conventional colon (":") at the end. Code labels mark the beginning of banks (offset=0), and only one label is allowed per bank. This reflects the fact, that branches or calls are made to banks, rather than offsets.

Code labels that mark the beginning of TRAP handler functions must be unique  and referenced by prefixing their identifier by an asterisk ("*").

Such label references will generate an opcode that will trigger a TRAP and call the handler function referenced by the label.
Other labels are referenced by prefixing their identifier with "<" (for backward references, i.e. the label is defined earlier in the source code than the reference to it), or with ">" (forward reference, i.e. the label is defined later than the reference to it in the source code). If a label is not unique, the reference goes to the nearest occurrence of it.

#### Instruction Mnemonics

Mnemonics for Signal PAIR and ALU instructions generate the opcode they map to, for instance "ENTER" generates opcode 112. For ALU instructions, the signed offset can be omitted and defaults to 0 ("SLA+1", "ADD" = "ADD+0", etc.)

Mnemonics for transfer PAIR instructions are two-letter uppercase words such as "FA" (take F into A). Mnemonics of the form ("Nx") for transferring a literal embedded in the code stream into a target register, must be followed by that number, ALU mnemonic or other value to be transferred. For instance "NF SLA" transfers the ALU opcode corresponding to SLS into the function selector register F. "NA 80h" transfers the number 128 into register A.

#### Get-Put Mnemonics

The format for these mnemonics is register + memory location + offset + operation with no intervening space. Register must be lower case "a" or "b". Memory location is upper case G or L. Offset is in the range 0-7. Operation is lower case "g" (get) or "p" (put). 
So, "aG3g" instruction gets the value from the fourth location of global memory (same as bank G, offset 83h) and loads it into register A.

# Part A. Computation

This part explains the core features of the CPU, and for brevity and clarity is limited in scope to those that are intended for computation and addressing internal memory. Part B explains additional, structurally equivalent features that are dedicated to communication with peripherals and can be listed as  mere extensions to what has already been said.

## Addressing

In Sonne, the primary means of addressing memory are 8-bit numbers called offsets, ranging from 0-255. This offset range is partitioned into three segments. As further explained below, the segment in which a particular offset is located determines, which 8-bit bank prefix register is used in generating the effective address. The notation for this is [bank,offset]. This segmentation mechanism achieves, that an 8-bit offset carries a fully implied bank register, considerably extending the addressable range in an intuitive way.

### Banks

Bank numbers are stored in dedicated bank registers. Which bank register is used in accessing memory depends on the segment (see below) in which a given offset address is located, and also depends on whether the memory access is fetching (reading the code stream), or instead reading or writing other data.

This is much simpler in practice than how it sounds in full technical detail, much like the engineering details of a car can seem overwhelming when in reality driving is quite easy.

As has been said, the 256 offset places are partitioned into three segments. A given offset is located in exactly one of the three segments, and a given offset thus uniquely implies the bank prefix register used to create the effective address used.

#### ROW/EXIT segment

If the offset is in the range 0-127, bank register R (Row) is used during data access and bank register E (EXIT) is used when fetching (reading the code stream).

#### GLOBAL and LOCAL segment

The remaining two offset ranges apply for both data access and instruction fetch.

##### Global segment

If the offset is in the range 128-191, bank register G (Global) is used.

This 64 byte segment serves as a global (i.e. persistent during subroutine calls) data repository. The first eight addresses (128-135) of it are referred to as G0-G7 (G for global) and there are special single-cycle instructions for transferring these values to and from the two accumulator registers. These locations are essentially additional registers.

##### Local segment

If the offset is in the range 192-255, bank register L (Local) is used.

This 64 byte segment serves as a stack frame, which can be swapped out by the ENTER and LEAVE instruction. The first eight addresses (192-199) of this segment are referred to as L0-L7 (L for local) and there are special single-cycle instructions for transferring these values to and from the two accumulator registers. These locations are essentially additional registers local to the current subroutine!

#### CPU Internal Addressing

You may have noticed that, since the ROW segment (and also the GLOBAL+LOCAL segments combined) span only 128 bytes instead of the full range of 256 bytes, incrementing a bank register seems to leave 128 bytes of each bank unused.

In order to utilise all of the memory, only the first 7 bits of the offsets are appended to the bank number. The high order bit is actually used as an indicator of LOCAL/GLOBAL access and *prefixed* to the bank number.

In other words, the ROW/EXIT segment and the combined GLOBAL+LOCAL segments live in two adjacent memory blocks of 256 banks each with only 128 bytes per bank instead of 256.

Please see the schematics of the CPU to see how this works in detail to avoid "holes" in the address space.

### Offsets

During instruction fetch, the implied offset is retrieved from the program counter (PC).

During data access, the implied offset is retrieved from the working register W. W is a construct that refers to the last recently written-to accumulator register, A or B. Rather than being a separate register, W is simply a mechanism to point to either of the two accumulator registers.

Whenever a transfer instruction is executed to load data into A or B, "W" is updated to point to that register. For example, if the instruction xA is executed, then the source value is transferred into register A, and W is set to A. Similarly, if the instruction xB is executed, the source value is transferred into register B, and W is set to B.

### Memory access

During instruction fetch, an instruction is fetched from [E, PC].

All other memory operations are achieved by transferring register values into or out of the M (Memory) pseudo-register. The implied memory location is always [R/G/L, W]. In this notation, R/G/L stands for the bank prefix register corresponding to the offset segment in which the offset stored in the working register W is located.

For example, if A is the working register and contains the number 20, the effective address will be the concatenation of the bank address prefix in register R (since the offset 20 lies in that segment) and the offset 20 itself. Therefore, the instruction FM for instance will store the ALU result F into [R, 20].

There is also a dedicated instruction type (Get-Put) which allows transferring values in and out of the accumulator registers into the first 8 locations of the global and local segment, respectively. This is identical to addressing offsets 80-87h (first 8 bytes of global segment) and C0-C7h (first 8 bytes of local segment) but requires only a single instruction and no overhead for setting up address offsets.

## Arithmetic-Logic Computations

The Sonne model includes an Arithmetic-Logic Unit (ALU) connected to the two accumulator registers A and B. The ALU can perform a set of 16 operations on the contents of A and B, controlled by writing an ALU opcode to the function register (F). Results are retrieved by reading from register F. A zero flag used in conditional branching (see below) is computed together with the result. Writing an ALU opcode to F computes the respective function. Reading from F yields the result.

Writing into registers A or B sets that register to be the new working register (W), and sets a HOLD flag (H) which freezes the result in F, until another ALU opcode is written into F which frees the hold (resets H).

This mechanism is required so that the working register (A or B) can be used as destination offset for storing the ALU result. Without it, altering A or B to contain an address offset for storing the ALU result, would generally trigger an additional ALU operation that could modify the result, which is still in F, before it's even stored. By activating HOLD, the ALU freezes its current state, allowing for modifications to A or B for addressing purposes without triggering a new, potentially result-altering, ALU operation. The HOLD is released when the next ALU opcode is written, allowing for the resumption of regular ALU operations. In essence, HOLD ensures the proper sequence and integrity of computational and memory operations.


### ALU opcodes

An ALU opcode is a byte, in which bits 0-4 encode one of the following 16 instructions, and bits 5-7 encode a signed 3-bit binary number which is added to the result. 

Opcode bits 0-3 to the left, followed by mnemonic, description, and pseudo-code

|--- | --- | --- |--- |
| 0 |   IDA | Identity A | F := A |
| 1 |   IDB | Identity B | F := B |
| 2 |   OCA | Ones Complement A | F := ~A |
| 3 |  OCB | Ones Complement B | F := ~B |
| 4 |   SLA | Shift left A | F := A<<1 |
| 5 |   SLB | Shift left B | F := B<<1 |
| 6 |   SRA | Shift right A | F := A>>1 |
| 7 |   SRB | Shift right B | F := B>>1 |
| 8 |   AND | Logical A AND B | F := A&B |
| 9 |   IOR | Logical A OR B | F := A\|B |
| 10 |  EOR | Logical A XOR B | F := A\^B |
| 11 |  ADD | Bits 0-7 of A+B | F := (A+B)&255 (ignore carry) |
| 12 |  CYB | Carry bit of A+B | F := A+B; F := (F>255)?1:0 |
| 13 |  ALB | A less than B | F := (A<B)? 0xFF:0 |
| 14 |  AEB |A equals B | F := (A==B)? 0xFF:0 |
| 15 |  AGB |A greater than B | F:= (A>B)? 0xFF:0 |

## Exit Instructions

The term EXIT instruction refers to any instruction that overwrites register E (EXIT), which contains the currently active bank prefix for fetching instructions.  EXIT instructions are either transfer instructions into J, E, T (JUMPS) or into C (CALLS), the RET instruction (RETURN from subroutine) or TRAP instructions.

### Branching

Branching is done to banks, not offsets. During a branch, the program counter (PC) register holding the current instruction offset is set to zero, and the EXIT bank register is loaded with the target bank to which control control is transferred.

Even though it might seem limiting to only be able to branch to the start of a bank, this can actually lead to clean and well-structured code, as each bank can be thought of as a self-contained part of code that does one specific task within a function.

#### Registers J, E, T

These are pseudo-registers, that can only occur as register-targets in transfers. Transferring a value to J does an unconditional jump to that "branch". You can't jump to a byte offset, only to the first byte offset (0) of a branch. E means Else - the branch is taken when the ALU result is zero. T means Then - the branch is taken when the ALU result is not zero.

The LID instruction (see below) is an implicit jump to offset 0 of next bank. LID resets PC to 0 and increments the bank register by 1.

### Calling and TRAP mechanism

#### Registers C, X, Y

Transferring a value to C executes a subroutine call.  Executing a TRAP instruction has the same effect, except that with C, TRAP destinations can be calculated and provided by a source register at run-time.

During a call or TRAP (see below), the "branch" (register E) get's stored in Y, and the byte offset (register PC) of the instruction gets stored in X. Following that, register E is overwritten with the target bank number, and PC is set to zero.  By altering these values in X and Y, the return address can be redirected, for example in coding custom control-flow instructions.

Registers PC and E can be restored from X and Y by executing RET. This "undoes" the call and returns to the point just after the instruction which triggered the call. Unlike other CPU's, only one call-level is retained. Sonne does not manage a dedicated return stack, and you must save X and Y (see above) into the local segment yourself when implementing nested calls. These return coordinates then essentially become local variables.

#### Local Variables

The local segment can be used as a stack frame for local variables such as the subroutine return address. Executing ENTER decrements the L bank register, and LEAVE increments this register. Since bank registers are used implicitly, any subsequent memory access to the local offset segment will "see" whatever frame is mediated by L.

## TRAP instructions

TRAP instructions are otherwise indistinguishable from other instruction types, except they have bit 7 set and bit 6 cleared. The target bank number is encoded in the remaining lower 6 bits (0-5), hence banks 0-63 can be used to store trap handler functions. These are typically thought of as instruction set extensions stored in ROM.

## Get-Put Instructions

These instructions copy values between the first eight bytes (G0-G7) of the global segment or the first eight bytes (L0-L7) of the local segment and registers A or B.

Example:

The mnemonic "aG3g" ("a G3 get") would mean: "a": Register A is the target of the operation. "G": the global segment is being used. "3": The third location of the global segment is being accessed. "g": Get operation, the value from the specified memory location is being read and loaded into register A.

So, "aG3g" instruction gets the value from the third location of global memory and loads it into register A.

### Opcode Format

Get-Put instructions are those in which bits 6-7 are both set. Bits 0-5 encode the operation of each such instruction in the following way.
Bit 5 encodes the source or destination register: 0=A, 1=B.
Bit 4 encodes the segment: 0=Global, 1=Local.
Bit 3 encodes the operation: 0=Read (Get), 1=Write (Put).
Bits 0-2 encode the byte offset within the segment.


## PAIR Instructions

### Opcode Format

Instructions of type PAIR have bit 7 set to 0. There are two subtypes. If bits 0-3 are all zero, bits 4-6 encode a signal. If bits 0-3 are not all zero, the instruction is a register-to-register transfer, where bits 0-3 encode the target register and bits 4-6 encode the source register.


### Signals (bits 0-3 all zero)

These instructions trigger hardware functions.
NOP: No Operation

<Communication signals, see part B below>

LEAVE: Decrement bank register L
ENTER: Increment bank register L


### Register Transfers (bits 0-3 not all zero)

These instructions are mnemonically represented as "ST", where S represents one of the 8 source registers, and T represents one of the 15 target registers (target register 0 being SIG, a special case used only as a marker to identify SIGNAL instructions, see above). They copy the value of the source register into the target register, or in the case of pseudo-registers, interpreting the source value in a specific way for operations such as branching.

Note regarding register X and Y:  Once these registers are saved to local variables for instance, they can be used as a general purpose registers within the current function.

#### TARGET registers:

To name a target register without a source register attached in mnemonic form, prefix it with an "x" that stands for any source register.

xR (Set ROW bank prefix register)

xM (Store into [R/L/G, W])

xX (Set PC offset value that RET jumps to). If this register is saved to a local variable, for instance, it can be used as a general purpose register within the current function.

xY (Set EXIT bank value that RET jumps to)

<Communication target registers, see part B below>

xF (Set ALU opcode, deactivate ALU hold)

xA (Set accumulator register A, and set A as working register, activate ALU hold)

xB (Set accumulator register B, and set B as working register, activate ALU hold)

xJ (Unconditional jump, set EXIT bank register to the transferred value)

xT (Jump and set EXIT bank register to the transferred value, if ALU result not zero)

xE (Jump and set EXIT bank register to the transferred value, if ALU result zero)

xC (Call subroutine, save EXIT bank register to Y, save PC into X, set PC to 0, overwrite EXIT bank with transferred value)

xG (Set GLOBAL bank prefix register)


#### SOURCE registers:

To name a source register without any target register attached in mnemonic form, add an "x" to it that represents any target register.

Nx (Take number literal from [EXIT,PC], then increment PC)

Rx (Take value of ROW bank prefix register)

Mx (Load memory value from [R/L/G, W])

Xx (Take value of register X)

Yx (Take value of register Y)

Fx (Take ALU result value)

<Communication source registers, see part B below>


### Scrounging

Transfer instructions such as MM, XX, YY etc, which would essentially do nothing ("NOP") or conflict with the timing requirements and hardware complexity, are detected (preselected) by a logic module called the scrounger, and they are treated as entirely different instructions -- their opcode is "repurposed". For instance, the opcode with regular mnemonic NM is scrounged and repurposed as opcode for the RET instruction, because to allow fetching a byte from the code stream and storing it into memory within the same instruction cycle would unreasonably complicate the CPU hardware.

Not all such "NOP" opcodes are "scrounged", and the others could potentially be used as CPU instruction set extensions in future versions.

MM => LID (Close current bank): increment EXIT, set PC to 0
NM => RET (Return): restore PC, EXIT from registers X, Y respectively


# Part B. Communication

Sonne comes with a built-in communication subsystem that can handle both serial and parallel communication. These additional features are for controlling device selection, serial input/output, and parallel input/output.

For instance, there are additional SIGNAL instructions for controlling the serial clock line and putting the parallel bus into a tristate mode, and also dedicated registers for transferring data bytes into and out of the CPU in parallel or serial form.

## Communication Registers

On the hardware side, communication is done using the following implements:

### D

An 8-bit write-only device select register D. The bit pattern stored in this register is intended to be used for enabling/disabling devices attached to the serial/parallel buses. Writing a device select pattern is done by transferring values into register D.

In the reference implementation, the device select byte logically consists of two four-bit halves (nybbles). Each nybble selects one of 16 enable signals via an attached 4-bit decoder. For instance, the lower nybble could generate an enable signal for an input (listening) device on the bus, while the higher nybble could generate a similar signal that enables the transmitting output device required for a particular data bus transfer.

By convention, the value zero selects a null device (not connected), or some initialisation logic, since both nybbles of the D register will typically be zero upon power-up or reset.

### SOR

A write-only parallel-to-serial shift register for serialising an output byte (SOR, serial output register).

Writing an output value for serialisation is done by writing to register S.

### SIR

A read-only serial-to-parallel shift register for deserialising an input byte (SIR, serial input register).

Reading a deserialised input byte is done by reading register S.

### POR

A write-only tri-state register with 8-bit parallel output (POR, parallel output register)

Writing an output byte onto the parallel bus is done by writing to register P. If the parallel bus is in tri-state mode (OFF instruction), a write-operation will end tri-state mode.

### PIR

A read-only 8-bit parallel input register (PIR, parallel input register).

Reading the current 8-bit value on the parallel bus is done by reading register P.


## Communication Instructions

The following additional SIGNAL instructions (see part A above) operate on communication registers:

CSI: shift in one bit from serial data input line into SIR

CSO: shift out one bit from SOR onto serial data output line

SCL: set serial clock line high

SCH: set serial clock line low

OFF: put parallel bus into tristate mode

## Serial IO and implementing SPI

The Serial Peripheral Interface (SPI) protocol can be implemented using the Sonne processor's instructions, including SCL, SCH, CSI, and CSO, along with proper configuration of control signals. Here's a description of how the SPI protocol can be implemented with the Sonne instructions:

In addition to the serial input and output shift registers, the Sonne processor utilises the D register for device selection and deselection in serial communication.

Device Selection: Before communicating with a specific device connected to the serial bus, the corresponding bit pattern representing the device must be set in the D register. This selection process ensures that the desired device is enabled for communication. Data Transmission: To transmit data to the selected device, the processor loads the data to be sent into a register or memory location. The CSO (Clock Serial Out) instruction is then executed, which clocks the serial output shift register. As each bit is shifted out, it is sent to the selected device through the serial bus. Device Deselection: After data transmission is complete, the selected device needs to be deselected to allow other devices to communicate on the bus. This is done by clearing the bit pattern in the U or V register corresponding to the device. Deselecting the device prevents further communication with it. Data Reception: To receive data from an external device, the CSI (Clock Serial In) instruction is used. It clocks the serial input shift register, allowing the processor to receive one bit of data at a time from the selected device. The received data can then be processed or stored in registers or memory. By utilising the U and V registers for device selection and deselection, the Sonne processor can communicate with different devices connected to the serial bus in a controlled and synchronised manner. It allows for flexible communication with multiple devices using a serial interface such as SPI, ensuring proper device selection and enabling reliable data transmission and reception.

CPOL (Clock Polarity): The CPOL parameter determines the idle state of the clock signal. In Sonne, the SCL (Set Clock Low) and SCH (Set Clock High) instructions can be used to control the clock signal's state. To configure CPOL=0 (clock idles low), execute SCL to set the clock signal low during the idle state. To configure CPOL=1 (clock idles high), execute SCH to set the clock signal high during the idle state. CPHA (Clock Phase): The CPHA parameter determines the edge of the clock signal where data is captured or changed. In Sonne, the CSI (Clock Serial In) and CSO (Clock Serial Out) instructions can be used to control data transfer on each clock transition. To configure CPHA=0 (data captured on the leading edge), execute CSI before the clock transition to capture the incoming data. To configure CPHA=1 (data captured on the trailing edge), execute CSI after the clock transition to capture the incoming data. Similarly, to transmit data on the leading or trailing edge, execute CSO before or after the clock transition, respectively. Slave Select (SS) Signal: In SPI, a Slave Select signal is used to enable/disable specific devices on the bus during communication. In Sonne, the U and V registers can be utilised to control the Slave Select (SS) signal. Before initiating communication with a specific device, set the corresponding bit pattern in the U or V register to select the device. After communication, clear the bit pattern in the U or V register to deselect the device. By combining the SCL, SCH, CSI, and CSO instructions along with appropriate configuration of the control signals, the Sonne processor can effectively implement the SPI protocol. This allows for synchronised and controlled data transfer with SPI-compatible devices, including the flexibility to configure CPOL, CPHA, and SS signals to meet the specific requirements of the SPI interface.





