#pragma once

#include "main.h"

#if defined(LINUX) || defined(FREEBSD) || defined(__FreeBSD__) || defined(__OpenBSD__)
char* itoa(int value, char* result, int base);
char* strrev(char *s);
#endif