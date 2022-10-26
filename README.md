
Welcome to this project!

This repo contains all files you need to build and program an 8-bit CPU that I have designed.

You would need to have the two PCBs fabricated: Firstly, there's the CPU board itself.
It is actually a micro-controller, in that this board implements RAM and an IO interface.
Then secondly, there is an IO expansion board, that fits on top of the CPU board via Arduino type stackable headers.

The CPU board looks like this:

![CPU board](https://github.com/Dosflange/Sonne/blob/main/board_and_screws.jpg)

These plastic spacers go between the CPU board and the IO board. The distance/length of the spacer part (without the protruding thread) is 2 cm.

![CPU board](https://github.com/Dosflange/Sonne/blob/main/board_screws.jpg)

Both modules go together like so:

![CPU board](https://github.com/Dosflange/Sonne/blob/main/board_sandwich.jpg)

The assembled development kit i.e. hardware portion of this project:

![CPU board](https://github.com/Dosflange/Sonne/blob/main/board_ready.jpg)

### Address space

The address space of this CPU is unusual. Eight bits give you 256 address places. The first 128 places form a region, which can be switched to point to another memory segment ("bank switching"). The next 64 places after that are fixed (for global variables). The remaining 64 places form a region again, which can be switched out to point to another memory segment (for local variables).

### Instruction format

There are four types of instructions. (1) The high order four bits of the instruction word are all zero: These are sixteen "signal" instructions that take no operands, where the signal is encoded in bits 0-3. (2) The high order bit is zero, but bits 4-6 are not all zero: These are register-to-register transfers, where bits 4-6 encode the source register, bits 0-3 encode the target register. (3) The high order bit is one, bit 6 is 0: These are micro-calls, where bits 0-5 encode the call-target. (4) Bits 6 and 7 or both one: These are register-to-memory transfers, from the two accumulator registers to the first 8 bytes of the global or local memory section, where bit 5 encodes which register it is, bit 4 encodes global or local, bit 3 is whether it's load or store, and bits 0-2 encode the address offset. (See the opcode_matrix file in the repo for details.)






