
; This file assembles with assem.c
; Write object code into NOR-flash EEPROM on main board
; Install IO-Board, flash SPI EEPROM with some user code (mul16.asm)
; Below code copies SPI EEPROM into RAM and executes user code


@BOOTSTRAP

  SCL

  LV 0
  LV 0010.0000b     ; Chip select 25LC SPI EEPROM
  
  LS 0000.0011b     ; Set 25LC READ instruction
  *SPI_WRBYTE
  LS 0              ; EEPROM high order address byte
  *SPI_WRBYTE
  LS 0              ; Low order address byte
  *SPI_WRBYTE.

  LQ 80h FR.        ; Do 128 rows
  @OUTER
      LQ 80h qL0p   ; Do 128 bytes
      LG 0.         ; Target address
        @INNER
         *SPI_RDBYTE
         SM ING
         qL0g DEQ qL0p LT <INNER
      RQ INQ FR
      LT <OUTER

  LV 0.              ; Deselect EEPROM, ends READ instruction
  LB 80h.            ; Run RAM code, no return

@STOP LB <STOP.

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


