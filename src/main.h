#pragma once

#ifndef INC_MAIN_H
#define INC_MAIN_H

#pragma warning (disable: 4005 4996)


#include "SDK/amx/amx.h"
#include "SDK/plugincommon.h"


#include <list>

using std::list;
extern list<AMX*> p_Amx;


typedef void (*logprintf_t)(char* format, ...);


#ifndef NULL
	#define NULL 0
#endif

#endif