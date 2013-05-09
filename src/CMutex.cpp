#pragma once

#include "CMutex.h"

Mutex* Mutex::m_pInstance = NULL;
#ifdef WIN32
bool Mutex::m_gEnable = true;
#else
bool Mutex::m_gEnable = false;
#endif

Mutex::Mutex() {
#ifdef WIN32
	m_mutexHandle = CreateMutex(NULL, FALSE, "mysql_r20");
#else
	//m_mutexHandle = PTHREAD_MUTEX_INITIALIZER;
	pthread_mutexattr_t mutexAttr;
	pthread_mutexattr_init(&mutexAttr);
	pthread_mutexattr_settype(&mutexAttr, PTHREAD_MUTEX_RECURSIVE);
	pthread_mutex_init(&m_mutexHandle, &mutexAttr);
#endif
}

Mutex::~Mutex() {
#ifdef WIN32
	CloseHandle(m_mutexHandle);
#else
	pthread_mutex_destroy(&m_mutexHandle);
#endif
	m_pInstance = NULL;
	m_gEnable = false;
}

Mutex* Mutex::getInstance() {
	if (m_pInstance == NULL) {
		m_pInstance = new Mutex();
	}
	return m_pInstance;
}

void Mutex::_lockMutex() {
	if (m_gEnable) {
#ifdef WIN32
		WaitForSingleObject(m_mutexHandle, INFINITE);
#else
		pthread_mutex_lock(&m_mutexHandle);
#endif
	}
}

void Mutex::_unlockMutex() {
	if (m_gEnable) {
#ifdef WIN32
		ReleaseMutex(m_mutexHandle);
#else
		pthread_mutex_unlock(&m_mutexHandle);
#endif
	}
}