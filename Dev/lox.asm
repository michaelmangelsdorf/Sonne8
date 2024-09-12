

@20:interpret

(Locate and interpret a routine)

    L0/Dividend (Assert register content at this point)
    O/Divisor

    MYCONST = 49h (A defined constant)

    OWN i6 (Save return offset)
    nd LOCBASE
    failsrp@

	(The token wasn't recognized)

        no ARG
        mo
        nc VSrch
        ifRz >InterpSucc
        r1 (Look-up failed)
        call PrMsg
    @

    InterpSucc@
      RET
    @
@
