
// Sonne CPU Simulator (rev. Daffodil)
// 2023 Dec 16  Michael Mangelsdorf

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>


uint8_t	pc_low, pc_low_copy,
		pc_high,
		rreg,
		dreg,
		greg,
		aacc, bacc, *wptr, // Pointer to most recently set
		sptr,
		tristate,
		sclock,
		serial_in_byte,
		serial_out_byte,
		parallel_bus_out,
		parallel_bus_in,
		zflag,
		alu_offs, alu_copy,
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
	tristate = 0;
	parallel_bus_out = source;
	// Parallel bus device select via bits in D register
}

void calc_zflag()
{
	 zflag = ((uint8_t) ((uint8_t) alu_copy + (uint8_t) alu_offs) == 0) ? 1:0;
}

uint16_t effective()
{
	uint8_t addr_high = 0;
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
{	uint8_t f = memory[256*pc_high + pc_low++];
	return f;
}

void exec_TRAP( uint8_t instr)
{
	uint8_t addr_high = instr;
	pc_low_copy = pc_low;
	pc_low = 0;	
    memory[sptr*256 + 255] = pc_high;
	pc_high = addr_high;
}

void act_pseudo( uint8_t dest, uint8_t source)
{
	switch (dest & 15) {
	case 0: break; // SIG This case handled by caller
	case 1: rreg = source; break; // R
	case 2: memory[effective()] = source; break; // M
	case 3: pc_low_copy = source; break; // X
	case 4: break; // ALU increment This case handled by caller
	case 5: serial_out_byte = source; quit=1; break; // S
	case 6: par_out( source); break; // P
	case 7: alu_copy = source; alu_offs = 0; break;
	case 8: dreg = source; break; // D
	case 9: greg = source; break; // G
	case 10: pc_low = source; break; // J
	case 11: if (!zflag) pc_low = source; break; // T
	case 12: if (zflag)  pc_low = source; break; // E
	case 13: break; // ALU decrement THis case handled
	case 14: aacc = source; wptr = &aacc; break; // A
	case 15: bacc = source; wptr = &bacc; break; // B
	}
}



uint8_t get_pseudo( uint8_t source)
{
	switch (source) {
	case 0: return fetch(); // N
	case 1: return rreg; // R
	case 2: return memory[effective()]; // M
	case 3: return pc_low_copy; // X
	case 4: return 0; break; // ALU case handled 
	case 5: return serial_in_byte; // S
	case 6: return par_in(); // P
	default: return alu_copy + alu_offs; // F
	}
}

void shift_in()
{
	uint8_t bit = 1; // Dummy of a physical bit stream clocked by sclock
	serial_in_byte <<= 1; // MSB first
	serial_in_byte |= bit;
	// Emulates a 74HC595 shift register
	// Serial bus device select via bits in D register

	printf("A=%02X(%d), B=%02X(%d) z=%02X CARRY=%d\n",
	 aacc, aacc, bacc, bacc, zflag, (int) aacc + (int) bacc > 255 ? 1 : 0);
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
	uint8_t source = (instr >> 4) & 7; /* High order 3 bits */
	if (source==4) {
		switch(dest){
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
		case 13: alu_copy = (aacc < bacc) ? 255 : 0; break; // ALB
		case 14: alu_copy = (aacc == bacc) ? 255 : 0; break; // AEB
		case 15: alu_copy = (aacc > bacc) ? 255 : 0; break; // AGB
		}
		alu_offs = 0;
		calc_zflag();
	}
	else if (dest==0) { // SIG
		switch (source) {
		case 0: break; /* NOP */
		case 1: shift_in(); break; /* SSI */
		case 2: shift_out(); break; /* SSO */
		case 3: sclock = 0; break; /* SCL */
		case 4: sclock = 1; break; /* SCH */
		case 5: tristate = 1; break; /* HIZ */
		case 6: sptr++;
			    pc_low_copy = memory[sptr*256 + 254];
				break; /* LEAVE */
		case 7: memory[sptr*256 + 254] = pc_low_copy;
				sptr--;
				break; /* ENTER */
		}
	}
	else if (dest==4) {
		alu_offs = source; // ALU increment
		calc_zflag();
	}
	else if (dest==13) {
		alu_offs = source | (31 << 3); // ALU decrement
		calc_zflag();
	}
	else { // Transfer
		if ((source == 0) && (dest == 2)) { // Scrounge RET
			pc_low = pc_low_copy;
			pc_high = memory[sptr*256 + 255];
		}
		else if ((source == 2) && (dest == 2)) {pc_high++; pc_low=0;} // Scrounge LID
		else if ((source == 1) && (dest == 1)) {exec_TRAP( *wptr); } // Scrounge CALL
		else act_pseudo( dest, get_pseudo( source));
	}
}


void exec_GETPUT( uint8_t instr)
{
	uint8_t offs = instr & 7;
	if (instr & 32) { /* B register */
		if (instr & 16) { /* Local */
			if (instr & 8) memory[sptr*256 + 128 + 64 + offs] = bacc;
			else {
				bacc = memory[sptr*256 + 128 + 64 + offs];
				wptr = &bacc;
			}
		}
		else { /* Global */
			if (instr & 8) memory[greg*256 + 128 + offs] = bacc;
			else {
				bacc = memory[greg*256 + 128 + offs];
				wptr = &bacc;
			}
		}
	}
	else { /* A register */
		if (instr & 16) { /* Local */
			if (instr & 8) memory[sptr*256 + 128 + 64 + offs] = aacc;
			else {
				aacc = memory[sptr*256 + 128 + 64 + offs];
				wptr = &aacc;
			}
		}
		else { /* Global */
			if (instr & 8) {
				memory[greg*256 + 128 + offs] = aacc;
			}
			else {
				aacc = memory[greg*256 + 128 + offs];
				wptr = &aacc;
			}
		}
	}
}



void decode()
{
	uint8_t instr = fetch();
	//printf("%02X\n", instr);
	if ((instr & 128) == 0) exec_SIGNAL( instr ); /* Strip off b7 */
	else if (instr & 64) exec_GETPUT( instr & 63); /* Strip off b7 and b6 */
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
	 printf("Missing Daffofil\n");
	exit(0);	
	}

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

