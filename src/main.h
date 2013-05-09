#pragma once

#pragma warning (disable: 4005 4700 996)
#if (defined(WIN32) || defined(_WIN32) || defined(_WIN64))
#include "windows.h"
#else
#include "pthread.h"
#endif
#include <ctime>
#include <sstream>
#include <fstream>
#include <iostream>
#include <vector>
#include <queue>
#include <list>
#include <map>
#include <cstring>
#include <cmath>
#include "mysql_include/mysql.h"
#if defined(LINUX) || defined(FREEBSD) || defined(__FreeBSD__) || defined(__OpenBSD__)
#include "stdarg.h"
#include <complex>
#include <algorithm>
#include <unistd.h>
#endif
#include "SDK/amx/amx.h"
#include "SDK/plugincommon.h"
#include "misc.h"
#include "CAmxString.h"
#include "CMutex.h"
#include "CMySQLHandler.h"
#include "CScripting.h"

using namespace std;

typedef void (*logprintf_t)(char* format, ...);

#ifdef WIN32
#define SLEEP(x) { Sleep(x); }
#else
#define SLEEP(x) { usleep(x * 1000); }
typedef unsigned long DWORD;
typedef unsigned int UINT;
#endif

#if !defined NULL
#define NULL 0
#endif

#define is_string_char(c) (c == 'z' || c == 's' || c == 'e')

#define ERROR_INVALID_CONNECTION_HANDLE(function, id) \
	Log(LOG_ERROR, ">> %s() - Invalid connection handle. (ID = %d).", function, id);