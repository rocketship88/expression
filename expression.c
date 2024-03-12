// expression.c

#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#endif
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <stdint.h>

#include <errno.h>
/*

Lookup codes. Note that when it's faster and easier to just check the ascii character 
then that is done with a litteral. When checking if something is a valid number, then
these arrays are used. 

*/



// --------------------------------------------------------
#define Debugx
#define Debugaxxx

#define NOT_IN_SET_7F  0x7f  //token value meaning not in the set
#define DEC_PNT_7D     0x7d  //the decimal point
#define SCI_EXP_6E     0x6e      // e/E notation
#define SKIP_CHAR_7E   0x7e      // underlines and commas

#define INVALID_0     0  // to check for any chars that are not allowed, all the ok on
#define REGULAR_1     1  // to check for any chars that are not allowed, all the ok on
#define LEFTPAREN_2   2 // unused, just checks for the litteral, but the validity checker
#define RIGHTPAREN_3  3 // can 
#define NULL_4        4 // not really used

#define ERROR_PAREN_NOT_BALANCED 2
#define ERROR_RECURSION 3
#define STATUS_OK 0
#define RECURSION_MAX 10 // maximum recursion depth
/*

Token typing arrays. To return a value for an input character
e.g. numbers hex, '0'-'9' return 0..9 and 'a'-'f' and 'A'-'F' both return 10-15


*/
#define GENERATE_ARRAYSx
#ifdef GENERATE_ARRAYS
static unsigned char Letters[128]; // filled by setupascii one time on startup, valid characters for quick check
static unsigned char Numbers[128]; // either the value of an ascii digit ('0' = 0) or  NOT_IN_SET_7F  for scanning numbers
static unsigned char Numbersf[128]; // same but with a decimal point
static unsigned char Numbershex[128]; // with a-f A-F
static unsigned char Numbersbin[128]; // just 0,1
static unsigned char Numbersoct[128]; // just 0..7
static int Defined = 0;  // indicates these are not setup, setupascii will have to init them
#else
static int Defined = 1;  // indicates these are setup below, by doing the dump, so don't need to setup at runtime
static unsigned char Letters[128] = {
0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x02, 0x03,
0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x01, 0x00, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x00, 0x00, 0x00,
0x00, 0x00};
static unsigned char Numbers[128] = {
0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f,
0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f,
0x7f, 0x7f, 0x7e, 0x7f, 0x7f, 0x7f, 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f,
0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f,
0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7e, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f,
0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f,
0x7f, 0x7f };
static unsigned char Numbersf[128] = {
0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f,
0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f,
0x7f, 0x7f, 0x7e, 0x7f, 0x7d, 0x7f, 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f,
0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x6e, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f,
0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7e, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x6e, 0x7f, 0x7f, 0x7f,
0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f,
0x7f, 0x7f };
static unsigned char Numbershex[128] = {
0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f,
0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f,
0x7f, 0x7f, 0x7e, 0x7f, 0x7f, 0x7f, 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f,
0x7f, 0x7f, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f,
0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7e, 0x7f, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x7f, 0x7f,
0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f,
0x7f, 0x7f };
static unsigned char Numbersbin[128] = {
0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f,
0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f,
0x7f, 0x7f, 0x7e, 0x7f, 0x7f, 0x7f, 0x00, 0x01, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f,
0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f,
0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7e, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f,
0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f,
0x7f, 0x7f };
static unsigned char Numbersoct[128] = {
0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f,
0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f,
0x7f, 0x7f, 0x7e, 0x7f, 0x7f, 0x7f, 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f,
0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f,
0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7e, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f,
0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f,
0x7f, 0x7f };
#endif

static double fractions_of_ten [25] = {1.0 , 
0.1,
0.01,
0.001,
0.0001,
0.00001,
0.000001,
0.0000001,
0.00000001,
0.000000001,
0.0000000001,
0.00000000001,
0.000000000001,
0.0000000000001,
0.00000000000001,
0.000000000000001,
0.0000000000000001,
0.00000000000000001,
0.000000000000000001,
0.0000000000000000001,
0.00000000000000000001,
0.000000000000000000001,
0.0000000000000000000001,
0.00000000000000000000001,
0.000000000000000000000001}; // total of 25

static double multiples_of_ten [25] = {1.0 , 
10.0,
100.0,
1000.0,
10000.0,
100000.0,
1000000.0,
10000000.0,
100000000.0,
1000000000.0,
10000000000.0,
100000000000.0,
1000000000000.0,
10000000000000.0,
100000000000000.0,
1000000000000000.0,
10000000000000000.0,
100000000000000000.0,
1000000000000000000.0,
10000000000000000000.0,
100000000000000000000.0,
1000000000000000000000.0,
10000000000000000000000.0, // total of 25
100000000000000000000000.0}; // total of 25



static int nfractions = sizeof(fractions_of_ten) / sizeof (double);
static int nmultiples = sizeof(multiples_of_ten) / sizeof (double);

#define windowsxxx 1
#ifdef windows
   #define INT_32 int32_t
   #define INT_64 int64_t
#else
   #define INT_32 int32_t
   #define INT_64 long long
#endif

/* 

local version of power function for INT_32  and INT_64 

*/

static int mystrcmp (const char *p1, const char *p2)
{
  const unsigned char *s1 = (const unsigned char *) p1+1;
  const unsigned char *s2 = (const unsigned char *) p2+1;
  unsigned char c1, c2;
  
  do
    {
      c1 = (unsigned char) *s1++;
      c2 = (unsigned char) *s2++;
      if (c2 == '\0')
        break;
      }
  while (c1 == c2);

  return c1 - c2;
}


static INT_32 Powl(INT_32 base,  INT_32 exp) {
    int i;
    INT_32 result = 1;
    for (i = 0; i < exp; i++)
        result *= base;
    return result;
 }
static INT_64 Powll(INT_64 base,  INT_64 exp) {
    int i;
    INT_64 result = 1;
    for (i = 0; i < exp; i++)
        result *= base;
    return result;
 }
 

 
// --------------------------------------------------------
/* 

local version of string to double, since if it's an integer only, we can be so much faster,
But if there's an . or e/E we will be slower, but can process it now using our mystrtod function

*/
static INT_32 mystol (char*, char**);
static INT_64 mystoll(char*, char**);

