

77A, 14B *divmod8 0R NOP RDY  ;= 57h
endless@ NJ <endless
CLOSE

@mul8 ; Multiply A * B, result in A and B 
NEW wL3

   aL1                    ; Initialize copy multiplicand (low order)
   bL0                    ; Save multiplier
   NA 0 aL2               ; Clear high-order
   NN 8                   ; Initialize loop counter, 8 bits

loop@
   NB 1, L1a AND
   NF >skip
   L0a L2b ADD, rL2       ; Add multiplier if low order lsb set

skip@
   NB 1, L2a AND rG0      ; Check if high order lsb set
   L1a SRA, rL1           ; Shift low order byte right
   L2a SRA, rL2           ; Shift high order byte right
   
   G0a IDA, NF >done
   NA 80h, L1b IOR rL1

done@
   NI <loop               ; Decrement loop counter
   L1a
   L2b

L3w OLD RET
CLOSE


@divmod8 ; Divide A by B, division result in A, remainder in B
NEW wG0

aL0                       ; Dividend
bL1                       ; Divisor
NA 1, aL2                 ; Shift counter first 1 bit to MSB
NA 0, aL3                 ; Initialise quotient to zero

L1a IDA NF >ELOOP         ; Skip if divisor zero

NA 80h                    ; MSB mask
MSB_SHIFT@                ; Shift divisor left so that first 1 bit is at MSB
 L1b                      ; Load divisor
 AND NT >DIVIDE           ; Skip when MSB set
 SLB rL1                  ; Shift divisor left and update
 L2r R1+ rL2              ; Increment shift counter and update
 NJ <MSB_SHIFT

DIVIDE@
 L3b SLB rL3              ; Shift quotient left and update
 L1a OCA R1+ RA           ; Negate divisor
 L0b CAR                  ; Dividend check borrow bit
 NF >REP

 ADD rL0                  ; Accept subtraction, update dividend
 L3r R1+ rL3              ; Increment quotient

REP@
 L1a SRA rL1              ; Shift divisor right for next subtraction
 L2r R1- rL2              ; Decrement counter
 NT <DIVIDE               ; Branch back if not zero

ELOOP@ L3a, L0b

G0w OLD
RET
CLOSE

