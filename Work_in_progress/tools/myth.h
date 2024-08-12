
#include <u.h>
#include <libc.h>

enum myth_io_event {SELECT, POSEDGE, NEGEDGE, BUSY, READY, DONE};

struct myth_vm
{
        uchar pagebyte[256][128]; /*32K*/
        uchar globalbyte[256][64]; /*16K*/
        uchar localbyte[256][64]; /*16K*/

        uchar xMx;  /*Implied memory address offset*/

        /*Callback function for IO events*/
        void (*iocb)(struct myth_vm*, enum myth_io_event);

        uchar e;    /*Device ENABLE register */

        uchar sclk; /*Serial clock state bit*/
        uchar miso; /*Serial input line state bit*/
        uchar mosi; /*Serial output line state bit*/
        uchar sir;  /*Serial input register, modelled after HC595*/
        uchar sor;  /*Serial output register, modelled after HC165*/

        uchar busy; /*POR inverted tristate flag (0:tristate)*/
        uchar pir;  /*Parallel input register*/
        uchar por;  /*Parallel output register*/

        uchar j;    /*JUMP register (program counter)*/
        uchar o;    /*ORIGIN register (pending program counter)*/
        uchar c;    /*CALL register (current code page index)*/
        uchar d;    /*DATA register (pending code page index)*/

        uchar g;    /*GLOBAL segment index*/
        uchar l;    /*LOCAL segment index*/

        uchar a;    /*A accumulator register*/
        uchar b;    /*B accumulator register*/
        uchar r;    /*RESULT accumulator register*/
        uchar i;    /*INNER register (loop counter)*/
};

void myth_reset(struct myth_vm *vm);

uchar myth_fetch(struct myth_vm *vm);
void myth_step(struct myth_vm *vm);

void myth_exec_pair(struct myth_vm *vm, uchar opcode);
void myth_exec_gput(struct myth_vm *vm, uchar opcode);
void myth_exec_trap(struct myth_vm *vm, uchar opcode);
void myth_exec_alu(struct myth_vm *vm, uchar opcode);
void myth_exec_adj(struct myth_vm *vm, uchar opcode);
void myth_exec_sys(struct myth_vm *vm, uchar opcode);

void myth_call(struct myth_vm *vm, uchar dstpage);
void myth_ret(struct myth_vm *vm);

void myth_register_iocb(struct myth_vm *vm,
        void (*iocb)(struct myth_vm*, enum myth_io_event));


/*  The 'REGx' notation means: REG into (something)
    i.e. REG is a source
*/

#define Nx 0 /*from code literal (NUMBER)*/
#define Mx 1 /*from MEMORY*/
#define Dx 2 /*from DATA page index*/
#define Ox 3 /*from ORIGIN offset*/
#define Rx 4 /*from RESULT accumulator*/
#define Ix 5 /*from VALUE register*/
#define Sx 6 /*from SERIAL input*/
#define Px 7 /*from PARALLEL input*/


/*  The 'xREG' notation means: (something) into REG
    i.e. REG is a destination
*/

#define xG 0  /*to GLOBAL segment index*/
#define xM 1  /*to MEMORY*/
#define xD 2  /*to DATA page index*/
#define xO 3  /*to ORIGIN offset*/
#define xR 4  /*to RESULT accumulator*/
#define xI 5  /*to VALUE register*/
#define xS 6  /*to SERIAL output*/
#define xP 7  /*to PARALLEL output*/
#define xE 8  /*to ENABLE register*/
#define xJ 9  /*to JUMP page index*/
#define xW 10 /*to WHILE page index*/
#define xT 11 /*to TRUE page index*/
#define xF 12 /*to FALSE page index*/
#define xC 13 /*to CALL page index*/
#define xA 14 /*to A accumulator*/
#define xB 15 /*to B accumulator*/

#define IDA 0 /*Identity A*/
#define IDB 1 /*Identity B*/
#define OCA 2 /*Ones' complement of A*/
#define OCB 3 /*Ones' complement of B*/
#define SLA 4 /*Shift left A*/
#define SLB 5 /*Shift left B*/
#define SRA 6 /*Shift right A*/
#define SRB 7 /*Shift right B*/
#define AND 8 /*A AND B*/
#define IOR 9 /*A OR B*/
#define EOR 10 /*A XOR B*/
#define ADD 11 /*A + B*/
#define CAR 12 /*Carry of A + B (0 or 1)*/
#define ALB 13 /*255 if A<B else 0*/
#define AEB 14 /*255 if A=B else 0*/
#define AGB 15 /*255 if A>B else 0*/

#define RET 0 /*Return from CALL*/
#define SSI 1 /*Serial Shift In*/
#define SSO 2 /*Serial Shift Out*/
#define SCL 3 /*Set serial Clock Low*/
#define SCH 4 /*Set serial Clock High*/
#define RDY 5 /*Ready/Tristate*/
#define NEW 6 /*Create stack frame*/
#define OLD 7 /*Resume stack frame*/




