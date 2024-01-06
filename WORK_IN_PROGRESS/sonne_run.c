
// Sonne CPU Simulator (rev. Daffodil)
// 2023 Dec 16  Michael Mangelsdorf

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#define LHS_N     0  // Pseudo source register stands for Number literal
#define LHS_M     1  // Pseudo source register stands for Memory cell
#define LHS_F     2  // ("Frame") high-order address prefix for offsets 0-127
#define LHS_G     3  // ("Global") high-order prefix for offsets 128-191
#define LHS_L     4  // ("Local") high-order prefix for offsets 192-255
#define LHS_S     5  // ("Serial")
#define LHS_P     6  // ("Parallel")
#define LHS_R     7  // ("Result") ALU result

#define RHS_SIG   0  // ("Signals")
#define RHS_M     1  // ("Memory")
#define RHS_F     2  // ("Frame")
#define RHS_G     3  // ("Global")
#define RHS_L     4  // ("Local")
#define RHS_S     5  // ("Serial")
#define RHS_P     6  // ("Parallel")
#define RHS_R     7  // ("Result")

#define RHS_D     8  // ("Device")
#define RHS_J     9  // ("Jump")
#define RHS_I     10 // ("If")
#define RHS_E     11 // ("Else")
#define RHS_C     12 // ("Call")
#define RHS_OFFS  13 // ("ALU Offset")
#define RHS_A     14 // ("A accumulator")
#define RHS_B     15 // ("B Accumulator")

uint8_t	pc_offs, pc_offs_copy_r,
		pc_bank, pc_bank_copy_f,
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
		alu_offs, alu_R,
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
	return (uint8_t) alu_R + (uint8_t) alu_offs;
}

void calc_zflag()
{
	 zflag = (fval() == 0) ? 1:0;
}

uint16_t effective()
{
	uint8_t addr_high;
	if ( *wptr < 128) addr_high = pc_bank_copy_f;
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
	pc_offs_copy_r = pc_offs;
	pc_offs = 0;	
    pc_bank_copy_f = pc_bank;
	pc_bank = instr;
}

void act_pseudo( uint8_t dest, uint8_t source)
{
	switch (dest & 15) {
	case RHS_SIG: break; // SIG (This case handled by caller)
	case RHS_M: memory[effective()] = source; break;
	case RHS_F: pc_bank_copy_f = source; break;
	case RHS_G: greg = source; break;
	case RHS_L: lreg = source; break;
	case RHS_S: serial_out_byte = source; quit=1; break;
	case RHS_P: par_out( source); break;
	case RHS_R: alu_R = source; alu_offs=0; calc_zflag(); break;
	case RHS_D: dreg = source; break;
	case RHS_J: pc_offs = source; break;
	case RHS_I: if (!zflag) pc_offs = source; break;
	case RHS_E: if (zflag)  pc_offs = source; break;
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
	case LHS_F: return pc_bank_copy_f;
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
		case 6: pc_offs_copy_r = memory[lreg*256+0xC4]; lreg++; break; /* LEAVE */
		case 7: lreg--;	memory[lreg*256+0xC4] = pc_offs_copy_r ; break; /* ENTER */
		}
	}
	else if (dest==RHS_OFFS) { // Add/Subtract ALU offset
		alu_offs = source < 4 ? source : (uint8_t) (0xF8 + source); // Sign extend
		calc_zflag();
	}
	else { // Transfer
		if ((source == LHS_N) && (dest == RHS_M)) { // Scrounge RET
			pc_offs = pc_offs_copy_r;
			pc_bank = pc_bank_copy_f;
		}
		else if ((source == LHS_M) && (dest == RHS_M)) {pc_bank++; pc_offs=0;} // Scrounge LID
		else if ((source == LHS_F) && (dest == RHS_F)) {*wptr = wclip;} // Scrounge CLIP
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
		case 0: alu_R = aacc; break; // IDA
		case 1: alu_R = bacc; break; // IDB
		case 2: alu_R = ~aacc; break; // OCA
		case 3: alu_R = ~bacc; break; // OCB
		case 4: alu_R = aacc << 1; break; // SLA
		case 5: alu_R = bacc << 1; break; // SLB
		case 6: alu_R = aacc >> 1; break; // SRA
		case 7: alu_R = bacc >> 1; break; // SRB
		case 8: alu_R = aacc & bacc; break; // AND
		case 9: alu_R = aacc | bacc; break; // IOR
		case 10: alu_R = aacc ^ bacc; break; // EOR
		case 11: alu_R = aacc + bacc; break; // ADD
		case 12: alu_R = (int) aacc + (int) bacc > 255 ? 1 : 0; break; // CYB
		case 13: alu_R = (aacc < bacc) ? 255 : 0; break; // ALB
		case 14: alu_R = (aacc == bacc) ? 255 : 0; break; // AEB
		case 15: alu_R = (aacc > bacc) ? 255 : 0; break; // AGB
		}
		alu_offs = 0;
		calc_zflag();
	}
	else {
		switch(alu_opcode) {
		case 0: wclip = aacc; wptr = &aacc; loc = wptr; break;
		case 1: wclip = bacc; wptr = &bacc; loc = wptr; break;
		case 2: loc = &alu_R; break;
		}
		uint16_t addr = instr & 8 ? // Bit 3 global / locals
			 lreg*256 + 128 + 64 + offs : greg*256 + 128 + offs;
		if (instr & 4) { // PUT
			 if (loc==&alu_R) memory[addr] = fval();
			 else memory[addr] = *loc;
		}
		else { // GET
			*loc = memory[addr];
			if (loc == &alu_R) {
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

