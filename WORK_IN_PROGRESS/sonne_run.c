
// Sonne CPU Simulator (rev. Daffodil)
// 2023 Dec 16  Michael Mangelsdorf

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* The following are constants used for decoding
   COPY PAIRS.
   These are instructions for copying a source
   register into a target register, both specified
   in the opcode.

   (Some registers are "pseudo-registers". They
   behave like registers, but there's more than
   meets the eye. The J register for example is
   a pseudo-register. Copying a value into it
   jumps to that location. More details below.)

   Mnemonically, COPY PAIRS appear in the form of
   two letter words in lower-case, first (to the left)
   the source register, then the target register
   (to the right), for example "na", where the
   source register is N, and the target register
   of the copy process is A: Copy N into A. */

/*
 * LHS Left-Hand-Side, 8 source register
 */

#define LHS_N     0  /* PSEUDO REG Number */

/* N is a read-only pseudo-register.
   It stands for the byte following the current instruction
   in memory. This byte is called a code literal.
   When N occurs as the source, that byte is fetched
   and copied into the target register.
   The CPU then skips this byte and points to the instruction
   following it.
   Example: "na 2 NOP" stores the byte literal 2 in alpha,
   and then executes the NOP instruction.
   Example: "nj 20h" copies the byte literal into
   pseudo-register j (jump).
   This transfers control to a new address told by the
   byte value,
   and hence starts executing instructions from
   address offset 20h in the current bank.
 */

#define LHS_M     1  /* PSEUDO REG Memory */

/* M is a pseudo-register. You can use it both as source
   and target of a COPY PAIR.

   When you read from it (source/left-hand side), you get
   the value of a specific memory cell.
   The address offset of that memory cell is contained
   in the WORKING accumulator register.
   The working accumulator register is either A (alpha)
   or B (beta), whichever has been WRITTEN to last.
   Example: "na 5 mr" stores the number 5 into A, making
   it the working accumulator, and then copies the memory
   value stored at address offset 5 into register R.

   When you write to it (target/right-hand side), you store
   the source value into a specific memory cell.
   The address offset of that memory cell is contained
   in the WORKING accumulator register.
   The working accumulator register is either A (alpha)
   or B (beta), whichever has been WRITTEN to last.
   Example: "nb 3 rm" stores the number 3 into B, making
   it the working accumulator, and then stores the value of
   register R into memory at address offset 5.
 */

#define LHS_E     2  /* BANK REG Exit */

/* The E register is a bank register.
   It has two special properties.

   1. If you change it, you should generally restore its
   previous value before executing a RET instruction to
   return from a subroutine.
   The CPU assumes that the E register contains the bank
   (high-order byte if the address) of the instruction
   to return to once the subroutine has finished.
   Behind the scenes, whenever you execute a call (xC) or
   a TRAP instruction (see below), the CPU saves the
   number to return to into the E register.
   During RET, the original bank number is restored from E.
   
   2. Whenever you use the M (Memory) pseudo-register
   using the xM and Mx instructions, and the address offset
   in the working accumulator register is in the range
   of 0-127 (128 bytes from 0-7Fh), called Exit segment,
   the implied memory bank (high-order byte) is in E.

   Example: "ne 10 na 3 mb" sets the bank number for
   offsets 0-127 to 10. Then the a accumulator gets set
   to 3 and by writing to it becomes the working accumulator.
   Since the offset 3 in the working accumulator is in
   the range 0-127 (the Exit segment),
   "mb" transfers the value at memory
   address [10*256+3] into the b register.
*/


#define LHS_G     3  /* BANK REG Global */

/* The G register is a bank register.
   This means it has a special property.
   Whenever you use the M (Memory) pseudo-register
   using the xM and Mx instructions, and the address offset
   in the working accumulator register is in the range
   of 128-191 (64 bytes from 80h - BFh), called Global segment,
   the implied memory bank (high-order byte) is in G.

   Example: "ng 10 na 130 mb" sets the bank number for
   offsets 128-191 to 10. Then the a accumulator gets set
   to 130 and by writing to it becomes the working accumulator.
   Since the offset 130 in the working accumulator is in
   the range 128-191 (the Global segment),
   "mb" transfers the value at memory
   address [10*256+130] into the b register.

   The GETPUT instructions allow you to copy
   registers A, B and R between the first four locations
   of the Global segment using their shortcut names G0-G3.
   Example: aG0 copies A into G0 (address offset 80h).
   Example: G3r copies G3 (address offset 83h) into R.
    */


