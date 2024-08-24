.NH 1
Control Flow
.LP
In this architecture, calls and traps go to page heads (offset zero). Other branches (jumps) go to offsets within a page.
.RS
.NH 2
Relative in-page Branching
.NH 3
Destinations J, JW, JF, JT
.LP
These are pseudo-registers, that can only occur as target registers of transfer instructions.
.LP
Transferring an offset value into J (Jump) does an unconditional branch to that offset within the current code page.
.LP
F stands for FALSE: Transferring an offset value into JF branches only if R is FALSE (zero).
.LP
T stands for TRUE: Transferring an offset value into JT branches only if R is TRUE (not zero).
.LP
W means iterate: Transferring an offset value into JW decrements the loop counter register I, and then branches, if that register is zero.
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