.NH 1
Control Flow
.LP
In this architecture, calls and traps go to page heads (byte offset zero).
Other branches (jumps) are always relative offsets within the current page.
.RS
.NH 2
Relative in-page Branching
.NH 3
Destinations J, W, F, T
.LP
These are pseudo-registers, that can only occur as target registers of transfer instructions.
.LP
Writing a value into J (Jump) adds this value to J (Program Counter) as a signed number.
This also applies for the F, T and W instructions described below.
.LP
F stands for FALSE: Writing into F branches only if R is FALSE (zero).
.LP
T stands for TRUE: Writing into T branches only if R is TRUE (not zero).
.LP
W stands for WHILE: Transferring an offset value into W branches
if the loop counter register I is not zero.
After checking (post-decrement), the I register is decremented by 1 in either case.
.PDFPIC figures/jumps-decode.pdf
.bp
.NH 2
Page-head Calls and Traps
.LP
Transfering a page index byte into C executes a call to offset 0 of that page ("Page head").
This is equivalent to executing a TRAP instruction encoding the same page index, as described below.
.PDFPIC figures/traps.pdf
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