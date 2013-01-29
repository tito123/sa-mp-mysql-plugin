#pragma once

#include "misc.h"


// Misc

#if defined(LINUX) || defined(FREEBSD) || defined(__FreeBSD__) || defined(__OpenBSD__)
// http://www.jb.man.ac.uk/~slowe/cpp/itoa.html
char* itoa( int value, char* result, int base ) 
{
	// check that the base if valid
	if (base < 2 || base > 16) 
	{ 
		*result = 0; 
		return result; 
	}
	char* out = result;
	int quotient = value;
	do 
	{
		*out = "0123456789abcdef"[ abs( quotient % base ) ];
		++out;
		quotient /= base;
	} 
	while ( quotient );

	if ( value < 0 && base == 10) *out++ = '-';
	std::reverse( result, out );
	*out = 0;
	return result;
}

char* strrev( char* s )
{
	char  c;
	char* s0 = s - 1;
	char* s1 = s;
	while (*s1) ++s1;
	while (s1-- > ++s0)
	{
		c = *s0;
		*s0 = *s1;
		*s1 =  c;
	}
	return s;
}
#endif

int IsNumeric(const char *numstring)
{
	while(*numstring)  {
		if (!((*numstring <= '9' && *numstring >= '0') || (*numstring == '-' || *numstring== '+'))) {
			return 0;
		}
		numstring++;
	}
	return 1;
}

std::string stringvprintf(const char *format, va_list args)
{
	int length = vsnprintf(NULL,0,format,args);
	char *chars = new char[length + 1];
	length = vsnprintf(chars, length + 1, format, args);
	std::string result(chars);
	delete chars;
	return result;
}