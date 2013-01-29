#pragma once

#include "CAmxString.h"

int StrAmx::SetString(AMX* amx, cell param, string str, int len)
{
	cell *destination;
	amx_GetAddr( amx, param, &destination );
	if(!len)
		amx_SetString( destination, str.c_str(), 0, 0, str.length()+1);
	else
		amx_SetString( destination, str.c_str(), 0, 0, len);
	return 1;
}

void StrAmx::SetCString(AMX* amx, cell param, char* str, int len)
{
	cell *destination;
	amx_GetAddr(amx, param, &destination);
	if(!len)
		amx_SetString(destination, str, 0, 0, strlen(str)+1);
	else
		amx_SetString(destination, str, 0, 0, len);
}

std::string StrAmx::GetString(AMX* amx, cell param)
{
	cell *pString;
	char *szDest;
	int nLen;
	amx_GetAddr(amx,param,&pString);
	amx_StrLen(pString, &nLen);
	szDest = new char[nLen+1]; 
	amx_GetString(szDest, pString, 0, UNLIMITED);
	szDest[nLen] = '\0';
	string szReturn(szDest);
	delete szDest;
	return szReturn;
}

int StrAmx::GetCString(AMX* amx, cell param, char*& dest)
{
	cell *ptr;
	int len;
	amx_GetAddr(amx, param, &ptr);
	amx_StrLen(ptr, &len);
	dest = (char*)malloc((len * sizeof(char))+1);
	amx_GetString(dest, ptr, 0, UNLIMITED);
	dest[len] = '\0';
	return len;
}