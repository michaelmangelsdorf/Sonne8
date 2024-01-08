
/* Assembler for the Sonne micro-controller v2
 * Author: mim@ok-schalter.de (Michael Mangelsdorf)
 * Refer to http://ok-schalter.de/sonne for details
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define LHS_N     0  // 
#define LHS_M     1  // 
#define LHS_F     2  // 
#define LHS_G     3  //
#define LHS_L     4  //
#define LHS_S     5  // 
#define LHS_P     6  // 
#define LHS_R     7  // 

#define RHS_SIG   0  // 
#define RHS_M     1  // 
#define RHS_F     2  // 
#define RHS_G     3  //
#define RHS_L     4  //
#define RHS_S     5  // 
#define RHS_P     6  // 
#define RHS_R     7  // 

#define RHS_D     8  // 
#define RHS_J     9  // 
#define RHS_I     10 // 
#define RHS_E     11 // 
#define RHS_C     12 // 
#define RHS_OFFS  13 //
#define RHS_A     14 //
#define RHS_B     15 //

unsigned pass; /* counts assembly passes */

/* Generate a table of all possible instruction mnemonics */
/* Array index equates to operation code */

char mnemo_decoder[256][12]; /* Various formats */

void gen_opcodes()
{
    char *lhs_T[] = {
     "n", "m", "f", "g", "l", "s", "p", "r"
    };

    char *lhs_S[] = {
     "NOP", "SSI", "SSO", "SCL",
     "SCH", "HIZ", "LEAVE", "ENTER"
    };

    char *rhs[] = {
     "SIG", "m", "f", "g",
     "l", "s", "p", "r",
     "d", "j", "i", "e",
     "c", "OFFS", "a", "b"
    };

    char *aluop[] = {
     "IDA", "IDB", "OCA", "OCB",
     "SLA", "SLB", "SRA", "SRB",
     "AND", "IOR", "EOR", "ADD",
     "CYF", "ALB", "AEB", "AGB"
    };

    char *alu_offs[] = {
     "r0+", "r1+", "r2+", "r3+",
     "r4-", "r3-", "r2-", "r1-"
    };

    char numstr[] = "0";

    // APPLY instructions (opcodes 0-127)
        for (int j=0; j<8; j++)
        {
            for (int i=0; i<16; i++)
           {
            /* Handle "diagonal"/scrounge signals */
            if (i==RHS_M && j==LHS_N) {
                strcpy( mnemo_decoder[j*16+i], "RET" );
                //printf("%s %d %d\n", mnemo_decoder[j*16+i], j*16, i);
            }
            else if (i==RHS_M && j==LHS_M) {
                strcpy( mnemo_decoder[j*16+i], "LID" );
            }
            else if (i==RHS_F && j==LHS_F) {
                strcpy( mnemo_decoder[j*16+i], "CLIP" );
            }
            else if (i==RHS_SIG) { /* Signals */
                strcpy( mnemo_decoder[j*16+i], lhs_S[j]);
                //printf("%s %d %d\n", mnemo_decoder[j*16+i], j*16, i);
            }
            else if (i==RHS_OFFS) { /* ALU increment */
                strcpy( mnemo_decoder[j*16+i], alu_offs[j]);
            }
            else { /* Transfers */ 
                strcpy( mnemo_decoder[j*16+i], lhs_T[j] );
//                if (j==RHS_B || j==RHS_BR || j==RHS_BRZ || j==RHS_BT) {
//                    strcat( mnemo_decoder[i*16+j], "'");
//                }
                strcat( mnemo_decoder[j*16+i], rhs[i]);
                //printf("%s %d %d\n", mnemo_decoder[j*16+i], j*16, i);
            }
        }
    }

    // TRAP instructions (64 opcodes, 128-191)
    for (int i=128; i<192; i++) {
        sprintf(mnemo_decoder[i], "*%d", i-128);
    }

    // GETPUT instructions (64 opcodes, 192-255)
    for (int i=0; i<4; i++)
        for (int j=0; j<2; j++)
            for (int l=0; l<2; l++)
                for (int k=0; k<4; k++)
                {
                    numstr[0] = 48 + k; // ASCII 0 + the number
                    // IB54=AQ, IB3=Global/Local, IB2=Get/Put, IB01=OFFS
                    if (i==3) {
                         // ALU instructions
                         strcpy( mnemo_decoder[i*16 + j*8 + l*4 + k + 192], aluop[ + j*8 + l*4 + k]);
                         //printf("%s\n", mnemo_decoder[i*16 +  + j*8 + l*4 + k + 192]);
                    }
                    else if (j==0) { // GET
                        strcpy( mnemo_decoder[i*16 + j*8 + l*4 + k + 192], l==0 ? "G" : "L");
                        strcat( mnemo_decoder[i*16 + j*8 + l*4 + k + 192], numstr);
                       switch (i) {
                        case 0: strcat( mnemo_decoder[i*16 + j*8 + l*4 + k + 192], "a"); break;
                        case 1: strcat( mnemo_decoder[i*16 + j*8 + l*4 + k + 192], "b"); break;
                        case 2: strcat( mnemo_decoder[i*16 + j*8 + l*4 + k + 192], "r"); break;
                        }
                    }
                    else { // PUT
                       switch (i) {
                        case 0: strcpy( mnemo_decoder[i*16 + j*8 + l*4 + k + 192], "a"); break;
                        case 1: strcpy( mnemo_decoder[i*16 + j*8 + l*4 + k + 192], "b"); break;
                        case 2: strcpy( mnemo_decoder[i*16 + j*8 + l*4 + k + 192], "r"); break;
                        }
                        strcat( mnemo_decoder[i*16 + j*8 + l*4 + k + 192], l==0 ? "G" : "L");
                        strcat( mnemo_decoder[i*16 + j*8 + l*4 + k + 192], numstr);
                    }
                }

}


