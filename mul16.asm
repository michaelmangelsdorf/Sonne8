
; Program NOR flash EEPROM on mainboard with bootstrap code (bootstrap.asm)
; Attach IO board
; Assemble below code with assem.c
; Flash IO-board serial EEPROM with resulting object code
; This will multiply 7x13 and print result in hex (5Bh) on 7-segment display

;g1 multiplicand
;g2 multiplier
;g3 low result
;g4 high result

;l1 lsb
;l2 msb
;l3 loop counter
;l4 multiplier
;l5 temp

RAM

LA 7 AG1P
LA 13 AG2P

AG1G AL1P     ; initialize copy multiplicand
LA 0 AL2P     ; clear msb
LA 8 AL3P     ; initialize loop counter
LID

@loop 
LQ 1, AL1G, LF AND, LE >skip   ; add multiplier if low order lsb set
AG2G QL2G LF ADD, FQ QL2P
LID

@skip ; shift right
LQ 1 AL2G, LF AND, FQ, QL5P  ; check if high order lsb set
AL1G, LF SRA, FQ QL1P
AL2G, LF SRA, FQ QL2P
AL5G, LF IDA, LE >done
LA 80h, QL1G, LF IOR, FA AL1P
LID

@done
AL3G, DEA, AL3P, LF IDA, LT <loop

LU 0
AL1G, LF IDA, FP
LU 1
LU 0.
@STOP1 LB <STOP1.