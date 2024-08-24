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