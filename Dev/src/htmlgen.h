
#ifndef __HTMLGEN_H__
#define __HTMLGEN_H__ 1


/* Helper code for generating HTML files in C
   Requires Plan9 build environment
   (https://github.com/9fans/plan9port)

   Change linetrm string for Windows type CRLF line endings
   Change whitesp string if you prefer leading tabs
*/

#include <u.h>
#include <libc.h>

#define MAX_LINE_LENGTH 120

int indentl; /*HTML file indentation level*/
int fdesc; /*Output file descriptor*/
char *linetrm = "\n";  /*Line termination string use Unix*/
char *whitesp = "  "; /*White space string per indentation level*/
char indentstr[ MAX_LINE_LENGTH+1]; /*Current indentation whitespace*/
char strbuf[ MAX_LINE_LENGTH+1];


void line( char* str) /*Adds str at current indent level*/
{
        fprint( fdesc, "%s%s%s", indentstr, str, linetrm);
}

void
addindent() /*Add one level of indentation*/
{
        strcpy( indentstr, "");
        int i;
        if ( (++indentl * strlen( whitesp) >= MAX_LINE_LENGTH)){
                exits("ERR: Max indentation level exceeded");
        } 
        for ( i=0; i<indentl; i++){
                strcat( indentstr, whitesp);
        }
}

void
rmvindent() /*Remove one level of indentation*/
{
        strcpy( indentstr, "");
        int i;
        if ( --indentl < 0){
                exits("ERR: Negative indentation requested");
        } 
        for ( i=0; i<indentl; i++){
                strcat( indentstr, whitesp);
        }
}

void meta(char* str) /*Add meta tag with str argument*/
{
        fprint(fdesc, "%s<meta %s>%s", indentstr, str, linetrm);
}

void lo(char *tag) /*Open tag then newline*/
{
        fprint(fdesc, "%s<%s>%s", indentstr, tag, linetrm);
        addindent();
}

void lc(char *tag) /*Close tag then newline*/
{
        rmvindent();
        fprint(fdesc, "%s</%s>%s", indentstr, tag, linetrm);
}

void tag(char *tag, char *str) /*Enclose str in tag pair*/
{
        fprint(fdesc, "%s<%s>%s</%s>%s", indentstr, tag, str, tag, linetrm);
}

/*
void
main ()
{   
     char *fname = "x.html";
     create(fname, 0, 0666);
     fdesc = open(fname, OWRITE);
     if (fdesc != -1){

        line("<!DOCTYPE html>");
        line("<link rel=\"stylesheet\" href=\"x.css\">");
        lo("html");
            lo("head");
                meta("charset=\"UTF-8\"");
                lo("title");
                line("My webpage");
                lc("title");
            lc("head");
            lo("body");
                lo("ul");
                        tag("li", "My item");
                        tag("li", "My other item");
                lc("ul");
            lc("body");
        lc("html");
     
        
        close(fdesc);
     }
}
*/

#endif
