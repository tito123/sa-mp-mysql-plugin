#pragma once
#ifndef INC_CLOG_H
#define INC_CLOG_H

#define _CRT_SECURE_NO_WARNINGS
#ifndef NULL
	#define NULL 0
#endif

#include <stdarg.h>
#include <cstring>
#include <string>
using std::string;

#define WINVER 0x0501
#define _WIN32_WINNT 0x0501
#define BOOST_THREAD_DONT_USE_CHRONO
#include "boost/thread/thread.hpp"
#include "boost/lockfree/queue.hpp"


enum e_LogLevel {
	LOG_NONE = 0,
	LOG_ERROR = 1,
	LOG_WARNING = 2,
	LOG_DEBUG = 4
};

enum e_CallbackStatus {
	CB_BEGIN = 1,
	CB_END = 2
};

enum e_LogType {
	LOG_TYPE_TEXT = 1,
	LOG_TYPE_HTML = 2
};

class CLog {
public:
	static CLog *Get() {
		if(m_Instance == NULL)
			m_Instance = new CLog;
		return m_Instance;
	}
	static void Delete() {
		delete m_Instance;
	}

	void Initialize(const char *logfile);
	void LogFunction(unsigned int status, bool threaded, char *funcname, char *msg, ...);
	void StartCallback(const char *cbname);
	void EndCallback();

	void SetLogLevel(unsigned int loglevel) {
		m_LogLevel = loglevel;
	}
	bool IsLogLevel(unsigned int loglevel) {
		return !!(m_LogLevel & loglevel);
	}
	void SetLogType(unsigned int logtype);
	
private:
	static CLog *m_Instance;
	
	struct m_SLogData {
		m_SLogData() :
			Status(LOG_NONE),
			Name(NULL), Msg(NULL),
			IsCallback(false),
			IsThreaded(false)
		{}

		unsigned int Status;
		char *Name, *Msg;
		
		bool IsCallback;
		bool IsThreaded;

		~m_SLogData() {
			free(Name);
			free(Msg);
		}
	};

	CLog() : 
		m_LogLevel(LOG_ERROR | LOG_WARNING), 
		m_LogThread(NULL), 
		m_LogThreadAlive(true),
		m_LogType(LOG_TYPE_TEXT),

		IsTableOpen(false),
		IsCallbackActive(false),
		IsCallbackOpen(false),
		ToggleHeader(false)
	{}
	~CLog();

	void ProcessLog();
	void TextLog(unsigned int level, char* text);

	unsigned int m_LogType;
	unsigned int m_LogLevel;
	boost::thread *m_LogThread;
	bool m_LogThreadAlive;
	char m_LogFileName[32];

	boost::lockfree::queue<
			m_SLogData*, 
			boost::lockfree::fixed_sized<true>,
			boost::lockfree::capacity<10000>
		> m_LogQueue;




	bool IsTableOpen;
	string CallbackName;
	bool IsCallbackActive;
	bool IsCallbackOpen;
	bool ToggleHeader;

	bool IsCallbackOpenF();
	void CloseTable(string &dest);
	void OpenTable(string &dest, bool header = false);
	void OpenCallback(string &dest);
	void CloseCallback(string &dest);
	void SetActiveCallback(string cbname);

	
	
};



#endif