void
myth_reset(struct myth_vm *vm)
{
        vm->c = 0;
        vm->j = 0;

        vm->xMx =0;

        vm->sclk = 0;
        vm->miso = 0;
        vm->mosi = 0;
        vm->sir = 0;
        vm->sor = 0;

        vm->busy = 0;
        vm->pir = 0;
        vm->por = 0;
}


uchar*
myth_cmemptr(struct myth_vm *vm) /*Return effective CODE memory pointer*/
{
        if(vm->j < 0x80)
                return &(vm->pagebyte[ vm->c][ vm->j]);

        else if (vm->j < 0xC0)
                return &(vm->globalbyte[ vm->g][ vm->j - 0x80]);
        else
                return &(vm->localbyte[ vm->l][ vm->j - 0xC0]);
}


uchar*
myth_dmemptr(struct myth_vm *vm) /*Return effective DATA memory pointer*/
{
        if(vm->xMx < 0x80)
                return &(vm->pagebyte[ vm->d][ vm->xMx]);
        
        else if(vm->xMx < 0xC0)
                return &(vm->globalbyte[ vm->g][ vm->xMx - 0x80]);
        else
                return &(vm->localbyte[ vm->l][ vm->xMx - 0xC0]);
}


uchar
myth_fetch(struct myth_vm *vm) /*Fetch next byte in CODE stream, increment PC*/
{
        uchar val = *myth_cmemptr(vm);
        (vm->j)++;
        return val;
}


void
myth_step(struct myth_vm *vm)
{

        uchar opcode = myth_fetch(vm);

        /*SCROUNGING (remap certain PAIR combinations)*/

        #define scrounge_NM 0x80 + xM>>3 + Nx
        if(opcode == scrounge_NM) return; /*remap NM to NOP*/

        #define scrounge_MM 0x80 + xM>>3 + Mx
        if(opcode == scrounge_MM)
                if (vm->iocb) (*(vm->iocb))(vm, DONE);

        /*Decode priority encoded opcode*/
        /*Execute decoded instruction*/

        if(opcode&0x80) myth_exec_pair(vm, opcode);
        else if(opcode&0x40) myth_exec_gput(vm, opcode);
        else if(opcode&0x20) myth_exec_trap(vm, opcode);
        else if(opcode&0x10) myth_exec_alu(vm, opcode);
        else if(opcode&0x08) myth_exec_adj(vm, opcode);
        else myth_exec_sys(vm, opcode);
}


void
myth_call(struct myth_vm *vm, uchar dstpage)
{
        vm->o = vm->j;
        vm->j = 0;
        vm->d = vm->c;
        vm->c = dstpage;
}


uchar
myth_exec_pair_srcval(struct myth_vm *vm, uchar opcode)
{

        uchar srcreg = opcode & 7; /*Zero except low order 3 bits*/
        switch(srcreg){
                case Nx: return myth_fetch(vm); /*pseudo reg*/
                case Mx: return *myth_dmemptr(vm); /*pseudo reg*/
                case Dx: return vm->d;
                case Ox: return vm->o;
                case Rx: return vm->r;
                case Ix: return vm->i;
                case Sx: return vm->sir;
                case Px: return vm->pir;

                #define DUMMY 0
                default: return DUMMY;
                 /*Never reached, silence compiler warning*/
        }
}


void
myth_exec_pair(struct myth_vm *vm, uchar opcode)
{
        uchar srcval = myth_exec_pair_srcval(vm, opcode);
        uchar dstreg = (opcode >> 3) & 15; /* Zero except bits 3-6 at LSB*/
        switch(dstreg){
                case xG: vm->g = srcval; break;
                case xM: *myth_dmemptr(vm) = srcval; break; /*pseudo reg*/
                case xD: vm->d = srcval; break;
                case xO: vm->o = srcval; break;
                case xR: vm->r = srcval; break;
                case xI: vm->i = srcval; break;
                case xS: vm->sor = srcval; break;
                case xP:
                        vm->por = srcval;
                        vm->busy = 1;
                        if(vm->iocb) (*(vm->iocb))(vm, BUSY);
                        break;

                case xE:
                        vm->e = srcval;
                        if(vm->iocb) (*(vm->iocb))(vm, SELECT);
                        break;

                case xJ: /*pseudo reg*/
                        vm->j += (signed char) srcval;
                        break;

                case xW: /*pseudo reg*/
                        if (vm->i) vm->j += (signed char) srcval;
                        (vm->i)--; /*Post decrement always*/
                        break; 

                case xT: /*pseudo reg*/
                        if (vm->r) vm->j += (signed char) srcval;
                        break;

                case xF: /*pseudo reg*/
                        if (!vm->r) vm->j = (signed char) srcval;
                        break;

                case xC: myth_call(vm, srcval); break; /*pseudo reg*/
                case xA:
                        vm->a = srcval;
                        vm->xMx = srcval;
                        break;
                case xB:
                        vm->b = srcval;
                        vm->xMx = srcval;
                        break;
        }
}


