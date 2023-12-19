
@Cold            ; Daffodil Firmware for Sonne micro-controller
   nc >End.      ; Dec 2023 Michael Mangelsdorf <mim@ok-schalter.de>
CLOSE            ; See https://github.com/Dosflange/Sonne

; ----------------------------- mul8 ----------------------------------------

@mul8            ; Multiply A x B, result in A and B 
ENTER

   aL1p,         ; Initialize copy multiplicand (low order)
   bL4p.         ; Save multiplier
   na 0, aL2p.   ; Clear high-order
   na 8, aL3p.   ; Initialize loop counter, 8 bits

loop@
   nb 1, aL1g, nf AND.
   ne >skip.
   aL4g, bL2g, nf ADD.            ; Add multiplier if low order lsb set
   fb, bL2p.

skip@
   nb 1, aL2g, nf AND.
   fb, bL5p.                      ; Check if high order lsb set
   
   aL1g, nf SRA, fb bL1p.         ; Shift low order byte right
   aL2g, nf SRA, fb bL2p.         ; Shift high order byte right
   
   aL5g, nf IDA, ne >done.
   na 80h, bL1g, nf IOR, fa aL1p.

done@
   aL3g, nf IDA-1, fa.
   aL3p, nt <loop.
   aL1g.
   bL2g.

LEAVE RET
CLOSE

; ---------------------------------------------------------------------------


@End
   na 10, nb 13, *mul8.
   aG0p.
   ns 0.
   stop@ nj. <stop
CLOSE

