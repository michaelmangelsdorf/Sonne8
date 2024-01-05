
@Cold            ; Firmware for Sonne micro-controller rev. Daffodil
   na >End EXIT  ; Dec 2023 Michael Mangelsdorf <mim@ok-schalter.de>
CLOSE            ; See https://github.com/Dosflange/Sonne

; CLOSE is an assembler directive,
; that object code goes into the next 256 byte bank

; f2Lime, Lawn, Lava, Lake
; a2Garnet, Gold, Grape2b

; ----------------------------- mul8 ----------------------------------------

@mul8 ; Multiply A * B, result in A and B 
ENTER

   aL1                     ; Initialize copy multiplicand (low order)
   bL0                     ; Save multiplier

         nb C8h rm         ; Save return address

   na 0 aL2                ; Clear high-order
   na 8 aL3                ; Initialize loop counter, 8 bits

loop@
   nb 1, L1a AND,
   ne >skip
   L0a, L2b ADD           ; Add multiplier if low order lsb set
   rL2

skip@
   nb 1, L2a AND 
   rG0                     ; Check if high order lsb set
   L1a SRA, rL1           ; Shift low order byte right
   L2a SRA, rL2           ; Shift high order byte right
   
   G0a IDA, ne >done
   na 80h, L1b IOR,
   rL1

done@
   L3a IDA r1- rL3
   ni <loop
   L1a
   L2b

nb C8h mr
LEAVE RET
CLOSE

; ----------------------------- divmod8 -------------------------------------

@divmod8 ; Divide A by B, division result in A, remainder in B
ENTER

aL0                       ; Dividend
bL1                       ; Divisor

      nb C8h rm           ; Save return address

na 1, aL2                 ; Shift counter
na 0, aL3                 ; Initialise quotient to zero

L1a IDA ne >ELOOP         ; Skip if divisor zero

na 80h.                    ; MSB mask
MSB_SHIFT@                 ; Shift divisor left so that first 1 bit is at MSB
 L1b                      ; Load divisor
 AND ni >DIVIDE            ; Skip when MSB set
 SLB rL1                  ; Shift divisor left and update
 
 L2b
 IDB r1+ rL2              ; Increment shift counter and update
 nj <MSB_SHIFT

DIVIDE@
 L3b SLB rL3             ; Shift quotient left and update
 L1a                      ; Divisor
 L0b                      ; Dividend
 OCA r1+ ra                ; Negate divisor
 CYF                       ; Check borrow bit
 ne >REP

 ADD ra                    ; Accept subtraction
 aL0                      ; Update dividend
 L3a IDA r1+ rL3         ; Increment quotient

REP@
 L1a SRA rL1             ; Shift divisor right for next subtraction
 L2a IDA ne >ELOOP        ; Check if counter value zero
 IDA r1- rL2
 ni <DIVIDE

ELOOP@ L3a, L0b

nb C8h mr
LEAVE RET
CLOSE

; ----------------------------- asc_to_num ----------------------------------

@asc_to_num ; Convert ASCII code in A to a number, result in A
ENTER
nb C8h rm

   aL1                    ; Save A
   nb 48                   ; ASCII code of letter '0'
   OCB r1+ rb              ; Subtract it from A
   ADD rb rL2

   na 10 AGB               ; Result must be smaller than 10, if decimal
   ni >done

   L1a                    ; Not a decimal, try hexadecimal
   nb 10 ADD ra            ; Add 10 (value of hex letter 'A')
   nb 65                   ; ASCII code of letter 'A'
   OCB r1+ rb              ; Substract it from A
   ADD rL2

   done@ L2a

nb C8h mr
LEAVE RET
CLOSE

; ----------------------------- num_to_asc ----------------------------------

@num_to_asc ; Convert number in A to ASCII code
ENTER
nb C8h rm

   nb 10                   ; Number must be <10 for ASCII range '0' - '9'
   ALB ni >is_deci

   nb -10
   ADD ra                  ; Subtract 10
   nb 65                   ; ASCII code of letter 'A'
   ADD ra
   nj >done

   is_deci@
   nb 48                   ; ASCII code of letter '0'
   ADD ra
   done@

nb C8h mr
LEAVE RET
CLOSE

; ----------------------------- num_to_string -------------------------------

@num_to_string ; Convert value in A to ASCII string in global
               ; Number base in B

ENTER aL1
na C8h rm, na C9h fm
   
   na 88h aL3                     ; Result string base address
   na 2
   AEB
   ni >is_bin
   na 10
   AEB
   ni >is_dec
   is_hex@ na >pow16 nj >join      ; Set base address of divisor table
   is_dec@ na >pow10 nj >join
   is_bin@ na >pow2

   join@ aL2
      nf <num_to_string            ; set table addr high
   rep@
      L1a
      L2b                         ; set table addr low
      mb                           ; get table entry
      IDB ne >done                 ; zero entry marks end of table
      *divmod8
      bL0
      *num_to_asc
      L3b IDA rm IDB r1+ rL3     ; Store, and increment strint ptr
      L0b bL1                    ; Remainder becomes new A
      L2b IDB r1+ rL2.           ; Point to next divisor
      nj <rep

   done@

na C8h mr, na C9h mf
LEAVE RET

pow2@ 128 64 32 16 8 4 2 1 0
pow10@ 100 10 1 0
pow16@ 16 1 0

CLOSE

; ----------------------------- string_to_num -------------------------------

@string_to_num   ; Convert ASCII string in global to value in B
                 ; Number base in B

ENTER
na C8h rm, na C9h fm

   na 0 aL3                       ; Conversion result, initialise to zero
   na 2 AEB                        ; Check which base it is
   ni >is_bin
   na 10
   AEB
   ni >is_dec
   is_hex@ na >pow16 nj >join      ; Set base address of mult table
   is_dec@ na >pow10 nj >join
   is_bin@ na >pow2

   join@
      aL1                         ; Points to first multiplier in table
      na 80h                       ; Addr of first ASCII digit (left-most)

   seek@                           ; Get address of str termination byte
      aL2 IDA r1+ ra
      mb
      IDB
      ni <seek

   nf <string_to_num               ; set table addr high
   rep@
      L2b ma                      ; Points to current digit
      *asc_to_num                  ; Convert to number in A
      L1b mb
       *mul8
      L3b ADD rL3                ; Add to result
      L1b IDB r1+ rL1
      L2r r1- L2a rL2
      nb 80h
      AEB ne <rep

   done@ L3b

na C8h mr, na C9h mf
LEAVE RET

pow2@ 1 2 4 8 16 32 64 128 0
pow10@ 1 10 100 0
pow16@ 1 16 0

CLOSE

; ---------------------------------------------------------------------------

@End
   nb 2
   *string_to_num
   SSI
   ns 0
   stop@ nj <stop
  ; r8- This causes the program to fail (no SSI output!)
CLOSE




