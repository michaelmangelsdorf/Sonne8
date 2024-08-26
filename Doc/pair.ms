\#
\#
\#
\#
\#
\#
\#
\#
.NH 3
PAIR Instructions
.LP
PAIR opcodes encode instructions which
transfer a byte value from a source to a destination.
The source is encoded in the four low-order bits
0-3, and the destination is encoded in the three
high-order bits 4-6.
.LP
This manual uses the notation, that sources
have an "x" suffixed to their letter
(example: Gx "G into"), and destinations are
prefixed with an "x" (example: xG "into G").
.LP
Sources and destinations can be physical registers,
but in some cases are used to control more
complex functionality.
.PP
For instance, the Gx source corresponds to the value
of a physical hardware register, and writing to
the xG destination sets the value of that same register.
However, the Sx source for example represents the
current state of the serial input byte, whereas the
xS destination
sets the output data byte to be serialised.
\#
\#
\#
\#
\#
\#
\#
\#
.SH 4
The following table shows a list of all sources
and destination that can occur in PAIR opcodes.
When an "x" is marked as both prefix and suffix,
the letter can occur as both source and destination.
.LP
.TS
tab(@); lb l lb l .
Nx   @ Number literal in code stream @ xE  @ Enable register @
xO   @ Origin register @ xA @ Add I to G@
xGx  @ G register  @ xD  @ Data page index @
xRx  @ R register  @ xJ  @ Jump (set PC) @
xIx  @ I register  @ xW @ Jump while I@
xLx @ Local memory byte @ xT @ Jump if R nz@
xMx @ Data memory byte  @ xF @ Jump if R z@
xSx  @ Serial byte  @ xC  @@
xPx  @ Parallel byte  @@@
.TE  @   @@@

\#
\#
\#
\#
\#
\#
\#
\#
.SH 4
Scrounging
.LP
Not all combinations of source and destination in
opcodes of type PAIR are useful or practicable.
The instruction GG for example would transfer the value
of G into itself, an operation that does nothing.
.LP
Other combinations such as NMD would need to fetch
a number literal from code memory, and then within
the same instruction store this value into data memory.
On the hardware-level, this would require important
structural changes to accomodate for this.
.LP
Instead, the opcodes for such instructions of limited
usefulness are detected and remapped to other
instructions before they are executed.
.bp
.LP
The following table shows this mapping as of revision
step LOX. The mnemonics in the first column are for
"hypothetical" instructions and the assembler will
generate a warning about them being remapped.
Although currently most of these map to NOP, this
should not be relied upon as they are reserved for
instruction set extensions.
.TS
tab(@); r lb l .
NML => @ INO @ Increment register O
NMD => @ DEO @ Decrement register O
MLML => @ NOP @ (reserved)
MLML => @ NOP @ (reserved)
MDML => @ NOP @ (reserved)
MDMD => @ NOP@ (reserved)
GG =>  @ NOP @ (reserved)
RR =>  @ NOP@ (reserved)
II =>  @ NOP@ (reserved)
.TE
\#
\#
\#
\#
\#
\#
\#
\#