static double mystrtod(char* stringIn, char** endp) {
    char *s = stringIn;
    int isneg = 0;
    register INT_64  l = 0;
    INT_64 value;
    register INT_64  code;
    char * number_table; char *p;
    
    int digits = 0; // count the number of digits after the decimal point
    INT_64 m = 0;   // acumulate the fractionsl value if any
    int exponent = 0; 
    double final_double;

    if ( *s == '-' ) {
        isneg = 1;
        s++;
    } else if (*s == '+') {
        s++;
    }
    while (*s == ' ') s++; // skip spaces, we don't allow tabs here

    number_table = Numbersf;
    l = (INT_64 )number_table[*s++];
    if ( l == 0 ) {
        if (*s == 'x' || *s == 'X' || *s == 'b' || *s == 'B' ||  *s == 'o' || *s == 'O' ||   *s == 'd'  || *s == 'D' ) {
            value = mystoll(stringIn,&p);
            if (endp) *endp = (char*)p;
            return (double) value; 
        }
    }

    if (l ==  NOT_IN_SET_7F ) {// first time, no need to x 10 just check for invalid char (not a number)
        if (endp) *endp = (char*)stringIn; // this is an error, return with end pointer = start
        return 0;
    } else if (l ==  SKIP_CHAR_7E ) { // _ or , at the beginning, so zero our accumulating l
        l = 0;
    } else if (l ==  DEC_PNT_7D ) { // its a .
//      return strtod(stringIn,endp); // nothing for us to do, just pass on to strtod for the entire .xxx 
        s--; // backup so the dot is grabbed in the while loop, we just have a value of 0.xxx
        l = 0; // start with left of dot being 0
    }
    while(1) {
        code = (INT_64 )number_table[*s++];
        if (code ==  DEC_PNT_7D  || code ==  SCI_EXP_6E ) { // found a decimal point or the letter e/E
            if ( code ==  SCI_EXP_6E  ) { char *p=NULL;
                exponent =  mystol(s, &p);
                s = p+1; // this gets decremented at the end
                break;
            } else {
                
            }
            code = (INT_64 )number_table[*s++];
            if ( code <= 10 ) { // if it isn't a digit, it could be undersscore or other, so in the below while we take care of that
                m = code; // but it was a digit (no hex etc. here) so start with that
                digits++;
            } else {
                s--; // backup one, we probably have a decimal 123. with nothing after it, we'll just read the . again below
            }
            while(1) { 
                code = (INT_64 )number_table[*s++];
                if       ( code ==  DEC_PNT_7D   || code ==  NOT_IN_SET_7F  ||  code ==  SCI_EXP_6E  ) { // we're done on another dot or othere non digits
                    break;
                } else if ( code ==  SKIP_CHAR_7E  ) { // skip on under or comma
                    continue;
                } else {
                    m = m * 10 + code; // no hex or other base in decimal numbers
                    digits++;
                    continue;               
                }
                
            }
            if ( code ==  SCI_EXP_6E  ) {
                s--;
                continue; // saw the e go back and get the exponent
            } else {
                break;    // we're done, it was either a dot that didn't belong there, or it was the next operator
            }
        } else if ( code ==  NOT_IN_SET_7F  ) { // end of a number
            break;
        } else if (code ==  SKIP_CHAR_7E ) { //just skip these
            continue;
        } else {
            l = l * 10 + code; // no hex or other base in decimal numbers
        }
    }

   if (endp) *endp = (char*)s-1;
    if ( digits == 0 && exponent == 0) { // we got only an integer part, but could be an E following
                                        // it's only an integer, in l and no exponent or it was 0, 
    } else { // combine the parts
//      printf(" ok now wrap it up we got an l, an m, possible exponent and digits\n"  );
        if ( digits > 24 ) {
            final_double = (double) l;
        } else {
            final_double = fractions_of_ten[digits] * (double) m + (double) l;
           
        }
        if ( isneg ) {
            final_double = -final_double;
        }
        if ( exponent < 0) {int i;
            // ok, need to do some multiplies or divides by 10 or .1
            if ( exponent > -nmultiples ) {
                final_double = final_double * fractions_of_ten[-exponent];
            } else {
                for (i=0; i< abs(exponent) ; i++) {
                    final_double = final_double * (double) .1;
                }
            }
        } else if (exponent > 0) {int i; 
            if ( exponent < nmultiples ) {
                final_double = final_double * multiples_of_ten[exponent];
            } else {
                for (i=0; i< exponent ; i++) {
                    final_double = final_double * (double) 10.0;
                }
            }
        }
        return (double)final_double;            
    }

	// here we finish with an integer part only, or we did the above
	// combining of the int.frac parts in l and m, either way just check for -

    if ( isneg ) {
        l = -l;
    }
    return (double)l;
}
#if 0
static double mystrtod_old(char* stringIn, char** endp) { // this just for reference use to compare results between old and new versions
    char *s = stringIn;
    int isneg = 0;
    register INT_64  l = 0;
    INT_64 value;
    register INT_64  code;
    char * number_table; char *p;

    if ( *s == '-' ) {
        isneg = 1;
        s++;
    } else if (*s == '+') {
        s++;
    }
    while (*s == ' ') s++; // skip spaces, we don't allow tabs here

    number_table = Numbersf;
    l = (INT_64 )number_table[*s++];
    if ( l == 0 ) {
        if (*s == 'x' || *s == 'X' || *s == 'b' || *s == 'B' ||  *s == 'o' || *s == 'O' ||   *s == 'd'  || *s == 'D' ) {
            value = mystoll(stringIn,&p);
            if (endp) *endp = (char*)p;
            return (double) value; 
        }
    }

    if (l ==  NOT_IN_SET_7F ) {// first time, no need to x 10 just check for invalid char (not a number)
        if (endp) *endp = (char*)stringIn; // this is an error, return with end pointer = start
        return 0;
    } else if (l ==  SKIP_CHAR_7E ) { // _ or , at the beginning, so zero our accumulating l
        l = 0;
    } else if (l ==  DEC_PNT_7D  || l ==  SCI_EXP_6E ) { // starts with a . or e
        return strtod(stringIn,endp); // nothing for us to do, just pass on to strtod for the entire .xxx 
    }
    while(1) {
        code = (INT_64 )number_table[*s++];
        if (code ==  DEC_PNT_7D  || code ==  SCI_EXP_6E ) { // found a decimal point or the letter e/E, so gotta use strtod anyway, oh well, we tried!
            return strtod(stringIn,endp); // if it had an exponent, we give up and just do it the slow way 
        } else if ( code ==  NOT_IN_SET_7F  ) { // end of a number
            break;
        } else if (code ==  SKIP_CHAR_7E ) { //just skip these
            continue;
        } else {
            l = l * 10 + code; // no hex or other base in decimal numbers
        }
    }

    
   if (endp) *endp = (char*)s-1;

    if ( isneg ) {
        l = -l;
    }
    return (double)l;
}
#endif
// --------------------------------------------------------
/* 

local version of string to INT_64 , we can now handle the 4 bases

*/