#define SYMSIZE 26

/* Provide a table for frame mnemonics */

char frame_mnemo[256][16][SYMSIZE];
uint8_t frame_refs[256][16];
uint8_t offslabels;
uint8_t frame_lid[256]; /* #Bytes defined in each frame */
uint8_t frame_mnemo_export[256]; /* Non-zero value: export */

uint8_t theDamned;

/* Generate a dictionary for number names */

struct {
    char name[SYMSIZE];
    uint8_t val;
    uint8_t frame;
    int type; // bit map, bit 0=export
} defmap[8192];
unsigned defmap_topindex;

#define BYTE_TO_BINARY_PATTERN "%c%c%c%c.%c%c%c%c"
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

        /* Create ASCII names and others */

        create_def("ASCII.backspace", 8);
        create_def("ASCII.linefeed", 10);
        create_def("ASCII.escape", 27);
        create_def("ASCII.space", 32);
        create_def("ASCII.exclamation_mark", 33);
        create_def("ASCII.double_quote", 34);
        create_def("ASCII.hashmark", 35);
        create_def("ASCII.dollar", 36);
        create_def("ASCII.percent", 37);
        create_def("ASCII.ampersand", 38);
        create_def("ASCII.single_quote", 39);
        create_def("ASCII.opening_parenthesis", 40);
        create_def("ASCII.closing_parenthesis", 41);
        create_def("ASCII.asterisk", 42);
        create_def("ASCII.plus_sign", 43);
        create_def("ASCII.comma", 44);
        create_def("ASCII.minus_sign", 45);
        create_def("ASCII.full_stop", 46);
        create_def("ASCII.forward_slash", 47);
        
        // ASCII 0-9 48-57

        create_def("ASCII.colon", 58);
        create_def("ASCII.semicolon", 59);
        create_def("ASCII.less_than_sign", 60);
        create_def("ASCII.equal_sign", 61);
        create_def("ASCII.greater_than_sign", 62);
        create_def("ASCII.question_mark", 63);
        create_def("ASCII.at_sign", 64);

        // ASCII A-Z 65-90

        create_def("ASCII.opening_bracket", 91);
        create_def("ASCII.backward_slash", 92);
        create_def("ASCII.closing_bracket", 93);
        create_def("ASCII.caret", 94);
        create_def("ASCII.underscore", 95);
        create_def("ASCII.back_tick", 96);

        // ASCII a-z 97-122

        create_def("ASCII.opening_brace", 123);
        create_def("ASCII.pipe_symbol", 124);
        create_def("ASCII.closing_brace", 125);
        create_def("ASCII.tilde", 126);
        
        create_def("ASCII.del", 127);

        // Create GLOBAL variable names

        create_def("G0", 128);
        create_def("G1", 129);
        create_def("G2", 130);
        create_def("G3", 131);
        create_def("G4", 132);
        create_def("G5", 133);
        create_def("G6", 134);
        create_def("G7", 135);

        // Create LOCAL variable names

        create_def("L0", 192);
        create_def("L1", 193);
        create_def("L2", 194);
        create_def("L3", 195);
        create_def("L4", 196);
        create_def("L5", 197);
        create_def("L6", 198);
        create_def("L7", 199);


    }




