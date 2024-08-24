.NH 1
Assembler syntax
.RS
.LP
.NH 2
Comments
.LP
Comments are introduced by a semicolon (";"). Everything on the same line after the semicolon is ignored.
.NH 2
Phrasing
.LP
Commas ("," and ".") can be used for grouping "phrases" of instructions that logically belong together. They don't generate code and are just for visual clarity.
.NH 2
Closing
.LP
The assembler directive CLOSE "closes" the current page for object code placement. Subsequent code is placed from offset zero of the following page.
.NH 2
Number Literals
.LP
Decimal numbers from 0-255 can be included the source text as literals, and be prefixed by an optional minus sign. Hexadecimal numbers must be in two uppercase digits and marked with the suffix "h", for instance: "80h" for 128. Binary numbers must be formatted as two 4-bit groups separated by an underscore ("_") and have the suffix "b", for instance: "0010_0000b" for 32.
.NH 2
Page Labels
.LP
Page labels are alphanumeric identifiers, preceded with an at-sign ("@xyz"). Page labels mark the beginning of pages (offset=0), and only one page label is allowed per page. Only page labels can be targets of CALL and TRAP instructions.
.NH 2
Offset Labels
.LP
Offset labels are identifiers *ending* with an at-sign ("xyz@"). Offset labels can be targets of xJ, xI, xT, xF instructions, or be used to reference data locations within a page.
.NH 2
Trap Labels
.LP
An asterisk placed before a page label encodes a trap call to that address.
Such label references will generate an opcode that will trigger a TRAP and call the handler function referenced by the label.
.NH 2
Label References
.LP
Bank and offset labels are referenced by prefixing their identifier with "<" (for backward references, i.e. the label is defined earlier in the source code than the reference to it), or with ">" (forward reference, i.e. the label is defined later than the reference to it in the source code). If a label is not unique, the reference goes to the nearest occurrence of it in the given direction. A label reference is just a numerical value and can be used as such, too.
.NH 2
GETPUT
.LP
GETPUT instructions are written similar to x[LO]x memory transfers,
replacing O by the required offset (0-7), and prefixing or suffixing
by either R, O, D or G.
.LP
Examples: r[L0] ("PUT r into local var 0"), [L4]o ("GET o from local var 4")
.NH 2
Shortcut Notations
.LP
Instructions of type Nx (fetch literal value and place in register) have an alternative syntax:
9A place 9 into A, generates NA 9
<LABEL:J generates NJ <LABEL
33hR and 33h:R both generate NR 33h
.RE
