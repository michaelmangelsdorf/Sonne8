
/* 
    Generate a map file of String Literals (asciimem.txt)
    like mnemonics and numbers to opcodes.
    Also create various other text files such
    as an opcode matrix.

   Myth Project
  Jan-2024 Michael Mangelsdorf
  Copyr. <mim@ok-schalter.de>
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define LHS_N     0
#define LHS_M     1
#define LHS_L     2
#define LHS_G     3
#define LHS_R     4
#define LHS_I     5
#define LHS_S     6
#define LHS_P     7

#define RHS_O     0
#define RHS_M     1
#define RHS_L     2
#define RHS_G     3
#define RHS_R     4
#define RHS_I     5
#define RHS_S     6
#define RHS_P     7
#define RHS_E     8
#define RHS_A     9
#define RHS_D     10
#define RHS_J     11
#define RHS_W     12
#define RHS_T     13
#define RHS_F     14
#define RHS_C     15

/* Generate a table of all possible instruction mnemonics */
/* Array index equates to operation code */

char mnemo_decoder[256][12]; /* Various formats */


char *rhs[] = {
"o", "m", "l", "g",
"r", "i", "s", "p",
"e", "a", "d", "j",
"w", "t", "f", "c"
};

void gen_opcodes()
{
    char *lhs[] = {
     "n", "m", "l", "g",
     "r", "i", "s", "p" 
    };

    char *sys[] = {
     "NOP", "SSI", "SSO", "SCL", "SCH", "RET", "FAR", "ORG",
     "P4", "P1", "P2", "P3", "M4", "M3", "M2", "M1"
    };

    char *aluop[] = {
     "IDR", "IDO", "OCR", "OCO",
     "SLR", "SLO", "SRR", "SRO",
     "AND", "IOR", "EOR", "ADD",
     "CAR", "RLO", "REO", "RGO"
    };

    char numstr[] = "0";




    // SYS instructions (16 opcodes, 0-15)
    for (int i=0; i<16; i++) {
        strcpy(mnemo_decoder[i], sys[i]);
    }

    // ALU instructions (16 opcodes, 16-31)
    for (int i=16; i<32; i++) {
        strcpy( mnemo_decoder[i], aluop[i-16]);
    }

    // TRAP instructions (32 opcodes, 32-63)
    for (int i=32; i<64; i++) {
        sprintf(mnemo_decoder[i], "*%d", i-32);
    }

    // GETPUT instructions (64 opcodes, 64-127)      
    for (int offs=0; offs<8; offs++)
      for (int regn=0; regn<4; regn++)
        for (int gp=0; gp<2; gp++)
                {
                    numstr[0] = 48 + offs; // ASCII 0 + the number
                    unsigned index = regn*16 + gp*8 + offs + 64;
                    if (gp==0) { // GET
                        strcpy( mnemo_decoder[index], numstr);
                        //strcat( mnemo_decoder[index], "]");
                       switch (regn) {
                        case 0: strcat( mnemo_decoder[index], "r"); break;
                        case 1: strcat( mnemo_decoder[index], "o"); break;
                        case 2: strcat( mnemo_decoder[index], "d"); break;
                        case 3: strcat( mnemo_decoder[index], "g"); break;
                        }
                    }
                    else { // PUT
                       switch (regn) {
                        case 0: strcpy( mnemo_decoder[index], "r"); break;
                        case 1: strcat( mnemo_decoder[index], "o"); break;
                        case 2: strcat( mnemo_decoder[index], "d"); break;
                        case 3: strcat( mnemo_decoder[index], "g"); break;
                        }
                        //strcpy( mnemo_decoder[index], "[");
                        strcat( mnemo_decoder[index], numstr);
                    }
                }


    // PAIR 128-255
    for (int j=0; j<8; j++)
    {
           for (int i=0; i<16; i++)
           {
             /*scrounge*/
           if (j==LHS_N && i==RHS_L) strcpy( mnemo_decoder[j*16+i + 128], "INO");
           else if (j==LHS_N && i==RHS_M) strcpy( mnemo_decoder[j*16+i + 128], "DEO");
           else if(j==LHS_L && i==RHS_L) strcpy( mnemo_decoder[j*16+i + 128], "---");
           else if(j==LHS_L && i==RHS_M) strcpy( mnemo_decoder[j*16+i + 128], "---");
           else if(j==LHS_M && i==RHS_L) strcpy( mnemo_decoder[j*16+i + 128], "---");
           else if(j==LHS_M && i==RHS_M) strcpy( mnemo_decoder[j*16+i + 128], "---");
           else if(j==LHS_G && i==RHS_G) strcpy( mnemo_decoder[j*16+i + 128], "---");
           else if(j==LHS_R && i==RHS_R) strcpy( mnemo_decoder[j*16+i + 128], "---");
           else if(j==LHS_I && i==RHS_I) strcpy( mnemo_decoder[j*16+i + 128], "---");
           else {     
                /* Transfers */
                strcpy( mnemo_decoder[j*16+i + 128], lhs[j] );
//                if (j==RHS_B || j==RHS_BR || j==RHS_BRZ || j==RHS_BT) {
//                    strcat( mnemo_decoder[i*16+j], "'");
//                }
                strcat( mnemo_decoder[j*16+i + 128], rhs[i]);
                //printf("%s %d %d\n", mnemo_decoder[j*16+i], j*16, i);
            }
          }
    }

}