static INT_64 mystoll(char* stringIn, char** endp) {
    char *s = stringIn;
    int isneg = 0;
    register INT_64  l = 0;
    register INT_64  code,base;
    char * number_table;

    if ( *s == '-' ) {
        isneg = 1;
        s++;
    } else if (*s == '+') {
        s++;
    }
    while (*s == ' ') s++; // skip spaces, we don't allow tabs here
    base = 10;
    number_table = Numbers;
    if ( *s == '0' ) {
        if       ( s[1] == 'x' || s[1] == 'X' ) {
            base = 16;
            number_table = Numbershex;
            s++;s++;

        } else if ( s[1] == 'b'|| s[1] == 'B' ) {
            base = 2;
            number_table = Numbersbin;
            s++;s++;
        
        } else if ( s[1] == 'o' || s[1] == 'O' ) {
            base = 8;
            number_table = Numbersoct;
            s++; s++;
        } else if ( s[1] == 'd' || s[1] == 'D' ) {
            s++; s++;
        }
    }
    l = (INT_64 )number_table[*s++];
    if (l ==  NOT_IN_SET_7F ) {// first time, no need to x 10 just check for invalid char (not a number)
        if (endp) *endp = (char*)stringIn; // this is an error, return with end pointer = start
        return 0;
    } else if (l ==  SKIP_CHAR_7E ) { // _ or , at the beginning, so zero our accumulating l
        l = 0;
    }
    while(1) {
        code = (INT_64 )number_table[*s++];
        if ( code ==  NOT_IN_SET_7F  ) { // end of a number
            break;
        } else if (code ==  SKIP_CHAR_7E ) { //just skip these
            continue;
        } else {
//          l = (l<<3) +l +l + code; // times 10, but doesn't look faster, so don't use it
            l = l * base + code;
        }
    }

    
   if (endp) *endp = (char*)s-1;

    if ( isneg ) {
        l = -l;
    }
    return l;
}


// --------------------------------------------------------
/* 

local version of string to INT_32, we can now handle the 4 bases

*/




static INT_32 mystol(char* stringIn, char** endp) {
    char *s = stringIn;
    int isneg = 0;
    register INT_32  l = 0;
    register INT_32  code,base;
    char * number_table;

    if ( *s == '-' ) {
        isneg = 1;
        s++;
    } else if (*s == '+') {
        s++;
    }
    base = 10;
    number_table = Numbers;
    if ( *s == '0' ) {
        if       ( s[1] == 'x' || s[1] == 'X' ) {
            base = 16;
            number_table = Numbershex;
            s++;s++;

        } else if ( s[1] == 'b'|| s[1] == 'B' ) {
            base = 2;
            number_table = Numbersbin;
            s++;s++;
        
        } else if ( s[1] == 'o' || s[1] == 'O' ) {
            base = 8;
            number_table = Numbersoct;
            s++; s++;
        } else if ( s[1] == 'd' || s[1] == 'D' ) {
            s++; s++;
        }
    }
    l = (INT_32 )number_table[*s++];
    if (l ==  NOT_IN_SET_7F ) {// first time, no need to x 10 just check for invalid char (not a number)
        if (endp) *endp = (char*)stringIn; // this is an error, return with end pointer = start
        return 0;
    } else if (l ==  SKIP_CHAR_7E ) { // _ or , at the beginning, so zero our accumulating l
        l = 0;
    }
    while(1) {
        code = (INT_32 )number_table[*s++];
        if ( code ==  NOT_IN_SET_7F  ) { // end of a number
            break;
        } else if (code ==  SKIP_CHAR_7E ) { //just skip these
            continue;
        } else {
//          l = (l<<3) +l +l + code; // times 10, but doesn't look faster, so don't use it
            l = l * base + code;
        }
    }

    
   if (endp) *endp = (char*)s-1;

    if ( isneg ) {
        l = -l;
    }
    return l;
}





/* 

setup the arrays with values for valid characters and numbers

*/



