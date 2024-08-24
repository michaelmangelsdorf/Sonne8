
.bp
.NH 1
Bus communication
.LP
This micro-controller has integrated circuitry for handling serial and
parallel communication. This is facilitated by dedicated hardware-registers
and instructions.
.NH 2
E (ENABLE) register
.RS
.LP
The 8-bit Enable register is used to control the enable state of devices
attached to the serial or parallel bus lines.
To this end, the register is divided into two four-bit groups for
device selection.
.LP
The
.B
low-order
.R
four-bit group (EL) drives a 4-to-16 line inverted
decoder, which maps the bit pattern in EL 1 of 16 possible,
mutually exclusive
.B
active-low
.R
select signals (SL0-15).
.LP
Select signal S0 is reserved to select a NULL device ("nothing").
SL1 corresponds to the internal POR register output enable signal (PAR_OE).
SL2-15 can be used freely for application hardware.
.LP
The
.B
high-order
.R
four-bit group (EH) drives a 4-to-16 line non-inverted decoder,
which maps the bit pattern in EH to 1 of 16 possible, mutually exclusive
.B
active-high
.R
select signals (SH0-15).
.LP
Select signal S0 is reserved to select a NULL device ("nothing").
SH1 corresponds to the internal PIR register copy select signal (PAR_CP).
SH2-15 can be used freely for application hardware.
.PDFPIC figures/ereg.pdf
.RE
.bp
.NH 2
Communication Registers
.RS
.NH 3
SOR (Serial Output Register)
.QP
A write-only parallel-to-serial shift register for serialising an output byte,
modelled after a 74HC165 chip. 
Writing an output value for serialisation is done by writing the value
into register S. The value is clocked out/serialized by pulsing the SCLK
clock line. This happens when the SCL/SCH instructions are used repeatedly.
SCL-SCH-SCL gives a posititve clock edge. SCH-SCL-SCH generates an inverted clock.
Eight clock cycles are required to process a byte.
.NH 3
SIR (Serial Input Register)
.QP
A read-only serial-to-parallel shift register for de-serialising an incoming
bit stream into an input byte, modelled after a 74HC595 chip.
This is done by executing the SCL/SCH instructions
repeatedly.
SCL-SCH-SCL gives a positive clock edge. SCH-SCL-SCH generates an inverted clock.
Eight clock cycles are required to process a byte.
Reading a deserialised input byte is done by reading register S.
.PDFPIC figures/sirsor.pdf
.bp
.NH 3
POR (Parallel Output Register)
.QP
A write-only tri-state register with 8-bit parallel output, modelled after
a 74HC574 chip.
Writing an output byte onto the parallel bus is a two step process.
First, the data byte must be latched into the register by writing it
into P. Then, the register output must be enabled by selecting PAR_OE
in the E register, as described above.
.NH 3
PIR (Parallel Input Register)
.QP
A read-only 8-bit parallel input register, modelled after a 74HC574 chip.
Latching the current 8-bit value of
the parallel bus into the register is done by selecting SH1 in E.
The latched data byte can then be read from P.
The bus is assumed to pull-down, so when all bus-devices are in
tri-state mode, a zero value is registered.
.PDFPIC figures/pirpor.pdf
.RE
.bp
.NH 2
Communication Instructions
.RS
.QP
The following Signal instructions contained in the SYS group
operate on the communication registers:
.NH 3
SSI (Shift Serial In)
.QP
Shift in one bit from serial data input line into SIR (at LSB position)
.NH 3
SSO (Shift Serial Out)
.QP
Shift out one bit from SOR onto serial data output line (from MSB position)
.NH 3
SCH
.QP
Set serial clock line high
.NH 3
SCL
.QP
Set serial clock line low
.RE
.NH 2
Parallel Communication
.RS
.LP
The Myth controller interfaces to an external bidirectional
8-bit wide bus (IO bus).
.LP
It can communicate on this bus by writing a data byte
into P (POR register), and then enabling SL1 in the E register
by setting its lower nybble to 1.
Setting the bit to 1 switches the register from
tri-state output to active output, so that the byte value
is output on the bus lines.
.LP
While the output is active, other devices on the bus
can read the data byte by enabling the corresponding SHx signal
in E while keeping SL1 (the low-order nybble of E) set to 1.
.LP
Deselecting SL1 in E again (setting the low-order nybble to
a value different from 1) tristates the POR output,
so that other devices can put data bytes on the IO bus.
.LP
Enabling LH1 in E (setting the high-order nybble to 1)
latches a data byte into the PIR. This byte can then be
read from the P register.
.LP
Once a data byte has been read,
the PIR input should be deselected again in E by setting
the high-order nybble to a value different from 1.
.RE
.NH 2
Serial Communication
.RS
.LP
The Serial Peripheral Interface (SPI)
protocol can be implemented using the device enable register E,
serial registers SIR and SOR, and instructions SCL, SCH, SSI, and SSO.
.NH 2
Device Selection
.LP
Before communicating with a specific device connected to the serial bus,
the corresponding selector bit representing the device must be set in the E register.
.NH 2
Data Transmission
.LP
To transmit data to the selected device, the processor writes the dataus
to be sent into the SOR (Serial Output) register.
The SSO (Serial Shift Out) instruction is then used,
which clocks the serial output shift register and produces a data bit
on the MOSI line. Using the instruction sequence SCL SCH SCL (Serial Clock Low/High),
a positive edge clock pulse is generated.
As each bit is shifted out, it is sent to the selected device through the serial bus.
The passive device processes the transmitted bit and the cycle repeats.
.NH 2
Data Reception
.LP
To receive data from an external device, the SSI (Serial Shift In)
instruction is used. It clocks the serial input shift register, allowing the
processor to receive one bit of data at a time from the selected device via
the MISO line. The received data can then be read from the S register.
Clocking is done as for "Data Transmission".
.NH 2
CPOL (Clock Polarity)
.LP
The CPOL parameter determines the idle state of the clock signal.
The Myth controller provides signals SCL (Serial Clock Low) and SCH (Serial Clock High)
instructions which can be used to control the clock signal's state.
To configure CPOL=0 (clock idles low), execute SCL to set the clock signal low during
the idle state. To configure CPOL=1 (clock idles high), execute SCH to set the clock
signal high during the idle state.
.NH 2
CPHA (Clock Phase)
.LP
The CPHA parameter determines the edge of the clock signal where data is captured
or changed. The Myth controller provides instructions SSI (Serial shift in) and
SSO (Serial shift out) to control data transfer on each clock transition.
To configure CPHA=0 (data captured on the leading edge), execute SSI before the clock
transition to capture the incoming data. To configure CPHA=1 (data captured on the trailing edge),
execute SSI after the clock transition to capture the incoming data.
Similarly, to transmit data on the leading or trailing edge, execute SSO before or
after the clock transition, respectively.
.NH 2
Device Deselection
.LP
After data transmission is complete, the selected device needs to be deselected
to allow other devices to communicate on the bus. This is done by clearing the corresponding selector bit for the device in the E register.
.RE


