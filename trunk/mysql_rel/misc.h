#pragma once

#include "main.h"

int float_len(float f);
int IsNumeric(const char *numstring);
#if defined(LINUX) || defined(FREEBSD) || defined(__FreeBSD__) || defined(__OpenBSD__)
char* itoa(int value, char* result, int base);
char* strrev(char *s);
#endif
std::string stringvprintf(const char *format, va_list args);