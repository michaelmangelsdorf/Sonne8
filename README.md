
Welcome to this project!

This repo contains all files you need to build and program an 8-bit CPU that I have designed und built.

You would need to have the two PCBs fabricated: Firstly, there's the CPU board itself.
It is actually a micro-controller, in that this board implements RAM and an IO interface.
Then secondly, there is an IO expansion board, that fits on top of the CPU board via Arduino type stackable headers.

The CPU board looks like this:

![CPU board](https://github.com/Dosflange/Sonne/blob/main/board_and_screws.jpg)

These plastic spacers go between the CPU board and the IO board. The distance/length of the spacer part (without the protruding thread) is 2 cm.

![CPU board](https://github.com/Dosflange/Sonne/blob/main/board_screws.jpg)

Both modules go together like so:

![CPU board](https://github.com/Dosflange/Sonne/blob/main/board_sandwich.jpg)

The assembled development kit i.e. hardware portion of this project:

![CPU board](https://github.com/Dosflange/Sonne/blob/main/board_ready.jpg)

## Address space

The address space of this CPU is unusual. Eight bits give you 256 address places. The first 128 places form a region, which can be switched to point to another memory segment ("bank switching"). The next 64 places after that are fixed (for global variables). The remaining 64 places form a region again, which can be switched out to point to another memory segment (for local variables).

### Global and Local regions

Addresses 128-135 are referred to as G0-G7 (G for global) in the assembler mnemonic for register-memory transfer instructions.
Addresses 192-199 are referred to as L0-L7 (L for local) in the assembler mnemonic for register-memory transfer instructions.

### Leave and Enter

The LEAVE signal increments the local address prefix. This causes the final 64-bytes ("local" segment) of the address space to point to the previous stack frame. The ENTER signal decrements the local address prefix, causing a new stack frame to appear in the "local" segment.

## Instruction format

There are four types of instructions. (1) The high order four bits of the instruction word are all zero: These are sixteen "signal" instructions that take no operands, where the signal is encoded in bits 0-3. (2) The high order bit is zero, but bits 4-6 are not all zero: These are register-to-register transfers, where bits 4-6 encode the source register, bits 0-3 encode the target register. (3) The high order bit is one, bit 6 is 0: These are micro-calls (TRAP calls), where bits 0-5 encode the call-target. (4) Bits 6 and 7 or both one: These are register-to-memory transfers, from the two accumulator registers to the first 8 bytes of the global or local memory section, where bit 5 encodes which register it is, bit 4 encodes global or local, bit 3 is whether it's load or store, and bits 0-2 encode the address offset. (See the opcode_matrix file in the repo for details.)

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

### A, G and M

M is a pseudo-register. Reading or writing it transfers between memory and a register operand.
By default, the G register holds the byte offset to which reading from or writing to M will go in memory. The G register can be forced to take on this role by executing the SETG signal. The SETA signal switches to register A to provide the byte offset.

The CLIP signal restores the previously held value of G. Writing to G places the current value into a buffer register, before it is overwritten. G is the default address register upon reset.

G Register and A Register (Memory Addressing):

The G register and A register are both involved in memory addressing operations in the Sonne CPU.
The G register holds a byte offset that, when combined with the frame value stored in the R register, forms the effective memory address for instruction fetches.

An internal flag determines whether A or G is used as the byte offset during data access memory operations.
SETA Instruction:
The SETA instruction is used to set the internal flag to that the A register should be used as the byte offset during data access memory operations. The PULL instruction does this as a side-effect.

When executing SETG or during a reset, the internal bit is flipped, indicating that G should be used as the byte offset during data access memory operations.
This ensures that the byte offset is provided by the G register for memory addressing.
By toggling the internal flag using instructions like SETA, PULL, SETG, and during reset, the Sonne CPU can determine whether the A or G register is 

### R

The R register holds the bank prefix of the 128 byte "row" region from 0-127 of the address space. Changing R switches all 128 bytes simultaneously by address mapping.

### B, E, T

These are pseudo-registers, that can only occur as register-targets in transfers. Transfering a value to B does an unconditional jump to that "branch". You can't jump to a byte offset, only to the first byte offset (0) of a branch. E means Else - the branch is taken when the ALU result is zero. T means Then - the branch is taken when the ALU result is not zero.

### C, X, Y

The C register is a pseudo register, that can only occur as a register-target in transfers. Transfering a value to C executes a subroutine call. The "branch" get's stored in Y, the byte offset for instruction fetches within that "branch" gets stored in X. By writing values into X and Y, the return address can be redirected. The RET signal returns from the subroutine, by restoring the "branch" and byte offset from X and Y. The PULL signal transfers the current X into A, and the current Y into R.

### Q, A, F

Q and A are two accumulator registers. These are the two input operands connected to the ALU (Arithmetic-Logic Unit). F stands for Function. Writing an ALU opcode to F computes the respective function. Reading from F yields the result.

A and Q are not readable directly. The intended way is to execute one of the identity functions of the ALU, IDA or IDQ.

To prepare ALU input values, program the ALU, and retrieve the result, the following steps are typically involved:

Load Operand Values: The input values for the ALU operation need to be loaded into the A and Q registers. This can be done by transferring values from memory or other registers to the A and Q registers using appropriate transfer instructions.
Set ALU Opcode: Determine the specific ALU operation you want to perform and set the corresponding ALU opcode. The ALU opcode is written to the F register, which programs the ALU to perform the desired operation.
Execute ALU Operation: Trigger the execution of the ALU operation by performing an ALU instruction that uses the A and Q registers as source operands and the F register as the target. The ALU will perform the specified operation on the input operands and store the result in the F register.
Retrieve Result: After the ALU operation is executed, the result will be stored in the F register. Retrieve the result from the F register for further processing or storing in memory if needed.
These steps ensure that the input values are properly prepared, the ALU is programmed with the desired operation, the ALU operation is executed, and the result is retrieved for subsequent use or storage.

In Sonne, the F register functions differently depending on whether it is used as an input register or when it is read from. This distinction is important and can be easily overlooked.

When the F register is used as an input register, it serves as a parameter to program the ALU's operation. The value written to the F register determines the specific ALU opcode, which in turn determines the arithmetic or logical operation to be performed. It essentially configures the ALU's behavior for the subsequent operation.

On the other hand, when the F register is read from, it holds the most recent result produced by the ALU. This result reflects the outcome of the operation executed using the previously programmed ALU opcode. In this context, the F register acts as a storage location for the ALU result, allowing it to be accessed by subsequent instructions or transferred to other registers.

This distinction highlights the dual role of the F register in Sonne: as a configuration parameter for the ALU when written to, and as a storage location for the ALU result when read from. It is important to understand this behavior to properly utilize the ALU and interpret the effects of instructions involving the F register.

### ALU instructions

ALU instruction words (to be written to the F register) are bytes. The low order 4 bits encode the following 16 instructions:

```
0  IDQ ("Identity Q" / OP:=Q) DEFAULT (!)
1  IDA ("Identity A" / OP:=A)
2  OCQ ("Ones Complement Q" / OP:=~Q)
3  OCA ("Ones Complement A" / OP:=~A)
4  SLQ ("Shift left Q" / OP:=Q<<1)
5  SLA ("Shift left A" / OP:=A<<1)
6  SRQ ("Shift right arithmetic Q" / OP:=Q>>1)
7  SRA ("Shift right arithmetic A" / OP:=A>>1)
8  AND (OP:=Q&A)
9  IOR (OP:=Q|A)
10 EOR (OP:=Q^A)
11 ADD ("Add" / OP:=Q+A, bits 0-7)
12 CYF ("Carry Flag" / OP:= Carry, 9th bit of Q+A) 00h or 01h
13 QLA ("Flag: Q less than P" / OP:= (Q<A)? 0xFF:0
14 QEA ("Flag: Q equals A" / OP := (Q==A)? 0xFF:0
15 QGA ("Flag: Q greater than A" / OP:= (Q>A)? 0xFF:0
```

The high order 4 bits hold a signed 3 bit offset which is added to the ALU result. In the assembly language, these mnemonics can be followed by an optional number term, such as IDQ+2, SLA+1 etc. The default ALU operation on reset is "IDQ+0".
Read ALU results from the F register.


## IO

### U, V, S, P

Each bit in the U and V registers selects an implementation specific IO device (for example SPI device select etc). Writing to P (Parallel) transfers a byte onto the tri-state IO bus. Reading from P transfers a byte from the IO-bus. Use the OFF signal to tri-state the bus. Writing to P also removes the tri-state.

Writing to the S (Serial) register puts that byte into a shift-register for serialization. The CSO signal ("clock serial out") shifts out one bit on the MOSI line. Reading from S yields the currently deserialized byte in the shift-register. The CSI signal ("clock serial in") shifts in one bit from the MISO line. The SCH and SCL signals respectively toggle the serial master clock high or low.

The serial interface facilitates serial communication between the CPU and external devices.
The S (Serial) register acts as a shift register, allowing data to be transmitted or received bit by bit.
The CSI (Clock Serial In) instruction clocks the serial input shift register, reading in data.
The CSO (Clock Serial Out) instruction clocks the serial output shift register, sending out data.
Device Selection:
The U and V registers are involved in device selection for serial or parallel communication.
By setting specific bit patterns in the U or V registers, the CPU can select specific devices connected to the bus for communication.

### Parallel IO

Parallel Interface:
The parallel interface enables communication between the CPU and external devices using a parallel bus.
The P (Parallel) register is used for data transfer. Reading from the P register retrieves a byte value from the parallel bus, and writing to it sends a byte value onto the bus.
The OFF instruction can be used to tristate the parallel bus, disabling its output when necessary.

### Serial IO

The Serial Peripheral Interface (SPI) protocol can be implemented using the Sonne processor's instructions, including SCL, SCH, CSI, and CSO, along with proper configuration of control signals. Here's a description of how the SPI protocol can be implemented with the Sonne instructions:

In addition to the serial input and output shift registers, the Sonne processor utilizes the U and V registers for device selection and deselection in serial communication.

Here's an expanded summary of the serial communication capabilities of Sonne, including the roles of U and V:

Device Selection: Before communicating with a specific device connected to the serial bus, the corresponding bit pattern representing the device must be set in either the U or V register. This selection process ensures that the desired device is enabled for communication.
Data Transmission: To transmit data to the selected device, the processor loads the data to be sent into a register or memory location. The CSO (Clock Serial Out) instruction is then executed, which clocks the serial output shift register. As each bit is shifted out, it is sent to the selected device through the serial bus.
Device Deselection: After data transmission is complete, the selected device needs to be deselected to allow other devices to communicate on the bus. This is done by clearing the bit pattern in the U or V register corresponding to the device. Deselecting the device prevents further communication with it.
Data Reception: To receive data from an external device, the CSI (Clock Serial In) instruction is used. It clocks the serial input shift register, allowing the processor to receive one bit of data at a time from the selected device. The received data can then be processed or stored in registers or memory.
By utilizing the U and V registers for device selection and deselection, the Sonne processor can communicate with different devices connected to the serial bus in a controlled and synchronized manner. It allows for flexible communication with multiple devices using a serial interface such as SPI, ensuring proper device selection and enabling reliable data transmission and reception.

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
Before initiating communication with a specific device, set the corresponding bit pattern in the U or V register to select the device.
After communication, clear the bit pattern in the U or V register to deselect the device.
By combining the SCL, SCH, CSI, and CSO instructions along with appropriate configuration of the control signals, the Sonne processor can effectively implement the SPI protocol. This allows for synchronized and controlled data transfer with SPI-compatible devices, including the flexibility to configure CPOL, CPHA, and SS signals to meet the specific requirements of the SPI interface.


## Trap calls

When a TRAP instruction is encountered during program execution, it triggers a transparent subroutine call to a user-defined TRAP handler routine. This handler routine acts as the implementation of the custom instruction, enabling the execution of specialized operations that go beyond the capabilities of the standard built-in instructions.

User-defined instructions implemented through trap handlers are indistinguishable from pre-existing built-in instructions. They allow developers to create new instructions that appear and behave just like native instructions to the processor.

Flexible Byte Lengths:
Custom instructions defined through trap calls can have variable byte lengths. The PULL instruction facilitates this. It allows the custom instruction's handler routine to access subsequent bytes in the instruction stream, enabling the retrieval of data or parameters specific to the functionality of the custom instruction. This flexibility opens up possibilities for instructions with different byte lengths tailored to specific requirements, such as immediate addressing.

####1 **TRAP Instruction:** The TRAP mechanism is triggered by executing a specific TRAP instruction. In the Sonne architecture, there are 64 such instructions, ranging from T00h (opcode 80h) to T3Fh (opcode BFh). The specific trap to be executed is determined by bits 0-6 of the opcode.

####2 **Saving the Execution Context:** When a TRAP instruction is executed, the Sonne processor first saves the current frame and byte offset in dedicated hardware registers. This context saving is crucial as it allows the processor to resume execution from where it left off once the trap handler has finished its work.

####3 **Calling the Trap Handler:** After saving the execution context, the processor branches to a specific location in memory where the handler for the triggered trap resides. This memory address is determined by bits 0-6 of the TRAP opcode.

####4 **Writing the Trap Handler:** You as the programmer can write a trap handler to perform any desired operation. This could be a simple operation such as adding a new arithmetic function, or it could be a complex routine for interacting with the operating system or hardware. The trap handler needs to be written in Sonne machine language and loaded into the memory location that corresponds to the trap number.

####5 **Returning from the Trap:** Once the trap handler has completed its operation, it needs to return control back to the main program. This is typically done by executing a special return-from-trap instruction that restores the previously saved execution context. The processor will then resume executing instructions from where it left off.

In this way, the TRAP mechanism in Sonne architecture allows you to effectively "extend" the instruction set with your own custom operations. By strategically writing and calling these trap handlers, you can enhance the functionality of the processor in ways that go beyond the capabilities of the built-in instructions. 

## XFER (transfer) mnemonics are formatted as follows:

[Register][Memory Scope][Memory Location][Operation]

Each of these components has specific valid values:

Register (a or q): This indicates the register involved in the transfer operation.
a: Indicates that the accumulator (A) register is the source or target of the operation.
q: Indicates that the data (Q) register is the source or target of the operation.
Memory Scope (G or L): This indicates whether the operation involves global or local memory.
G: Indicates that the operation is interacting with global memory.
L: Indicates that the operation is interacting with local memory.
Memory Location (0-7): This is a digit specifying the particular location in memory being accessed. This value ranges from 0 to 7.
Operation (g or p): This specifies whether the operation is a 'get' or 'put'.
g: Get operation. The value from the specified memory location is being read and loaded into the specified register.
p: Put operation. The value from the specified register is being written to the specified memory location.
An example of an XFER mnemonic following this format is "aG3g", which denotes a 'get' operation that loads the value from the third location in global memory into the accumulator (A) register.

### Examples

The mnemonic "aG3g" ("a G3 get") would mean:

"a": Register A is the target of the operation.
"G": Global memory is being used.
"3": The third location in global memory is being accessed.
"g": Get operation, the value from the specified memory location is being read and loaded into register A.
So, "aG3g" instruction gets the value from the third location of global memory and loads it into register A.

Here are examples of few other XFER instructions:

"aL7p": This instruction would put the value from register A into the seventh location of local memory.
"qG2g": This instruction would get the value from the second location of global memory and load it into register Q.
"qL5p": This instruction would put the value from register Q into the fifth location of local memory.
Remember that in these mnemonics, the first character ("a" or "q") denotes the register being operated upon, the second character ("G" or "L") specifies the scope of the memory being accessed (Global or Local), the third character (a digit from 0-7) specifies the location in the specified memory, and the last character ("g" or "p") specifies the operation (Get or Put).


