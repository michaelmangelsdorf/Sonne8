
; This file assembles with assem.c
; Write object code into NOR-flash EEPROM on main board
; Install IO-Board, flash SPI EEPROM with some user code (mul16.asm)
; Below code copies SPI EEPROM into RAM and executes user code

@BOOTSTRAP

  SCL
  LD 0
  LD 0000.1000b     ; Chip select 25LC SPI EEPROM

  LS 0000.0011b     ; Set 25LC READ instruction
  *SPI_WRBYTE
  LS 0              ; EEPROM high order address byte
  *SPI_WRBYTE
  LS 0              ; Low order address byte
  *SPI_WRBYTE.

  LR 80h.           ; Start at row 80h (RAM)
  @OUTER
      LB 80h        ; Do 128 bytes
      LA FFh.       ; Target address -1, set A working
        @INNER
         *SPI_RDBYTE
         LF IDA+1 FA
         SM
         LF IDB-1 FB LT <INNER
      RB LF IDB+1 FR
      LT <OUTER

  LD 0.              ; Deselect EEPROM, ends READ instruction
  LJ 80h.            ; Run RAM code

  @NEVER LJ <NEVER.  ; Never reaches here

@SPI_RDBYTE! ; bit bang SPI interface
    SCH, SCL
    CSI, CSO, SCH SCL
    CSI, CSO, SCH SCL
    CSI, CSO, SCH SCL
    CSI, CSO, SCH SCL
    CSI, CSO, SCH SCL
    CSI, CSO, SCH SCL
    CSI, CSO, SCH SCL
    CSI, CSO
   RET.

@SPI_WRBYTE! ; bit bang SPI interface
    SCH SCL,
    CSO SCH SCL, CSO SCH SCL, CSO SCH SCL
    CSO SCH SCL, CSO SCH SCL, CSO SCH SCL, CSO SCH SCL
    RET.

