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
