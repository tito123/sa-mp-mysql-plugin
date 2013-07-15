#pragma once

#include "CAmxString.h"

void StrAmx::SetCString(AMX* amx, cell param, const char *str, int len) {
	cell *destination;
	amx_GetAddr(amx, param, &destination);
	if (!len) {
		amx_SetString(destination, str, 0, 0, strlen(str) + 1);
	} else {
		amx_SetString(destination, str, 0, 0, len);
	}
}
