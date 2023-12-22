
@Cold            ; Daffodil Firmware for Sonne micro-controller
   na >End CALL  ; Dec 2023 Michael Mangelsdorf <mim@ok-schalter.de>
CLOSE            ; See https://github.com/Dosflange/Sonne

; ----------------------------- mul8 ----------------------------------------

@mul8            ; Multiply A x B, result in A and B 
ENTER

   aL1p         ; Initialize copy multiplicand (low order)
   bL4p         ; Save multiplier
   na 0 aL2p    ; Clear high-order
   na 8 aL3p    ; Initialize loop counter, 8 bits

loop@
   nb 1, aL1g AND, ne >skip
   aL4g, bL2g ADD              ; Add multiplier if low order lsb set
   fb bL2p

skip@
   nb 1, aL2g AND 
   fb, bL5p                    ; Check if high order lsb set
   aL1g SRA, fb bL1p           ; Shift low order byte right
   aL2g SRA, fb bL2p           ; Shift high order byte right
   
   aL5g IDA, ne >done
   na 80h, bL1g IOR, fa aL1p

done@
   aL3g IDA f1- fa aL3p
   nt <loop
   aL1g
   bL2g
LEAVE RET
CLOSE

; ----------------------------- divmod8 -------------------------------------

@divmod8           ; Divide A by B, division result in A, remainder in B
ENTER

aL0p                       ; Dividend
bL1p                       ; Divisor
na 1, aL2p                 ; Shift counter
na 0, aL3p                 ; Initialise quotient to zero

aL1g IDA ne >ELOOP         ; Skip if divisor zero

na 80h.                    ; MSB mask
MSB_SHIFT@                 ; Shift divisor left so that first 1 bit is at MSB
 bL1g                      ; Load divisor
 AND nt >DIVIDE            ; Skip when MSB set
 SLB fb bL1p               ; Shift divisor left and update
 
 bL2g
 IDB f1+ fb, bL2p          ; Increment shift counter and update
 nj <MSB_SHIFT

DIVIDE@
 bL3g SLB fb, bL3p         ; Shift quotient left and update
 aL1g                      ; Divisor
 bL0g                      ; Dividend
 OCA f1+ fa                ; Negate divisor
 CYF                       ; Check borrow bit
 ne >REP

 ADD fa                    ; Accept subtraction
 aL0p                      ; Update dividend
 aL3g IDA f1+ fa, aL3p     ; Increment quotient

REP@
 aL1g SRA fa, aL1p         ; Shift divisor right for next subtraction
 aL2g IDA ne >ELOOP        ; Check if counter value zero
 IDA f1- fa, aL2p
 nt <DIVIDE

ELOOP@ aL3g, bL0g

LEAVE RET
CLOSE

; ----------------------------- asc_to_num ----------------------------------

@asc_to_num  ; Convert ASCII code in A to a number, result in A
ENTER

   aL1p           ; Save A
   nb 48          ; ASCII code of letter '0'
   OCB f1+ fb     ; Subtract it from A
   ADD fb bL2p

   na 10 AGB            ; Result must be smaller than 10, if decimal
   nt >done

   aL1g                ; Not a decimal, try hexadecimal
   nb 10 ADD fa        ; Add 10 (value of hex letter 'A')
   nb 65               ; ASCII code of letter 'A'
   OCB f1+ fb          ; Substract it from A
   ADD fb bL2p

   done@ aL2g

LEAVE RET
CLOSE

; ----------------------------- num_to_asc ----------------------------------

@num_to_asc  ; Convert number in A to ASCII code
ENTER

   nb 10               ; Number must be <10 for ASCII range '0' - '9'
   ALB nt >is_deci

   nb -10
   ADD fa              ; Subtract 10
   nb 65               ; ASCII code of letter 'A'
   ADD fa
   nj >done

   is_deci@
   nb 48               ; ASCII code of letter '0'
   ADD fa
   done@

LEAVE RET
CLOSE

; ----------------------------- num_to_string -------------------------------

@num_to_string
   ; Convert value in A to ASCII string in global
   ; Number base in B

ENTER 
   
   aL1p
   na 88h aL3p                 ; Result string base address
   na 2
   AEB
   nt >is_bin
   na 10
   AEB
   nt >is_dec
   is_hex@ na >pow16 nj >join  ; Set base address of divisor table
   is_dec@ na >pow10 nj >join
   is_bin@ na >pow2

   join@ aL2p
      nr <num_to_string        ; set table addr high
   rep@
      aL1g
      bL2g                     ; set table addr low
      mb                       ; get table entry
      IDB ne >done             ; zero entry marks end of table
      *divmod8
      bL4p
      *num_to_asc
      bL3g IDA fm IDB f1+ fb bL3p  ; Store, and increment strint ptr
      bL4g bL1p                    ; Remainder becomes new A
      bL2g IDB f1+ fb bL2p         ; Point to next divisor
      nj <rep

   done@

LEAVE RET

pow2@ 128 64 32 16 8 4 2 1 0
pow10@ 100 10 1 0
pow16@ 16 1 0

CLOSE

; ----------------------------- string_to_num -------------------------------

@string_to_num
   ; Convert ASCII string in global to value in A
   ; Number base in B

ENTER    ; To Do

   done@

LEAVE RET

pow2@ 128 64 32 16 8 4 2 1 0
pow10@ 100 10 1 0
pow16@ 16 1 0

CLOSE

; ---------------------------------------------------------------------------

@End
   na 76, nb 2
    *num_to_string
   ns 0
   stop@ nj <stop
CLOSE




