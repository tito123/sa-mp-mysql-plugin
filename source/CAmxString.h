#pragma once

#include "../main.h"

class StrAmx
{
public:
	std::string GetString( AMX* amx, cell param );
	int	SetString( AMX* amx, cell param, std::string str, int len = 0);
	void SetCString( AMX* amx, cell param, char* str, int len = 0);
	int GetCString(AMX* amx, cell param, char*& dest);
};