char wordbuf[SYMSIZE];
char tempbuf[SYMSIZE];
char *src; /* Will point to in-memory copy of source file */
long srclength; /* Number of bytes in src file */

/*  Read source file into a memory buffer */

    char* readsrcf( const char* fname)
    {
        FILE* f = fopen( fname,"rb");
        if (!f) return NULL;

        fseek( f, 0, SEEK_END);
        srclength = ftell( f) + 1;
        fseek( f, 0, SEEK_SET);

        char *buffer = malloc (srclength);
        if (buffer) {
          fread (buffer, 1, srclength-1, f);
          buffer[srclength]='\0';
        }
        fclose( f);
        return buffer;
    }

uint8_t isinram = 0; // Horrible hack for RAM branch targets, all so broken, so lost

uint8_t ROM_mem[256*256]; /* 128 (ROM only) frames * 128 bytes = 32k */
unsigned ROM_srcLine[256*256]; /* Which line number generated the output byte */

unsigned objcursor; /* holds current object code byte index */
unsigned objframe; /* hold highest object code frame index */

/* Clear structures */

    void clear()
    {
        for (int i=0; i<(256*256); i++) ROM_mem[i] = 0;

        for (int i=0; i<256; i++) 
            strcpy(frame_mnemo[i][offslabels],"");

    }

unsigned cursor; /* holds current source text character index */
unsigned lines; /* tracks number of lines processed */

/* Get a source character relative relative to current position */

    int nextchar( int displacement){
        unsigned index = cursor + displacement;
        if (index < srclength) return src[index];
        else return 0;
    }

/* Move text cursor forward if not outside source buffer */

    int cursor_fwd( int displacement){
    	unsigned index = cursor + displacement;
    	if (index >= srclength) return -1;
    	else cursor = index;
    	return 0;
    }

/* Skip the rest of the current line, including any number of trailing
 * line breaks.
 */

    int skipline(){
    	while (src[cursor]!='\n')
    		if (cursor_fwd(1)) return -1;
    	return 0;
    }

/* Skip any number of space characters */

    int skipspace(){
    	while (src[cursor]==' ' || src[cursor]=='\t')
    		if (cursor_fwd(1)) return -1;
    	return 0;
    }


/* Validate label references */

    int find_backref(char* refstr)
    {
        //printf("BACKW REFSTR=%s\n", refstr);
        unsigned here;
            here = objframe;
            for (int i=here; i>=0; i--) {
                for(int j=0; j<16; j++)
                    if (!strcmp(refstr, frame_mnemo[i][j])) {
                        theDamned = i;
                        return frame_refs[i][j];
                    }
            }
        return -1;
    }

    int find_fwdref(char* refstr)
    {
        //printf("FWD REFSTR=%s\n", refstr);
        unsigned here;
            here = objframe;
            for (int i=here; i<256; i++) {
                for(int j=0; j<16; j++) {
                    //if (!strcmp(refstr, "skip") && i<4)
                    //    printf("i=%d, j=%d, %s, searching for:%s %d\n", i,j, frame_mnemo[i][j], refstr, frame_refs[i][j]);
                    if (!strcmp(refstr, frame_mnemo[i][j])) {
                        theDamned = i;
                        return frame_refs[i][j];
                    }
                }
            }
        return -1;
    }

    int find_mcref(char* refstr)
    {
        //printf("FWD REFSTR=%s\n", refstr);
        unsigned here;
        for (int i=0; i<256; i++)
            for(int j=0; j<16; j++)
                if (!strcmp(refstr, frame_mnemo[i][j])) return i;
        return -1;
    }




