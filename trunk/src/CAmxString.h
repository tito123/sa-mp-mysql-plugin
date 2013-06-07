#pragma once

#include "main.h"

#include <cstring>
#include <string>

using std::string;

class StrAmx {
public:
	void GetString(AMX* amx, cell param, string &dest);
	int	SetString(AMX* amx, cell param, std::string str, int len = 0);
	void SetCString(AMX* amx, cell param, char* str, int len = 0);
	int GetCString(AMX* amx, cell param, char*& dest);
};