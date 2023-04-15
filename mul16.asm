
;G1 multiplicand
;G2 multiplier
;G3 low result
;G4 high result

;L1 lsb
;L2 msb
;L3 loop counter
;L4 multiplier
;L5 temp

RAM

LA 7 aG1p
LA 13 aG2p

aG1g aL1p     ; initialize copy multiplicand
LA 0 aL2p     ; clear msb
LA 8 aL3p     ; initialize loop counter
LID

@loop 
LQ 1, aL1g, LF AND, LE >skip   ; add multiplier if low order lsb set
aG2g qL2g LF ADD, FQ qL2p
LID

@skip ; shift right
LQ 1 aL2g, LF AND, FQ, qL5p  ; check if high order lsb set
aL1g, LF SRA, FQ qL1p
aL2g, LF SRA, FQ qL2p
aL5g, LF IDA, LE >done
LA 80h, qL1g, LF IOR, FA aL1p
LID

@done
aL3g, DEA, aL3p, LF IDA, LT <loop

LU 0
aL1g, LF IDA, FP
LU 1
LU 0.
@STOP1 LB <STOP1.