/* Try to find the frame and the type of a defmap entry */

    int defmap_frame_and_type(char* buf, uint8_t* typefound)
    {
        for (int i=0; i<defmap_topindex; i++)
        {
            if (!strcmp( defmap[i].name, buf)) {
                //printf ("Found def %s\n", wordbuf);
                *typefound = defmap[i].type;
                return defmap[i].frame;
            }
        }
        return -1;
    }

/* Try to find an entry matching wordbuf in precompiled mnemonics */

    int compare_to_tables(char* buf)
    {
        for (int i=0; i<256; i++)
        {
            if (!strcmp( mnemo_decoder[i], buf)) {
                //printf ("Found mnemonic %s (%02X)\n", wordbuf, i);
                return i;
            }
        }

        for (int i=0; i<defmap_topindex; i++)
        {
            if (!strcmp( defmap[i].name, buf)) {
                //printf ("Found def (or number literal!) %s\n", buf);
                return defmap[i].val;
            }
        }
        return -1;
    }


/* Store an object code byte and move on */

    void store( uint8_t byte)
    {
            ROM_srcLine[objframe * 256 + objcursor] = lines;
            ROM_mem[objframe * 256 + objcursor] = byte;
            frame_lid[objframe]++;

        if (++objcursor > 255) {
            printf("Code frame overflow in line %d\n", lines);
            exit(0);
        }
    }


int labelallowed; /* Only one frame label per full-stop */
int endframe;
int handled;
int ramgap;