static void setupascii() { // this is our table creator used to tokenize values by their ascii char value
    if ( Defined ) {
        return;
    }
    int i;
    char *ascii = "()\n ^*/%+-.abcdefghijklmnopqrstuvwxyzABCDEF0123456789_,"; // note . is allowed here for floating point, but will error out in our int routines
    char *numbers = "0123456789abcdefABCDEF"; // all the valid numbers up to hex digits
    
    for (i=0; i< 128 ; i++) { // first clear all to 0 or 7f, then fill in from the above ascii and numbers
        Letters[i] = 0;
        Numbers[i] =  NOT_IN_SET_7F ;   // need to use this code since 0 is valid
        Numbersf[i] =  NOT_IN_SET_7F ;  // decimal point numbers
        Numbershex[i] =  NOT_IN_SET_7F ;
        Numbersbin[i] =  NOT_IN_SET_7F ;
        Numbersoct[i] =  NOT_IN_SET_7F ;
    }
    Numbersf['.'] =  DEC_PNT_7D ;      //  the decimal point  
    Numbersf['e'] =  SCI_EXP_6E ;      //  the exponent  letters
    Numbersf['E'] =  SCI_EXP_6E ;      //  the exponent  
    Numbersf['_'] =  SKIP_CHAR_7E ;      //  our underscore or comma  
    Numbersf[','] =  SKIP_CHAR_7E ;      //  our underscore or comma  
    
    Numbers['_'] =  SKIP_CHAR_7E ;  // _ and , allowed in numbers, we just skip over them
    Numbers[','] =  SKIP_CHAR_7E ;
        
    Numbershex['_'] =  SKIP_CHAR_7E ;   // _ and , allowed in numbers, we just skip over them
    Numbershex[','] =  SKIP_CHAR_7E ;   
    
    Numbersbin['_'] =  SKIP_CHAR_7E ;   // _ and , allowed in numbers, we just skip over them
    Numbersbin[','] =  SKIP_CHAR_7E ;   
    
    Numbersoct['_'] =  SKIP_CHAR_7E ;   // _ and , allowed in numbers, we just skip over them
    Numbersoct[','] =  SKIP_CHAR_7E ;   
    
    size_t len = strlen(ascii);
    for (i=0; i< len ; i++) {
        Letters[ascii[i]] = 1;  
    }
    Letters['('] = 2; // for functions or plain parenthesis
    Letters[')'] = 3;
    Letters[0] =   4; // if we see a null byte, code it as 4
    
    for (i=0; i< 16 ; i++) { // these are table lookup for the 4 bases, so we don't have to do number - '0' to get the number from the ascii code
        if ( i < 2 ) {
            Numbersbin[numbers[i]] = i; 
        }
        if ( i < 8 ) {
            Numbersoct[numbers[i]] = i; 
        }
        if ( i < 10 ) {
            Numbers[numbers[i]] = i;    
            Numbersf[numbers[i]] = i;   // decimal point same as decimal, but allows a decimal point also
        }
        if ( i < 16 ) {
            Numbershex[numbers[i]] = i; // lowercase
            if ( i > 9 ) {
                Numbershex[numbers[i+6]] = i;   // lowercase
            }
        }
        
    }
    Defined = 1; // we only do this once this data is static
}

 
static void dump_arrays(char *filename) {
static int Defined = 0;  // indicates these are setup

static unsigned char* arry[] = { Letters,Numbers,Numbersf,Numbershex, Numbersbin , Numbersoct };
static unsigned char* arryname[] = { "Letters","Numbers","Numbersf","Numbershex", "Numbersbin" , "Numbersoct" };
int i,j,k,byte;
#include <errno.h>
#include <stdio.h>
#include <string.h>
    FILE* io = fopen(filename, "w");
    if (io == NULL) {
        fprintf(stderr, "cannot open file '%s': %s\n",
            filename, strerror(errno));
        return;
    }
    for (i=0; i< 6 ; i++) {
        printf("\nstatic unsigned char %s[128] = {\n", arryname[i]);
        fprintf(io,"\nstatic unsigned char %s[128] = {\n", arryname[i]);
        for (j = 0,k=1; j < 128; j++,k++) {
            byte = (int)arry[i][j];
            printf("0x%02x", byte);
            fprintf(io,"0x%02x", byte);
            if (j != 127) {
                printf(", ");
                fprintf(io,", ");
            }
            if (k > 20) {
                printf("\n");
                fprintf(io,"\n");
                k = 0;
            }
        }
        printf("};\n");
        fprintf(io,"};\n");
    }
    fclose(io);
}
 
 
// --------------------------------------------------------

/* 

checks the input string against the  Letters array, kind is not implemented (was to be float/int)

*/


static int eval0x( char* stringIn,int kind) { // quick check for unbalanced parens and any bad chars in expression
    int parlev = 0;
    register int position = 0; // don't know if compiler honors register, but it can't hurt
    register char* s = stringIn;
    register int code;
    for (;*s != '\0'; ) {
        code = Letters[*s++];
        if       ( code == 1 ) { // most will come through here
            continue;
        } else if ( code == 2 ) {
            parlev++;
            continue;
        } else if ( code == 3 ) {
            if ( --parlev < 0 ) {
                return ERROR_PAREN_NOT_BALANCED; // if it goes negative, we've got a close not matching an open ( ))  
            }
            continue;
        } else {
            position = (int) (s - stringIn); // compute position of the error, return +1000, since unbal parens returns a 2
            return position +1000;
        }
        
    }
    if ( parlev != 0 ) {
        return ERROR_PAREN_NOT_BALANCED;
    }
    return STATUS_OK; // ok
}
 
 
// --------------------------------------------------------

/* 

first version of input checking, w/o opt on compiler, this was much slower
perhaps it would work better with opt, since it was the switch statement that
was slowing it down

*/


static int eval0( char* stringIn,int kind) {
    int parlev = 0;
    register int position = 0;
    register char* s = stringIn;
    for (;*s != '\0'; s++,position++) {
        switch (*s) {
            case '(': parlev++ ; break;
            case ')': parlev-- ; break;
            
            case '\n':case ' ':case '^':case '*':case '/':case '%':case '+':case '-':case '.':

            case 'a':case 'b':case 'c':case 'd':case 'e':
            case 'f':case 'g':case 'h':case 'i':case 'j':case 'k':case 'l':
            case 'm':case 'n':case 'o':case 'p':case 'q':case 'r':case 's':
            case 't':case 'u':case 'v':case 'w':case 'x':case 'y':case 'z':
            
//            case 'A':break;case 'B':break;case 'C':break;case 'D':break;case 'E':break;
//            case 'F':break;case 'G':break;case 'H':break;case 'I':break;case 'J':break;case 'K':break;case 'L':break;
//            case 'M':break;case 'N':break;case 'O':break;case 'P':break;case 'Q':break;case 'R':break;case 'S':break;
//            case 'T':break;case 'U':break;case 'V':break;case 'W':break;case 'X':break;case 'Y':break;case 'Z':break;
            
            case '0':case '1':case '2':case '3':case '4':case '5':case '6':case '7':case '8':case '9':
            break;


            default: return position +1000;
            break;
        }
        
    }
    if ( parlev != 0 ) {
        return 2;
    }
    return 0; // ok
}

/*

To add a function is simple. There are 3 steps. First, add an enum like below.
Then go to afunc and add a mystrcmp with the name to be used by this evaluator
Then go to each of the 3 types of parsers, 32/64/double and if appropriate, add
it to the switch statement by copying one of the cases. Only single argument
functions are supported.

*/


enum function_codes {eERROR=0,eABS,eINT,eSIN,eCOS, eROUND, eSQRT, eRAD, eFLOOR}; // return one of these for function or an error



