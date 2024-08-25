.NH 3
GETPUT Instructions
.LP
GETPUT opcodes encode instructions which
transfer values between registers R, O, G, and D
and the 8 top-most byte offsets of the
local page.
Bits 0-2 of the opcode encode a byte offset
into the local memory page, which is added
to the base address F8.
Bit 3 encodes whether the instruction is a GET
or a PUT operation.
Bits 4-5 encode the following operand registers
in order: ROGD.

