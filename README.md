Ongoing project around an 8-bit microcontroller I've designed.

In the prototype folder, there are production files and the legacy repo for a working PCB. I've had it manufactured by JLCPCB and it works fine at around 8 MHz clock, which corresponds to around 1 million instructions per second. The components are all 74HC-series CMOS chips. I basically wanted to design a CPU I could actually build, not an FPGA fantasy like
![my previous](https://github.com/Dosflange/Paver)
one (which I still like and it works fine).

The PCB is shown below.

![CPU board](https://github.com/Dosflange/Myth/blob/main/Controller-Board_abu.jpg)

Since then, I've redesigned the architecture. All this is in the top-level folders.
The work in the "Schematics" folder is based on the KiCad files I used for PCB production, but it's diverged a lot, and I am mainly using it to document my progress. It shouldn't be hard to test it out and make another board though.

Probably the quickest way to understand what this little CPU does is by reading the C emulator
[header file](https://github.com/Dosflange/Myth/blob/main/Dev/src/myth.h).