// --------------------------------------------------------

/* 

determine which function is being called, news is the number of chars scanned

*/


static int afunc(char* stringIn, int* news) { // parse a function up to opening paren
    char buf[10];           // accumulate function name
    char* p = buf;          // points to where to store next char
    int paren = 0;          // flag for having seen the opening paren
    int i;                  // count of chars not including spaces, limits what we write to buf
    char c;
    char* s = stringIn;
    for (i=0; i<= 7 ; )  {
         c = *s;
         if (  c >= 'a' &&  c <= 'z') {
            *p++ = *s++;
            i++;
            continue;
        } else if (  c == '(' ) {
            paren++;
            break;
        } else if (  c == ' ' ) {
            s++;
            continue;
        } else if (  c == '\0' ) {
            break;
        } else {
            return eERROR;
        }
    }
    if ( i < 3 || i > 5 || paren !=1 ) {
        return eERROR;
    }
    *p = '\0';                      // null terminate the buffer
    *news =  (int) ( s - stringIn); // how many chars did we scan, return to caller
    
    
    switch (buf[0]) { // so we only do one or two mystrcmp's, based on the first letter,  Note mystrcmp starts comparing at the 2nd char
        case 'a':
            if ( mystrcmp(buf,"abs") == 0 ) {
                return eABS;
            }  
            break;
        case 'f':
            if ( mystrcmp(buf,"floor") == 0) {
                return eFLOOR;
            }  
            break;
        
        case 'i':
            if ( mystrcmp(buf,"int") == 0 ) {
                return eINT;
            }  
            break;
        case 's':
            if ( mystrcmp(buf,"sqrt") == 0  ) {
                return eSQRT;
            }  
 
             if ( mystrcmp(buf,"sin") == 0  ) {
                return eSIN;
            }  
            break;
       case 'c':
        
            if ( mystrcmp(buf,"cos") == 0  ) {
                return eCOS;
            }  
            break;
        case 'r':
            if ( mystrcmp(buf,"round") == 0  ) {
                return eROUND;
            }  
            if ( mystrcmp(buf,"rad") == 0  ) {
                return eRAD;
            }  
            break;
        default:
        break;
    }
    return eERROR;
    
    
}


// --------------------------------------------------------

/* 

Evaluate an expression, recursive decent parser, single left
to right, with recursion for ()'s and fucntion()'s

Note the integer INT_32 version doesn't have all the comments, it was written
before this was commented, but it's the same code except for using INT_32 s instead
of doubles

double floating version, biggest slowdown is the strtod function
So, we have our own version, which if there's no decimal point does an
integer conversion. That also checks for a 0x, 0b, etc. and if found
it can't be a float, so it calls the integer conversion. If it does
run into the decimal point or the exponent, e, then it has to use strtod.
Hopefully not all that often.

*/


