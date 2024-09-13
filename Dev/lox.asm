

P[COLD]0
           nc Interpret  (LOX Bootstrap Firmware for Myth micro-controller)
                         (Author: mim@ok-schalter.de - Michael/Dosflange@github)
           END           (Lox has copied its null-separated command line arguments)
           00h           (into 7F80h-7FEFh. Application specific opcode END exits LOX.)

P[Interpret]20h

           OWN        (********** Look up a word in the dictionary *******************)
           i6
           nd LOXBASE
           no LOXBASE.ARG
           mo
           nc VSrch
           nt >InterpSucc
           nr 1        (Look-up failed)
           nc PrMsg
           6i
           RET

O[InterpSucc]
           o0         (Save target offset)
           no 2         (Compare to type 2 - CMD)
           REO        (Target type still in R)
           nf >InterpRFail
           0i         (Restore target offset)
           COR        (Target page still in G)

O[InterpRFail]
           nr 2
           nc PrMsg
           6i
           RET
O[NextArg]
           OWN        (********** Advance LOX ARG PTR to next string ****************)
           i6
           nd LOXBASE
           no LOXBASE.ARG
           mo
           nc SkipToNULL (DO points to char following NULL)
           nd LOXBASE      (Ignore updated page index in D)
           IDO
           no LOXBASE.ARG
           rm         (Store updated offset in ARG variable)
           6i
           RET

P[Mul8]
           OWN        (Multiplies R by O result in R and O)
           i6
           o1         (Initialise copy of multiplicand - low order)
           r0         (Save multiplier)
           nr 00h    (Clear high-order result)
           r2
           ni         (Initilise loop counter, 8 bits)
           07h
O[Mul8Loop]
           no 01h        (Bit mask for bit 0)
           1r
           AND        (Check if multiplicand bit 0 set)
           nf >Mul8Skip        (Skip if not)
           0r
           2o
           ADD        (Add multiplier to high order result)
           r2
O[Mul8Skip]
           no 01h        (Bit mask for bit 0)
           2r
           AND
           r3         (Flag whether high order LSB is set)
           1r
           SRR        (Shift low-order result right)
           r1
           2r
           SRR        (Shift high-order result right)
           r2
           3r         (Test whether high-order bit set)
           nf >Mul8Done
           nr 80h        (Bit mask for MSB)
           1o
           IOR        (Set MSB if LSB of high order was shifted down into low-order)
           r1
O[Mul8Done]
           nw <Mul8Loop
           1o         (Result low-order)
           2r         (Result high-order)
           6i         (Restore return address)
           RET

P[DivMod8]
           OWN        (Divide R by G, return quotient in R, remainder in G)
           i6
           r0         (Save dividend)
           o1         (Save divisor)
           ni 00h        (Initialise shift counter to number of positions)
                        (of first 1-bit)
           nr 00h        (Initialise quotient to zero)
           r3
           1r         (Skip if divisor is zero)
           nf >DivMod8Quit
           nr         (Set MSB mask)
           80h
           r5

O[DivMod8Sh]

           5r         (Shift divisor left until first 1 bit at MSB position)
           1o
           AND        (Compare to mask)
           nt >DivMod8Div (Exit shift loop once MSB set)
           SLO        (Shift divisor)
           r1         (Store result)
           ir         (Increment shift counter)
           P1
           ri
           nj <DivMod8Sh

