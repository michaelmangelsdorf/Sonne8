

77A, 14B *divmod8 0R NOP RDY  ;= 57h
;7A 3B *mul8 NOP
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
 L2R P1 RL2              ; Increment shift counter and update
 NJ <MSB_SHIFT

DIVIDE@
 L3B SLB RL3              ; Shift quotient left and update
 L1A OCA P1 RA           ; Negate divisor
 L0B CAR                  ; Dividend check borrow bit
 NF >REP

 ADD RL0                  ; Accept subtraction, update dividend
 L3R P1 RL3              ; Increment quotient

REP@
 L1A SRA RL1              ; Shift divisor right for next subtraction
 L2R M1 RL2              ; Decrement counter
 NT <DIVIDE               ; Branch back if not zero

ELOOP@ L3A, L0B

G0W OLD RET
CLOSE

; ----------------------------- asc_to_num ----------------------------------

@asc_to_num ; Convert ASCII code in A to a number, result in A
NEW WG0

   AL1                     ; Save A
   NB 48                   ; ASCII code of letter '0'
   OCB P1 RB              ; Subtract it from A
   ADD RB RL2

   NA 10 AGB               ; Result must be smaller than 10, if decimal
   NT >done

   L1A                     ; Not a decimal, try hexadecimal
   NB 10 ADD RA            ; Add 10 (value of hex letter 'A')
   NB 65                   ; ASCII code of letter 'A'
   OCB P1 RB              ; Substract it from A
   ADD RL2

   done@ L2A

G0W OLD RET
CLOSE

; ----------------------------- num_to_asc ----------------------------------

@num_to_asc ; Convert number in A to ASCII code
NEW WG0

   NB 10                   ; Number must be <10 for ASCII range '0' - '9'
   ALB NT >is_deci

   NB -10
   ADD RA                  ; Subtract 10
   NB 65                   ; ASCII code of letter 'A'
   ADD RA
   NJ >done

   is_deci@
   NB 48                   ; ASCII code of letter '0'
   ADD RA
   done@

G0W OLD RET
CLOSE

; ----------------------------- num_to_string -------------------------------

@num_to_string ; Convert value in A to ASCII string in global
               ; Number base in B

NEW WG0  ;REA?D

   AL1
   NA 88h AL3                    ; Result string base address
   NA 2
   AEB
   NT >is_bin
   NA 10
   AEB
   NT >is_dec
   is_hex@ NA >pow16 NJ >join    ; Set base address of divisor table
   is_dec@ NA >pow10 NJ >join
   is_bin@ NA >pow2

   join@ AL2
      ND <num_to_string          ; set table addr high
   rep@
      L1A
      L2B                        ; set table addr low
      MB                         ; get table entry
      IDB NF >done               ; zero entry marks end of table
      *divmod8
      BL0
      *num_to_asc
      L3B IDA RM IDB P1 RL3     ; Store, and increment strint ptr
      L0B BL1                    ; Remainder becomes new A
      L2B IDB P1 RL2            ; Point to next divisor
      NJ <rep

   done@

G0W OLD RET

pow2@ 128 64 32 16 8 4 2 1 0
pow10@ 100 10 1 0
pow16@ 16 1 0

CLOSE

; ----------------------------- string_to_num -------------------------------

@string_to_num   ; Convert ASCII string in global+4 to value in B
                 ; Number base in B

NEW WG0, ; REA?D

   NA 0 AL3                       ; Conversion result, initialise to zero
   NA 2 AEB                       ; Check which base it is
   NT >is_bin
   NA 10
   AEB
   NT >is_dec
   is_hex@ NA >pow16 NJ >join     ; Set base address of mult table
   is_dec@ NA >pow10 NJ >join
   is_bin@ NA >pow2

   join@
      AL1                         ; Points to first multiplier in table
      NA 84h                      ; Addr of first ASCII digit (left-most)

   seek@                          ; Get address of str termination byte
      AL2 IDA P1 RA
      MB
      IDB
      NT <seek

   ND <string_to_num              ; set table addr high
   rep@
      L2B MA                      ; Points to current digit
      *asc_to_num                 ; Convert to number in A
      L1B MB *mul8
      
      L3B ADD RL3                 ; Add to result
      L1B IDB P1 RL1
      L2R M1 L2A RL2
      NB 84h
      AEB NF <rep

   done@ L3B

G0W OLD RET

pow2@ 1 2 4 8 16 32 64 128 0
pow10@ 1 10 100 0
pow16@ 1 16 0

CLOSE

; ---------------------------------------------------------------------------

















