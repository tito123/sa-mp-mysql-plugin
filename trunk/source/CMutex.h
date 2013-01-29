#pragma once

#include "../main.h"

class Mutex 
{

public:
	static bool m_gEnable;
	static Mutex* getInstance();
	void _lockMutex();
	void _unlockMutex();
	~Mutex();

protected:
	Mutex();

private:
	static Mutex* m_pInstance;
#ifdef WIN32
	HANDLE m_mutexHandle;
#else
	pthread_mutex_t m_mutexHandle;
#endif

};