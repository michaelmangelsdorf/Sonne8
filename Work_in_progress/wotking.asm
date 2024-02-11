; Assemble with sasm.c
; Run with mythlib.c
; Generates .MIF file for Quartus II

77A, 14B *divmod8 0R NOP RDY  ;= 57h
endless@ NJ <endless
CLOSE

@mul8 ; Multiply A * B, result in A and B 
NEW WL3

   AL1                    ; Initialize copy multiplicand (low order)
   BL0                    ; Save multiplier
   NA 0 AL2               ; Clear high-order
   NN 8                   ; Initialize loop counter, 8 bits

loop@
   NB 1, L1A AND
   NF >skip
   L0A L2B ADD, RL2       ; Add multiplier if low order lsb set

skip@
   NB 1, L2A AND RG0      ; Check if high order lsb set
   L1A SRA, RL1           ; Shift low order byte right
   L2A SRA, RL2           ; Shift high order byte right
   
   G0A IDA, NF >done
   NA 80h, L1B IOR RL1

done@
   NI <loop               ; Decrement loop counter
   L1A
   L2B

L3W OLD RET
CLOSE


@divmod8 ; Divide A by B, division result in A, remainder in B
NEW WG0

AL0                       ; Dividend
BL1                       ; Divisor
NA 1, AL2                 ; Shift counter first 1 bit to MSB
NA 0, AL3                 ; Initialise quotient to zero

L1A IDA NF >ELOOP         ; Skip if divisor zero

NA 80h                    ; MSB mask
MSB_SHIFT@                ; Shift divisor left so that first 1 bit is at MSB
 L1B                      ; Load divisor
 AND NT >DIVIDE           ; Skip when MSB set
 SLB RL1                  ; Shift divisor left and update
 L2R P1 RL2               ; Increment shift counter and update
 NJ <MSB_SHIFT

DIVIDE@
 L3B SLB RL3              ; Shift quotient left and update
 L1A OCA P1 RA            ; Negate divisor
 L0B CAR                  ; Dividend check borrow bit
 NF >REP

 ADD RL0                  ; Accept subtraction, update dividend
 L3R P1 RL3               ; Increment quotient

REP@
 L1A SRA RL1              ; Shift divisor right for next subtraction
 L2R M1 RL2               ; Decrement counter
 NT <DIVIDE               ; Branch back if not zero

ELOOP@ L3A, L0B

G0W OLD RET
CLOSE