/* We assert that this is called when cursor is at the first character
 * of a new line, following a line break character.
 * Assemble this line.
 */

    int beginline()
    {
    	lines++;
        if (src[cursor] == '\n') {
            if (cursor_fwd(1)) return 0; /* skip empty lines */
            else return -1;
        }
        while (skipspace() == 0)  /* Skip leading white space */
        {
            endframe = 0;
            /* We assume that the current word does not end with full-stop */

            handled = 0; /* We haven't handled this word so far */

            /* We are now positioned at the first character of something */
            /* What is it? */

            if (src[cursor] == '\n') { /* It's a newline character, skip it */
                if (cursor_fwd(1)) return 0;
                else return 1; /* Enter next line */
            }
        	
            if (src[cursor] == ';') { /* Beginning of a comment, skip it */
        		if (skipline()) return 0;
                if (cursor_fwd(1)) return 0; /* Skip the newline character */
                else return 1; /* Enter next line */
            }

            /* It's something non-trivial, copy it into the word buffer */
            int k=0;
            while (nextchar(k)!=' ' && nextchar(k)!='\t' && nextchar(k)!='\n')
            {
                wordbuf[k] = nextchar(k);
                if (cursor + k >= srclength) {
                    printf("Unexpected end of file\n");
                    exit(0);
                }
                if (k++ > 79) {
                    printf("Buffer overrun - string literal too long\n");
                    exit(0);
                }
            }
            wordbuf[k] = 0; /* Force null termination */

            if (k>0 && wordbuf[0] == '"') {
                printf("String %s\n", wordbuf);
                // Compile string
                for (int i=1; i<k; i++) {
                    printf("%c ", wordbuf[i]);
                    store(wordbuf[i]);
                }
                printf("\n");
                handled=1;
            }


            if (k>0 && wordbuf[k-1]==',' && wordbuf[0] != '"') wordbuf[k-1]='\0'; /* Ignore commas */
            else
            if (k>0 && wordbuf[k-1]=='.' && wordbuf[0] != '"') {
                wordbuf[k-1]='\0'; /* Strip the full-stop */
                //endframe = 1;
            }

            /* The word is now a zero terminated string in wordbuf */
            /* Now check what we got there... */

            /* First of all, is it short enough to be managed as a symbol? */
            if (strlen(wordbuf)>=SYMSIZE) {
                printf("Max. symbol length exceeded for '%s', line %d\n", wordbuf, lines);
                exit(0);
            }

            if (!strcmp(wordbuf,"RAM")) {
                printf("-RAM-\n");
                //ramgap = objcursor ? objframe : objframe -1;
                //objframe = 128;
                //objcursor = 0;
                handled = 1;
                labelallowed = 1;
                handled = 1;
                isinram = 1;
            }
            

            /* Check for offset label definition */
            if (!handled && strlen(wordbuf)>1) {
                int endpos = strlen(wordbuf)-1;
                if (wordbuf[endpos]=='@' && wordbuf[0] != '"') {
                    wordbuf[endpos] = '\0';
                    /* ! prefix means export this symbol, don't throw away */
                    /* pass ==2 avoid duplicate export symbols */
                    if (wordbuf[0]=='!' && pass==2) {
                    }
                    else frame_mnemo_export[objframe] = 0;
                    strcpy(frame_mnemo[objframe][offslabels], wordbuf);
                    frame_refs[objframe][offslabels] = objcursor;
                    //printf("Defining %s@ objframe=%d offslabels=%d offs:%d\n", wordbuf, objframe, offslabels, objcursor);
                    offslabels++;
                    handled = 1;
                }
            }

            /* Check for offset label #frame reference */
            if (!handled && strlen(wordbuf) && wordbuf[0]=='#') {
                uint8_t type;
                k = defmap_frame_and_type( wordbuf+1, &type);
                if (k != -1) {
                    store(k);
                    handled = 1;
                }
            }

            /* Check for frame label definition */
            if (wordbuf[0]=='@') {
                if (!handled && labelallowed)
                { /* Check if it's a frame label definition @LABEL */
                        if (strlen(wordbuf)<2) {
                            printf("Empty label definition, line %d\n", lines);
                            exit(0);
                        }

                        if (objcursor) {
                           printf("Label definition in anonymous frame, line %d\n** ERROR **\n", lines);
                           exit(0);
                        }

                        int endpos = strlen(wordbuf)-1;
                        if (wordbuf[endpos]=='!'  && wordbuf[0] != '"') {
                                wordbuf[endpos] = '\0';
                                frame_mnemo_export[objframe] = 1;
                                //printf("Found export label: %s\n", wordbuf);
                        }
                        else frame_mnemo_export[objframe] = 0;
                        strcpy(frame_mnemo[objframe][offslabels], wordbuf+1);
                        frame_refs[objframe][offslabels] = objframe;
                        //printf("Defining @%s objframe=%d offslabels=%d\n", wordbuf+1, objframe, offslabels);
                        offslabels++;
                        labelallowed = 0; /* only 1 frame label per full stop */
                        handled = 1;
                }
                if (!handled)
                {
                    printf("Duplicate frame label %s, line %d\n** ERROR **\n", wordbuf, lines);
                    exit(0);
                }
            }

            if (!handled) /* Check if it's a label reference <LABEL or >LABEL */
            {
                if (wordbuf[0]=='<') {
                    if (strlen(wordbuf)<2) {
                        printf("Empty back reference, line %d\n** ERROR **\n", lines);
                        exit(0);
                    }
                    k = find_backref(wordbuf+1);
                    if (theDamned == objframe && k>=objcursor) k=-1; //Handle offset labels
                    handled = 1;
                }
                if (wordbuf[0]=='>') {
                    if (strlen(wordbuf)<2) {
                        printf("Empty forward reference, line %d\n** ERROR **\n", lines);
                        exit(0);
                    }
                    k = find_fwdref(wordbuf+1);
                    if (theDamned == objframe && k<objcursor) k=-1; //Handle offset labels
                    //printf("searching for >%s\n",wordbuf+1);
                    handled = 1;
                }
                if (handled && k == -1) {
                    if (pass==2) {
                        printf("Unresolved reference '%s', line %d\n** ERROR **\n", wordbuf, lines);
                        exit(0);
                    }
                }
                if (handled) {
                     store(isinram ? k+128 : k);
                     //printf("k was %d for %s\n", k, wordbuf);   
                }
            }


            if (!handled) {
                /* Check precompiled tables of mnemonics and number literals */
                k = compare_to_tables(wordbuf);
                if (k != -1) {
                    store(k);
                    handled = 1;
                }
            }

            if (!handled) {
                /* See if it contains an equal sign (a DEF) */
                char *p = strstr(wordbuf,"=");
                char *p_equalsign = p;
                if (p) {
                    p++;
                    k = 0;
                    do tempbuf[k++] = *p; while (*(p++) != '\0');
                    
                    /* tempbuf now holds the thing that the DEFined label */
                    /* is supposed to equate to */
                    /* Look it up to see if we know what they mean */
                    k = compare_to_tables(tempbuf);
                    if (k == -1)
                    {
                        printf("Unknown assignment value: %s, line %d\n** ERROR **\n", tempbuf, lines);
                        exit(0);
                    }
                    /* OBliterate the = character in wordbuf to leave only DEF label */
                    *p_equalsign = '\0';

                    if (compare_to_tables(wordbuf) != -1) {
                            printf("WARNING: Label definition %s redefines previous!\n", wordbuf);
                    }

                    /* ! prefix means export this symbol, don't throw away */
                    /* pass ==1 avoid duplicate export symbols */
                    if (pass==1){
                        if (wordbuf[0]=='!') {
                            defmap[defmap_topindex].type |= 1;
                            /* Skip ! */
                            strcpy( defmap[defmap_topindex].name, wordbuf+1 );
                        }
                        else strcpy(defmap[defmap_topindex].name, wordbuf);
                        defmap[defmap_topindex++].val = k;
                    }        
                    handled = 1;
                }
            }

            if (!handled) {
                if (wordbuf[0]=='*') {
                    if (strlen(wordbuf)<3) {
                        printf("Empty micro-call reference, line %d\n** ERROR **\n", lines);
                        exit(0);
                    }
                    /* Check if it's a micro-call */
                    k = find_mcref(wordbuf+1);
                    if (k != -1) {
                        handled = 1;
                        store(k | 0x80);
                        //store(k); // Fling is gone
                    } else {
                        if (pass==2) {
                            printf("Unresolved micro-call reference '%s', line %d\n** ERROR **\n", wordbuf, lines);
                            exit(0);
                        }
                    }
                }
            }

            if (!handled) {
                if (wordbuf[0]=='"') {
                    if (strlen(wordbuf)<2) {
                        printf("Empty string literal, line %d\n** ERROR **\n", lines);
                        exit(0);
                    }
                    for (k=1; k<strlen(wordbuf); k++) store(wordbuf[k]);
                    handled = 1;
                }
            }

            if (!handled && !strcmp(wordbuf,"CLOSE")) handled=1;

            if (!handled)
            //if (!handled && pass==2)
            {
               printf("Unknown symbol: %s, line %d\n", wordbuf, lines);
            }
            

            /*  -- Below only finishing up with the current word -- */

            /* Any full-stop closes the frame AFTER handling the preceding word
               Full-stop is an alias for LID, RET also closes the frame
             */
            if (endframe || !strcmp(wordbuf,"CLOSE"))
            {
                    if (objframe+1 > 255){
                        printf("Frame index overflow in line %d\n** ERROR **\n", lines);
                        exit(0);
                    }

                if (endframe) store(RHS_M * 16 + LHS_M); // LID opcode
                
                if (endframe || !strcmp(wordbuf,"CLOSE"))
                objframe++;
                objcursor = 0;
                labelallowed = 1;
                endframe = 0;
                offslabels = 0;
            }

            /* Skip over it, watch out it may be followed by newline */
            while (src[cursor]!=' ' && src[cursor]!='\t')
            {
                if (src[cursor]=='\n') {
                    if (cursor_fwd(1)) return 0; /* End of line */
                    else return -1;
                }
                else if (cursor_fwd(1)) return 0;
            }
            /* Try the next word */
        }
        return 0;
    }

