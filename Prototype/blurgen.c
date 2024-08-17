
#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <ctype.h>
#include <math.h>

/*

This program generates the 1MB ROM image for the
Sonne controller's ALU binary look-up ROM 3 ("BLUR3")

Author: mim@ok-schalter.de (Michael Mangelsdorf)
Project homepage: http://ok-schalter.de/sonne

Generate 16 maps, each 256*256 bytes long

Each map corresponds to a binary ALU operation.
Burn the resulting object file to an AT27C080 1MB OTP ROM.

Alternatively, since these ROMs are expensive (>10 USD)
use the optional IO board which replaces the ROM,
see project website

Mnemonics:

"IDQ", "IDA", "OCQ", "OCA",
         "SLQ", "SLA", "SRQ", "SRA",
         "AND", "IOR", "EOR", "ADD",
         "CYF", "QLA", "QEA", "QGA"

IDQ/ICA Identity value of Q/A
OCQ/OCA Ones complement Q/A
SLQ Shift left Q register
SRQ Shift right Q register etc.
CYF Carry flag of sum Q+A
QLA Q less than A
QEA Q equals A
QGA Q greater than A

0: IDQ   8: AND
1: IDA   9: IOR
2: OCQ   10: EOR
3: OCA   11: ADD
4: SLQ   12: CYF (value, 0 or 1)
5: SLA   13: QLA (flag, 0 or FF)
6: SRQ   14: QEA (flag, 0 or FF)
7: SRA   15: QGA (flag, 0 or FF)

*/

int main( int argc, char* argv[0]){
	
      FILE* f;
      unsigned d;
      uint8_t a, b, t, r;

      printf("Generating blur3.obj (16*256*256=1MB)\n");
      
      f = fopen("blur3.obj","w+");

      // Horrible code, but verified output

      /* IDA */
      for (b=0; b<=255; b++)
      {
        for (a=0; a<=255; a++) {
          fprintf( f, "%c", a);
          if (a == 255) break;
        }
        if (b == 255) break;
      }
        
       /* IDB  */
      for (b=0; b<=255; b++)
      {
        for (a=0; a<=255; a++) {
          fprintf( f, "%c", b);
          if (a == 255) break;
        }
        if (b == 255) break;
      }


      /* OCA */
      for (b=0; b<=255; b++)
      {
        for (a=0; a<=255; a++) {
          fprintf( f, "%c", ~a);
          if (a == 255) break;
        }
        if (b == 255) break;
      }
        
       /* OCB  */
      for (b=0; b<=255; b++)
      {
        for (a=0; a<=255; a++) {
          fprintf( f, "%c", ~b);
          if (a == 255) break;
        }
        if (b == 255) break;
      }

      /* Shift left A */
      for (b=0; b<=255; b++)
      {
        for (a=0; a<=255; a++) {
          r = a << 1;
          fprintf( f, "%c", r);
          if (a == 255) break;
        }
        if (b == 255) break;
      }

      /* Shift left B */
      for (b=0; b<=255; b++)
      {
        for (a=0; a<=255; a++) {
          r = b << 1;
          fprintf( f, "%c", r);
          if (a == 255) break;
        }
        if (b == 255) break;
      }

      /* Shift right A */
      for (b=0; b<=255; b++)
      {
        for (a=0; a<=255; a++) {
          r = a >> 1;
          fprintf( f, "%c", r);
          if (a == 255) break;
        }
        if (b == 255) break;
      }

      /* Shift right B */
      for (b=0; b<=255; b++)
      {
        for (a=0; a<=255; a++) {
          r = b >> 1;
          fprintf( f, "%c", r);
          if (a == 255) break;
        }
        if (b == 255) break;
      }

       /* A AND B  */
      for (b=0; b<=255; b++)
      {
        for (a=0; a<=255; a++) {
          fprintf( f, "%c", a & b);
          if (a == 255) break;
        }
        if (b == 255) break;
      }

       /* A IOR B  */
      for (b=0; b<=255; b++)
      {
        for (a=0; a<=255; a++) {
          fprintf( f, "%c", a | b);
          if (a == 255) break;
        }
        if (b == 255) break;
      }

       /* A EOR B  */
      for (b=0; b<=255; b++)
      {
        for (a=0; a<=255; a++) {
          fprintf( f, "%c", a ^ b);
          if (a == 255) break;
        }
        if (b == 255) break;
      }

       /* A ADD B low order  */
      for (b=0; b<=255; b++)
      {
        for (a=0; a<=255; a++) {
          r = a + b;
          fprintf( f, "%c", r);
          if (a == 255) break;
        }
        if (b == 255) break;
      }

       /* A ADD B high order (carry) */
      for (b=0; b<=255; b++)
      {
        for (a=0; a<=255; a++) {
          d = a + b;
          r = (d > 255) ? 1 : 0;
          fprintf( f, "%c", r);
          if (a == 255) break;
        }
        if (b == 255) break;
      }

//      /* A SUM B signed overflow (overflow) */
//      for (b=0; b<=255; b++)
//      {
//        for (a=0; a<=255; a++) {
//          r = 0; // assume no overflow, adding mixed sign cannot overflow
//          if ((a>127)
//               && (b>127)
//               && (((a+b)<128))) r = 0xFF; // adding two negatives must be negative
//          else if ((a<128)
//                   && (b<128)
//                   && ((a+b)>127)) r = 0xFF; // adding two positives must be positive
//          fprintf( f, "%c", r);
//          if (a == 255) break;
//        }
//        if (b == 255) break;
//      }     

      /* A less than B / A minus B unsigned overflow */
      for (b=0; b<=255; b++)
      {
        for (a=0; a<=255; a++) {
          r = (a<b) ? 0xFF : 0;
          fprintf( f, "%c", r);
          if (a == 255) break;
        }
        if (b == 255) break;
      }

      /* A equals B */
      for (b=0; b<=255; b++)
      {
        for (a=0; a<=255; a++) {
          r = (a==b) ? 0xFF : 0;
          fprintf( f, "%c", r);
          if (a == 255) break;
        }
        if (b == 255) break;
      }

      /* A greater than B / B minus A unsigned overflow */
      for (b=0; b<=255; b++)
      {
        for (a=0; a<=255; a++) {
          r = (a>b) ? 0xFF : 0;
          fprintf( f, "%c", r);
          if (a == 255) break;
        }
        if (b == 255) break;
      }



      fclose( f);
}






