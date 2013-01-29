#pragma once

#include "CMutex.h"

Mutex* Mutex::m_pInstance = NULL;
bool Mutex::m_gEnable = false;

Mutex::Mutex()
{
	// constructor
#ifdef WIN32
	m_mutexHandle = CreateMutex(NULL, FALSE, LPCWSTR("mysql_r7"));
#else
	m_mutexHandle = PTHREAD_MUTEX_INITIALIZER;
	//pthread_mutex_init(&m_mutexHandle, NULL); 
#endif
}

Mutex::~Mutex()
{
	// deconstructor
#ifdef WIN32
	CloseHandle(m_mutexHandle);
#else
	pthread_mutex_destroy(&m_mutexHandle);
#endif
	m_pInstance = NULL;
	m_gEnable = false;
}

Mutex* Mutex::getInstance() 
{
	// based on the singleton structure
	if(m_pInstance == NULL) {
		m_pInstance = new Mutex();
	}
	return m_pInstance;
}

void Mutex::_lockMutex()
{
	if(m_gEnable) {
	#ifdef WIN32
		WaitForSingleObject(m_mutexHandle, INFINITE);
	#else
		pthread_mutex_lock(&m_mutexHandle);
	#endif
	}
}

void Mutex::_unlockMutex()
{
	if(m_gEnable) {
	#ifdef WIN32
		ReleaseMutex(m_mutexHandle);
	#else
		pthread_mutex_unlock(&m_mutexHandle);
	#endif
	}
}