#define LHS_L     4  /* BANK REG Local */

/* The L register is a bank register.
   It has two special properties.

   1. Whenever you use the M (Memory) pseudo-register
   using the xM and Mx instructions, and the address offset
   in the working accumulator register is in the range
   of 192-255 (64 bytes from C0h - FFh), called Local segment,
   the implied memory bank (high-order byte) is in L.

   Example: "nl 10 na 200 mb" sets the bank number for
   offsets 192-255 to 10. Then the a accumulator gets set
   to 200 and by writing to it becomes the working accumulator.
   Since the offset 200 in the working accumulator is in
   the range 192-255 (the Local segment),
   "mb" transfers the value at memory
   address [10*256+200] into the b register.

   The GETPUT instructions allow you to copy
   registers A, B and R between the first four locations
   of the Local segment using their shortcut names L0-L3.
   Example: aL0 copies A into L0 (address offset C0h).
   Example: L3r copies G3 (address offset C3h) into R.

   2. There are two special instructions that act on the
   L register. ENTER decrements L by 1, and LEAVE increments
   L by 1. These two instructions can be used to create
   and remove 64-byte stack frames for local variables.

   The CPU itself uses this feature. During ENTER, it creates
   a new stack frame (decrements L) and then saves the
   address offset of the instruction to return to in
   location C4h (of that stack frame).

   During LEAVE, it retrieves this offset from location C4h
   of the current stack frame, and reverts to the old stackframe
   (that of the calling function) by incrementing L.

   For this reason, you should only change L in unusual
   situations, and then restore the original value before
   executing "xC" (subroutine call) or TRAP, or a RET.
    */

#define LHS_S     5  /* PSEUDO REG Serial */

/* The S register is the most complex register of the CPU.
   It is used for serial communication with other devices.
   Write operations work differently than read operations,
   and they don't even go to the same place conceptually.
   You cannot for example store into S and expect that
   value to be there when you read from it in the next
   instruction, because the read is from a different place
   than where the write goes. See below.

   Writing into S ("xS") loads an invisible byte register
   called the SOR (Serial output register).
   Using the SSO (Serial shift out) instruction, you can
   shift this invisible byte to the left by one bit.
   The most significant bit (MSB) is sent out onto a serial
   output transmission line ("a wire") coming out of the CPU.
   By executing 8 SSO instructions successively, all eight
   bits of the byte are transmitted (serialized).
   When you write to S again before this process is
   complete, the remaining value in S is overwritten.
   By executing the instructions SCH (Serial clock high)
   and SCL (Serial clock low) in between SSO instructions,
   you can create a pulse, or clock signal, on
   a separate wire coming out of the CPU,
   to tell the recipient device that a bit is ready.

   Reading from S ("sx") loads the value contained in an
   invisible byte register
   called the SIR (Serial input register).
   Using the SSI (Serial shift in) instruction, you can
   shift this invisible byte to the left by one bit.
   The most significant bit (MSB) is discarded, and bit 0
   is loaded from a serial input transmission "wire"
   going into the CPU.
   By executing 8 SSI instructions successively, all eight
   bits of the byte are filled with serial input bits.
   When you read from S again before this process is
   complete, you get intermediate values.
   By executing the instructions SCH (Serial clock high)
   and SCL (Serial clock low) in between SSI instructions,
   you can create a pulse, or clock signal, on
   a separate wire coming out of the CPU,
   to tell the recipient device that a bit has been read.

   The D register (see below) lets you select, which serial
   device you would like to communicate with.
*/

#define LHS_P     6  /* PSEUDO REG Parallel */

