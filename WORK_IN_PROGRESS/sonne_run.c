
// Sonne CPU Simulator (rev. Daffodil)
// 2023 Dec 16  Michael Mangelsdorf

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#define LHS_N     0  // 
#define LHS_M     1  // 
#define LHS_R     2  // 
#define LHS_G     3  //
#define LHS_L     4  //
#define LHS_S     5  // 
#define LHS_P     6  // 
#define LHS_F     7  // 

#define RHS_SIG   0  // 
#define RHS_M     1  // 
#define RHS_R     2  // 
#define RHS_G     3  //
#define RHS_L     4  //
#define RHS_S     5  // 
#define RHS_P     6  // 
#define RHS_F     7  // 

#define RHS_D     8  // 
#define RHS_J     9  // 
#define RHS_T     10 // 
#define RHS_E     11 // 
#define RHS_INC   12 // 
#define RHS_DEC   13 //
#define RHS_A     14 //
#define RHS_B     15 //

uint8_t	pc_low, pc_low_copy_f,
		pc_high, pc_high_copy_r,
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

uint8_t fval()
{
	return (uint8_t) alu_copy + (uint8_t) alu_offs;
}

void calc_zflag()
{
	 zflag = (fval() == 0) ? 1:0;
}

uint16_t effective()
{
	uint8_t addr_high;
	if ( *wptr < 128) addr_high = pc_high_copy_r;
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
	pc_low_copy_f = pc_low;
	pc_low = 0;	
    pc_high_copy_r = pc_high;
	pc_high = addr_high;
}

void act_pseudo( uint8_t dest, uint8_t source)
{
	switch (dest & 15) {
	case RHS_SIG: break; // SIG (This case handled by caller)
	case RHS_M: memory[effective()] = source; break;
	case RHS_R: pc_high_copy_r = source; break;
	case RHS_G: greg = source; break;
	case RHS_L: sptr = source; break;
	case RHS_S: serial_out_byte = source; quit=1; break;
	case RHS_P: par_out( source); break;
	case RHS_F: alu_copy = source; alu_offs=0; calc_zflag(); break;
	case RHS_D: dreg = source; break;
	case RHS_J: pc_low = source; break;
	case RHS_T: if (!zflag) pc_low = source; break;
	case RHS_E: if (zflag)  pc_low = source; break;
	case RHS_INC: break; // ALU increment, handled
	case RHS_DEC: break; // ALU increment, handled
	case RHS_A: aacc = source; wptr = &aacc; break;
	case RHS_B: bacc = source; wptr = &bacc; break;
	}
}

uint8_t get_pseudo( uint8_t source)
{
	switch (source) {
	case LHS_N: return fetch();
	case LHS_M: return memory[effective()];
	case LHS_R: return pc_high_copy_r;
	case LHS_G: return greg;
	case LHS_L: return sptr; 
	case LHS_S: return serial_in_byte;
	case LHS_P: return par_in();
	case LHS_F: return fval();
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


void exec_SIGNAL( uint8_t instr)
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
		case 6: sptr++;
				break; /* LEAVE */
		case 7: sptr--;
				break; /* ENTER */
		}
	}
	else if (dest==RHS_INC) { // Add ALU offset
		alu_offs = source;
		//printf("Alu offs = %02X, resylt=%d\n", alu_offs, fval());
		calc_zflag();
	}
	else if (dest==RHS_DEC) { // Subtract ALU offset
		alu_offs = (uint8_t) (0xF8 + source); // Sign extend
		//printf("Alu offs = %02X, resylt=%d\n", alu_offs, fval());
		calc_zflag();
	}
	else { // Transfer
		if ((source == LHS_N) && (dest == RHS_M)) { // Scrounge RET
			pc_low = pc_low_copy_f;
			pc_high = pc_high_copy_r;
		}
		else if ((source == LHS_M) && (dest == RHS_M)) {pc_high++; pc_low=0;} // Scrounge LID
		else if ((source == LHS_R) && (dest == RHS_R)) {exec_TRAP( *wptr); }  // Scrounge CALL
		else if ((source == LHS_G) && (dest == LHS_G)) {pc_low=0; pc_high = *wptr;} // Scrounge EXIT
		else act_pseudo( dest, get_pseudo( source));
	}
}

void exec_GETPUT( uint8_t instr)
{
	uint8_t *act = NULL;
	uint8_t offs = instr & 3;
	switch(instr >> 4) {
		case 0: wptr = &aacc; act = wptr; break;
		case 1: wptr = &bacc; act = wptr; break;
		case 2: act = &alu_copy; break;
		case 3: //ALU ops
				switch(instr & 15){
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
		break;
	}
	if (act!=NULL) {
		uint16_t addr = instr & 8 ? // Bit 4 global / locals
			 sptr*256 + 128 + 64 + offs : greg*256 + 128 + offs;
		if (instr & 4)
			 if (act==&alu_copy) memory[addr] = fval();
			 else memory[addr] = *act; // Bit 3 get / put
		else {
			*act = memory[addr];
			if (act==&alu_copy) {
				alu_offs = 0;
				calc_zflag();
			}
		}
	}
}



void decode()
{
	uint8_t instr = fetch();
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
	 printf("Missing Daffodil\n");
	exit(0);	
	}


	// Prepare input args
	memory[128] = '1';
	memory[129] = '1';
	memory[130] = '0';
	memory[131] = '1';
	memory[132] = '1';
	memory[133] = '0';
	memory[134] = 0;

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

