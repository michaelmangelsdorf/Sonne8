
@Cold            ; Firmware for Sonne micro-controller rev. Daffodil
   na >End EXIT  ; Dec 2023 Michael Mangelsdorf <mim@ok-schalter.de>
CLOSE            ; See https://github.com/Dosflange/Sonne


; ----------------------------- mul8 ----------------------------------------

@mul8 ; Multiply A * B, result in A and B 
ENTER

   aL1p                     ; Initialize copy multiplicand (low order)
   bL0p                     ; Save multiplier

         nb C8h fm

   na 0 aL2p                ; Clear high-order
   na 8 aL3p                ; Initialize loop counter, 8 bits

loop@
   nb 1, aL1g AND,
   ne >skip
   aL0g, bL2g ADD           ; Add multiplier if low order lsb set
   fL2p

skip@
   nb 1, aL2g AND 
   fG0p                     ; Check if high order lsb set
   aL1g SRA, fL1p           ; Shift low order byte right
   aL2g SRA, fL2p           ; Shift high order byte right
   
   aG0g IDA, ne >done
   na 80h, bL1g IOR,
   fL1p

done@
   aL3g IDA f1- fL3p
   nt <loop
   aL1g
   bL2g

nb C8h mf
LEAVE RET
CLOSE

; ----------------------------- divmod8 -------------------------------------

@divmod8 ; Divide A by B, division result in A, remainder in B
ENTER

aL0p                       ; Dividend
bL1p                       ; Divisor

      nb C8h fm

na 1, aL2p                 ; Shift counter
na 0, aL3p                 ; Initialise quotient to zero

aL1g IDA ne >ELOOP         ; Skip if divisor zero

na 80h.                    ; MSB mask
MSB_SHIFT@                 ; Shift divisor left so that first 1 bit is at MSB
 bL1g                      ; Load divisor
 AND nt >DIVIDE            ; Skip when MSB set
 SLB fL1p                  ; Shift divisor left and update
 
 bL2g
 IDB f1+ fL2p              ; Increment shift counter and update
 nj <MSB_SHIFT

DIVIDE@
 bL3g SLB fL3p             ; Shift quotient left and update
 aL1g                      ; Divisor
 bL0g                      ; Dividend
 OCA f1+ fa                ; Negate divisor
 CYF                       ; Check borrow bit
 ne >REP

 ADD fa                    ; Accept subtraction
 aL0p                      ; Update dividend
 aL3g IDA f1+ fL3p         ; Increment quotient

REP@
 aL1g SRA fL1p             ; Shift divisor right for next subtraction
 aL2g IDA ne >ELOOP        ; Check if counter value zero
 IDA f1- fL2p
 nt <DIVIDE

ELOOP@ aL3g, bL0g

nb C8h mf
LEAVE RET
CLOSE

; ----------------------------- asc_to_num ----------------------------------

@asc_to_num ; Convert ASCII code in A to a number, result in A
ENTER
nb C8h fm

   aL1p                    ; Save A
   nb 48                   ; ASCII code of letter '0'
   OCB f1+ fb              ; Subtract it from A
   ADD fb fL2p

   na 10 AGB               ; Result must be smaller than 10, if decimal
   nt >done

   aL1g                    ; Not a decimal, try hexadecimal
   nb 10 ADD fa            ; Add 10 (value of hex letter 'A')
   nb 65                   ; ASCII code of letter 'A'
   OCB f1+ fb              ; Substract it from A
   ADD fL2p

   done@ aL2g

nb C8h mf
LEAVE RET
CLOSE

; ----------------------------- num_to_asc ----------------------------------

@num_to_asc ; Convert number in A to ASCII code
ENTER
nb C8h fm

   nb 10                   ; Number must be <10 for ASCII range '0' - '9'
   ALB nt >is_deci

   nb -10
   ADD fa                  ; Subtract 10
   nb 65                   ; ASCII code of letter 'A'
   ADD fa
   nj >done

   is_deci@
   nb 48                   ; ASCII code of letter '0'
   ADD fa
   done@

nb C8h mf
LEAVE RET
CLOSE

; ----------------------------- num_to_string -------------------------------

@num_to_string ; Convert value in A to ASCII string in global
               ; Number base in B

ENTER aL1p
na C8h fm, na C9h rm
   
   na 88h aL3p                     ; Result string base address
   na 2
   AEB
   nt >is_bin
   na 10
   AEB
   nt >is_dec
   is_hex@ na >pow16 nj >join      ; Set base address of divisor table
   is_dec@ na >pow10 nj >join
   is_bin@ na >pow2

   join@ aL2p
      nr <num_to_string            ; set table addr high
   rep@
      aL1g
      bL2g                         ; set table addr low
      mb                           ; get table entry
      IDB ne >done                 ; zero entry marks end of table
      *divmod8
      bL0p
      *num_to_asc
      bL3g IDA fm IDB f1+ fL3p     ; Store, and increment strint ptr
      bL0g bL1p                    ; Remainder becomes new A
      bL2g IDB f1+ fL2p.           ; Point to next divisor
      nj <rep

   done@

na C8h mf, na C9h mr
LEAVE RET

pow2@ 128 64 32 16 8 4 2 1 0
pow10@ 100 10 1 0
pow16@ 16 1 0

CLOSE

; ----------------------------- string_to_num -------------------------------

@string_to_num   ; Convert ASCII string in global to value in B
                 ; Number base in B

ENTER
na C8h fm, na C9h rm

   na 0 aL3p                       ; Conversion result, initialise to zero
   na 2 AEB                        ; Check which base it is
   nt >is_bin
   na 10
   AEB
   nt >is_dec
   is_hex@ na >pow16 nj >join      ; Set base address of mult table
   is_dec@ na >pow10 nj >join
   is_bin@ na >pow2

   join@
      aL1p                         ; Points to first multiplier in table
      na 80h                       ; Addr of first ASCII digit (left-most)

   seek@                           ; Get address of str termination byte
      aL2p IDA f1+ fa
      mb
      IDB
      nt <seek

   nr <string_to_num               ; set table addr high
   rep@
      bL2g ma                      ; Points to current digit
      *asc_to_num                  ; Convert to number in A
      bL1g mb
       *mul8
      bL3g ADD fL3p                ; Add to result
      bL1g IDB f1+ fL1p
      fL2g f1- aL2g fL2p
      nb 80h
      AEB ne <rep

   done@ bL3g

na C8h mf, na C9h mr
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
CLOSE




