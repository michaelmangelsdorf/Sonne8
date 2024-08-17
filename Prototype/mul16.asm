

; This file assembles with assem.c
; Write object code into SPI EEPROM on IO board


;G3 low result
;G4 high result

;L1 lsb
;L2 msb
;L3 loop counter
;L4 multiplier
;L5 temp

RAM

LA 7 aG1p  ;G1 multiplicand
LA 13 aG2p ;G2 multiplier

aG1g aL1p     ; initialize copy multiplicand
LA 0 aL2p     ; clear msb
LA 8 aL3p     ; initialize loop counter, 8 bits
LID

@loop
LB 1, aL1g, LF AND, LE >skip   ; add multiplier if low order lsb set
aG2g bL2g LF ADD, FB bL2p
LID

@skip ; shift right
LB 1 aL2g, LF AND, FB, bL5p  ; check if high order lsb set
aL1g, LF SRA, FB bL1p
aL2g, LF SRA, FB bL2p
aL5g, LF IDA, LE >done
LA 80h, bL1g, LF IOR, FA aL1p
LID

@done
aL3g, LF IDA-1 FA, aL3p
LF IDA, LT <loop

LD 0
aL1g, LF IDA, FP     ; Display result 7*13=91 = 5Bh
LD 0001.0000b
LD 0.
@STOP1 LJ <STOP1.


