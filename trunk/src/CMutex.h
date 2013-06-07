#pragma once

#ifndef INC_CMUTEX_H
#define INC_CMUTEX_H


#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#else
#include "pthread.h"
#include <unistd.h>
#endif

class CMutex {

public:
	void Lock();
	void Unlock();

	CMutex();
	~CMutex();

private:
#ifdef WIN32
	CRITICAL_SECTION m_mutexHandle;
#else
	pthread_mutex_t m_mutexHandle;
#endif

};

#endif