void
mifgen( char *fname)
{
    FILE *f;
    int i;
    f = fopen( fname, "wb");
    if (f) {
        fprintf( f, "-- MCBastard output image\n\n");
        fprintf( f, "DEPTH = %d;\n", 0x10000);
        fprintf( f, "WIDTH = 8;\n");
        fprintf( f, "ADDRESS_RADIX = HEX;\n");
        fprintf( f, "DATA_RADIX = HEX;\n");
        fprintf( f, "CONTENT\n");
        fprintf( f, "BEGIN\n\n");

        for (i = 0; i<256*256; i++) {
            fprintf( f, "%04X : %02X;\n", i, ROM_mem[ i]);
        }

        fprintf( f, "\nEND;\n");
        fclose( f);
    }
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

int main (int arc, char **argv)
{
    char* mynum[11];
	printf("Assembler\n");
	printf("(See http://ok-schalter.de/sonne for details)\n\n");
    char* asmfname = "daffodil.asm";
	src = readsrcf( asmfname);
    if (!src) {
        printf("Requires input file '%s'\n\n", asmfname);
        
        exit(0);
    }
    srclength--;
	printf("%ld bytes read\n", srclength);

	clear();
    pass = 0;



    gen_opcodes(); //for (int i=0; i<128; i++) printf("%s\n", mnemo_decoder[i]);
    FILE *f = fopen("opcode_matrix.txt","w");
    fprintf(f,"\n      LSB ");
    for (int i=0; i<16; i++) fprintf(f, NYBBLE_TO_BINARY_PATTERN " ", NYBBLE_TO_BINARY(i));
    fprintf(f,"\n\n          ");
    for (int i=0; i<16; i++) fprintf(f,"%-4X ",i);
    fprintf(f,"\nMSB       ");
    for (int i=0; i<16; i++) fprintf(f,"---- ");
    fprintf(f,"\n");
    for (int i=0; i<16; i++) {
        if (i==8 || i==12) fprintf(f,"\n");
        fprintf(f, NYBBLE_TO_BINARY_PATTERN " " "%X_   ", NYBBLE_TO_BINARY(i), i);
        for (int j=0; j<16; j++) {
                        fprintf(f, "%-4s ", mnemo_decoder[i*16+j]);
                        if (j==0 && i!=6 && i!=7) fprintf(f," "); // LEAVE ENTER size
        }
        fprintf(f,"\n");
    }

    fprintf(f,"\n\n\n\n");

    


    fclose(f);



    populate_defmap(); //for (int i=0; i<defmap_topindex; i++) printf("%s %d\n", defmap[i].name, defmap[i].val);

    for (pass=1; pass<3; pass++) {
        printf("Pass %d\n", pass);

        for (int i=0; i<256; i++) frame_lid[i] = 0;
        for (int i=0; i<256; i++) ROM_srcLine[i] = 0;

        ramgap = -1;
        cursor = 0;
        objcursor = 0;
        objframe = 0;
        lines=0;
        labelallowed=1;
        offslabels=0;
    	while (beginline()); /* Traverse source-text by lines */
    }

	    printf("%d lines processed\n", lines);

        /* Write the lid values into pos 7Fh of each frame.
         This convention serves to make use of the usually large
        proportion of unused space after the initial train of code.
        The lid value at a fixed location provides the offset of
        the first unused byte in each frame. */

        for (int i=0; i<256; i++) {
            ROM_mem[ 256*i + 0xFF ] = frame_lid[i]; 
        }

        /* Generate exported symbols table */
        #define BIG 16*1024
        char symtab[BIG];
        memset( symtab, 0, BIG);

        /* Format symbols as: STRING SPACE BYTE <NEXT STRING> */
        unsigned symtab_index = 0;


        symtab[symtab_index++] = '\0'; // Instructions
        symtab[symtab_index++] = 1;
        for (int i=0; i<256; i++) {
            sprintf(tempbuf, "%s %c", mnemo_decoder[i], i);
            for (int j=0; j<strlen(mnemo_decoder[i])+2; j++) symtab[symtab_index++] = tempbuf[j];
        }

        symtab[symtab_index++] = '\0'; // Frame labels
        symtab[symtab_index++] = 3;
        for (int i=0; i<objframe; i++)
            if (frame_mnemo_export[i]) {
                 sprintf(tempbuf, "%s %c", frame_mnemo[i][offslabels], i);
                for (int j=0; j<strlen(frame_mnemo[i][offslabels])+2; j++) symtab[symtab_index++] = tempbuf[j];
            }

        symtab[symtab_index++] = '\0'; // Definitions
        symtab[symtab_index++] = 4;
        for (int i=0; i<defmap_topindex; i++) {
            if (strlen(symtab)>=BIG) {
                printf("Exported symbols table exceeds 16k limit!\n");
                exit(0);
            }
            if (defmap[i].type & 1) {
                sprintf(tempbuf, "%s %c", defmap[i].name, defmap[i].val); 
                for (int j=0; j<strlen(defmap[i].name) +2; j++) {
                    symtab[symtab_index++] = tempbuf[j];
                }
            }
        }

        printf("Export symbols table: %d bytes\n", symtab_index);
        printf("\n");

        /* Cram exported symbols table into unused gap in object file */
        /* Use the lid values of each frame for this. */
        /* Space from offset "lid" to offset 7Fh of each frame is empty! */

        uint8_t offs;
        uint8_t val, tablecode=0;
        unsigned max_index = symtab_index;
        symtab_index = 0;
        int i;
//        for (i=0; i<128; i++) {
//            offs =  ROM_mem[128*i + 0x7F]; /* Set offs to frame lid */
//            if (offs==0x7F) {
//                 printf("Full frame error - lid convention!\n");
//                 exit(0);
//            }
//            while (offs < 0x7F) {
//                val = symtab[symtab_index++];
//                ROM_mem[128*i + offs++] = val;
//                if (symtab_index == max_index) break;
//            }
//            if (symtab_index == max_index) break;
//        }


    printf("Writing object file...\n\n");

    f = fopen("daffodil.obj","w");
    fwrite( ROM_mem, 1, 256*256, f);
    fclose(f);

    //mifgen("debug.mif"); // MIF file for Quartus import

      /* Output a concordance for debugging */

    f = fopen("debug.txt","w");

    fprintf(f, "\nAssembly output (pass 2)\n");
    fprintf(f, "\nSuppressing output of embedded export symbols!\n\n");
    //fprintf(f, "Frame.Offset      Object Code       Line#  Source text\n\n");
    fprintf(f, "Frame.Offset     Object Code        Lid Line#  Source text\n\n");
    int k = 0;
    lines = 1;
    int outputlen;
    int lead_frame=0, lead_offset=0;

    while (k<srclength) {

        /* Write object code generated for line*/

        lead_frame = -1;
        lead_offset = -1;
        for (int frame=0; frame<=objframe; frame++)
            for (int offset=0; offset<256; offset++)
                if (ROM_srcLine[frame*256 + offset] == lines) {
                    if (lead_frame == -1) lead_frame = frame;
                    if (lead_offset == -1) lead_offset = offset;
                }

        if (lead_frame != -1 || lead_offset != -1)
        {
            fprintf(f, "%02X.%02X:  ", lead_frame, lead_offset);
        }
        else fprintf(f, "        ");

        outputlen = 0;
        int linefill = 0;
        for (int frame=0; frame<=objframe; frame++) {
            for (int offset=0; offset<256; offset++) {
                if (ROM_srcLine[frame*256 + offset] == lines) {
                    if (!(++linefill%8)) {
                        fprintf(f, "\n        ");
                        outputlen -= 21;
                    }
                    lead_frame = frame;
                    lead_offset = offset;
                    fprintf(f, "%02X ", ROM_mem[frame*256+offset]);
                    outputlen += 3;
                    if (offset>frame_lid[frame]) break;
                }
            }
        }

        if (outputlen) outputlen = outputlen>(24+3) ? 24 : outputlen-3;
        else outputlen = -3;

        /* pad missing hex output */
        for (int i=(24-outputlen); i>0; i--) fprintf(f, " ");
        if ( lead_frame != -1 && frame_lid[lead_frame])
            fprintf(f, " %3d %04d  ", frame_lid[lead_frame], lines);
        else
            fprintf(f, "     %04d  ", lines);

        while (k<srclength && src[k++] != '\n')
        {
            fprintf(f, "%c", src[k-1]);
        }
        lines++;
        fprintf(f, "\n");
    }

    fclose(f);

}
