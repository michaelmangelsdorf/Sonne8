UPDATE: There is now a [successor project](https://github.com/michaelmangelsdorf/myth).

Computer project around an 8-bit microcontroller I've designed.

There are design files for a working single-board computer built around it in the 'Prototype' folder. I've had the PCB for it manufactured by JLCPCB during various revisions. The computer now works fine at 8 MHz system clock, which corresponds to around 1 million instructions per second. The main components are all 74HC-series CMOS chips. I basically wanted to design a CPU I could actually build from parts, not just inside an FPGA like [my previous](https://github.com/Dosflange/Paver) 16-bit machine.

The current reference schematics I'm using are [here](https://github.com/Dosflange/Myth/blob/main/docs/static/myth.pdf).
If I were to build another board, it would probably be based on the KiCad files in the 'Schematics' folder, where I
separated the schematics into functional modules.

So what this image shows is not an application board with an Arduino or anything, it is the actual microcontroller. The current version
has exactly 100 chips.

![CPU board](https://github.com/Dosflange/Myth/blob/main/Controller-Board_abu.jpg)

There are two software emulators I've written for this system. The code is very small, hopefully it's a good reference for finding out about the CPU.
The decoder-based [implementation](https://github.com/Dosflange/Myth/blob/main/Dev/src/clox/myth.h) is more high-level, and
the [dispatch table](https://github.com/Dosflange/Myth/blob/main/Dev/src/clox/vtable.c) version better suited to finding out about particular instructions.

[Documentation WIP](https://michaelmangelsdorf.github.io/Myth/)

__Update Sep.24__:

I've written a new assembler for this machine in Go(lang). The first thing I ported from the old format was
this multiplication routine. It multiplies two unsigned 8-bit numbers and returns a 16-bit result.
You can try it out with the tools in the 'Dev' folder.

```
Goldie/LOX assembly log
Page.Offset  Object code  Lid Line# Source text

                        048 0062  ;****** ***************************************************************
                        048 0063  P[Mul8]+  (Multiplies R by O, result in R and O)
                        048 0064  ;****** ***************************************************************
                        048 0065  
21.00:  07 5E           043 0066        OWN i6        (Save return pointer in L7/L6)
21.02:  79              043 0067        o1            (Multiplicand into L1, turns into low order result)
21.03:  68              043 0068        r0            (Multiplier into L0)
21.04:  10 6A           043 0069        CLR r2        (Clear high-order result, copy to L2)
21.06:  85 07           043 0070        ni 07h        (Initialise loop counter, 8 bits)
                        043 0071  
                        043 0072      O[Mul8Loop]
                        043 0073   
21.08:  80 01           043 0074        no 01h        (Bit mask for LSB)
21.0A:  61 18           043 0075        1r AND        (Check if multiplicand has LSB set)
21.0C:  8E 12           043 0076        nf >Mul8Skip  (Skip if not)
21.0E:  60 72 1B 6A     043 0077        0r 2o ADD r2  (Add multiplier to high order result)
                        043 0078  
                        043 0079      O[Mul8Skip]
                        043 0080  
21.12:  80 01           043 0081        no 01h        (Bit mask for LSB)
21.14:  62 18 6B        043 0082        2r AND r3     (Prepare flag of whether high order LSB is set)
21.17:  61 16 69        043 0083        1r SRR r1      (Shift low-order result right)
21.1A:  62 16 6A        043 0084        2r SRR r2       (Shift high-order result right)
                        043 0085  
21.1D:  63 8E 25        043 0086        3r nf >Mul8Done  (Check flag from earlier - HO LSB set?)
21.20:  84 80           043 0087        nr 80h          (Bit mask for MSB)
21.22:  71 19 69        043 0088        1o IOR r1     (Handle shift-result carry bit into MSB)
                        043 0089  
                        043 0090      O[Mul8Done]
                        043 0091  
21.25:  8C 08           043 0092        nw <Mul8Loop
21.27:  71              043 0093        1o            (Result low-order)
21.28:  62              043 0094        2r            (Result high-order)
21.29:  56 05           043 0095        6i RET        (Restore return pointer from L7/L6)
```