/* The P register is used for parallel communication with
   other devices via an 8 bit parallel bus.
   This bus has a tri-state property. It has a third state
   besides 0 or 1, which essentially removes the CPU
   from the bus wires, so that other devices can communicate
   between themselves.

   Keep in mind that when you store into P, and then read
   back from P, the value may not be the same, because the
   values are read across an electronic bus, and protocol
   errors or noise may occur. See below.

   Writing into P ("xP") loads an invisible byte register
   called the POR (Parallel output register).
   This sets the bus to "no tristate", so that the byte
   that was copied into P is sent to the bus wires coming
   out of the CPU.

   Reading from P ("Px") samples the byte encoded on the
   bus wires into a hidden register called the PIR
   (Parallel input register) and copies it into the
   target register. If the bus is in tristate mode
   ("disconnected"), you get zero.

   You can put the bus into tri-state mode manually by
   executing the HIZ signal instruction.

   The D register (see below) lets you select, which parallel
   device you would like to communicate with.
*/

#define LHS_R     7  /* REG Return */

/* The R register is special in various ways.
  
   1. When you execute an ALU function, the CPU computes
   that function between the A and B accumulators, and
   stores the result in R.
   Example: "na 2 nb 3 ADD" stores the number 5 into R.

   2. There are special instructions for incrementing and
   decrementing R by small numbers.
   Example: "nr 16 r1+" stores 16 into R, and then increments
   R by 1 to 17.

   3. During "xC" (subroutine call) or TRAP, the CPU
   saves the address offset of the instruction to return to
   in the R register, overwriting its value.
   (Then, when you execute ENTER to create a stack frame, it
   additionally stores R into location C4h of the local segment
   (in the new stack frame).

   4. The GETPUT instructions allow you to copy
   registers A, B and R between the first four locations
   of the Global segment and the Local segment respectively,
   by using their shortcut names G0-G3 and L0-L3.
   Example: rG0 copies A into G0 (address offset 80h).
   Example: L3r copies L3 (address offset C3h) into R.
*/


/*
 *
 * RHS Right-Hand-Side, 14 target registers,
   2 LANE SPECIFIERS
 *
 */

#define RHS_SIG   0  /* MARKER "Signal"*/

/* This is not a target register, but a marker.
   The value 0 in the target place of the opcode
   only signals to the CPU, that this Pair is not a
   Copy Pair, and that the source part
   encodes one of 8 so-called signals.
 */

#define RHS_M     1  /* PSEUDO REG Memory - explained above */
#define RHS_E     2  /* BANK REG Exit - explained above */
#define RHS_G     3  /* BANK REG Global - explained above */
#define RHS_L     4  /* BANK REG Local - explained above */
#define RHS_S     5  /* PSEUDO REG Serial - explained above */
#define RHS_P     6  /* PSEUDO REG Parallel - explained above */
#define RHS_R     7  /* REG Return - explained above */

#define RHS_D     8  /* REG Device */

/* The D register is not readable. The
   bit pattern you write into it is used to
   select or deselect hardware devices so
   that you can communicate with them over
   the serial and parallel CPU interface.
*/

#define RHS_J     9  /* PSEUDO REG Jump */

/* This is a write-only pseudo-register.
   Writing a value into J ("xJ") sets the address offset
   of the next instruction to that value.
   The effect of this is that control-flow "jumps" or
   "branches" to that location in the code.
*/

#define RHS_T     10 /* PSEUDO REG True */

/* T is a write-only pseudo-register.
   Writing a value into T ("xT") "jumps" or "branches"
   to that address in the code, *if*
   the R register is not zero (condition "TRUE").

   (It's a convention that a value of zero means logically
   FALSE, and a non-zero value means TRUE.)
*/

#define RHS_F     11 /* PSEUDO REG False */

/* F is a write-only pseudo-register.
   Writing a value into F ("xF") "jumps" or "branches"
   to that address in the code, *if*
   the R register is zero (condition "FALSE").

   (It's a convention that a value of zero means logically
   FALSE, and a non-zero value means TRUE.)
*/

#define RHS_C     12 /* PSEUDO REG Call */

