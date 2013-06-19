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

#define BOOST_THREAD_DONT_USE_CHRONO
#include "boost/thread/thread.hpp"
#include "boost/lockfree/queue.hpp"


int sprintf2(char *out, const char *format, ...);

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
	void LogFunction(unsigned int status, char *funcname, char *msg, bool threaded=false/*, ...*/);
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
		m_LogType(LOG_TYPE_TEXT)
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

	bool IsCallbackOpenF() {
		return IsCallbackOpen;
	}
	inline void CloseTable(string &dest) {
		if(IsTableOpen == true) {
			dest.append("\t\t</tbody>\n\t</table>\n");
			IsTableOpen = false;
		}
	}
	inline void OpenTable(string &dest, bool header = false) {
		OpenCallback(dest);
		if(IsTableOpen == false) {
			dest.append("\t<table");
			if(IsCallbackOpen == true)
				dest.append(" class=left2");
			dest.append(">\n\t\t");
			
			if( (ToggleHeader && IsCallbackOpen) || header == true) {
				dest.append("<thead>\n\t\t\t<th>Time</th>\n\t\t\t<th>Function</th>\n\t\t\t<th>Status</th>\n\t\t\t<th>Message</th>\n\t\t</thead>\n\t\t");
				ToggleHeader = false;
			}
			dest.append("<tbody>\n\t\t");
			IsTableOpen = true;
		}
	}
	inline void OpenCallback(string &dest) {
		if(IsCallbackActive == true) {
			dest.append("\t<table class=left2>\n\t\t<th bgcolor=#C0C0C0 >In callback \"");
			dest.append(CallbackName);
			dest.append("\"</th>\n\t</table>\n");
			IsCallbackOpen = true;
			IsCallbackActive = false;
			ToggleHeader = true;
		}
	}
	inline void CloseCallback(string &dest) {
		if(IsCallbackActive == false) {
			CloseTable(dest);
		}
		IsCallbackOpen = false;
		CallbackName.clear();
	}
	inline void SetActiveCallback(string cbname) {
		CallbackName = cbname;
		IsCallbackActive = true;
	}

	
	
};



#endif