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

.PDFPIC figures/decoder.pdf
.bp

.so sys.ms
.so adjust.ms
.so alu.ms
.so trap.ms
.so getput.ms
.so pair.ms
.RE