/* C is a write-only pseudo-register.
   Writing a value into C ("xC") does a subroutine call
   to the first offset of that bank.
   Example: "nc 99 NOP" runs the subroutine at the
   beginning of bank 99, and then continues with the NOP.
   (This is the same as executing a TRAP instruction.)
   
   The CPU does two things behind the scenes:

   1. It saves the address offset of the next instruction
   into the R register, so that the CPU can come back
   ("return") to it when the subroutine has finished.
   The address offset then gets set to zero.

   2. It then saves the current instruction bank
   into to E register and places the value written into C
   into the current instruction bank register.

   The next instruction executed is the first instruction
   byte (offset 0) of the requested bank.
   The RET ("Return") instruction reverses this process.
*/

#define RHS_OFFS  13 /* MARKER "Offset" */

/* This is not a target register, but a marker.
   The value 13 in the target place of the opcode
   only signals to the CPU, that this Pair is not a
   Copy Pair, and that the source part
   encodes an offset to be added/subtracted from R.
 */


#define RHS_A     14 /* REG Alpha */
#define RHS_B     15 /* REG Beta */

/* These are the two accumulator registers.
   They have two functions:

   1. The accumulator register last recently written to
   is marked as the WORKING accumulator and by
   definition holds the implied offset for reading
   and writing to and from memory using the M register.

   2. ALU functions take one or both of the accumulator
   registers as arguments. The result is put in R.
   The instruction "IDA" (Identity A) copies A into R.
   The instriction "IDB" (Identity B) copies B into R.

   The GETPUT instructions allow you to copy
   registers A, B and R between the first four locations
   of the Local segment using their shortcut names L0-L3.
   Example: aL0 copies A into L0 (address offset C0h).
   Example: L3b copies G3 (address offset C3h) into B.
   */




/* Three COPY PAIRS are "scrounged" and
   repurposed as additional Signals:

   NM is executed as RET
   MM is executed as LID
   EE is executed as CLIP.

   NM and MM are not implemented because
   the source byte would have to be fetched
   from memory and then stored back into
   memory during the same instruction.
   This was impractical for timing reasons.

   In future CPU extensions, other pairs
   such as GG, LL etc. could also be
   scrounged as these are essentially NOPs. */




uint8_t	pc_offs, rreg,
		pc_bank, ereg,
		dreg,
		greg,
		aacc, bacc, *wptr, // Pointer to most recently set
		wclip,
		lreg,
		tristate,
		sclock,
		serial_in_byte,
		serial_out_byte,
		parallel_bus_out,
		parallel_bus_in,
		zflag,
		alu_offs,
		memory[256*256];

uint8_t quit;

uint8_t par_in()
{
	parallel_bus_in = 255; // Dummy for 8 bit tri-state bus sample
	return parallel_bus_in;
	// Parallel bus device select via bits in D register
}

void par_out( uint8_t source)
{
	// Parallel bus device select via bits in D register
	parallel_bus_out = source;
	tristate = 0;
}

uint8_t fval()
{
	return (uint8_t) rreg + (uint8_t) alu_offs;
}

void calc_zflag()
{
	 zflag = (fval() == 0) ? 1:0;
}

uint16_t effective()
{
	uint8_t addr_high;
	if ( *wptr < 128) addr_high = ereg;
	else if ( *wptr < 192) addr_high = greg;
	else addr_high = lreg;
	return 256*addr_high + *wptr;
}


void reset_cpu()
{
	wptr = &aacc;
	tristate = 1;
}

uint8_t fetch()
{	uint8_t f = memory[256*pc_bank + pc_offs++];
	return f;
}

void exec_TRAP( uint8_t instr)
{
	rreg = pc_offs;
	pc_offs = 0;
	alu_offs = 0;
    ereg = pc_bank;
	pc_bank = instr;
}

