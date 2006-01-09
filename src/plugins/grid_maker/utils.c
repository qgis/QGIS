/* Taken from txt2dbf 1.0.2 by Frank Koormann, see
 * http://www.usf.uni-osnabrueck.de/~fkoorman/software/dbftools.en.html
 */

/*
 * $Source$
 * 
 * $Author$
 * 
 * $Revision$
 *
 * Description: utils.c
 *	collection of useful functions:
 *	- getline
 *	- do_nothing	
 *	- tabtok
 *	explanations see below. *
 *
 */

#include "utils.h"

/* getline -------------------------------------------------------------*/
/* reads a line (limited by \n) out of stream fp, returns last 
 * char, esp eof                                                        */
int getline(FILE *fp, char s[] )
{	int c, i;

	i=0;
	while ( (c=getc(fp))!=EOF && c!='\n' )
		s[i++]=c;
	s[i]='\0';
	return c;
}

/* do_nothing ----------------------------------------------------------*/
/* dto.									*/
void do_nothing( void )
{
}

/* tabtok --------------------------------------------------------------*/
/* like strtok, breaks a string in sequences delimited by tabs, but do not
 * overreads sequences of directly followed tabs: like "\t\t\ttest" is
 * divided by strtok into "test" but by tabtok into "", "", "", "test"  */
char *tabtok( char *s ) 
{        
        static char *b, *e; 
 
        if ( s == NULL ) 
                b = e; 
        else 
                b = s; 
 
        if ( b == NULL ) 
                return b; 
        else 
                e = b;
         
        while ( (*e != '\t') && (*e != '\0') )
                e++; 
         
        if ( *e == '\0' ) 
                e = NULL; 
        else 
                *e = '\0', e++;
 
        return b; 
} 

/* dtok --------------------------------------------------------------*/
/* like strtok, breaks a string in sequences delimited by delim, but do not
 * overreads sequences of directly followed delims: like "\t\t\ttest" is
 * divided by strtok into "test" but by dtok into "", "", "", "test"  */
char *dtok( char *s, char delim ) 
{        
        static char *b, *e; 
 
        if ( s == NULL ) 
                b = e; 
        else 
                b = s; 
 
        if ( b == NULL ) 
                return b; 
        else 
                e = b;
         
        while ( (*e != delim ) && (*e != '\0') )
                e++; 
         
        if ( *e == '\0' ) 
                e = NULL; 
        else 
                *e = '\0', e++;
 
        return b; 
} 