#define SYMSIZE 26

/* Generate a dictionary for number names */

char frame_mnemo[256][16][SYMSIZE];
struct {
    char name[SYMSIZE];
    uint8_t val;
    uint8_t frame;
    int type; // bit map, bit 0=export
} defmap[8192];
unsigned defmap_topindex;

#define BYTE_TO_BINARY_PATTERN "%c%c%c%c_%c%c%c%c"
#define BYTE_TO_BINARY(byte)  \
  (byte & 0x80 ? '1' : '0'), \
  (byte & 0x40 ? '1' : '0'), \
  (byte & 0x20 ? '1' : '0'), \
  (byte & 0x10 ? '1' : '0'), \
  (byte & 0x08 ? '1' : '0'), \
  (byte & 0x04 ? '1' : '0'), \
  (byte & 0x02 ? '1' : '0'), \
  (byte & 0x01 ? '1' : '0') 

#define NYBBLE_TO_BINARY_PATTERN "%c%c%c%c"
#define NYBBLE_TO_BINARY(nybble)  \
  (nybble & 0x08 ? '1' : '0'), \
  (nybble & 0x04 ? '1' : '0'), \
  (nybble & 0x02 ? '1' : '0'), \
  (nybble & 0x01 ? '1' : '0') \

    void create_def( char* label, uint8_t val) {
            sprintf(defmap[defmap_topindex].name, "%s", label);
            defmap[defmap_topindex].val = val;
            defmap_topindex++;
    }

    void populate_defmap()
    {
        defmap_topindex = 0;
        /* Create decimal numbers */
        for (int i=0; i<256; i++) {
            sprintf(defmap[defmap_topindex].name, "%d", i);
            defmap[defmap_topindex].val = i;
            defmap_topindex++;
        }

        /* Create negative decimals */
        for (int i=1; i<=128; i++) {
            sprintf(defmap[defmap_topindex].name, "-%d", i);
            defmap[defmap_topindex].val = -i & 0xFF;
            defmap_topindex++;
        }

        /* Create zero-padded hex numbers */
        for (int i=0; i<256; i++) {
            sprintf(defmap[defmap_topindex].name, "%02X", i);
            strcat(defmap[defmap_topindex].name, "h");
            defmap[defmap_topindex].val = i;
            defmap_topindex++;
        }
        /* Create unpadded hex numbers (nybbles) */
        for (int i=0; i<16; i++) {
            sprintf(defmap[defmap_topindex].name, "%X", i);
            strcat(defmap[defmap_topindex].name, "h");
            defmap[defmap_topindex].val = i;
            defmap_topindex++;
        }
        /* Create binary numbers byte */
        for (int i=0; i<256; i++) {
            sprintf(defmap[defmap_topindex].name, BYTE_TO_BINARY_PATTERN, BYTE_TO_BINARY(i));
            strcat(defmap[defmap_topindex].name, "b");
            defmap[defmap_topindex].val = i;
            defmap_topindex++;
        }
        /* Create binary numbers nybble */
        for (int i=0; i<16; i++) {
            sprintf(defmap[defmap_topindex].name, NYBBLE_TO_BINARY_PATTERN, NYBBLE_TO_BINARY(i));
            strcat(defmap[defmap_topindex].name, "b");
            defmap[defmap_topindex].val = i;
            defmap_topindex++;
        }
        /* Create printable ASCII chars */
        for (int i=32; i<127; i++) {
            if (i==34) sprintf(defmap[defmap_topindex].name, "'\"'");
            else sprintf(defmap[defmap_topindex].name, "'%c'", i);
            defmap[defmap_topindex].val = i;
            defmap_topindex++;
        }
        
        /* Create sugar */
        sprintf(defmap[defmap_topindex].name, "'NUL'");
        defmap[defmap_topindex].val = 0;
        defmap_topindex++;
        sprintf(defmap[defmap_topindex].name, "'CR'");
        defmap[defmap_topindex].val = 13;
        defmap_topindex++;
        sprintf(defmap[defmap_topindex].name, "'LF'");
        defmap[defmap_topindex].val = 10;
        defmap_topindex++;


    }

