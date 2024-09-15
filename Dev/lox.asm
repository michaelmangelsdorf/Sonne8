
(LOX Bootstrap Firmware for Myth micro-controller)
(Author: mim@ok-schalter.de - Michael/Dosflange@github)

;******* ********************************************************************
P[COLD]0
;******* ********************************************************************

 (Lox has copied its null-separated command line arguments into 7F80h-7FEFh.)

        nc Interpret
        END (Application specific opcode END exits LOX.)


;************** *************************************************************
P[Interpret]20h (Look up a word in the dictionary)
;************** *************************************************************

         OWN, i6
         nd LOXBASE     (Page where the search string is located)
         no LOXBASE.ARG (Offset of variable with offset is stored)
         mo
         nc VSrch    (Load offset of search string and look it up)
         nt >InterpSucc

         (Look-up failed)
         nr 1, nc PrMsg   (Prints 'not found')
         6i, RET

      O[InterpSucc]

         (Look-up succeeded, check correct type of entry)

         o0    (Save target offset)
         no 2  (Compare to type 2 - CMD)
         REO   (Target type still in R)

         nf >InterpRFail (Skip if type byte didn't match)

         (Jump to CMD)
         0i    (Restore target offset)
         COR   (Target page still in D)

      O[InterpRFail]

         nr 2, nc PrMsg  (Prints 'not a command')
         6i, RET

;*****************
        O[NextArg]
;*****************

         OWN i6          (Advance LOX ARG PTR to next string)
         nd LOXBASE
         no LOXBASE.ARG
         mo
         nc SkipToNULL   (DO points to char following NULL)
         nd LOXBASE      (Ignore updated page index in D)
         IDO
         no LOXBASE.ARG
         rm              (Store updated offset in ARG variable)
         6i RET


;****** *********************************************************************
P[Mul8]+  (Multiplies R by O result in R and O)
;****** *********************************************************************

         OWN i6        (Save return pointer in L7/L6)
         o1            (Multiplicand into L1, turns into low order result)
         r0            (Multiplier into L0)
         CLR r2        (Clear high-order result, copy to L2)
         ni 07h        (Initilise loop counter, 8 bits)

      O[Mul8Loop]
      
         no 01h        (Bit mask for LSB)
         1r AND        (Check if multiplicand has LSB set)
         nf >Mul8Skip  (Skip if not)
         0r 2o ADD r2  (Add multiplier to high order result)

      O[Mul8Skip]

         no 01h        (Bit mask for LSB)
         2r AND r3     (Flag whether high order LSB is set)
         1r SRR r1      (Shift low-order result right)
         2r SRR r2       (Shift high-order result right)

         3r nf >Mul8Done  (Check flag from earlier - HO LSB set?)
         nr 80h          (Bit mask for MSB)
         1o IOR r1     (Handle shift-result carry bit into MSB)

      O[Mul8Done]

         nw <Mul8Loop
         1o            (Result low-order)
         2r            (Result high-order)
         6i RET        (Restore return pointer from L7/L6)


;********* ******************************************************************
P[DivMod8]+  (Divide R by G, return quotient in R, remainder in G)
;********* ******************************************************************

         OWN i6
         r0         (Save dividend)
         o1         (Save divisor)
         ni 00h     (Initialise shift counter to number of positions
                     of first 1-bit)

         nr 00h r3          (Initialise quotient to zero)
         1r nf >DivMod8Quit (Skip if divisor is zero)
      
         nr 80h r5        (Set MSB mask)

      O[DivMod8Sh]

         5r      (Shift divisor left until first 1 bit at MSB position)
         1o AND          (Compare to mask)
         nt >DivMod8Div  (Exit shift loop once MSB set)
         SLO             (Shift divisor)
         r1              (Store result)
         ir P1 ri        (Increment shift counter)
         nj <DivMod8Sh

      O[DivMod8Div]

         3o SLO     (Shift quotient left)
         r3         (Store shifted quotient for latter)
         1r OCR P1  (Negate divisor)
         r4         (Save this)
         0o CAR     (Get dividend and see if adding the negated divisor
                     produces a borrow bit)
         
         nf >DivMod8Rep  (If not, don't accept the subtraction)
         
         4r ADD     (Accept subtraction result to dividend in O)
         r0         (Store new dividend)
         3r P1 r3   (Increment quotient)

      O[DivMod8Rep]

         1r SRR r1 (Shift divisor right for next subtraction)
         
         nw <DivMod8Div (Branch back if not zero, decrement I counter)

      O[DivMod8Quit]

         3r 0o  (Save quotient and remainder)
         6i RET


;************ ***************************************************************
P[SkipToNULL]+
;************ ***************************************************************

         OWN i6  (Advance string pointer D/O to next NULL char)

      O[SkipNFind0]
      
         mr              (D:O points to char, load it into R)
         nf >SkipNAt0    (If char zero, done)
         na 1            (Else increment D/O and check again)
         nj <SkipNFind0

      O[SkipNAt0] 6i RET


;******* ********************************************************************
P[VSrch]+  (Look-up a zero terminated string at pointer D/O)
;******* ********************************************************************

         OWN, i6
         d0 o1               (String pointer)
         d4 o5               (Create a copy of it)
         nd BASEVOCAB, no 0  (Pointer to first entry in look-up table)
         d2 o3               (Create a copy of it)
 
      O[VSrchNxtCh]

         4d 5o, mr           (Current character in string)
         2d 3o, mo           (Current character in table entry)
         
         REO nf >VSrchMismat (Compare the characters, skip if not equal)
         IDO nf >VSrchFound  (Characters match, we're done if both are NULL)
         
         4d 5o, na 1, d4 o5  (Advance string ptr)
         2d 3o, na 1, d2 o3  (Advance table entry ptr)

         nj <VSrchNxtCh      (Branch back, check next character)

      O[VSrchMismat]

         0r r4, 1r r5    (Reset string ptr for another round of matching)

         2d 3o           (Seek to the NULL terminator of the current entry)
         nc SkipToNULL

         na 4, d2 o3     (Skip the current entry which didn't match)
         mr              (Check if there's another entry - mustn't be NULL)
         
         nf >VSrchFail   (None of the entries matched)
         nj <VSrchNxtCh  (Else match this new entry against the string)

      O[VSrchFound]

         2d 3o, na 1  (Advance table pointer to type byte)
         mr r2, na 1  (Load type data)
         mr r3, na 1  (load page index data)
         mo           (load offset data)
         3d 2r        (Return page index in D, type in R, offset in O)
         6i RET

      O[VSrchFail]

         CLR  (Not found: type result = 0)
         6i RET


;******* ********************************************************************
P[PrMsg]+  (Receives a message index in R, print the corresponding message)
;******* ********************************************************************

         OWN i6
         r0, nd PrMsg d1  (Pointer to string table in L1/L2)
         no >PrMsg0 o2

      O[PrMsgNxtCh]

         1d 2o mr   (Load message ID of table entry)
         0o         (Compare with requested message ID)
         REO
         nt >PrMsgFound   (If both numbers match)

      O[PrMsgSkip]  (Skip the current message string, not the one we want)

           2o na 1 o2     (Increment to next msg char and see if NULL)
           mr
           nf >PrMsgAt0   (Yes, it's the end of this message string)
           nj <PrMsgSkip  (This wasn't a NULL yet, keep looking)

      O[PrMsgAt0]

           na 1 o2        (Skip over the NULL character)
           mr             (load next character)
           nt <PrMsgNxtCh (If it wasn't NULL, it's an ID: check next entry)
           nj >PrMsgQuit  (It was the last message string, lookup failed)

      O[PrMsgFound]

           nd LOXBASE
           no LOXBASE.POS
           mo o3 (Load current printing position)

      O[PrMsgCpy]

           1d 2o    (Pointer to character)
           mr       (Put character into R for printing)

           nf >PrMsgDone  (It's the NULL byte, stop printing)
           na 1 o2        (Advance to next char) 

           nd LOXBASE 3o, rm  (Store character)
           na 1 o3            (Increment and save the printing position)
           nj <PrMsgCpy       (Print the next character)

      O[PrMsgDone]

           nd LOXBASE (Update POS variable)
           no LOXBASE.POS
           3r rm

      O[PrMsgQuit] 6i RET

;**************
      O[PrMsg0]
;**************

           0, "Ready",     'NUL'
           1, "Not found", 'NUL'
           2, "Not a cmd", 'NUL'
           3, "Crater",    'NUL'


;******* ********************************************************************
P[ready]+
;******* ********************************************************************

           CLR
           nr 3
           nc PrMsg
           6i RET

;********* ******************************************************************
P[NextArg]+
;********* ******************************************************************

           OWN i6  (Advance LOX ARG PTR to next string)

           nd LOXBASE
           no LOXBASE.ARG
           mo

           nc SkipToNULL  (DO points to char following NULL)
           nd LOXBASE     (Ignore updated page index in D)
           
           IDO, no LOXBASE.ARG
           rm  (Store updated offset in ARG variable)
           
           6i RET


;************** *************************************************************
P[BASEVOCAB]40h
;************** *************************************************************

  (Predefined string literals
     This is a linear search list
     in the format:
     Z-Terminated string
     Type byte: 81h
     Unused: 0
     Numeric value)

    (Positive Decimal Numbers)

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
    "10", 'NUL', 81h, 0, 0Ah
    "11", 'NUL', 81h, 0, 0Bh
    "12", 'NUL', 81h, 0, 0Ch
    "13", 'NUL', 81h, 0, 0Dh
    "14", 'NUL', 81h, 0, 0Eh
    "15", 'NUL', 81h, 0, 0Fh
    "16", 'NUL', 81h, 0, 10h
    "17", 'NUL', 81h, 0, 11h
    "18", 'NUL', 81h, 0, 12h
    "19", 'NUL', 81h, 0, 13h
    "20", 'NUL', 81h, 0, 14h
    "21", 'NUL', 81h, 0, 15h
    "22", 'NUL', 81h, 0, 16h
    "23", 'NUL', 81h, 0, 17h
    "24", 'NUL', 81h, 0, 18h
    "25", 'NUL', 81h, 0, 19h
    "26", 'NUL', 81h, 0, 1Ah
    "27", 'NUL', 81h, 0, 1Bh
    "28", 'NUL', 81h, 0, 1Ch
    "29", 'NUL', 81h, 0, 1Dh
    "30", 'NUL', 81h, 0, 1Eh
    "31", 'NUL', 81h, 0, 1Fh
    "32", 'NUL', 81h, 0, 20h
    "33", 'NUL', 81h, 0, 21h
    "34", 'NUL', 81h, 0, 22h
    "35", 'NUL', 81h, 0, 23h
    "36", 'NUL', 81h, 0, 24h
    "37", 'NUL', 81h, 0, 25h
    "38", 'NUL', 81h, 0, 26h
    "39", 'NUL', 81h, 0, 27h
    "40", 'NUL', 81h, 0, 28h
    "41", 'NUL', 81h, 0, 29h
    "42", 'NUL', 81h, 0, 2Ah
    "43", 'NUL', 81h, 0, 2Bh
    "44", 'NUL', 81h, 0, 2Ch
    "45", 'NUL', 81h, 0, 2Dh
    "46", 'NUL', 81h, 0, 2Eh
    "47", 'NUL', 81h, 0, 2Fh
    "48", 'NUL', 81h, 0, 30h
    "49", 'NUL', 81h, 0, 31h
    "50", 'NUL', 81h, 0, 32h
    "51", 'NUL', 81h, 0, 33h
    "52", 'NUL', 81h, 0, 34h
    "53", 'NUL', 81h, 0, 35h
    "54", 'NUL', 81h, 0, 36h
    "55", 'NUL', 81h, 0, 37h
    "56", 'NUL', 81h, 0, 38h
    "57", 'NUL', 81h, 0, 39h
    "58", 'NUL', 81h, 0, 3Ah
    "59", 'NUL', 81h, 0, 3Bh
    "60", 'NUL', 81h, 0, 3Ch
    "61", 'NUL', 81h, 0, 3Dh
    "62", 'NUL', 81h, 0, 3Eh
    "63", 'NUL', 81h, 0, 3Fh
    "64", 'NUL', 81h, 0, 40h
    "65", 'NUL', 81h, 0, 41h
    "66", 'NUL', 81h, 0, 42h
    "67", 'NUL', 81h, 0, 43h
    "68", 'NUL', 81h, 0, 44h
    "69", 'NUL', 81h, 0, 45h
    "70", 'NUL', 81h, 0, 46h
    "71", 'NUL', 81h, 0, 47h
    "72", 'NUL', 81h, 0, 48h
    "73", 'NUL', 81h, 0, 49h
    "74", 'NUL', 81h, 0, 4Ah
    "75", 'NUL', 81h, 0, 4Bh
    "76", 'NUL', 81h, 0, 4Ch
    "77", 'NUL', 81h, 0, 4Dh
    "78", 'NUL', 81h, 0, 4Eh
    "79", 'NUL', 81h, 0, 4Fh
    "80", 'NUL', 81h, 0, 50h
    "81", 'NUL', 81h, 0, 51h
    "82", 'NUL', 81h, 0, 52h
    "83", 'NUL', 81h, 0, 53h
    "84", 'NUL', 81h, 0, 54h
    "85", 'NUL', 81h, 0, 55h
    "86", 'NUL', 81h, 0, 56h
    "87", 'NUL', 81h, 0, 57h
    "88", 'NUL', 81h, 0, 58h
    "89", 'NUL', 81h, 0, 59h
    "90", 'NUL', 81h, 0, 5Ah
    "91", 'NUL', 81h, 0, 5Bh
    "92", 'NUL', 81h, 0, 5Ch
    "93", 'NUL', 81h, 0, 5Dh
    "94", 'NUL', 81h, 0, 5Eh
    "95", 'NUL', 81h, 0, 5Fh
    "96", 'NUL', 81h, 0, 60h
    "97", 'NUL', 81h, 0, 61h
    "98", 'NUL', 81h, 0, 62h
    "99", 'NUL', 81h, 0, 63h
    "100", 'NUL', 81h, 0, 64h
    "101", 'NUL', 81h, 0, 65h
    "102", 'NUL', 81h, 0, 66h
    "103", 'NUL', 81h, 0, 67h
    "104", 'NUL', 81h, 0, 68h
    "105", 'NUL', 81h, 0, 69h
    "106", 'NUL', 81h, 0, 6Ah
    "107", 'NUL', 81h, 0, 6Bh
    "108", 'NUL', 81h, 0, 6Ch
    "109", 'NUL', 81h, 0, 6Dh
    "110", 'NUL', 81h, 0, 6Eh
    "111", 'NUL', 81h, 0, 6Fh
    "112", 'NUL', 81h, 0, 70h
    "113", 'NUL', 81h, 0, 71h
    "114", 'NUL', 81h, 0, 72h
    "115", 'NUL', 81h, 0, 73h
    "116", 'NUL', 81h, 0, 74h
    "117", 'NUL', 81h, 0, 75h
    "118", 'NUL', 81h, 0, 76h
    "119", 'NUL', 81h, 0, 77h
    "120", 'NUL', 81h, 0, 78h
    "121", 'NUL', 81h, 0, 79h
    "122", 'NUL', 81h, 0, 7Ah
    "123", 'NUL', 81h, 0, 7Bh
    "124", 'NUL', 81h, 0, 7Ch
    "125", 'NUL', 81h, 0, 7Dh
    "126", 'NUL', 81h, 0, 7Eh
    "127", 'NUL', 81h, 0, 7Fh
    "128", 'NUL', 81h, 0, 80h
    "129", 'NUL', 81h, 0, 81h
    "130", 'NUL', 81h, 0, 82h
    "131", 'NUL', 81h, 0, 83h
    "132", 'NUL', 81h, 0, 84h
    "133", 'NUL', 81h, 0, 85h
    "134", 'NUL', 81h, 0, 86h
    "135", 'NUL', 81h, 0, 87h
    "136", 'NUL', 81h, 0, 88h
    "137", 'NUL', 81h, 0, 89h
    "138", 'NUL', 81h, 0, 8Ah
    "139", 'NUL', 81h, 0, 8Bh
    "140", 'NUL', 81h, 0, 8Ch
    "141", 'NUL', 81h, 0, 8Dh
    "142", 'NUL', 81h, 0, 8Eh
    "143", 'NUL', 81h, 0, 8Fh
    "144", 'NUL', 81h, 0, 90h
    "145", 'NUL', 81h, 0, 91h
    "146", 'NUL', 81h, 0, 92h
    "147", 'NUL', 81h, 0, 93h
    "148", 'NUL', 81h, 0, 94h
    "149", 'NUL', 81h, 0, 95h
    "150", 'NUL', 81h, 0, 96h
    "151", 'NUL', 81h, 0, 97h
    "152", 'NUL', 81h, 0, 98h
    "153", 'NUL', 81h, 0, 99h
    "154", 'NUL', 81h, 0, 9Ah
    "155", 'NUL', 81h, 0, 9Bh
    "156", 'NUL', 81h, 0, 9Ch
    "157", 'NUL', 81h, 0, 9Dh
    "158", 'NUL', 81h, 0, 9Eh
    "159", 'NUL', 81h, 0, 9Fh
    "160", 'NUL', 81h, 0, A0h
    "161", 'NUL', 81h, 0, A1h
    "162", 'NUL', 81h, 0, A2h
    "163", 'NUL', 81h, 0, A3h
    "164", 'NUL', 81h, 0, A4h
    "165", 'NUL', 81h, 0, A5h
    "166", 'NUL', 81h, 0, A6h
    "167", 'NUL', 81h, 0, A7h
    "168", 'NUL', 81h, 0, A8h
    "169", 'NUL', 81h, 0, A9h
    "170", 'NUL', 81h, 0, AAh
    "171", 'NUL', 81h, 0, ABh
    "172", 'NUL', 81h, 0, ACh
    "173", 'NUL', 81h, 0, ADh
    "174", 'NUL', 81h, 0, AEh
    "175", 'NUL', 81h, 0, AFh
    "176", 'NUL', 81h, 0, B0h
    "177", 'NUL', 81h, 0, B1h
    "178", 'NUL', 81h, 0, B2h
    "179", 'NUL', 81h, 0, B3h
    "180", 'NUL', 81h, 0, B4h
    "181", 'NUL', 81h, 0, B5h
    "182", 'NUL', 81h, 0, B6h
    "183", 'NUL', 81h, 0, B7h
    "184", 'NUL', 81h, 0, B8h
    "185", 'NUL', 81h, 0, B9h
    "186", 'NUL', 81h, 0, BAh
    "187", 'NUL', 81h, 0, BBh
    "188", 'NUL', 81h, 0, BCh
    "189", 'NUL', 81h, 0, BDh
    "190", 'NUL', 81h, 0, BEh
    "191", 'NUL', 81h, 0, BFh
    "192", 'NUL', 81h, 0, C0h
    "193", 'NUL', 81h, 0, C1h
    "194", 'NUL', 81h, 0, C2h
    "195", 'NUL', 81h, 0, C3h
    "196", 'NUL', 81h, 0, C4h
    "197", 'NUL', 81h, 0, C5h
    "198", 'NUL', 81h, 0, C6h
    "199", 'NUL', 81h, 0, C7h
    "200", 'NUL', 81h, 0, C8h
    "201", 'NUL', 81h, 0, C9h
    "202", 'NUL', 81h, 0, CAh
    "203", 'NUL', 81h, 0, CBh
    "204", 'NUL', 81h, 0, CCh
    "205", 'NUL', 81h, 0, CDh
    "206", 'NUL', 81h, 0, CEh
    "207", 'NUL', 81h, 0, CFh
    "208", 'NUL', 81h, 0, D0h
    "209", 'NUL', 81h, 0, D1h
    "210", 'NUL', 81h, 0, D2h
    "211", 'NUL', 81h, 0, D3h
    "212", 'NUL', 81h, 0, D4h
    "213", 'NUL', 81h, 0, D5h
    "214", 'NUL', 81h, 0, D6h
    "215", 'NUL', 81h, 0, D7h
    "216", 'NUL', 81h, 0, D8h
    "217", 'NUL', 81h, 0, D9h
    "218", 'NUL', 81h, 0, DAh
    "219", 'NUL', 81h, 0, DBh
    "220", 'NUL', 81h, 0, DCh
    "221", 'NUL', 81h, 0, DDh
    "222", 'NUL', 81h, 0, DEh
    "223", 'NUL', 81h, 0, DFh
    "224", 'NUL', 81h, 0, E0h
    "225", 'NUL', 81h, 0, E1h
    "226", 'NUL', 81h, 0, E2h
    "227", 'NUL', 81h, 0, E3h
    "228", 'NUL', 81h, 0, E4h
    "229", 'NUL', 81h, 0, E5h
    "230", 'NUL', 81h, 0, E6h
    "231", 'NUL', 81h, 0, E7h
    "232", 'NUL', 81h, 0, E8h
    "233", 'NUL', 81h, 0, E9h
    "234", 'NUL', 81h, 0, EAh
    "235", 'NUL', 81h, 0, EBh
    "236", 'NUL', 81h, 0, ECh
    "237", 'NUL', 81h, 0, EDh
    "238", 'NUL', 81h, 0, EEh
    "239", 'NUL', 81h, 0, EFh
    "240", 'NUL', 81h, 0, F0h
    "241", 'NUL', 81h, 0, F1h
    "242", 'NUL', 81h, 0, F2h
    "243", 'NUL', 81h, 0, F3h
    "244", 'NUL', 81h, 0, F4h
    "245", 'NUL', 81h, 0, F5h
    "246", 'NUL', 81h, 0, F6h
    "247", 'NUL', 81h, 0, F7h
    "248", 'NUL', 81h, 0, F8h
    "249", 'NUL', 81h, 0, F9h
    "250", 'NUL', 81h, 0, FAh
    "251", 'NUL', 81h, 0, FBh
    "252", 'NUL', 81h, 0, FCh
    "253", 'NUL', 81h, 0, FDh
    "254", 'NUL', 81h, 0, FEh
    "255", 'NUL', 81h, 0, FFh

     (Negative Decimal Numbers)

    "-1", 'NUL', 81h, 0, FFh
    "-2", 'NUL', 81h, 0, FEh
    "-3", 'NUL', 81h, 0, FDh
    "-4", 'NUL', 81h, 0, FCh
    "-5", 'NUL', 81h, 0, FBh
    "-6", 'NUL', 81h, 0, FAh
    "-7", 'NUL', 81h, 0, F9h
    "-8", 'NUL', 81h, 0, F8h
    "-9", 'NUL', 81h, 0, F7h
    "-10", 'NUL', 81h, 0, F6h
    "-11", 'NUL', 81h, 0, F5h
    "-12", 'NUL', 81h, 0, F4h
    "-13", 'NUL', 81h, 0, F3h
    "-14", 'NUL', 81h, 0, F2h
    "-15", 'NUL', 81h, 0, F1h
    "-16", 'NUL', 81h, 0, F0h
    "-17", 'NUL', 81h, 0, EFh
    "-18", 'NUL', 81h, 0, EEh
    "-19", 'NUL', 81h, 0, EDh
    "-20", 'NUL', 81h, 0, ECh
    "-21", 'NUL', 81h, 0, EBh
    "-22", 'NUL', 81h, 0, EAh
    "-23", 'NUL', 81h, 0, E9h
    "-24", 'NUL', 81h, 0, E8h
    "-25", 'NUL', 81h, 0, E7h
    "-26", 'NUL', 81h, 0, E6h
    "-27", 'NUL', 81h, 0, E5h
    "-28", 'NUL', 81h, 0, E4h
    "-29", 'NUL', 81h, 0, E3h
    "-30", 'NUL', 81h, 0, E2h
    "-31", 'NUL', 81h, 0, E1h
    "-32", 'NUL', 81h, 0, E0h
    "-33", 'NUL', 81h, 0, DFh
    "-34", 'NUL', 81h, 0, DEh
    "-35", 'NUL', 81h, 0, DDh
    "-36", 'NUL', 81h, 0, DCh
    "-37", 'NUL', 81h, 0, DBh
    "-38", 'NUL', 81h, 0, DAh
    "-39", 'NUL', 81h, 0, D9h
    "-40", 'NUL', 81h, 0, D8h
    "-41", 'NUL', 81h, 0, D7h
    "-42", 'NUL', 81h, 0, D6h
    "-43", 'NUL', 81h, 0, D5h
    "-44", 'NUL', 81h, 0, D4h
    "-45", 'NUL', 81h, 0, D3h
    "-46", 'NUL', 81h, 0, D2h
    "-47", 'NUL', 81h, 0, D1h
    "-48", 'NUL', 81h, 0, D0h
    "-49", 'NUL', 81h, 0, CFh
    "-50", 'NUL', 81h, 0, CEh
    "-51", 'NUL', 81h, 0, CDh
    "-52", 'NUL', 81h, 0, CCh
    "-53", 'NUL', 81h, 0, CBh
    "-54", 'NUL', 81h, 0, CAh
    "-55", 'NUL', 81h, 0, C9h
    "-56", 'NUL', 81h, 0, C8h
    "-57", 'NUL', 81h, 0, C7h
    "-58", 'NUL', 81h, 0, C6h
    "-59", 'NUL', 81h, 0, C5h
    "-60", 'NUL', 81h, 0, C4h
    "-61", 'NUL', 81h, 0, C3h
    "-62", 'NUL', 81h, 0, C2h
    "-63", 'NUL', 81h, 0, C1h
    "-64", 'NUL', 81h, 0, C0h
    "-65", 'NUL', 81h, 0, BFh
    "-66", 'NUL', 81h, 0, BEh
    "-67", 'NUL', 81h, 0, BDh
    "-68", 'NUL', 81h, 0, BCh
    "-69", 'NUL', 81h, 0, BBh
    "-70", 'NUL', 81h, 0, BAh
    "-71", 'NUL', 81h, 0, B9h
    "-72", 'NUL', 81h, 0, B8h
    "-73", 'NUL', 81h, 0, B7h
    "-74", 'NUL', 81h, 0, B6h
    "-75", 'NUL', 81h, 0, B5h
    "-76", 'NUL', 81h, 0, B4h
    "-77", 'NUL', 81h, 0, B3h
    "-78", 'NUL', 81h, 0, B2h
    "-79", 'NUL', 81h, 0, B1h
    "-80", 'NUL', 81h, 0, B0h
    "-81", 'NUL', 81h, 0, AFh
    "-82", 'NUL', 81h, 0, AEh
    "-83", 'NUL', 81h, 0, ADh
    "-84", 'NUL', 81h, 0, ACh
    "-85", 'NUL', 81h, 0, ABh
    "-86", 'NUL', 81h, 0, AAh
    "-87", 'NUL', 81h, 0, A9h
    "-88", 'NUL', 81h, 0, A8h
    "-89", 'NUL', 81h, 0, A7h
    "-90", 'NUL', 81h, 0, A6h
    "-91", 'NUL', 81h, 0, A5h
    "-92", 'NUL', 81h, 0, A4h
    "-93", 'NUL', 81h, 0, A3h
    "-94", 'NUL', 81h, 0, A2h
    "-95", 'NUL', 81h, 0, A1h
    "-96", 'NUL', 81h, 0, A0h
    "-97", 'NUL', 81h, 0, 9Fh
    "-98", 'NUL', 81h, 0, 9Eh
    "-99", 'NUL', 81h, 0, 9Dh
    "-100", 'NUL', 81h, 0, 9Ch
    "-101", 'NUL', 81h, 0, 9Bh
    "-102", 'NUL', 81h, 0, 9Ah
    "-103", 'NUL', 81h, 0, 99h
    "-104", 'NUL', 81h, 0, 98h
    "-105", 'NUL', 81h, 0, 97h
    "-106", 'NUL', 81h, 0, 96h
    "-107", 'NUL', 81h, 0, 95h
    "-108", 'NUL', 81h, 0, 94h
    "-109", 'NUL', 81h, 0, 93h
    "-110", 'NUL', 81h, 0, 92h
    "-111", 'NUL', 81h, 0, 91h
    "-112", 'NUL', 81h, 0, 90h
    "-113", 'NUL', 81h, 0, 8Fh
    "-114", 'NUL', 81h, 0, 8Eh
    "-115", 'NUL', 81h, 0, 8Dh
    "-116", 'NUL', 81h, 0, 8Ch
    "-117", 'NUL', 81h, 0, 8Bh
    "-118", 'NUL', 81h, 0, 8Ah
    "-119", 'NUL', 81h, 0, 89h
    "-120", 'NUL', 81h, 0, 88h
    "-121", 'NUL', 81h, 0, 87h
    "-122", 'NUL', 81h, 0, 86h
    "-123", 'NUL', 81h, 0, 85h
    "-124", 'NUL', 81h, 0, 84h
    "-125", 'NUL', 81h, 0, 83h
    "-126", 'NUL', 81h, 0, 82h
    "-127", 'NUL', 81h, 0, 81h
    "-128", 'NUL', 81h, 0, 80h

    (Hexadecimal Numbers)

    "00h", 'NUL', 81h, 0, 00h
    "01h", 'NUL', 81h, 0, 01h
    "02h", 'NUL', 81h, 0, 02h
    "03h", 'NUL', 81h, 0, 03h
    "04h", 'NUL', 81h, 0, 04h
    "05h", 'NUL', 81h, 0, 05h
    "06h", 'NUL', 81h, 0, 06h
    "07h", 'NUL', 81h, 0, 07h
    "08h", 'NUL', 81h, 0, 08h
    "09h", 'NUL', 81h, 0, 09h
    "0Ah", 'NUL', 81h, 0, 0Ah
    "0Bh", 'NUL', 81h, 0, 0Bh
    "0Ch", 'NUL', 81h, 0, 0Ch
    "0Dh", 'NUL', 81h, 0, 0Dh
    "0Eh", 'NUL', 81h, 0, 0Eh
    "0Fh", 'NUL', 81h, 0, 0Fh
    "10h", 'NUL', 81h, 0, 10h
    "11h", 'NUL', 81h, 0, 11h
    "12h", 'NUL', 81h, 0, 12h
    "13h", 'NUL', 81h, 0, 13h
    "14h", 'NUL', 81h, 0, 14h
    "15h", 'NUL', 81h, 0, 15h
    "16h", 'NUL', 81h, 0, 16h
    "17h", 'NUL', 81h, 0, 17h
    "18h", 'NUL', 81h, 0, 18h
    "19h", 'NUL', 81h, 0, 19h
    "1Ah", 'NUL', 81h, 0, 1Ah
    "1Bh", 'NUL', 81h, 0, 1Bh
    "1Ch", 'NUL', 81h, 0, 1Ch
    "1Dh", 'NUL', 81h, 0, 1Dh
    "1Eh", 'NUL', 81h, 0, 1Eh
    "1Fh", 'NUL', 81h, 0, 1Fh
    "20h", 'NUL', 81h, 0, 20h
    "21h", 'NUL', 81h, 0, 21h
    "22h", 'NUL', 81h, 0, 22h
    "23h", 'NUL', 81h, 0, 23h
    "24h", 'NUL', 81h, 0, 24h
    "25h", 'NUL', 81h, 0, 25h
    "26h", 'NUL', 81h, 0, 26h
    "27h", 'NUL', 81h, 0, 27h
    "28h", 'NUL', 81h, 0, 28h
    "29h", 'NUL', 81h, 0, 29h
    "2Ah", 'NUL', 81h, 0, 2Ah
    "2Bh", 'NUL', 81h, 0, 2Bh
    "2Ch", 'NUL', 81h, 0, 2Ch
    "2Dh", 'NUL', 81h, 0, 2Dh
    "2Eh", 'NUL', 81h, 0, 2Eh
    "2Fh", 'NUL', 81h, 0, 2Fh
    "30h", 'NUL', 81h, 0, 30h
    "31h", 'NUL', 81h, 0, 31h
    "32h", 'NUL', 81h, 0, 32h
    "33h", 'NUL', 81h, 0, 33h
    "34h", 'NUL', 81h, 0, 34h
    "35h", 'NUL', 81h, 0, 35h
    "36h", 'NUL', 81h, 0, 36h
    "37h", 'NUL', 81h, 0, 37h
    "38h", 'NUL', 81h, 0, 38h
    "39h", 'NUL', 81h, 0, 39h
    "3Ah", 'NUL', 81h, 0, 3Ah
    "3Bh", 'NUL', 81h, 0, 3Bh
    "3Ch", 'NUL', 81h, 0, 3Ch
    "3Dh", 'NUL', 81h, 0, 3Dh
    "3Eh", 'NUL', 81h, 0, 3Eh
    "3Fh", 'NUL', 81h, 0, 3Fh
    "40h", 'NUL', 81h, 0, 40h
    "41h", 'NUL', 81h, 0, 41h
    "42h", 'NUL', 81h, 0, 42h
    "43h", 'NUL', 81h, 0, 43h
    "44h", 'NUL', 81h, 0, 44h
    "45h", 'NUL', 81h, 0, 45h
    "46h", 'NUL', 81h, 0, 46h
    "47h", 'NUL', 81h, 0, 47h
    "48h", 'NUL', 81h, 0, 48h
    "49h", 'NUL', 81h, 0, 49h
    "4Ah", 'NUL', 81h, 0, 4Ah
    "4Bh", 'NUL', 81h, 0, 4Bh
    "4Ch", 'NUL', 81h, 0, 4Ch
    "4Dh", 'NUL', 81h, 0, 4Dh
    "4Eh", 'NUL', 81h, 0, 4Eh
    "4Fh", 'NUL', 81h, 0, 4Fh
    "50h", 'NUL', 81h, 0, 50h
    "51h", 'NUL', 81h, 0, 51h
    "52h", 'NUL', 81h, 0, 52h
    "53h", 'NUL', 81h, 0, 53h
    "54h", 'NUL', 81h, 0, 54h
    "55h", 'NUL', 81h, 0, 55h
    "56h", 'NUL', 81h, 0, 56h
    "57h", 'NUL', 81h, 0, 57h
    "58h", 'NUL', 81h, 0, 58h
    "59h", 'NUL', 81h, 0, 59h
    "5Ah", 'NUL', 81h, 0, 5Ah
    "5Bh", 'NUL', 81h, 0, 5Bh
    "5Ch", 'NUL', 81h, 0, 5Ch
    "5Dh", 'NUL', 81h, 0, 5Dh
    "5Eh", 'NUL', 81h, 0, 5Eh
    "5Fh", 'NUL', 81h, 0, 5Fh
    "60h", 'NUL', 81h, 0, 60h
    "61h", 'NUL', 81h, 0, 61h
    "62h", 'NUL', 81h, 0, 62h
    "63h", 'NUL', 81h, 0, 63h
    "64h", 'NUL', 81h, 0, 64h
    "65h", 'NUL', 81h, 0, 65h
    "66h", 'NUL', 81h, 0, 66h
    "67h", 'NUL', 81h, 0, 67h
    "68h", 'NUL', 81h, 0, 68h
    "69h", 'NUL', 81h, 0, 69h
    "6Ah", 'NUL', 81h, 0, 6Ah
    "6Bh", 'NUL', 81h, 0, 6Bh
    "6Ch", 'NUL', 81h, 0, 6Ch
    "6Dh", 'NUL', 81h, 0, 6Dh
    "6Eh", 'NUL', 81h, 0, 6Eh
    "6Fh", 'NUL', 81h, 0, 6Fh
    "70h", 'NUL', 81h, 0, 70h
    "71h", 'NUL', 81h, 0, 71h
    "72h", 'NUL', 81h, 0, 72h
    "73h", 'NUL', 81h, 0, 73h
    "74h", 'NUL', 81h, 0, 74h
    "75h", 'NUL', 81h, 0, 75h
    "76h", 'NUL', 81h, 0, 76h
    "77h", 'NUL', 81h, 0, 77h
    "78h", 'NUL', 81h, 0, 78h
    "79h", 'NUL', 81h, 0, 79h
    "7Ah", 'NUL', 81h, 0, 7Ah
    "7Bh", 'NUL', 81h, 0, 7Bh
    "7Ch", 'NUL', 81h, 0, 7Ch
    "7Dh", 'NUL', 81h, 0, 7Dh
    "7Eh", 'NUL', 81h, 0, 7Eh
    "7Fh", 'NUL', 81h, 0, 7Fh
    "80h", 'NUL', 81h, 0, 80h
    "81h", 'NUL', 81h, 0, 81h
    "82h", 'NUL', 81h, 0, 82h
    "83h", 'NUL', 81h, 0, 83h
    "84h", 'NUL', 81h, 0, 84h
    "85h", 'NUL', 81h, 0, 85h
    "86h", 'NUL', 81h, 0, 86h
    "87h", 'NUL', 81h, 0, 87h
    "88h", 'NUL', 81h, 0, 88h
    "89h", 'NUL', 81h, 0, 89h
    "8Ah", 'NUL', 81h, 0, 8Ah
    "8Bh", 'NUL', 81h, 0, 8Bh
    "8Ch", 'NUL', 81h, 0, 8Ch
    "8Dh", 'NUL', 81h, 0, 8Dh
    "8Eh", 'NUL', 81h, 0, 8Eh
    "8Fh", 'NUL', 81h, 0, 8Fh
    "90h", 'NUL', 81h, 0, 90h
    "91h", 'NUL', 81h, 0, 91h
    "92h", 'NUL', 81h, 0, 92h
    "93h", 'NUL', 81h, 0, 93h
    "94h", 'NUL', 81h, 0, 94h
    "95h", 'NUL', 81h, 0, 95h
    "96h", 'NUL', 81h, 0, 96h
    "97h", 'NUL', 81h, 0, 97h
    "98h", 'NUL', 81h, 0, 98h
    "99h", 'NUL', 81h, 0, 99h
    "9Ah", 'NUL', 81h, 0, 9Ah
    "9Bh", 'NUL', 81h, 0, 9Bh
    "9Ch", 'NUL', 81h, 0, 9Ch
    "9Dh", 'NUL', 81h, 0, 9Dh
    "9Eh", 'NUL', 81h, 0, 9Eh
    "9Fh", 'NUL', 81h, 0, 9Fh
    "A0h", 'NUL', 81h, 0, A0h
    "A1h", 'NUL', 81h, 0, A1h
    "A2h", 'NUL', 81h, 0, A2h
    "A3h", 'NUL', 81h, 0, A3h
    "A4h", 'NUL', 81h, 0, A4h
    "A5h", 'NUL', 81h, 0, A5h
    "A6h", 'NUL', 81h, 0, A6h
    "A7h", 'NUL', 81h, 0, A7h
    "A8h", 'NUL', 81h, 0, A8h
    "A9h", 'NUL', 81h, 0, A9h
    "AAh", 'NUL', 81h, 0, AAh
    "ABh", 'NUL', 81h, 0, ABh
    "ACh", 'NUL', 81h, 0, ACh
    "ADh", 'NUL', 81h, 0, ADh
    "AEh", 'NUL', 81h, 0, AEh
    "AFh", 'NUL', 81h, 0, AFh
    "B0h", 'NUL', 81h, 0, B0h
    "B1h", 'NUL', 81h, 0, B1h
    "B2h", 'NUL', 81h, 0, B2h
    "B3h", 'NUL', 81h, 0, B3h
    "B4h", 'NUL', 81h, 0, B4h
    "B5h", 'NUL', 81h, 0, B5h
    "B6h", 'NUL', 81h, 0, B6h
    "B7h", 'NUL', 81h, 0, B7h
    "B8h", 'NUL', 81h, 0, B8h
    "B9h", 'NUL', 81h, 0, B9h
    "BAh", 'NUL', 81h, 0, BAh
    "BBh", 'NUL', 81h, 0, BBh
    "BCh", 'NUL', 81h, 0, BCh
    "BDh", 'NUL', 81h, 0, BDh
    "BEh", 'NUL', 81h, 0, BEh
    "BFh", 'NUL', 81h, 0, BFh
    "C0h", 'NUL', 81h, 0, C0h
    "C1h", 'NUL', 81h, 0, C1h
    "C2h", 'NUL', 81h, 0, C2h
    "C3h", 'NUL', 81h, 0, C3h
    "C4h", 'NUL', 81h, 0, C4h
    "C5h", 'NUL', 81h, 0, C5h
    "C6h", 'NUL', 81h, 0, C6h
    "C7h", 'NUL', 81h, 0, C7h
    "C8h", 'NUL', 81h, 0, C8h
    "C9h", 'NUL', 81h, 0, C9h
    "CAh", 'NUL', 81h, 0, CAh
    "CBh", 'NUL', 81h, 0, CBh
    "CCh", 'NUL', 81h, 0, CCh
    "CDh", 'NUL', 81h, 0, CDh
    "CEh", 'NUL', 81h, 0, CEh
    "CFh", 'NUL', 81h, 0, CFh
    "D0h", 'NUL', 81h, 0, D0h
    "D1h", 'NUL', 81h, 0, D1h
    "D2h", 'NUL', 81h, 0, D2h
    "D3h", 'NUL', 81h, 0, D3h
    "D4h", 'NUL', 81h, 0, D4h
    "D5h", 'NUL', 81h, 0, D5h
    "D6h", 'NUL', 81h, 0, D6h
    "D7h", 'NUL', 81h, 0, D7h
    "D8h", 'NUL', 81h, 0, D8h
    "D9h", 'NUL', 81h, 0, D9h
    "DAh", 'NUL', 81h, 0, DAh
    "DBh", 'NUL', 81h, 0, DBh
    "DCh", 'NUL', 81h, 0, DCh
    "DDh", 'NUL', 81h, 0, DDh
    "DEh", 'NUL', 81h, 0, DEh
    "DFh", 'NUL', 81h, 0, DFh
    "E0h", 'NUL', 81h, 0, E0h
    "E1h", 'NUL', 81h, 0, E1h
    "E2h", 'NUL', 81h, 0, E2h
    "E3h", 'NUL', 81h, 0, E3h
    "E4h", 'NUL', 81h, 0, E4h
    "E5h", 'NUL', 81h, 0, E5h
    "E6h", 'NUL', 81h, 0, E6h
    "E7h", 'NUL', 81h, 0, E7h
    "E8h", 'NUL', 81h, 0, E8h
    "E9h", 'NUL', 81h, 0, E9h
    "EAh", 'NUL', 81h, 0, EAh
    "EBh", 'NUL', 81h, 0, EBh
    "ECh", 'NUL', 81h, 0, ECh
    "EDh", 'NUL', 81h, 0, EDh
    "EEh", 'NUL', 81h, 0, EEh
    "EFh", 'NUL', 81h, 0, EFh
    "F0h", 'NUL', 81h, 0, F0h
    "F1h", 'NUL', 81h, 0, F1h
    "F2h", 'NUL', 81h, 0, F2h
    "F3h", 'NUL', 81h, 0, F3h
    "F4h", 'NUL', 81h, 0, F4h
    "F5h", 'NUL', 81h, 0, F5h
    "F6h", 'NUL', 81h, 0, F6h
    "F7h", 'NUL', 81h, 0, F7h
    "F8h", 'NUL', 81h, 0, F8h
    "F9h", 'NUL', 81h, 0, F9h
    "FAh", 'NUL', 81h, 0, FAh
    "FBh", 'NUL', 81h, 0, FBh
    "FCh", 'NUL', 81h, 0, FCh
    "FDh", 'NUL', 81h, 0, FDh
    "FEh", 'NUL', 81h, 0, FEh
    "FFh", 'NUL', 81h, 0, FFh

    (Single Digit Hex Numbers)

    "0h", 'NUL', 81h, 0, 00h
    "1h", 'NUL', 81h, 0, 01h
    "2h", 'NUL', 81h, 0, 02h
    "3h", 'NUL', 81h, 0, 03h
    "4h", 'NUL', 81h, 0, 04h
    "5h", 'NUL', 81h, 0, 05h
    "6h", 'NUL', 81h, 0, 06h
    "7h", 'NUL', 81h, 0, 07h
    "8h", 'NUL', 81h, 0, 08h
    "9h", 'NUL', 81h, 0, 09h
    "Ah", 'NUL', 81h, 0, 0Ah
    "Bh", 'NUL', 81h, 0, 0Bh
    "Ch", 'NUL', 81h, 0, 0Ch
    "Dh", 'NUL', 81h, 0, 0Dh
    "Eh", 'NUL', 81h, 0, 0Eh
    "Fh", 'NUL', 81h, 0, 0Fh

    (Eight-Bit Binary Numbers)

    "0000_0000b", 'NUL', 81h, 0, 00h
    "0000_0001b", 'NUL', 81h, 0, 01h
    "0000_0010b", 'NUL', 81h, 0, 02h
    "0000_0011b", 'NUL', 81h, 0, 03h
    "0000_0100b", 'NUL', 81h, 0, 04h
    "0000_0101b", 'NUL', 81h, 0, 05h
    "0000_0110b", 'NUL', 81h, 0, 06h
    "0000_0111b", 'NUL', 81h, 0, 07h
    "0000_1000b", 'NUL', 81h, 0, 08h
    "0000_1001b", 'NUL', 81h, 0, 09h
    "0000_1010b", 'NUL', 81h, 0, 0Ah
    "0000_1011b", 'NUL', 81h, 0, 0Bh
    "0000_1100b", 'NUL', 81h, 0, 0Ch
    "0000_1101b", 'NUL', 81h, 0, 0Dh
    "0000_1110b", 'NUL', 81h, 0, 0Eh
    "0000_1111b", 'NUL', 81h, 0, 0Fh
    "0001_0000b", 'NUL', 81h, 0, 10h
    "0001_0001b", 'NUL', 81h, 0, 11h
    "0001_0010b", 'NUL', 81h, 0, 12h
    "0001_0011b", 'NUL', 81h, 0, 13h
    "0001_0100b", 'NUL', 81h, 0, 14h
    "0001_0101b", 'NUL', 81h, 0, 15h
    "0001_0110b", 'NUL', 81h, 0, 16h
    "0001_0111b", 'NUL', 81h, 0, 17h
    "0001_1000b", 'NUL', 81h, 0, 18h
    "0001_1001b", 'NUL', 81h, 0, 19h
    "0001_1010b", 'NUL', 81h, 0, 1Ah
    "0001_1011b", 'NUL', 81h, 0, 1Bh
    "0001_1100b", 'NUL', 81h, 0, 1Ch
    "0001_1101b", 'NUL', 81h, 0, 1Dh
    "0001_1110b", 'NUL', 81h, 0, 1Eh
    "0001_1111b", 'NUL', 81h, 0, 1Fh
    "0010_0000b", 'NUL', 81h, 0, 20h
    "0010_0001b", 'NUL', 81h, 0, 21h
    "0010_0010b", 'NUL', 81h, 0, 22h
    "0010_0011b", 'NUL', 81h, 0, 23h
    "0010_0100b", 'NUL', 81h, 0, 24h
    "0010_0101b", 'NUL', 81h, 0, 25h
    "0010_0110b", 'NUL', 81h, 0, 26h
    "0010_0111b", 'NUL', 81h, 0, 27h
    "0010_1000b", 'NUL', 81h, 0, 28h
    "0010_1001b", 'NUL', 81h, 0, 29h
    "0010_1010b", 'NUL', 81h, 0, 2Ah
    "0010_1011b", 'NUL', 81h, 0, 2Bh
    "0010_1100b", 'NUL', 81h, 0, 2Ch
    "0010_1101b", 'NUL', 81h, 0, 2Dh
    "0010_1110b", 'NUL', 81h, 0, 2Eh
    "0010_1111b", 'NUL', 81h, 0, 2Fh
    "0011_0000b", 'NUL', 81h, 0, 30h
    "0011_0001b", 'NUL', 81h, 0, 31h
    "0011_0010b", 'NUL', 81h, 0, 32h
    "0011_0011b", 'NUL', 81h, 0, 33h
    "0011_0100b", 'NUL', 81h, 0, 34h
    "0011_0101b", 'NUL', 81h, 0, 35h
    "0011_0110b", 'NUL', 81h, 0, 36h
    "0011_0111b", 'NUL', 81h, 0, 37h
    "0011_1000b", 'NUL', 81h, 0, 38h
    "0011_1001b", 'NUL', 81h, 0, 39h
    "0011_1010b", 'NUL', 81h, 0, 3Ah
    "0011_1011b", 'NUL', 81h, 0, 3Bh
    "0011_1100b", 'NUL', 81h, 0, 3Ch
    "0011_1101b", 'NUL', 81h, 0, 3Dh
    "0011_1110b", 'NUL', 81h, 0, 3Eh
    "0011_1111b", 'NUL', 81h, 0, 3Fh
    "0100_0000b", 'NUL', 81h, 0, 40h
    "0100_0001b", 'NUL', 81h, 0, 41h
    "0100_0010b", 'NUL', 81h, 0, 42h
    "0100_0011b", 'NUL', 81h, 0, 43h
    "0100_0100b", 'NUL', 81h, 0, 44h
    "0100_0101b", 'NUL', 81h, 0, 45h
    "0100_0110b", 'NUL', 81h, 0, 46h
    "0100_0111b", 'NUL', 81h, 0, 47h
    "0100_1000b", 'NUL', 81h, 0, 48h
    "0100_1001b", 'NUL', 81h, 0, 49h
    "0100_1010b", 'NUL', 81h, 0, 4Ah
    "0100_1011b", 'NUL', 81h, 0, 4Bh
    "0100_1100b", 'NUL', 81h, 0, 4Ch
    "0100_1101b", 'NUL', 81h, 0, 4Dh
    "0100_1110b", 'NUL', 81h, 0, 4Eh
    "0100_1111b", 'NUL', 81h, 0, 4Fh
    "0101_0000b", 'NUL', 81h, 0, 50h
    "0101_0001b", 'NUL', 81h, 0, 51h
    "0101_0010b", 'NUL', 81h, 0, 52h
    "0101_0011b", 'NUL', 81h, 0, 53h
    "0101_0100b", 'NUL', 81h, 0, 54h
    "0101_0101b", 'NUL', 81h, 0, 55h
    "0101_0110b", 'NUL', 81h, 0, 56h
    "0101_0111b", 'NUL', 81h, 0, 57h
    "0101_1000b", 'NUL', 81h, 0, 58h
    "0101_1001b", 'NUL', 81h, 0, 59h
    "0101_1010b", 'NUL', 81h, 0, 5Ah
    "0101_1011b", 'NUL', 81h, 0, 5Bh
    "0101_1100b", 'NUL', 81h, 0, 5Ch
    "0101_1101b", 'NUL', 81h, 0, 5Dh
    "0101_1110b", 'NUL', 81h, 0, 5Eh
    "0101_1111b", 'NUL', 81h, 0, 5Fh
    "0110_0000b", 'NUL', 81h, 0, 60h
    "0110_0001b", 'NUL', 81h, 0, 61h
    "0110_0010b", 'NUL', 81h, 0, 62h
    "0110_0011b", 'NUL', 81h, 0, 63h
    "0110_0100b", 'NUL', 81h, 0, 64h
    "0110_0101b", 'NUL', 81h, 0, 65h
    "0110_0110b", 'NUL', 81h, 0, 66h
    "0110_0111b", 'NUL', 81h, 0, 67h
    "0110_1000b", 'NUL', 81h, 0, 68h
    "0110_1001b", 'NUL', 81h, 0, 69h
    "0110_1010b", 'NUL', 81h, 0, 6Ah
    "0110_1011b", 'NUL', 81h, 0, 6Bh
    "0110_1100b", 'NUL', 81h, 0, 6Ch
    "0110_1101b", 'NUL', 81h, 0, 6Dh
    "0110_1110b", 'NUL', 81h, 0, 6Eh
    "0110_1111b", 'NUL', 81h, 0, 6Fh
    "0111_0000b", 'NUL', 81h, 0, 70h
    "0111_0001b", 'NUL', 81h, 0, 71h
    "0111_0010b", 'NUL', 81h, 0, 72h
    "0111_0011b", 'NUL', 81h, 0, 73h
    "0111_0100b", 'NUL', 81h, 0, 74h
    "0111_0101b", 'NUL', 81h, 0, 75h
    "0111_0110b", 'NUL', 81h, 0, 76h
    "0111_0111b", 'NUL', 81h, 0, 77h
    "0111_1000b", 'NUL', 81h, 0, 78h
    "0111_1001b", 'NUL', 81h, 0, 79h
    "0111_1010b", 'NUL', 81h, 0, 7Ah
    "0111_1011b", 'NUL', 81h, 0, 7Bh
    "0111_1100b", 'NUL', 81h, 0, 7Ch
    "0111_1101b", 'NUL', 81h, 0, 7Dh
    "0111_1110b", 'NUL', 81h, 0, 7Eh
    "0111_1111b", 'NUL', 81h, 0, 7Fh
    "1000_0000b", 'NUL', 81h, 0, 80h
    "1000_0001b", 'NUL', 81h, 0, 81h
    "1000_0010b", 'NUL', 81h, 0, 82h
    "1000_0011b", 'NUL', 81h, 0, 83h
    "1000_0100b", 'NUL', 81h, 0, 84h
    "1000_0101b", 'NUL', 81h, 0, 85h
    "1000_0110b", 'NUL', 81h, 0, 86h
    "1000_0111b", 'NUL', 81h, 0, 87h
    "1000_1000b", 'NUL', 81h, 0, 88h
    "1000_1001b", 'NUL', 81h, 0, 89h
    "1000_1010b", 'NUL', 81h, 0, 8Ah
    "1000_1011b", 'NUL', 81h, 0, 8Bh
    "1000_1100b", 'NUL', 81h, 0, 8Ch
    "1000_1101b", 'NUL', 81h, 0, 8Dh
    "1000_1110b", 'NUL', 81h, 0, 8Eh
    "1000_1111b", 'NUL', 81h, 0, 8Fh
    "1001_0000b", 'NUL', 81h, 0, 90h
    "1001_0001b", 'NUL', 81h, 0, 91h
    "1001_0010b", 'NUL', 81h, 0, 92h
    "1001_0011b", 'NUL', 81h, 0, 93h
    "1001_0100b", 'NUL', 81h, 0, 94h
    "1001_0101b", 'NUL', 81h, 0, 95h
    "1001_0110b", 'NUL', 81h, 0, 96h
    "1001_0111b", 'NUL', 81h, 0, 97h
    "1001_1000b", 'NUL', 81h, 0, 98h
    "1001_1001b", 'NUL', 81h, 0, 99h
    "1001_1010b", 'NUL', 81h, 0, 9Ah
    "1001_1011b", 'NUL', 81h, 0, 9Bh
    "1001_1100b", 'NUL', 81h, 0, 9Ch
    "1001_1101b", 'NUL', 81h, 0, 9Dh
    "1001_1110b", 'NUL', 81h, 0, 9Eh
    "1001_1111b", 'NUL', 81h, 0, 9Fh
    "1010_0000b", 'NUL', 81h, 0, A0h
    "1010_0001b", 'NUL', 81h, 0, A1h
    "1010_0010b", 'NUL', 81h, 0, A2h
    "1010_0011b", 'NUL', 81h, 0, A3h
    "1010_0100b", 'NUL', 81h, 0, A4h
    "1010_0101b", 'NUL', 81h, 0, A5h
    "1010_0110b", 'NUL', 81h, 0, A6h
    "1010_0111b", 'NUL', 81h, 0, A7h
    "1010_1000b", 'NUL', 81h, 0, A8h
    "1010_1001b", 'NUL', 81h, 0, A9h
    "1010_1010b", 'NUL', 81h, 0, AAh
    "1010_1011b", 'NUL', 81h, 0, ABh
    "1010_1100b", 'NUL', 81h, 0, ACh
    "1010_1101b", 'NUL', 81h, 0, ADh
    "1010_1110b", 'NUL', 81h, 0, AEh
    "1010_1111b", 'NUL', 81h, 0, AFh
    "1011_0000b", 'NUL', 81h, 0, B0h
    "1011_0001b", 'NUL', 81h, 0, B1h
    "1011_0010b", 'NUL', 81h, 0, B2h
    "1011_0011b", 'NUL', 81h, 0, B3h
    "1011_0100b", 'NUL', 81h, 0, B4h
    "1011_0101b", 'NUL', 81h, 0, B5h
    "1011_0110b", 'NUL', 81h, 0, B6h
    "1011_0111b", 'NUL', 81h, 0, B7h
    "1011_1000b", 'NUL', 81h, 0, B8h
    "1011_1001b", 'NUL', 81h, 0, B9h
    "1011_1010b", 'NUL', 81h, 0, BAh
    "1011_1011b", 'NUL', 81h, 0, BBh
    "1011_1100b", 'NUL', 81h, 0, BCh
    "1011_1101b", 'NUL', 81h, 0, BDh
    "1011_1110b", 'NUL', 81h, 0, BEh
    "1011_1111b", 'NUL', 81h, 0, BFh
    "1100_0000b", 'NUL', 81h, 0, C0h
    "1100_0001b", 'NUL', 81h, 0, C1h
    "1100_0010b", 'NUL', 81h, 0, C2h
    "1100_0011b", 'NUL', 81h, 0, C3h
    "1100_0100b", 'NUL', 81h, 0, C4h
    "1100_0101b", 'NUL', 81h, 0, C5h
    "1100_0110b", 'NUL', 81h, 0, C6h
    "1100_0111b", 'NUL', 81h, 0, C7h
    "1100_1000b", 'NUL', 81h, 0, C8h
    "1100_1001b", 'NUL', 81h, 0, C9h
    "1100_1010b", 'NUL', 81h, 0, CAh
    "1100_1011b", 'NUL', 81h, 0, CBh
    "1100_1100b", 'NUL', 81h, 0, CCh
    "1100_1101b", 'NUL', 81h, 0, CDh
    "1100_1110b", 'NUL', 81h, 0, CEh
    "1100_1111b", 'NUL', 81h, 0, CFh
    "1101_0000b", 'NUL', 81h, 0, D0h
    "1101_0001b", 'NUL', 81h, 0, D1h
    "1101_0010b", 'NUL', 81h, 0, D2h
    "1101_0011b", 'NUL', 81h, 0, D3h
    "1101_0100b", 'NUL', 81h, 0, D4h
    "1101_0101b", 'NUL', 81h, 0, D5h
    "1101_0110b", 'NUL', 81h, 0, D6h
    "1101_0111b", 'NUL', 81h, 0, D7h
    "1101_1000b", 'NUL', 81h, 0, D8h
    "1101_1001b", 'NUL', 81h, 0, D9h
    "1101_1010b", 'NUL', 81h, 0, DAh
    "1101_1011b", 'NUL', 81h, 0, DBh
    "1101_1100b", 'NUL', 81h, 0, DCh
    "1101_1101b", 'NUL', 81h, 0, DDh
    "1101_1110b", 'NUL', 81h, 0, DEh
    "1101_1111b", 'NUL', 81h, 0, DFh
    "1110_0000b", 'NUL', 81h, 0, E0h
    "1110_0001b", 'NUL', 81h, 0, E1h
    "1110_0010b", 'NUL', 81h, 0, E2h
    "1110_0011b", 'NUL', 81h, 0, E3h
    "1110_0100b", 'NUL', 81h, 0, E4h
    "1110_0101b", 'NUL', 81h, 0, E5h
    "1110_0110b", 'NUL', 81h, 0, E6h
    "1110_0111b", 'NUL', 81h, 0, E7h
    "1110_1000b", 'NUL', 81h, 0, E8h
    "1110_1001b", 'NUL', 81h, 0, E9h
    "1110_1010b", 'NUL', 81h, 0, EAh
    "1110_1011b", 'NUL', 81h, 0, EBh
    "1110_1100b", 'NUL', 81h, 0, ECh
    "1110_1101b", 'NUL', 81h, 0, EDh
    "1110_1110b", 'NUL', 81h, 0, EEh
    "1110_1111b", 'NUL', 81h, 0, EFh
    "1111_0000b", 'NUL', 81h, 0, F0h
    "1111_0001b", 'NUL', 81h, 0, F1h
    "1111_0010b", 'NUL', 81h, 0, F2h
    "1111_0011b", 'NUL', 81h, 0, F3h
    "1111_0100b", 'NUL', 81h, 0, F4h
    "1111_0101b", 'NUL', 81h, 0, F5h
    "1111_0110b", 'NUL', 81h, 0, F6h
    "1111_0111b", 'NUL', 81h, 0, F7h
    "1111_1000b", 'NUL', 81h, 0, F8h
    "1111_1001b", 'NUL', 81h, 0, F9h
    "1111_1010b", 'NUL', 81h, 0, FAh
    "1111_1011b", 'NUL', 81h, 0, FBh
    "1111_1100b", 'NUL', 81h, 0, FCh
    "1111_1101b", 'NUL', 81h, 0, FDh
    "1111_1110b", 'NUL', 81h, 0, FEh
    "1111_1111b", 'NUL', 81h, 0, FFh

    (Four-Bit Binary Numbers)

    "0000b", 'NUL', 81h, 0, 00h
    "0001b", 'NUL', 81h, 0, 01h
    "0010b", 'NUL', 81h, 0, 02h
    "0011b", 'NUL', 81h, 0, 03h
    "0100b", 'NUL', 81h, 0, 04h
    "0101b", 'NUL', 81h, 0, 05h
    "0110b", 'NUL', 81h, 0, 06h
    "0111b", 'NUL', 81h, 0, 07h
    "1000b", 'NUL', 81h, 0, 08h
    "1001b", 'NUL', 81h, 0, 09h
    "1010b", 'NUL', 81h, 0, 0Ah
    "1011b", 'NUL', 81h, 0, 0Bh
    "1100b", 'NUL', 81h, 0, 0Ch
    "1101b", 'NUL', 81h, 0, 0Dh
    "1110b", 'NUL', 81h, 0, 0Eh
    "1111b", 'NUL', 81h, 0, 0Fh

    (Printable ASCII Characters)

    "'SP'", 'NUL', 81h, 0, 20h
    "'!'", 'NUL', 81h, 0, 21h
    "'"'", 'NUL', 81h, 0, 22h
    "'#'", 'NUL', 81h, 0, 23h
    "'$'", 'NUL', 81h, 0, 24h
    "'%'", 'NUL', 81h, 0, 25h
    "'&'", 'NUL', 81h, 0, 26h
    "'''", 'NUL', 81h, 0, 27h
    "'('", 'NUL', 81h, 0, 28h
    "')'", 'NUL', 81h, 0, 29h
    "'*'", 'NUL', 81h, 0, 2Ah  
    "'+'", 'NUL', 81h, 0, 2Bh
    "','", 'NUL', 81h, 0, 2Ch
    "'-'", 'NUL', 81h, 0, 2Dh
    "'.'", 'NUL', 81h, 0, 2Eh
    "'/'", 'NUL', 81h, 0, 2Fh
    "'0'", 'NUL', 81h, 0, 30h
    "'1'", 'NUL', 81h, 0, 31h
    "'2'", 'NUL', 81h, 0, 32h
    "'3'", 'NUL', 81h, 0, 33h
    "'4'", 'NUL', 81h, 0, 34h
    "'5'", 'NUL', 81h, 0, 35h
    "'6'", 'NUL', 81h, 0, 36h
    "'7'", 'NUL', 81h, 0, 37h
    "'8'", 'NUL', 81h, 0, 38h
    "'9'", 'NUL', 81h, 0, 39h
    "':'", 'NUL', 81h, 0, 3Ah
    "';'", 'NUL', 81h, 0, 3Bh
    "'<'", 'NUL', 81h, 0, 3Ch
    "'='", 'NUL', 81h, 0, 3Dh
    "'>'", 'NUL', 81h, 0, 3Eh
    "'?'", 'NUL', 81h, 0, 3Fh
    "'@'", 'NUL', 81h, 0, 40h
    "'A'", 'NUL', 81h, 0, 41h
    "'B'", 'NUL', 81h, 0, 42h
    "'C'", 'NUL', 81h, 0, 43h
    "'D'", 'NUL', 81h, 0, 44h
    "'E'", 'NUL', 81h, 0, 45h
    "'F'", 'NUL', 81h, 0, 46h
    "'G'", 'NUL', 81h, 0, 47h
    "'H'", 'NUL', 81h, 0, 48h
    "'I'", 'NUL', 81h, 0, 49h
    "'J'", 'NUL', 81h, 0, 4Ah
    "'K'", 'NUL', 81h, 0, 4Bh
    "'L'", 'NUL', 81h, 0, 4Ch
    "'M'", 'NUL', 81h, 0, 4Dh
    "'N'", 'NUL', 81h, 0, 4Eh
    "'O'", 'NUL', 81h, 0, 4Fh
    "'P'", 'NUL', 81h, 0, 50h
    "'Q'", 'NUL', 81h, 0, 51h
    "'R'", 'NUL', 81h, 0, 52h
    "'S'", 'NUL', 81h, 0, 53h
    "'T'", 'NUL', 81h, 0, 54h
    "'U'", 'NUL', 81h, 0, 55h
    "'V'", 'NUL', 81h, 0, 56h
    "'W'", 'NUL', 81h, 0, 57h
    "'X'", 'NUL', 81h, 0, 58h
    "'Y'", 'NUL', 81h, 0, 59h
    "'Z'", 'NUL', 81h, 0, 5Ah
    "'['", 'NUL', 81h, 0, 5Bh
    "'\'", 'NUL', 81h, 0, 5Ch
    "']'", 'NUL', 81h, 0, 5Dh
    "'^'", 'NUL', 81h, 0, 5Eh
    "'_'", 'NUL', 81h, 0, 5Fh
    "'`'", 'NUL', 81h, 0, 60h
    "'a'", 'NUL', 81h, 0, 61h
    "'b'", 'NUL', 81h, 0, 62h
    "'c'", 'NUL', 81h, 0, 63h
    "'d'", 'NUL', 81h, 0, 64h
    "'e'", 'NUL', 81h, 0, 65h
    "'f'", 'NUL', 81h, 0, 66h
    "'g'", 'NUL', 81h, 0, 67h
    "'h'", 'NUL', 81h, 0, 68h
    "'i'", 'NUL', 81h, 0, 69h
    "'j'", 'NUL', 81h, 0, 6Ah
    "'k'", 'NUL', 81h, 0, 6Bh
    "'l'", 'NUL', 81h, 0, 6Ch
    "'m'", 'NUL', 81h, 0, 6Dh
    "'n'", 'NUL', 81h, 0, 6Eh
    "'o'", 'NUL', 81h, 0, 6Fh
    "'p'", 'NUL', 81h, 0, 70h
    "'q'", 'NUL', 81h, 0, 71h
    "'r'", 'NUL', 81h, 0, 72h
    "'s'", 'NUL', 81h, 0, 73h
    "'t'", 'NUL', 81h, 0, 74h
    "'u'", 'NUL', 81h, 0, 75h
    "'v'", 'NUL', 81h, 0, 76h
    "'w'", 'NUL', 81h, 0, 77h
    "'x'", 'NUL', 81h, 0, 78h
    "'y'", 'NUL', 81h, 0, 79h
    "'z'", 'NUL', 81h, 0, 7Ah
    "'{'", 'NUL', 81h, 0, 7Bh
    "'|'", 'NUL', 81h, 0, 7Ch
    "'}'", 'NUL', 81h, 0, 7Dh
    "'~'", 'NUL', 81h, 0, 7Eh

    (ASCII Constants)

    "'NUL'", 'NUL', 81h, 0, 00h
    "'CR'", 'NUL', 81h, 0, 0Dh
    "'LF'", 'NUL', 81h, 0, 0Ah

    (SYS Instructions)

    "NOP", 'NUL', 81h, 0, 00h
    "SSI", 'NUL', 81h, 0, 01h
    "SSO", 'NUL', 81h, 0, 02h
    "SCL", 'NUL', 81h, 0, 03h
    "SCH", 'NUL', 81h, 0, 04h
    "RET", 'NUL', 81h, 0, 05h
    "COR", 'NUL', 81h, 0, 06h
    "OWN", 'NUL', 81h, 0, 07h

    (FIX Instructions)

    "P4", 'NUL', 81h, 0, 08h
    "P1", 'NUL', 81h, 0, 09h
    "P2", 'NUL', 81h, 0, 0Ah
    "P3", 'NUL', 81h, 0, 0Bh
    "M4", 'NUL', 81h, 0, 0Ch
    "M3", 'NUL', 81h, 0, 0Dh
    "M2", 'NUL', 81h, 0, 0Eh
    "M1", 'NUL', 81h, 0, 0Fh

    (ALU Instructions)

    "CLR", 'NUL', 81h, 0, 10h
    "IDO", 'NUL', 81h, 0, 11h
    "OCR", 'NUL', 81h, 0, 12h
    "OCO", 'NUL', 81h, 0, 13h
    "SLR", 'NUL', 81h, 0, 14h
    "SLO", 'NUL', 81h, 0, 15h
    "SRR", 'NUL', 81h, 0, 16h
    "SRO", 'NUL', 81h, 0, 17h
    "AND", 'NUL', 81h, 0, 18h
    "IOR", 'NUL', 81h, 0, 19h
    "EOR", 'NUL', 81h, 0, 1Ah
    "ADD", 'NUL', 81h, 0, 1Bh
    "CAR", 'NUL', 81h, 0, 1Ch
    "RLO", 'NUL', 81h, 0, 1Dh
    "REO", 'NUL', 81h, 0, 1Eh
    "RGO", 'NUL', 81h, 0, 1Fh

    (TRAP Instructions)

    "*0", 'NUL', 81h, 0, 20h
    "*1", 'NUL', 81h, 0, 21h
    "*2", 'NUL', 81h, 0, 22h
    "*3", 'NUL', 81h, 0, 23h
    "*4", 'NUL', 81h, 0, 24h
    "*5", 'NUL', 81h, 0, 25h
    "*6", 'NUL', 81h, 0, 26h
    "*7", 'NUL', 81h, 0, 27h
    "*8", 'NUL', 81h, 0, 28h
    "*9", 'NUL', 81h, 0, 29h
    "*10", 'NUL', 81h, 0, 2Ah
    "*11", 'NUL', 81h, 0, 2Bh
    "*12", 'NUL', 81h, 0, 2Ch
    "*13", 'NUL', 81h, 0, 2Dh
    "*14", 'NUL', 81h, 0, 2Eh
    "*15", 'NUL', 81h, 0, 2Fh
    "*16", 'NUL', 81h, 0, 30h
    "*17", 'NUL', 81h, 0, 31h
    "*18", 'NUL', 81h, 0, 32h
    "*19", 'NUL', 81h, 0, 33h
    "*20", 'NUL', 81h, 0, 34h
    "*21", 'NUL', 81h, 0, 35h
    "*22", 'NUL', 81h, 0, 36h
    "*23", 'NUL', 81h, 0, 37h
    "*24", 'NUL', 81h, 0, 38h
    "*25", 'NUL', 81h, 0, 39h
    "*26", 'NUL', 81h, 0, 3Ah
    "*27", 'NUL', 81h, 0, 3Bh
    "*28", 'NUL', 81h, 0, 3Ch
    "*29", 'NUL', 81h, 0, 3Dh
    "*30", 'NUL', 81h, 0, 3Eh
    "*31", 'NUL', 81h, 0, 3Fh

    (DIRO Instructions)

    "0d", 'NUL', 81h, 0, 40h
    "1d", 'NUL', 81h, 0, 41h
    "2d", 'NUL', 81h, 0, 42h
    "3d", 'NUL', 81h, 0, 43h
    "4d", 'NUL', 81h, 0, 44h
    "5d", 'NUL', 81h, 0, 45h
    "6d", 'NUL', 81h, 0, 46h
    "7d", 'NUL', 81h, 0, 47h
    "d0", 'NUL', 81h, 0, 48h
    "d1", 'NUL', 81h, 0, 49h
    "d2", 'NUL', 81h, 0, 4Ah
    "d3", 'NUL', 81h, 0, 4Bh
    "d4", 'NUL', 81h, 0, 4Ch
    "d5", 'NUL', 81h, 0, 4Dh
    "d6", 'NUL', 81h, 0, 4Eh
    "d7", 'NUL', 81h, 0, 4Fh
    "0i", 'NUL', 81h, 0, 50h
    "1i", 'NUL', 81h, 0, 51h
    "2i", 'NUL', 81h, 0, 52h
    "3i", 'NUL', 81h, 0, 53h
    "4i", 'NUL', 81h, 0, 54h
    "5i", 'NUL', 81h, 0, 55h
    "6i", 'NUL', 81h, 0, 56h
    "7i", 'NUL', 81h, 0, 57h
    "i0", 'NUL', 81h, 0, 58h
    "i1", 'NUL', 81h, 0, 59h
    "i2", 'NUL', 81h, 0, 5Ah
    "i3", 'NUL', 81h, 0, 5Bh
    "i4", 'NUL', 81h, 0, 5Ch
    "i5", 'NUL', 81h, 0, 5Dh
    "i6", 'NUL', 81h, 0, 5Eh
    "i7", 'NUL', 81h, 0, 5Fh
    "0r", 'NUL', 81h, 0, 60h
    "1r", 'NUL', 81h, 0, 61h
    "2r", 'NUL', 81h, 0, 62h
    "3r", 'NUL', 81h, 0, 63h
    "4r", 'NUL', 81h, 0, 64h
    "5r", 'NUL', 81h, 0, 65h
    "6r", 'NUL', 81h, 0, 66h
    "7r", 'NUL', 81h, 0, 67h
    "r0", 'NUL', 81h, 0, 68h
    "r1", 'NUL', 81h, 0, 69h
    "r2", 'NUL', 81h, 0, 6Ah
    "r3", 'NUL', 81h, 0, 6Bh
    "r4", 'NUL', 81h, 0, 6Ch
    "r5", 'NUL', 81h, 0, 6Dh
    "r6", 'NUL', 81h, 0, 6Eh
    "r7", 'NUL', 81h, 0, 6Fh
    "0o", 'NUL', 81h, 0, 70h
    "1o", 'NUL', 81h, 0, 71h
    "2o", 'NUL', 81h, 0, 72h
    "3o", 'NUL', 81h, 0, 73h
    "4o", 'NUL', 81h, 0, 74h
    "5o", 'NUL', 81h, 0, 75h
    "6o", 'NUL', 81h, 0, 76h
    "7o", 'NUL', 81h, 0, 77h
    "o0", 'NUL', 81h, 0, 78h
    "o1", 'NUL', 81h, 0, 79h
    "o2", 'NUL', 81h, 0, 7Ah
    "o3", 'NUL', 81h, 0, 7Bh
    "o4", 'NUL', 81h, 0, 7Ch
    "o5", 'NUL', 81h, 0, 7Dh
    "o6", 'NUL', 81h, 0, 7Eh
    "o7", 'NUL', 81h, 0, 7Fh

    (PAIR Instructions)

    "no", 'NUL', 81h, 0, 80h
    "END", 'NUL', 81h, 0, 81h

    "SCROUNGE_NL", 'NUL', 81h, 0, 82h

    "nd", 'NUL', 81h, 0, 83h
    "nr", 'NUL', 81h, 0, 84h
    "ni", 'NUL', 81h, 0, 85h
    "ns", 'NUL', 81h, 0, 86h
    "np", 'NUL', 81h, 0, 87h
    "ne", 'NUL', 81h, 0, 88h
    "na", 'NUL', 81h, 0, 89h
    "nb", 'NUL', 81h, 0, 8Ah
    "nj", 'NUL', 81h, 0, 8Bh
    "nw", 'NUL', 81h, 0, 8Ch
    "nt", 'NUL', 81h, 0, 8Dh
    "nf", 'NUL', 81h, 0, 8Eh
    "nc", 'NUL', 81h, 0, 8Fh
    "mo", 'NUL', 81h, 0, 90h

    "SCROUNGE_MM", 'NUL', 81h, 0, 91h
    "SCROUNGE_ML", 'NUL', 81h, 0, 92h

    "md", 'NUL', 81h, 0, 93h
    "mr", 'NUL', 81h, 0, 94h
    "mi", 'NUL', 81h, 0, 95h
    "ms", 'NUL', 81h, 0, 96h
    "mp", 'NUL', 81h, 0, 97h
    "me", 'NUL', 81h, 0, 98h
    "ma", 'NUL', 81h, 0, 99h
    "mb", 'NUL', 81h, 0, 9Ah
    "mj", 'NUL', 81h, 0, 9Bh
    "mw", 'NUL', 81h, 0, 9Ch
    "mt", 'NUL', 81h, 0, 9Dh
    "mf", 'NUL', 81h, 0, 9Eh
    "mc", 'NUL', 81h, 0, 9Fh
    "lo", 'NUL', 81h, 0, A0h

    "SCROUNGE_LM", 'NUL', 81h, 0, A1h
    "SCROUNGE_LL", 'NUL', 81h, 0, A2h

    "ld", 'NUL', 81h, 0, A3h
    "lr", 'NUL', 81h, 0, A4h
    "li", 'NUL', 81h, 0, A5h
    "ls", 'NUL', 81h, 0, A6h
    "lp", 'NUL', 81h, 0, A7h
    "le", 'NUL', 81h, 0, A8h
    "la", 'NUL', 81h, 0, A9h
    "lb", 'NUL', 81h, 0, AAh
    "lj", 'NUL', 81h, 0, ABh
    "lw", 'NUL', 81h, 0, ACh
    "lt", 'NUL', 81h, 0, ADh
    "lf", 'NUL', 81h, 0, AEh
    "lc", 'NUL', 81h, 0, AFh
    "do", 'NUL', 81h, 0, B0h
    "dm", 'NUL', 81h, 0, B1h
    "dl", 'NUL', 81h, 0, B2h

    "SCROUNGE_DD", 'NUL', 81h, 0, B3h

    "dr", 'NUL', 81h, 0, B4h
    "di", 'NUL', 81h, 0, B5h
    "ds", 'NUL', 81h, 0, B6h
    "dp", 'NUL', 81h, 0, B7h
    "de", 'NUL', 81h, 0, B8h
    "da", 'NUL', 81h, 0, B9h
    "db", 'NUL', 81h, 0, BAh
    "dj", 'NUL', 81h, 0, BBh
    "dw", 'NUL', 81h, 0, BCh
    "dt", 'NUL', 81h, 0, BDh
    "df", 'NUL', 81h, 0, BEh
    "dc", 'NUL', 81h, 0, BFh
    "ro", 'NUL', 81h, 0, C0h
    "rm", 'NUL', 81h, 0, C1h
    "rl", 'NUL', 81h, 0, C2h
    "rd", 'NUL', 81h, 0, C3h

    "SCROUNGE_RR", 'NUL', 81h, 0, C4h

    "ri", 'NUL', 81h, 0, C5h
    "rs", 'NUL', 81h, 0, C6h
    "rp", 'NUL', 81h, 0, C7h
    "re", 'NUL', 81h, 0, C8h
    "ra", 'NUL', 81h, 0, C9h
    "rb", 'NUL', 81h, 0, CAh
    "rj", 'NUL', 81h, 0, CBh
    "rw", 'NUL', 81h, 0, CCh
    "rt", 'NUL', 81h, 0, CDh
    "rf", 'NUL', 81h, 0, CEh
    "rc", 'NUL', 81h, 0, CFh
    "io", 'NUL', 81h, 0, D0h
    "im", 'NUL', 81h, 0, D1h
    "il", 'NUL', 81h, 0, D2h
    "id", 'NUL', 81h, 0, D3h
    "ir", 'NUL', 81h, 0, D4h

    "SCROUNGE_II", 'NUL', 81h, 0, D5h

    "is", 'NUL', 81h, 0, D6h
    "ip", 'NUL', 81h, 0, D7h
    "ie", 'NUL', 81h, 0, D8h
    "ia", 'NUL', 81h, 0, D9h
    "ib", 'NUL', 81h, 0, DAh
    "ij", 'NUL', 81h, 0, DBh
    "iw", 'NUL', 81h, 0, DCh
    "it", 'NUL', 81h, 0, DDh
    "if", 'NUL', 81h, 0, DEh
    "ic", 'NUL', 81h, 0, DFh
    "so", 'NUL', 81h, 0, E0h
    "sm", 'NUL', 81h, 0, E1h
    "sl", 'NUL', 81h, 0, E2h
    "sd", 'NUL', 81h, 0, E3h
    "sr", 'NUL', 81h, 0, E4h
    "si", 'NUL', 81h, 0, E5h
    "ss", 'NUL', 81h, 0, E6h
    "sp", 'NUL', 81h, 0, E7h
    "se", 'NUL', 81h, 0, E8h
    "sa", 'NUL', 81h, 0, E9h
    "sb", 'NUL', 81h, 0, EAh
    "sj", 'NUL', 81h, 0, EBh
    "sw", 'NUL', 81h, 0, ECh
    "st", 'NUL', 81h, 0, EDh
    "sf", 'NUL', 81h, 0, EEh
    "sc", 'NUL', 81h, 0, EFh
    "po", 'NUL', 81h, 0, F0h
    "pm", 'NUL', 81h, 0, F1h
    "pl", 'NUL', 81h, 0, F2h
    "pd", 'NUL', 81h, 0, F3h
    "pr", 'NUL', 81h, 0, F4h
    "pi", 'NUL', 81h, 0, F5h
    "ps", 'NUL', 81h, 0, F6h
    "pp", 'NUL', 81h, 0, F7h
    "pe", 'NUL', 81h, 0, F8h
    "pa", 'NUL', 81h, 0, F9h
    "pb", 'NUL', 81h, 0, FAh
    "pj", 'NUL', 81h, 0, FBh
    "pw", 'NUL', 81h, 0, FCh
    "pt", 'NUL', 81h, 0, FDh
    "pf", 'NUL', 81h, 0, FEh
    "pc", 'NUL', 81h, 0, FFh

    (Handcoded firmware "exports")

    "Interpret", 'NUL', 1, Interpret, 00h
    "Mul8",      'NUL', 1, Mul8,      00h
    "DivMod8",   'NUL', 1, DivMod8,   00h
    "VSrch",     'NUL', 1, VSrch,     00h
    "PrMsg",     'NUL', 1, PrMsg,     00h
    "ready",     'NUL', 2, ready,     00h

P[VTOPPAGE]
O[VTOPOFFSET] 'NUL'


;********* ******************************************************************
P[LOXBASE]7Fh
;********* ******************************************************************

    (The region from 00h to 7Fh of this page is the LOX output buffer.
    The VM when executing puts text here, which gets output when
    'lox' is run from the command line.)

    "Hello Worm! Good byte, world" 00h

    (The region from 80h to EFh is populated with 'lox' command line
     arguments, null separated.)

    (F0h to end of page are LOX system variables - don't relocate!)

    O[UNUSED]F0h  0h 0h 0h 0h 0h 0h 0h 
    O[ARG]        80h   (Offset in LOXBASE of current cmd line argument str)
    O[POS]        00h        (Current position in output text buffer)
    O[VTP]        VTOPPAGE
    O[VTO]        VTOPPAGE.VTOPOFFSET
    O[DESTP]      FFh
    O[DESTO]      00h
    O[SRCP]       FFh
    O[SRCO]       00h
    O[ECODE]      00h

