#pragma once

#pragma warning (disable:4005 4700 996)
#include "../SDK/amx/amx.h"
#include "../SDK/plugincommon.h"
#if (defined(WIN32) || defined(_WIN32) || defined(_WIN64))
	#include "windows.h"
#else
	#include "pthread.h"
#endif
#include "time.h"
#include "sstream"
#include "fstream"
#include "iostream"
#include "vector"
#include "queue"
#include "list"
#include "string.h"
#include "math.h"
#include "mysql_include/mysql.h"
#if defined(LINUX) || defined(FREEBSD) || defined(__FreeBSD__) || defined(__OpenBSD__)
	#include "stdarg.h"
	#include "complex"
	#include "algorithm"
#endif
#include "source/CMySQLHandler.h"
#include "source/CAmxString.h"
#include "source/CScripting.h"
#include "source/CMutex.h"
#include "misc.h"

typedef void (*logprintf_t)(char* format, ...);

//Kye's sleep macro
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

#define allocate(cast, len) (cast)malloc(len)
#define is_string_char(c) (c == 'z' || c == 's' || c == 'e')
#define VALID_CONNECTION_HANDLE(function, id) \
	if(id >= SQLHandle.size()) { \
		Debug(">> %s() - Invalid connection handle. (You set: %d, Highest connection handle ID is %d).",function,id+1,SQLHandle.size()); \
		return 0; \
	}


//using namespace std;
using std::string;
using std::stringstream;
using std::list;
using std::ofstream;
using std::basic_istringstream;
using std::queue;
using std::vector;
#if defined(LINUX) || defined(FREEBSD) || defined(__FreeBSD__) || defined(__OpenBSD__)
using std::complex;
#endif