void act_pseudo( uint8_t dest, uint8_t source)
{
	switch (dest & 15) {
	case RHS_SIG: break; // SIG (This case handled by caller)
	case RHS_M: memory[effective()] = source; break;
	case RHS_E: ereg = source; break;
	case RHS_G: greg = source; break;
	case RHS_L: lreg = source; break;
	case RHS_S: serial_out_byte = source; quit=1; break;
	case RHS_P: par_out( source); break;
	case RHS_R: rreg = source; alu_offs=0; calc_zflag(); break;
	case RHS_D: dreg = source; break;
	case RHS_J: pc_offs = source; break;
	case RHS_T: if (!zflag) pc_offs = source; break;
	case RHS_F: if (zflag)  pc_offs = source; break;
	case RHS_C: exec_TRAP(source); break;
	case RHS_OFFS: break; // ALU Offset
	case RHS_A: wclip = aacc; aacc = source; wptr = &aacc; break;
	case RHS_B: wclip = bacc; bacc = source; wptr = &bacc; break;
	}
}

uint8_t get_pseudo( uint8_t source)
{
	switch (source) {
	case LHS_N: return fetch();
	case LHS_M: return memory[effective()];
	case LHS_E: return ereg;
	case LHS_G: return greg;
	case LHS_L: return lreg; 
	case LHS_S: return serial_in_byte;
	case LHS_P: return par_in();
	case LHS_R: return fval();
	default: return 0; // Warning dummy
	}
}

void shift_in()
{
	uint8_t bit = 1; // Dummy of a physical bit stream clocked by sclock
	serial_in_byte <<= 1; // MSB first
	serial_in_byte |= bit;
	// Emulates a 74HC595 shift register
	// Serial bus device select via bits in D register

	printf("A=%02X(%d), B=%02X(%d) z=%02X CARRY=%d; f=%02X\n",
	 aacc, aacc, bacc, bacc, zflag, (int) aacc + (int) bacc > 255 ? 1 : 0, fval());
}

void shift_out() {
	uint8_t bit = (serial_out_byte & 128) >> 7; // MSB out first
	serial_out_byte <<= 1;
	// Bit should go out to a physical bit stream clocked by sclock
	// Emulates a 74HC165 shift register
	// Serial bus device select via bits in D register
}


void exec_PAIR( uint8_t instr)
{
	uint8_t dest = instr & 15;
	uint8_t source = (instr >> 4) & 7; /* High order 3 bits */
	if (dest==RHS_SIG) {
		switch (source) {
		case 0: break; /* NOP */
		case 1: shift_in(); break; /* SSI */
		case 2: shift_out(); break; /* SSO */
		case 3: sclock = 0; break; /* SCL */
		case 4: sclock = 1; break; /* SCH */
		case 5: tristate = 1; break; /* HIZ */
		case 6: rreg = memory[lreg*256+0xC4]; alu_offs = 0; lreg++; break; /* LEAVE */
		case 7: lreg--;	memory[lreg*256+0xC4] = rreg ; break; /* ENTER */
		}
	}
	else if (dest==RHS_OFFS) { // Add/Subtract ALU offset
		alu_offs = source < 4 ? source : (uint8_t) (0xF8 + source); // Sign extend
		calc_zflag();
	}
	else { // Transfer
		if ((source == LHS_N) && (dest == RHS_M)) { // Scrounge RET
			pc_offs = rreg;
			pc_bank = ereg;
		}
		else if ((source == LHS_M) && (dest == RHS_M)) {pc_bank++; pc_offs=0;} // Scrounge LID
		else if ((source == LHS_E) && (dest == RHS_E)) {*wptr = wclip;} // Scrounge CLIP
		else act_pseudo( dest, get_pseudo( source));
	}
}

