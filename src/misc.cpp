#pragma once

#include "misc.h"

#include <cmath>
#include <complex>
#include <algorithm>

#if defined(LINUX) || defined(FREEBSD) || defined(__FreeBSD__) || defined(__OpenBSD__)
// http://www.jb.man.ac.uk/~slowe/cpp/itoa.html
char* itoa(int value, char* result, int base) {
	if (base < 2 || base > 16) {
		*result = 0;
		return result;
	}
	char* out = result;
	int quotient = value;
	do {
		*out = "0123456789abcdef"[ abs(quotient % base) ];
		++out;
		quotient /= base;
	} while (quotient);
	if (value < 0 && base == 10) {
		*out++ = '-';
	}
	std::reverse(result, out);
	*out = 0;
	return result;
}
#endif