O[DivMod8Div]

           3o         (Shift quotient left)
           SLO
           r3         (Store shifted quotient for latter)
           1r         (Negate divisor)
           OCR        (Do this by getting the ones'complement)
           P1         (and adding 1 to it)
           r4         (Save this)
           0o         (Get the dividend and see if adding the negated divisor)
           CAR        (produces a borrow bit)
           nf         (If not, don't accept the subtraction)
           >DivMod8Rep
           4r         (Accept subtraction result)
           ADD        (Dividend still in O)
           r0         (Store new dividend)
           3r         (Increment quotient)
           P1
           r3

O[DivMod8Rep]

           1r
           SRR        (Shift divisor right for next subtraction)
           r1
           nw         (Decrement counter)
           <DivMod8Div (Branch back if not zero)

O[DivMod8Quit]

           3r
           0o
           6i
           RET

P[SkipToNULL]

           OWN        (********** Advance string pointer to next NULL char  **********)
           i6

O[SkipNFind0] mr         (DO points to char)

           nf         (If char zero, done)
           >SkipNAt0
           na         (Else increment DO and check again)
           1
           nj <SkipNFind0

O[SkipNAt0]
           6i
           RET

P[VSrch]
           OWN        (VOCAB Look-UP, search string address in D/O)
           i6
           d0         (Search zero terminated string in this page)
           o1         (at this offset)
           d4         (Create working copies)
           o5
           nd         (Set Data Pointer to first entry)
           BASEVOCAB
           no
           00h
           d2         (Current dict base ptr)
           o3         (and offs)

O[VSrchNxtCh]

           4d         (Compare current char in dict to char in search str)
           5o
           mr         (Char in sstr)
           2d
           3o
           mo         (Char in dict)
           REO
           nf >VSrchMismat
           IDO
           nf         (Matching chars, success if both NUL)
           >VSrchFound
           4d
           5o
           na         (Advance sstring ptr)
           1
           d4
           o5
           2d
           3o
           na 1        (Advance dict ptr)
           d2
           o3
           nj <VSrchNxtCh (Branch back, check next character)

O[VSrchMismat]

           0r         (Reset sstring ptr)
           r4
           1r
           r5
           2d         (Find next NULL byte)
           3o
           nc SkipToNULL
           d2
           o3
           na 4        (Skip NUL char and Data Bytes - Type+Page+Offs)
           d2
           o3
           mr         (Check if first byte of next data entry is NUL - End of list)

           nf >VSrchFail
           
           nj <VSrchNxtCh

O[VSrchFound]

           2d
           3o
           na         (Advance vocab ptr to type byte)
           1
           mr         (Type)
           r2
           na
           1
           mr         (Page)
           r3
           na
           1
           mo         (Offs)
           3d
           2r
           6i
           RET

O[VSrchFail]

           CLR        (Not found if result type = 0)
           6i
           RET

P[PrMsg]
           OWN
           i6
           r0
           nd PrMsg
           d1
           no >PrMsg0
           o2

O[PrMsgNxtCh]

           1d
           2o
           mr
           0o         (Compare message IDs)
           REO
           nt
           >PrMsgFound

O[PrMsgSkip]

           2o
           na         (Increment to next msg char)
           1
           o2
           mr
           nf
           >PrMsgChk0
           nj         (This wasn't a NULL, keep incrementing)
           <PrMsgSkip

O[PrMsgChk0]

           na 1       (Skip over the NULL char)
           o2
           mr         (Check double NULL)
           nt <PrMsgNxtCh (If it was not NULL, check next entry - then it's an ID)
           nj >PrMsgQuit  (Lookup failed)

O[PrMsgFound]

           nd LOXBASE
           no LOXBASE.POS
           mo         (Get current printing POS)
           o3

O[PrMsgCpy]

           1d         (Page of src char ptr)
           2o         (Offset of src char ptr)
           na 1        (Advance to next char)
           o2
           mr         (Char to print)
           nf         (Finished if NULL terminator)
           >PrMsgDone
           nd
           LOXBASE
           3o
           rm         (Store character in output buffer at POS)
           na
           1
           o3         (Advance printing pos)
           nj <PrMsgCpy

O[PrMsgDone]

           nd         (Update POS)
           LOXBASE
           no LOXBASE.POS
           3r
           rm

O[PrMsgQuit]

           6i
           RET

O[PrMsg0]
            0
           'R'
           'e'
           'a'
           'd'
           'y'
           'NUL'
           1
           'N'
           'o'
           't'
           'SP'
           'f'
           'o'
           'u'
           'n'
           'd'
           'NUL'
           2
           'N'
           'o'
           't'
           'SP'
           'a'
           'SP'
           'c'
           'm'
           'd'
           'NUL'
           3
           'C'
           'r'
           'a'
           't'
           'e'
           'r'
           'NUL'

P[ready]

           CLR
           nr
           3
           nc PrMsg
           6i
           RET

P[NextArg]

           OWN        (********** Advance LOX ARG PTR to next string ****************)
           i6
           nd LOXBASE
           no LOXBASE.ARG
           mo
           nc SkipToNULL  (DO points to char following NULL)
           nd         (Ignore updated page index in D)
           LOXBASE
           IDO
           no LOXBASE.ARG
           rm         (Store updated offset in ARG variable)
           6i
           RET

P[LOXBASE]
           'H'
           'e'
           'l'
           'l'
           'SP'
           'W'
           'o'
           'r'
           'l'
           'd'
           00h

O[SYSVARS]F7h

O[ARG]        80h        (Offset in LOXBASE of current cmd line argument str)
O[POS]        00h        (Current position in output text buffer - offset in LOXBASE)
O[VTP]        VTOPPAGE   (Hardcoded! Page index of final entry in symtab )
O[VTO]        VTOPOFFSET (Hardcoded! Check lox.c source-code. Offset of final entry)
O[DESTP]      FFh
O[DESTO]      00h
O[SRCP]       FFh
O[SRCO]       00h
O[ECODE]      00h

P[BASEVOCAB]40h

    "0", 'NUL', 81h, 0, 00h
    "1", 'NUL', 81h, 0, 01h
    "2", 'NUL', 81h, 0, 02h
    "3", 'NUL', 81h, 0, 03h
    "4", 'NUL', 81h, 0, 04h
    "5", 'NUL', 81h, 0, 05h
    "6", 'NUL', 81h, 0, 06h
    "7", 'NUL', 81h, 0, 07h
    "8", 'NUL', 81h, 0, 08h
    "9", 'NUL', 81h, 0, 09h