static double evaluate_d( char* stringIn, char** endp, int* thestatus,int level) {
    struct operand_d {
                                                                                            #ifdef Debuga
                                                                                                    void *address; // for debug
                                                                                                    long  pushed;
                                                                                            #endif
        double val  ; // the value of the current computation, either on the stack or in x
        char   op   ; // this is one of the litteral operators, but could be any constant
        char   prec ; // precedence level, currently 3 is the max, so really only need 4 of these
    } stack[5] = { 
                                                                                            #ifdef Debuga
                                                                                                NULL, 0,
                                                                                            #endif
    0.,0,0 }, *sp, x; // stack pointer and the active opperand we're building up 
                                                                                            #ifdef Debuga
                                                                                                stack[0].address = (void *)&stack[0].address; // debug
                                                                                                stack[1].address = (void *)&stack[1].address;
                                                                                                stack[2].address = (void *)&stack[2].address;
                                                                                                stack[3].address = (void *)&stack[3].address;
                                                                                                stack[4].address = (void *)&stack[3].address;
                                                                                            #endif
    double fval; // the value of the argument of a function
    char* p;     // pointer to input string text, next token after call to functrion arg or mystrtod
    char* s;     // pointer to current location in the input string
    char c;      // used to hold the current character, to avoid another *s
    int news;    // used to compute the new s after seeing a function( 
    enum function_codes f;
    char *fsave = stringIn; // the pointer of a function call name, in case of an error

    s  = stringIn;
    while (*s == ' ') s++; // skip whitespace
    
    c = *s;
    sp = stack;
    if ( ++level > RECURSION_MAX) {
                                                                                            #ifdef Debug
                                                                                                printf("%*s  evaluate_d: exceeded recursion depth %d at %s\n",level*5,".",level,s );
                                                                                            #endif
        *thestatus = ERROR_RECURSION;
        return 0.0;
    }
                                                                                            #ifdef Debuga
                                                                                                x.pushed = (long)0xbeefcafe; // debug
                                                                                            #endif
                                                                                            #ifdef Debug
                                                                                                printf("%*s  evaluate_d: depth=%d stringIn= [%s]\n",level*5,".",level,s );
                                                                                            #endif
    if (c  == '+' || c == '-') { // check for initial unitary + or - 
        x.val = 0.0;
        x.op  = c;
        x.prec = 3;  // give it highest priority
        *sp++ = x;  // pretend we started with a 0, so it's 0+... or 0-...
        s++;
                                                                                            #ifdef Debug
                                                                                                printf("%*s  push a 0.0 with uniary [%c] op at prec(%d)\n", level * 5, ".", c, x.prec);
                                                                                            #endif
    }
    while (1) {
        int status;
        while (*s == ' ') s++; // skip over spaces, we don't allow tabs here, much faster than using isspace
        if (*s == '(') {       // opening parens, we know they are balanced, we did that check separately
                                                                                            #ifdef Debug
                                                                                                printf("%*s  opening parens  \n" ,level*5,".");
                                                                                            #endif
            x.val = evaluate_d(s + 1, &p,&status,level); // recursive call
            s = p;
            if (status != STATUS_OK) {
                if ( status < 1000 ) {
                    *thestatus = status;
                    return 0.0;
                }
                goto error; // return a status value that also returns the position of the error
            }
            if (*s == ')')s++;
        } else if ( *s >= 'a' && *s <= 'z') { // a function is just a pair of ()'s with a value
            f = afunc(s,&news);               // functions are the only thing that has a-z letters
            fsave = s + 1;
            if ( f == eERROR ) {
                goto error;// return a status value that also returns the position of the error
            }
                                                                                            #ifdef Debug
                                                                                                printf("%*s  func begins with [%c%c...] recur... \n" ,level*5,".",s[0],s[1]);
                                                                                            #endif
            s = s+news;
            fval = evaluate_d(s + 1, &p,&status,level); // evaluate what's in the parens, recursively
            s = p;
            if (status != STATUS_OK) {
                if ( status < 1000 ) {
                    *thestatus = status;
                    return 0.0;
                }
                goto error; // return a status value that also returns the position of the error
            }
            if (*s == ')')s++;
             
  
             switch (f) { // do the corresponding function on our value HERE to add additional functions
    
                case eABS:       
                    if ( fval < 0.0 ) {
                        fval = -fval;
                    }
                    x.val = fval;
                    break;
                case eINT:    
					if ( fval < 0.0 ) {
                		x.val = floor(fval + 1.0);
					} else {
                		x.val = floor(fval);
					}
                	break;
                case eFLOOR: x.val = floor(fval)      ; break;                                  
                case eSIN:    x.val = sin(fval);      ; break;
                case eCOS:    x.val = cos(fval);      ; break;
                case eSQRT:   x.val = sqrt(fval);     ; break;
                case eROUND:  x.val = round(fval)     ; break;
                case eRAD:    x.val =fval * (3.141592653589793238462643/180.)     ; break;
                default: s=fsave; goto error;    break; // syntax error
            }


        } else { // if not parens or a function, it must be a number
            char ch;
            x.val = mystrtod(s, &p); // my own version, lots faster, don't know why however
//           x.val = strtod(s, &p); // converts the text to a number, and tell us where it stopped
                                                                                                #ifdef Debug
                                                                                                    printf("%*s  got next value  x.val=%.17g  characters scanned %d\n",level*5,".", x.val ,(int) (p-s) );
                                                                                                #endif
            while (*p == ' ') {  // skipping whitespace but incrementing s as well
                s++; p++;
            }
            ch = p[0];
            if ( ch == '+' || ch == '-' || ch == '*' || ch == '/'  || ch == ')' || ch == '\0' || ch == '^' ||   ch == '%' || ch == '\n') {
                // anything is ok, checking in order of likelihood
            } else {
                s++;
                goto error;// return a status value that also returns the position of the error
            }
            if ( s  == p && s[1] != '(') { // this means we have a bad function name
                goto error;// return a status value that also returns the position of the error
            }
            s = p;
        }
        while (*s == ' ') s++; // skip whitespace
        c = *s;
        if ( c == '*' ) {
            if ( s[1] == '*' ) { // if we see ** then eat one * and pretend we got the ^
                s++;
                c = '^';
            }
        }
        s++; // now we must have an operator, it's allways number op, or () op, or func() op
                                                                                                #ifdef Debug
                                                                                                    printf("%*s  got next operator or terminal  [%c%c] \n",level*5,".",c=='\n' ? 'n' : c, c=='\n' ? 'l' : ' ');
                                                                                                #endif
        switch (x.op = c) { // only 4 precendence levels so only need a stack of 4, but we allocate 5 for safety
            case '^': x.prec = 3; break;
            case '*':
            case '/':
            case '%': x.prec = 2; break;
            case '+':
            case '-': x.prec = 1; break;
            case '\n':
            case '\0':
            case ')': x.prec = 0; x.op = 0; s--; break; // here on close paren or null char or newline
            default:  goto error; // here on a non valid operator and return a status value that also returns the position of the error
        }
                                                                                                #ifdef Debuga
                                                                                                        x.address = NULL;
                                                                                                #endif
        while (sp > stack && x.prec <= sp[-1].prec) { 
                                                                                                #ifdef Debug
                                                                                                   printf("%*s  stack accum operation sp : [%zd]  sp(%c)  sp-1-prec( %d) x.val=%.17g\n",level*5,".", (sp - stack),sp[-1].op ,sp[-1].prec,x.val );
                                                                                                #endif
             switch ((--sp)->op) {                       // unwind the stack of operations and accum the pending values
                case '^': x.val = pow(sp->val, x.val); break;
                case '*': x.val = sp->val * x.val; break;
                case '/': x.val = sp->val / x.val; break;
                case '%': x.val = fmod(sp->val, x.val); break;
                case '+': x.val = sp->val + x.val; break;
                case '-': x.val = sp->val - x.val; break;
            }
                                                                                                #ifdef Debug
                                                                                                   printf("%*s  stack pop opcode (%c) sp->val=%.17g    new value of x.val=%.17g\n",level*5,".",sp[0].op,sp->val ,x.val);
                                                                                                    #ifdef Debuga
                                                                                                       sp[0].pushed = 0;
                                                                                                       sp[0].address = &sp[0];
                                                                                                    #endif
                                                                                                #endif
        }
        if (!x.op) break;

        *sp++ = x;
                                                                                                #ifdef Debug
                                                                                                   printf("%*s  pushed sp :  [%zd] x.val=%.17g x.op (%c) \n",level*5,".", (sp - stack),x.val, x.op   ); 
                                                                                                #endif
    }
    if (endp) *endp = (char*)s;
    *thestatus = 0;
                                                                                                #ifdef Debug
                                                                                                   printf("%*s  done return value x.val=%.17g\n",level*5,".",x.val);
                                                                                                #endif
    return x.val;
error:
    if (endp) *endp = (char*)s;
    *thestatus = (int) ( s-stringIn) +1000;
                                                                                                #ifdef Debug
                                                                                                   printf("%*s  error return %d\n",level*5,".",*thestatus);
                                                                                                #endif
    return 0.0;
} // end evaluate_d

 



/* 

double  version of the evaluator

*/



int evaluate_d_expression(char* stringIn ,double* result) { // evaluate an expression, 
    int status;
    if ( !Defined ) { // first time we define our letters and numbers
        setupascii();
    }
    status = eval0x(stringIn, 1); // quick syntax check for illegal characters and unbalanced parens
    //status = eval0 (stringIn, 1); // quick syntax check for illegal characters and unbalanced parens
    if (status  != 0 ) {
        return status; // error codes, 2=unbalanced parens, 1000+ = error with pointer to problem +1000
    }
    *result = evaluate_d(stringIn,NULL,&status,0); // now do the evaluate
    return status;
}