void
myth_exec_gput(struct myth_vm *vm, uchar opcode) /*Execute GETPUT instruction*/
{
        /* OPCODE
            BITS 0-1 encode registers ABRV
            BIT 2 encodes GET/PUT mode
            BIT 3 encodes GLOBAL/LOCAL segment
            BITS 4-5 encode address offset
        */

        uchar *mptr;
        uchar offs = (opcode >> 4) & 3; /*Zero except bits 4-5 at LSB*/
        
        if(opcode & 8) mptr = &(vm->localbyte[vm->l][0xFC + offs]);
        else
                mptr = &(vm->globalbyte[vm->g][0xFC + offs]);
        
        if(opcode & 4)
                switch(opcode & 3){ /*Zero except low order 2 bits*/
                        case 0: *mptr = vm->a;
                        case 1: *mptr = vm->b;
                        case 2: *mptr = vm->r;
                        case 3: *mptr = vm->i;
                }
        else
                switch(opcode & 3){ /*Zero except low order 2 bits*/
                        case 0: vm->a = *mptr;
                        case 1: vm->b = *mptr;
                        case 2: vm->r = *mptr;
                        case 3: vm->i = *mptr;
                }
}


void
myth_exec_trap(struct myth_vm *vm, uchar opcode)
{
        uchar dstpage = opcode & 31; /*Zero except low order 5 bits*/
        
        /* Split TRAP address rage evenly between RAM and ROM
           0-15: pages 112-127 (ROM - MSB clear)
           16-31: pages 240-255 (RAM - MSB set)
        */

        if (dstpage<16) dstpage += 112;
        else dstpage += 240;

        myth_call(vm, dstpage);
}


void
myth_exec_alu(struct myth_vm *vm, uchar opcode)
{
        switch(opcode & 15){/* Zero except low order 4 bits*/
                case IDA: vm->r = vm->a; break;
                case IDB: vm->r = vm->b; break;
                case OCA: vm->r = ~vm->a; break;
                case OCB: vm->r = ~vm->b; break;
                case SLA: vm->r = vm->a << 1; break;
                case SLB: vm->r = vm->b << 1; break;
                case SRA: vm->r = vm->a >> 1; break;
                case SRB: vm->r = vm->b >> 1; break;
                case AND: vm->r = vm->a & vm->b; break;
                case IOR: vm->r = vm->a | vm->b; break;
                case EOR: vm->r = vm->a ^ vm->b; break;
                case ADD: vm->r = vm->a + vm->b; break;
                case CAR:
                        vm->r = (int) vm->a + (int) vm->b > 255 ? 1 : 0;
                        break;
                case ALB: vm->r = (vm->a < vm->b) ? 255 : 0; break;
                case AEB: vm->r = (vm->a == vm->b) ? 255 : 0; break;
                case AGB: vm->r = (vm->a > vm->b) ? 255 : 0; break;
        }
}


void /*Adjust R by sign-extended offset*/
myth_exec_adj(struct myth_vm *vm, uchar opcode)
{
        switch(opcode & 7){ /*Zero except low order 3 bits*/
                case 0: vm->r += 4; break;
                case 1: vm->r += 1; break;
                case 2: vm->r += 2; break;
                case 3: vm->r += 3; break;
                case 4: vm->r -= 4; break;
                case 5: vm->r -= 3; break;
                case 6: vm->r -= 2; break;
                case 7: vm->r -= 1; break;
        }
}


void
myth_exec_sys(struct myth_vm *vm, uchar opcode)
{
        switch(opcode & 7){ /*Zero except low order 3 bits*/
                case RET:
                        vm->c = vm->d;
                        vm->j = vm->o;
                        break;
                case SSI:
                        /*Clocks in MISO line bit into LSB*/
                        vm->sir = ((vm->sir)<<1) + vm->miso;
                        break;
                case SSO:
                        vm->mosi = (vm->sor)&0x80 ? 1:0;
                        vm->sor <<= 1; /*Clocks out MSB first*/
                        break;
                case SCL:
                        if(vm->sclk){
                                if(vm->iocb) (*(vm->iocb))(vm, NEGEDGE);
                                vm->sclk = 0;
                        }
                        break;
                case SCH:
                        if(!vm->sclk){
                                if(vm->iocb) (*(vm->iocb))(vm, POSEDGE);
                                vm->sclk = 1;
                        }
                        break;
                case RDY:
                        if(vm->busy){
                                if(vm->iocb) (*(vm->iocb))(vm, READY);
                                vm->busy = 0;
                        }
                        break;

                case NEW: (vm->l)--; break;
                case OLD: (vm->l)++; break;
        }
}


void /*Register callback function for IO notifications*/
myth_register_iocb(struct myth_vm *vm,
        void (*iocb_func)(struct myth_vm*, enum myth_io_event))
{
        vm->iocb = iocb_func;
}




