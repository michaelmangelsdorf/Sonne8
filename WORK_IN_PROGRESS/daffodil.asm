
@COLD
   *mul8
   nc >end
LID

; ------------- Multiplication routine ---------------

@mul8
   na 7 aG1p     ; G1 multiplicand
   na 13 aG2p    ; G2 multiplier
   aG1g aL1p     ; initialize copy multiplicand (low order)
   na 0 aL2p     ; clear high-order
   na 8 aL3p     ; initialize loop counter, 8 bits

loop@
   nb 1, aL1g, nf AND, ne >skip   ; add multiplier if low order lsb set
   aG2g bL2g nf ADD, fb bL2p

skip@
   nb 1 aL2g, nf AND, fb, bL5p    ; check if high order lsb set
   aL1g, nf SRA, fb bL1p          ; shift right
   aL2g, nf SRA, fb bL2p
   aL5g, nf IDA, ne >done
   na 80h, bL1g, nf IOR, fa aL1p

done@
   aL3g, nf IDA-1 fa, aL3p
   nf IDA, nt <loop
   aL1g aG0p                      ; Display result 7*13=91 = 5Bh

LID

; ------------- Multiplication routine ---------------


@end
   ns 0
   stop@ nj <stop LID