/* 

INT_32  version of the evaluator

*/


int evaluate_l_expression(char* stringIn ,INT_32* result) { // evaluate an expression, 
    int status;
    if ( !Defined ) { // first time we define our letters and numbers
        setupascii();
    }
    status = eval0x(stringIn, 2); // quick syntax check for illegal characters and unbalanced parens
    //status = eval0 (stringIn, 2); // quick syntax check for illegal characters and unbalanced parens
    if (status  != 0 ) {
        return status; // error codes, 2=unbalanced parens, 1000+ = error with pointer to problem +1000
    }
    *result = (INT_32)lrint(evaluate_d(stringIn,NULL,&status,0)); // now do the evaluate
    return status;
}
/* 

INT_64  version of the evaluator

*/


int evaluate_ll_expression(char* stringIn ,INT_64 * result) { // evaluate an expression, 
    int status;
    if ( !Defined ) { // first time we define our letters and numbers
        setupascii();
    }
    status = eval0x(stringIn, 2); // quick syntax check for illegal characters and unbalanced parens
    //status = eval0 (stringIn, 2); // quick syntax check for illegal characters and unbalanced parens
    if (status  != 0 ) {
        return status; // error codes, 2=unbalanced parens, 1000+ = error with pointer to problem +1000
    }
    *result = (INT_64)llrint(evaluate_d(stringIn,NULL,&status,0));; // now do the evaluate
    return status;
}

/* 

main test program to test for timing and debuggin all 3 varity's of expressions

*/



static double started =-1;

