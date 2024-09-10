
#ifndef __HTMLGEN_H__
#define __HTMLGEN_H__ 1


/* Helper code for generating HTML files in C
   Requires Plan9 build environment
   (https://github.com/9fans/plan9port)

   Change linetrm string for Windows type CRLF line endings
   Change whitesp string if you prefer leading tabs
   Set hrefprefix80 to path prefix excluding /
   Set imgprefix80 to path prefix for image tag src attribs
*/

#include <u.h>
#include <libc.h>

#define MAX_LINE_LENGTH 120

int indentl; /*HTML file indentation level*/
int htmlgen_fdesc; /*Output file descriptor*/
char *linetrm = "\n";  /*Line termination string use Unix*/
char *whitesp = "  "; /*White space string per indentation level*/
char indentstr[ MAX_LINE_LENGTH+1]; /*Current indentation whitespace*/
char strbuf[ MAX_LINE_LENGTH+1];
char imgprefix80[80];

void line( char* str) /*Adds str at current indent level*/
{
        fprint( htmlgen_fdesc, "%s%s%s", indentstr, str, linetrm);
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

void
meta(char* str) /*Add meta tag with str argument*/
{
        fprint(htmlgen_fdesc, "%s<meta %s>%s", indentstr, str, linetrm);
}

void
lo(char *tag, char *attrib) /*Open tag then newline*/
{
        if(strlen(attrib)) fprint(htmlgen_fdesc, "%s<%s %s>%s",
                        indentstr, tag, attrib, linetrm);
        else fprint(htmlgen_fdesc, "%s<%s>%s",
                        indentstr, tag, linetrm);
        addindent();
}

void
lc(char *tag) /*Close tag then newline*/
{
        rmvindent();
        fprint(htmlgen_fdesc, "%s</%s>%s", indentstr, tag, linetrm);
}

void
tag(char *tag, char *attrib, char *str) /*Enclose str in tag pair*/
{
        if(strlen(attrib)!=0) fprint(htmlgen_fdesc, "%s<%s %s>%s</%s>%s",
                indentstr, tag, attrib, str, tag, linetrm);
        else fprint(htmlgen_fdesc, "%s<%s>%s</%s>%s",
                indentstr, tag, str, tag, linetrm);
}

char *
href( char *href, char *str)
{
        sprint(strbuf, "<a href=\"%s\">%s</a>", href, str, href);
        return strbuf;
}


#endif
