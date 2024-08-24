.NH 3
GETPUT Instructions
.LP
GETPUT opcodes encode instructions which
transfer values between registers R, O, G, and D
and the 8 top-most byte offsets of the
local page.
The assembler notation for these instructions
is \*[CODE]p[L0-7]g\*[CODE X].