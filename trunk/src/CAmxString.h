#pragma once

#include "main.h"

#include <cstring>


namespace StrAmx {
	void SetCString(AMX* amx, cell param, const char *str, int len = 0);

};