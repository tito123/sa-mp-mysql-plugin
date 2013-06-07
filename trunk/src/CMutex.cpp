#pragma once

#include "CMutex.h"

CMutex::CMutex() {
#ifdef WIN32
	InitializeCriticalSection(&m_mutexHandle);
#else
	pthread_mutexattr_t mutexAttr;
	pthread_mutexattr_init(&mutexAttr);
	pthread_mutexattr_settype(&mutexAttr, PTHREAD_MUTEX_RECURSIVE);
	pthread_mutex_init(&m_mutexHandle, &mutexAttr);
#endif
}

CMutex::~CMutex() {
#ifdef WIN32
	DeleteCriticalSection(&m_mutexHandle);
#else
	pthread_mutex_destroy(&m_mutexHandle);
#endif
}


void CMutex::Lock() {
#ifdef WIN32
	EnterCriticalSection(&m_mutexHandle);
#else
	pthread_mutex_lock(&m_mutexHandle);
#endif
}

void CMutex::Unlock() {
#ifdef WIN32
	LeaveCriticalSection(&m_mutexHandle);
#else
	pthread_mutex_unlock(&m_mutexHandle);
#endif
}