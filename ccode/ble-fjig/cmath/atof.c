/*
 * atof.c
 */

#include <stdlib.h>
#include <math.h>
//#include "printf.h"

#define PZERO 38			/* index of 1e0 in powten[]	*/
#define PMAX 76				/* highest index of powten[]	*/
#define HUGE_NOR 1.70141183460469228		/* normalised HUGE	*/

extern const double powten[];

double atof( const char *s ) /* convert string s to double */
{
    double val;
    int i, j, sign, esign, ex, ex1;

/* skip white space */
    for( i = 0; s[i] == ' ' || s[i] == '\n' || s[i] == '\t'; i++ )
    	;

/* set sign to be +1 or -1 */
    sign = 1;
    if( s[i] == '+' || s[i] == '-' )    /* sign */
	sign = ( s[i++] == '+') ? 1 : -1;

/* read digits and build value for as long as there are digits */
    ex = PZERO - 1;
    for( val = 0.0, j = PZERO; s[i] >= '0' && s[i] <= '9'; i++) 
    {
	if( j >= 0 )
	{
	    val += powten[j] * (s[i] - '0');
	}
	j--;
	ex++;
    }

/* if first non-digit is decimal point skip it and keep going */
/* if it is not a decimal point we fall through to test for exponent */
    if( s[i]  ==  '.' )
	i++;

/* continue to build value while the digits keep coming */
    for( ; s[i] >= '0' && s[i] <= '9'; i++ )
    {
	if( j >= 0 )
	    val += powten[j] * (s[i] - '0');
	j--;
    }

/* if non-digit was an exponent flag deal with exponent */
    if( s[i] == 'e' || s[i] == 'E' ) 
    {
	i++;
	esign = 1;

/* check for explicit + or - sign in exponent and deal with it */
	if( s[i]  ==  '+' || s[i]  ==  '-' )
	    esign = (s[i++] == '+') ? 1 : -1;
	for( ex1 = 0; s[i] >= '0' && s[i] <= '9'; i++ )
	    ex1 = 10 * ex1 + s[i] - '0';
	ex += ex1 * esign;
    }
/* incorporate exponent into val */
    if( ex < 0 )
	val = 0.0;
    else if( ex < PMAX || (ex  ==  PMAX && val < HUGE_NOR) )
	val *= powten[ex];
    else 
    {
	//cmemsg(FP_BIGI, s);
	val = HUGE_VAL;
    }
/* check that we have an acceptable termination; if not call error system
   before returning with what we have */
    val *= sign;
    return val;
}