#include <inttypes.h>
#include <math.h>
#include <stdio.h>
#include <time.h>
#include <sys/time.h>
void etimer (long count)
{
    
    double ms, elapsed;
    struct timeval  tv;
    gettimeofday(&tv, NULL);

    ms = (tv.tv_sec) * 1000 + (tv.tv_usec) / 1000 ; 
    
    if ( started < 0 ) {
        started = ms;
        printf("---start timer\n");
    } else {
        elapsed = ms - started;
        printf("---end  elapsed: %.17g ms - us per iteration: %.5g\n",elapsed, elapsed*1000. / (double) count);
    }
    return;
}
int main(int argc, char* argv[]) { // main on linux

    int status,n,i;
    char buf[100];
    double ansd;
    INT_64 ansll;
    INT_32 ansl;
    int doint = 2; // doing integer mode 1 = INT_32, 2 = INT_64, else double float (0)
    char* newp;
    char* expr;
    setupascii();
//    ansll=mystoll("-123,456x",&newp);
//    ansll=mystoll("0xbe_EF", &newp);
//    ansll=mystoll("0b1111_1111 ", &newp);
//    ansd = mystrtod_old(sin = "1e-2+", &newp);
//    ansd = mystrtod(sin = "1e-2+", &newp);
//    ansd = mystrtod_old(sin = "1e+2+", &newp);
//    ansd = mystrtod(sin = "1e+2+", &newp);
//    ansd = mystrtod(sin = "123.+", &newp);
//    ansd = mystrtod(sin = "1233.0000000000321+", &newp);
//    ansd = mystrtod(sin = "1231.000000000000321+", &newp);
//    ansd = mystrtod(sin = "1232.0000000000000000000000321+", &newp);
//    ansd = mystrtod(sin = "123+", &newp);
//    ansd = mystrtod(sin = "123.4_56+", &newp);
//    ansd = mystrtod(sin = "123.456e2+", &newp);
//    ansd = mystrtod(sin = "0xff_ff+", &newp);
//    ansd = mystrtod(sin = "0xff_ff+", &newp);
//    ansd = mystrtod(sin = "-456+", &newp);
//    ansd = mystrtod(sin = "456+", &newp);
//    ansd = mystrtod(sin = "0+", &newp);
//    ansd = mystrtod(sin="-123e+10+",&newp);
//    ansd = mystrtod(sin="-123e+10+",&newp);
//    ansd = mystrtod(sin = "-123.456E+2+", &newp);
//    ansd = mystrtod(sin = "123.456e2+", &newp);
//    ansd = mystrtod(sin = ".456+", &newp);
//    ansd = mystrtod(sin = "-.456+", &newp);
//    printf("-------------------------------------------\nbegin argc = %d if less than 3 args, interactive mode\n-------------------------------------------\n" ,argc);fflush(stdout);
    char *mode = "?";
    if ( argc <= 2 ) {
    }
    if ( argc == 3 && argv[1][0] == 'd' ) {
        printf("0 [%s] \n", argv[0] ); // command
        printf("1 [%s] \n", argv[1] ); // command
        printf("2 [%s] \n", argv[2] ); // command
        printf("2 [%s] \n", argv[2] ); // command
        setupascii();
        dump_arrays(argv[2]);
        printf("%s created \n",argv[2]);
        return 0;
    }
    if ( argc <= 1 ) {
        printf("\n---------------------------- expression evaluator -----------------------------\n");
        printf("Usage: %s mode ?count? ?'expression'? -- quotes needed if contains ()\n",argv[0]);
        printf("       %s mode                        -- interactive mode\n",argv[0]);
        printf("       %s d filename                  -- to output ascii tables\n\n",argv[0]);
        printf("0  [%s]\n", argv[0] ); // command
        printf("1  mode 0=double, 1=long, 2= wide (long long)\n"); // mode 0,1,2
        printf("2  count for timing, number of iterations\n" );
        printf("3  expression\n\n" );
        printf("To build with debug trace, compile with -D Debug\ngcc -D Debug  -O2 -o test expression.c  -lm\n" );
        printf("\nSome overflow checks:\n");
#ifdef windows
        INT_32 my_32bit_int = 0x7fffffff;
        INT_64 my_64bit_int = 0x7fffffffffffffff;
        printf("w   32 = %ld   %lx\n", my_32bit_int, my_32bit_int);
        my_32bit_int++;
        printf("+1  32 = %ld  %lx\n", my_32bit_int, my_32bit_int);
    
        printf("    64 = %lld    %llx\n", my_64bit_int, my_64bit_int);
        my_64bit_int++;
        printf("+1  64 = %lld   %llx\n", my_64bit_int, my_64bit_int);
#else
        INT_32 my_32bit_int = 0x7fffffff;
        INT_64 my_64bit_int = 0x7fffffffffffffff;
        printf("L  32 = %d    %x\n", my_32bit_int, my_32bit_int);
        my_32bit_int++;
        printf("+1 32 = %d   %x\n", my_32bit_int, my_32bit_int);
        
        printf("   64 = %lld    %llx\n", my_64bit_int, my_64bit_int);
        my_64bit_int++;
        printf("+1 64 = %lld   %llx\n", my_64bit_int, my_64bit_int);
#endif
        return 0;
    }
    
   
    doint = mystol(argv[1],NULL);
    
    if ( argc > 2  &&  1) { // selects  timing mode or interactive mode
        if       ( doint == 0 ) {
            mode = "double";
        } else if ( doint == 1  ) {
            mode = "short int (32 bit)";
        } else if ( doint == 2  ) {
            mode = "wide (64 bit)";
        } else {
            mode = "unknown will use wide (64 bit)";
        }
        n  = mystol(argv[2],NULL);
        if ( argv[3] == NULL ) {
            printf("No expression to evaluate\n" );
            return 1;
        }
        
        expr = argv[3];
        printf("\n0 [%s] \n", argv[0] ); // command
        printf("1 [%s] mode %s\n", argv[1],mode ); // mode 0,1,2
        printf("2 [%s] count \n", argv[2] );
        printf("3 [%s] expression\n", argv[3] );
         while (1) {
#ifdef Debug
            printf("Debug mode incompatible with timing tests, remove -D Debug from gcc\n");
            return 1;
#endif
             if ( doint == 1 ) { // 1 is integer eval
             
                printf("start l\n");etimer(n);
                for (i = 0; i < n; i++) {
                    status = evaluate_l_expression(expr, &ansl);
                    if (i < 2 || i > n-3) {
                        printf("i=[%d] stat=%d %d\n", i, status, ansl);
                    }
                }
                etimer(n);printf("end l\n");

            } else if ( doint == 2 ) {// 2 is INT_64 eval

                printf("start ll\n");etimer(n);
                for (i = 0; i < n; i++) {
                    status = evaluate_ll_expression(expr, &ansll);
                    if (i < 2 || i > n-3) {
                        printf("i=[%d] stat=%d %lld\n", i, status, ansll);
                    }
                }
                etimer(n);printf("end ll\n");
             } else {
                   
                printf("start d\n"); etimer(n);
                for (i = 0; i < n; i++) {
                    status = evaluate_d_expression(expr, &ansd);
                    if (i < 2 || i > n-3) {
                        printf("i=[%d] stat=%d %.17g \n", i, status, ansd);
                    }
                }
                 etimer(n);printf("end d\n");
                
             }  
             break;
         } // end while     
            
    } else { // --------------------------- debugging interactively ----------------
        
         printf("\nInteractive mode output dec hex octal (q for quit):\n");
         if ( doint == 0) { // double
         
//         status = evaluate_d_expression("1.5+abs(-2-1)",&ansd);
//         printf(" -> %.17g  status=%d\n", ansd,status );
            for (;;) {
                printf("eval- d> ");
                fflush(stdout);
                if (!fgets(buf, sizeof buf, stdin) || (buf[0] == 'q' && buf[1] == '\n')) {
                    break;
                }
                status = evaluate_d_expression(buf,&ansd);
                if ( status == 0 ) {
                    printf(" -> %.17g  status=%d ok\n", ansd, status);
                } else {
                    if (status >= 1000) {
                        printf("      %*s ^^^^ %d\n", status - 1000, "", status);
                    } else if (status == ERROR_PAREN_NOT_BALANCED) {
                        printf("Unbalanced parens\n");
                    } else if (status == ERROR_RECURSION) {
                        printf("exceeded recursion depth %d\n",RECURSION_MAX);
                    }
                    printf(" -> %.17g  status = %d  error\n", ansd, status);
                    
                }
            }
         } else if ( doint == 2 ) {// 2 is INT_64 eval
//          status = evaluate_ll_expression("1+abs(-2-1)",&ansll);
//          printf(" -> %lld  status=%d\n", ansll,status );
            for (;;) {
                printf("eval-ll> ");
                fflush(stdout);
                if (!fgets(buf, sizeof buf, stdin) || (buf[0] == 'q' && buf[1] == '\n')) {
                    break;
                }
                status = evaluate_ll_expression(buf,&ansll);
                if ( status == 0 ) {
                    printf(" -> %lld %llx %llo status=%d ok\n", ansll,ansll,ansll, status);
                } else {
                    if (status >= 1000) {
                        printf("      %*s ^^^^ %d\n", status - 1000, "", status);
                    } else if (status == ERROR_PAREN_NOT_BALANCED) {
                        printf("Unbalanced parens\n");
                    } else if (status == ERROR_RECURSION) {
                        printf("exceeded recursion depth %d\n",RECURSION_MAX);
                    }
                    printf(" -> %lld status = %d  error\n", ansll, status);
                    
                }
            }
         } else { // INT_32
                 
//          status = evaluate_l_expression("1+abs(-2-1)",&ansl);
//          printf(" -> %d  status=%d\n", ansl,status );
            for (;;) {
                printf("eval- l> ");
                fflush(stdout);
                if (!fgets(buf, sizeof buf, stdin) || (buf[0] == 'q' && buf[1] == '\n')) {
                    break;
                }
                status = evaluate_l_expression(buf,&ansl);
                if ( status == 0 ) {
                    printf(" -> %d   %x    %o    status=%d ok\n", ansl,ansl,ansl, status);
                } else {
                    if (status >= 1000) {
                        printf("      %*s ^^^^ %d\n", status - 1000, "", status);
                    } else if (status == ERROR_PAREN_NOT_BALANCED) {
                        printf("Unbalanced parens\n");
                    } else if (status == ERROR_RECURSION) {
                        printf("exceeded recursion depth %d\n",RECURSION_MAX);
                    }
                    printf(" -> %d status = %d  error\n", ansl, status);
                    
                }
            }
        }       
    }
    return 0;
}