void exec_ALU( uint8_t instr)
{
	uint8_t *loc = NULL;
	uint8_t offs = instr & 3;
	uint8_t alu_opcode = instr >> 4;
	if (alu_opcode == 3)
	{
		switch(instr & 15){
		case 0: rreg = aacc; break; // IDA
		case 1: rreg = bacc; break; // IDB
		case 2: rreg = ~aacc; break; // OCA
		case 3: rreg = ~bacc; break; // OCB
		case 4: rreg = aacc << 1; break; // SLA
		case 5: rreg = bacc << 1; break; // SLB
		case 6: rreg = aacc >> 1; break; // SRA
		case 7: rreg = bacc >> 1; break; // SRB
		case 8: rreg = aacc & bacc; break; // AND
		case 9: rreg = aacc | bacc; break; // IOR
		case 10: rreg = aacc ^ bacc; break; // EOR
		case 11: rreg = aacc + bacc; break; // ADD
		case 12: rreg = (int) aacc + (int) bacc > 255 ? 1 : 0; break; // CARRY
		case 13: rreg = (aacc < bacc) ? 255 : 0; break; // ALB
		case 14: rreg = (aacc == bacc) ? 255 : 0; break; // AEB
		case 15: rreg = (aacc > bacc) ? 255 : 0; break; // AGB
		}
		alu_offs = 0;
		calc_zflag();
	}
	else {
		switch(alu_opcode) {
		case 0: wclip = aacc; wptr = &aacc; loc = wptr; break;
		case 1: wclip = bacc; wptr = &bacc; loc = wptr; break;
		case 2: loc = &rreg; break;
		}
		uint16_t addr = instr & 4 ? // Bit 3 global / locals
			 lreg*256 + 128 + 64 + offs : greg*256 + 128 + offs;
		if (instr & 8) { // PUT
			 if (loc==&rreg) memory[addr] = fval();
			 else memory[addr] = *loc;
		}
		else { // GET
			*loc = memory[addr];
			if (loc == &rreg) {
				alu_offs = 0;
				calc_zflag();
			}
		}
	}
}



void decode()
{
	uint8_t instr = fetch();
	if ((instr & 128) == 0) exec_PAIR( instr ); /* Strip off b7 */
	else if (instr & 64) exec_ALU( instr & 63); /* Strip off b7 and b6 */
	else exec_TRAP( instr & 63); /* Strip off b7 and b6 */
}



//////////////////////////////////////////////////

int
rdimg()
{
	FILE *f;
	f = fopen("daffodil.obj", "r");
	if (f == NULL) return -1;
	fread(memory,1,65535,f);
	fclose(f);
	return 0;
}


int
wrimg()
{
	FILE *f;
	f = fopen("daffodil.obj", "rb+");
	fwrite(memory, 1, 65535, f);
	fclose(f);
	return 0;
}


int main() {
	reset_cpu();

	if (rdimg()) {
	 printf("Missing Daffodil\n");
	exit(0);	
	}


	// Prepare input args
	memory[128+4] = '1';
	memory[129+4] = '1';
	memory[130+4] = '0';
	memory[131+4] = '1';
	memory[132+4] = '1';
	memory[133+4] = '0';
	memory[134+4] = 0;

	for (long i=0; i<100000; i++) {
		decode();
		if (quit) {
			printf("Quit\n");
			break;
		}
	}

	//wrimg();

	printf("\nLocal Section\n");
	printf("Alpha: ");
	for (int i=192; i<256; i++)
		if (memory[i]>31 && memory[i]<127) printf("%c",memory[i]);
	printf("\nDec:\n");
	for (int i=192; i<224; i++)
		printf("%d ",memory[i]);
	printf("\n");
	for (int i=224; i<256; i++)
		printf("%d ",memory[i]);
	printf("\nHex:\n");
	for (int i=192; i<224; i++)
		printf("%02X ",memory[i]);
	printf("\n");
	for (int i=224; i<256; i++)
		printf("%02X ",memory[i]);
	printf("\n\n");

	printf("Global Section\n");
	printf("Alpha: ");
	for (int i=128; i<192; i++)
		if (memory[i]>31 && memory[i]<127) printf("%c",memory[i]);
	printf("\nDec:\n");
	for (int i=128; i<160; i++)
		printf("%d ",memory[i]);
	printf("\n");
	for (int i=160; i<192; i++)
		printf("%d ",memory[i]);
	printf("\nHex:\n");
	for (int i=128; i<160; i++)
		printf("%02X ",memory[i]);
	printf("\n");
	for (int i=160; i<192; i++)
		printf("%02X ",memory[i]);
	printf("\n");
}

