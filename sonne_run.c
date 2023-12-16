
// Sonne CPU Simulator (rev. Abuladdin)
// 2023 Dec 16  Michael Mangelsdorf

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>


uint8_t	pc_low, pc_low_copy,
		pc_high, pc_high_copy,
		rreg,
		dreg,
		greg,
		aacc, bacc, *wptr, // Pointer to most recently set
		sptr,
		tristate,
		sclock,
		serial_in_byte,
		serial_out_byte,
		zflag,
		alu_op, alu_copy, alu_lock,
		memory[256*256];


uint8_t par_in()
{
	return 0;
	// Parallel bus device select via bits in D register
}

void par_out( uint8_t source)
{
	tristate = 0;
	// Parallel bus device select via bits in D register
}

uint8_t alu_result()
{
	if (alu_lock) return alu_copy;
	uint8_t offs = alu_op & (15<<4);
	uint8_t op = alu_op & 15;
	switch(op){
	case 0: alu_copy = aacc; break; // IDA
	case 1: alu_copy = bacc; break; // IDB
	case 2: alu_copy = ~aacc; break; // OCA
	case 3: alu_copy = ~bacc; break; // OCB
	case 4: alu_copy = aacc << 1; break; // SLA
	case 5: alu_copy = bacc << 1; break; // SLB
	case 6: alu_copy = aacc >> 1; break; // SRA
	case 7: alu_copy = bacc >> 1; break; // SRB
	case 8: alu_copy = aacc & bacc; break; // AND
	case 9: alu_copy = aacc | bacc; break; // IOR
	case 10: alu_copy = aacc ^ bacc; break; // EOR
	case 11: alu_copy = aacc + bacc; break; // ADD
	case 12: alu_copy = (int) aacc + (int) bacc > 255 ? 1 : 0; break; // CYB
	case 13: alu_copy = aacc < bacc ? 255 : 0; break; // ALB
	case 14: alu_copy = aacc == bacc ? 255 : 0; break; // AEB
	case 15: alu_copy = aacc > bacc ? 255 : 0; break; // AGB
	}
	alu_copy += offs;
	zflag = (alu_copy == 0) ? 1:0;
	return alu_copy;
}


uint16_t effective()
{
	uint8_t addr_high;
	if ( *wptr < 128) addr_high = rreg;
	else if ( *wptr < 192) addr_high = greg;
	else addr_high = sptr;
	return 256*addr_high + *wptr;
}


void reset_cpu()
{
	wptr = &aacc;
	tristate = 1;
}

uint8_t fetch()
{
	return memory[256*pc_high + pc_low++];
}

void exec_TRAP( uint8_t instr)
{
	uint8_t addr_high = instr;
	pc_low_copy = pc_low;
	pc_low = 0;	
	pc_high_copy = pc_high;
	pc_high = addr_high;
}

void act_pseudo( uint8_t dest, uint8_t source)
{
	switch (dest & 15) {
	case 0: break; // SIG This case handled by caller
	case 1: rreg = source; break; // R
	case 2: memory[effective()] = source; break; // M
	case 3: pc_low_copy = source; break; // X
	case 4: pc_high_copy = source; break; // Y
	case 5: serial_out_byte = source; break; // S
	case 6: par_out( source); break; // P
	case 7: alu_op=source; alu_lock=0; break; // F
	case 8: dreg = source; break; // D
	case 9: greg = source; break; // G
	case 10: pc_high = source; pc_low = 0; break; // J
	case 11: if (!zflag) pc_high = source; pc_low = 0; break; // T
	case 12: if (zflag) pc_high = source; pc_low = 0; break; // E
	case 13: exec_TRAP( source); break; // C
	case 14: aacc = source; wptr = &aacc; alu_lock=1; break; // A
	case 15: bacc = source; wptr = &bacc; alu_lock=1; break; // B
	}
}



uint8_t get_pseudo( uint8_t source)
{
	switch (source & 7) {
	case 0: return fetch(); // L
	case 1: return rreg; // R
	case 2: return memory[effective()]; // M
	case 3: return pc_low_copy; // X
	case 4: return pc_high_copy; // Y
	case 5: return serial_in_byte; // S
	case 6: return par_in(); // P
	default: return alu_result(); // F
	}
}

void shift_in()
{
	uint8_t bit = 1; // Dummy of a physical bit stream clocked by sclock
	serial_in_byte <<= 1; // MSB first
	serial_in_byte |= bit;
	// Emulates a 74HC595 shift register
	// Serial bus device select via bits in D register
}

void shift_out() {
	uint8_t bit = (serial_out_byte & 128) >> 7; // MSB out first
	serial_out_byte <<= 1;
	// Bit should go out to a physical bit stream clocked by sclock
	// Emulates a 74HC165 shift register
	// Serial bus device select via bits in D register
}


void exec_SIGNAL( uint8_t instr)
{
	uint8_t dest = instr & 15;
	uint8_t source = (instr & 127) >> 4; /* Low order 3 bits */
	if (dest) { // Transfer
		if ((source == 0) && (dest == 2)) { // Scrounge RET
			pc_low = pc_low_copy;
			pc_high = pc_high_copy;
		}
		else if ((source == 2) && (dest == 2)) pc_high++; // Scrounge LID
		else act_pseudo( dest, get_pseudo( source));
	}
	else {
		switch (source) {
		case 0: break; /* NOP */
		case 1: shift_in(); break; /* SCI */
		case 2: shift_out(); break; /* SCO */
		case 3: sclock = 0; break; /* SCL */
		case 4: sclock = 1; break; /* SCH */
		case 5: tristate = 1; break; /* OFF */
		case 6: sptr++; break; /* LEAVE */
		case 7: sptr--; break; /* ENTER */
		}
	}
}

void exec_GETPUT( uint8_t instr)
{
	uint8_t offs = instr & 7;
	if (instr & 32) { /* B register */
		if (instr & 16) { /* Local */
			if (instr & 8) memory[sptr*256 + 128 + 64 + offs] = bacc;
			else bacc = memory[sptr*256 + 128 + 64 + offs];
		}
		else { /* Global */
			if (instr & 8) memory[greg*256 + 128 + offs] = bacc;
			else bacc = memory[greg*256 + 128 + offs];
		}
	}
	else { /* A register */
		if (instr & 16) { /* Local */
			if (instr & 8) memory[sptr*256 + 128 + 64 + offs] = aacc;
			else aacc = memory[sptr*256 + 128 + 64 + offs];
		}
		else { /* Global */
			if (instr & 8) memory[greg*256 + 128 + offs] = aacc;
			else aacc = memory[greg*256 + 128 + offs];
		}
	}
}



void decode()
{
	uint8_t instr = fetch();
	if ((instr & 128) == 0) exec_SIGNAL( instr & 0x127); /* Strip off b7 */
	else if (instr & 64) exec_GETPUT( instr & 63); /* Strip off b7 and b6 */
	else exec_TRAP( instr & 63); /* Strip off b7 and b6 */
}



int
rdimg()
{
	FILE *f;
	f = fopen("daffodil", "r");
	if (f == NULL) return -1;
	fread(memory,1,65535,f);
	fclose(f);
	return 0;
}


int
wrimg()
{
	FILE *f;
	f = fopen("daffodil", "rb+");
	fwrite(memory, 1, 65535, f);
	fclose(f);
	return 0;
}


int main() {
	reset_cpu();

	if (rdimg()) {
	 printf("Missing Daffofil\n");
	exit(0);	
	}

	for (int i=0; i<65535; i++) decode();

	wrimg();
}







