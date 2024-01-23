
; Sonne CPU rev. Schneelein
;  Demo source

; Assemble with sasm.c
; Run with sonne-vm.c

; Calculate 5*9 = 2Dh, result shows in sonne-vm.c register dump (reg A).

5A, 9B *mul8 RDY

CLOSE

; ----------------------------- mul8 ----------------------------------------

@mul8 ; Multiply A * B, result in A and B 
ELF
   Al1                    ; Initialize copy multiplicand (low order)
   Bl0                    ; Save multiplier
   NA 0 Al2               ; Clear high-order
   NI 8                   ; Initialize loop counter, 8 bits

loop@
   NB 1, l1A AND
   NF >skip
   l0A l2B ADD, Rl2       ; Add multiplier if low order lsb set

skip@
   NB 1, l2A AND Rg0      ; Check if high order lsb set
   l1A SRA, Rl1           ; Shift low order byte right
   l2A SRA, Rl2           ; Shift high order byte right
   
   g0A IDA, NF >done
   NA 80h, l1B IOR Rl1

done@
   ND <loop               ; Decrement loop counter
   l1A
   l2B

LLF RET
CLOSE

; ----------------------------- divmod8 -------------------------------------

@divmod8 ; Divide A by B, division result in A, remainder in B
ELF

Al0                       ; Dividend
Bl1                       ; Divisor
NA 1, Al2                 ; Shift counter first 1 bit to MSB
NA 0, Al3                 ; Initialise quotient to zero

l1A IDA NF >ELOOP         ; Skip if divisor zero

NA 80h                    ; MSB mask
MSB_SHIFT@                ; Shift divisor left so that first 1 bit is at MSB
 l1B                      ; Load divisor
 AND NT >DIVIDE           ; Skip when MSB set
 SLB Rl1                  ; Shift divisor left and update
 l2R R1+ Rl2              ; Increment shift counter and update
 NJ <MSB_SHIFT

DIVIDE@
 l3B SLB Rl3              ; Shift quotient left and update
 l1A OCA R1+ RA           ; Negate divisor
 l0B CAR                  ; Dividend check borrow bit
 NF >REP

 ADD Rl0                  ; Accept subtraction, update dividend
 l3R R1+ Rl3              ; Increment quotient

REP@
 l1A SRA Rl1              ; Shift divisor right for next subtraction
 l2R R1- Rl2              ; Decrement counter
 NT <DIVIDE               ; Branch back if not zero

ELOOP@ l3A, l0B

LLF RET
CLOSE


