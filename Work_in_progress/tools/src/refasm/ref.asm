
;77A, 14B *divmod8, NOP RDY CLOSE


@mul8 ; Multiply A * B, result in A and B 
NEW

   aL1                    ; Initialize copy multiplicand (low order)
   bL0                    ; Save multiplier
   0A aL2                 ; Clear high-order
   8I                     ; Initialize loop counter, 8 bits

loop@
   1B L1a AND
   NF >skip
   L0a L2b ADD, rL2       ; Add multiplier if low order lsb set

skip@
   1B, L2a AND rG0        ; Check if high order lsb set
   L1a SRA, rL1           ; Shift low order byte right
   L2a SRA, rL2           ; Shift high order byte right
   
   G0a IDA, NF >done
   80h:A, L1b IOR rL1

done@
   NW <loop               ; Decrement loop counter
   L1a
   L2b

OLD RET
CLOSE


@divmod8 ; Divide A by B, division result in A, remainder in B
NEW

aL0                       ; Dividend
bL1                       ; Divisor
1A, aL2                   ; Shift counter first 1 bit to MSB
0A, aL3                   ; Initialise quotient to zero

L1r NF >ELOOP             ; Skip if divisor zero

80h:A                     ; MSB mask
MSB_SHIFT@                ; Shift divisor left so that first 1 bit is at MSB
 L1b                      ; Load divisor
 AND NT >DIVIDE           ; Skip when MSB set
 SLB rL1                  ; Shift divisor left and update
 L2r P1 rL2               ; Increment shift counter and update
 NJ <MSB_SHIFT

DIVIDE@
 L3b SLB rL3              ; Shift quotient left and update
 L1a OCA P1 RA            ; Negate divisor
 L0b CAR                  ; Dividend check borrow bit
 NF >REP

 ADD rL0                  ; Accept subtraction, update dividend
 L3r P1 rL3               ; Increment quotient
 
REP@
 L1a SRA rL1              ; Shift divisor right for next subtraction
 L2r M1 rL2               ; Decrement counter
 NT <DIVIDE               ; Branch back if not zero

ELOOP@ L3a, L0b

OLD RET
CLOSE


;L3-as-divisor
;B-get-divisor SLB B-put-divisor