char* tobin( int i, char* mynum){
    mynum[0] = i&128 ? '1' : '0';
    mynum[1] = i&64 ? '1' : '0';
    mynum[2] = ' ';
    mynum[3] = i&32 ? '1' : '0';
    mynum[4] = i&16 ? '1' : '0';
    mynum[5] = i&8 ? '1' : '0';
    mynum[6] = ' ';
    mynum[7] = i&4 ? '1' : '0';
    mynum[8] = i&2 ? '1' : '0';
    mynum[9] = i&1 ? '1' : '0'; 
    mynum[10] = 0;
    return mynum;
}

void byteToBinaryString(unsigned char byte, char binaryString[9]) {
    int i;
    for (i = 7; i >= 0; i--) {
        binaryString[7 - i] = ((byte >> i) & 1) ? '1' : '0';
    }
    binaryString[8] = '\0';
}

int main (int argc, char **argv)
{

    char* mynum[11];
    printf("   String Literal mapper");
    printf("*/\n\n");

    populate_defmap();

    gen_opcodes();
    FILE *f = fopen("opcode_matrix.txt","w");
    fprintf(f,"\n     LSB ");
    for (int i=0; i<16; i++) fprintf(f, NYBBLE_TO_BINARY_PATTERN " ", NYBBLE_TO_BINARY(i));
    fprintf(f,"\n\n         ");
    for (int i=0; i<16; i++) fprintf(f,"%-4X ",i);
    fprintf(f,"\nMSB      ");
    for (int i=0; i<16; i++) fprintf(f,"---- ");
    fprintf(f,"\n");
    for (int i=0; i<16; i++) {
        if (i==1 || i==2 || i==4 || i==8) fprintf(f,"\n");
        fprintf(f, NYBBLE_TO_BINARY_PATTERN " " "%X_  ", NYBBLE_TO_BINARY(i), i);
        for (int j=0; j<16; j++) {
                        fprintf(f, "%-4s ", mnemo_decoder[i*16+j]);
        }
        fprintf(f,"\n");
    }
    fclose(f);


    f = fopen("mnemonics.txt","w");
    for(int i=0; i<=255; i++){
        fprintf(f, "%s\n", mnemo_decoder[i]);
    }
    fclose(f);

    f = fopen("myth_dict","w");
    for(int i=0; i<defmap_topindex; i++){
        fprintf(f, "%02X: %s\n", defmap[i].val, defmap[i].name);
    }
    fclose(f);

    f = fopen("mythdef","w");
    fprintf(f,"/* Struct for looking up opcodes for string literals\n");
    fprintf(f,"   such as numbers and mnemonics */\n");    
    fprintf(f, "struct { uchar val; char *str; } strlits[] = {\n");
    for(int i=0; i<defmap_topindex; i++){
        fprintf(f, "{0x%02X, \"%s\"}, ", defmap[i].val, defmap[i].name);
        if (i%3 == 0) fprintf(f,"\n");
    }
    for (int i=0; i<=255; i++){
        fprintf(f, "{0x%02X, \"%s\"}, ", i, mnemo_decoder[i]);
        if (i%4 == 0) fprintf(f,"\n");
    }
    fprintf(f, "\n};\n");
    fclose(f);


    f = fopen("asciimem.txt","w");
    int addr = 0;
    int page = 0;
    fprintf(f,"LOX Bootstrap Firmware for Myth/nAuthor: mim@ok-schalter.de (Michael/Dosflange@github)\n");
    while (addr<65536) {
           /* if (addr%256 == 0) {fprintf(f,"Page %.02Xh: \n",page);}
           for (int i=0; i<4; i++) {
                    fprintf(f, "%.04Xh:            ", addr++);
            }
            fprintf(f,"\n");
            if (addr%256 == 0) {fprintf(f,"\n"); page++;}*/
           if (addr%256 == 0) {fprintf(f,"         @\n");}
           fprintf(f, "    %.04X@ 00h        ;", addr++);
           fprintf(f,"\n");
           if (addr%256 == 0) {fprintf(f,"\n"); page++;}

    }
    fclose(f);

}



