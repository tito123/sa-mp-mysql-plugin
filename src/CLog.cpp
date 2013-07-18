#pragma once

#include "CLog.h"

#include <cstdio>

#include <string>
#include <queue>

using std::string;
using std::queue;



#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include "Windows.h" 
#define SLEEP(x) { Sleep(x); }
#else
#include "pthread.h"
#include <unistd.h>
#define SLEEP(x) { usleep(x * 1000); }
#endif


CLog *CLog::m_Instance = NULL;

void CLog::ProcessLog() {
	FILE *LogFile = fopen(m_LogFileName, "a");
	bool ThreadedTable = false;
	bool FirstInit = true;

	char TableColor[8];
	char StatusText[16]; 



	char StartLogTime[32];
	time_t StartLogTimeRaw;
	time(&StartLogTimeRaw);
	tm * StartLogTimeInfo;
	StartLogTimeInfo = localtime(&StartLogTimeRaw);
	strftime(StartLogTime, sizeof(StartLogTime), "%H:%M, %d.%m.%Y", StartLogTimeInfo);

	fprintf(LogFile, "<html>\n<head><title>MySQL Plugin log</title>\n<style>\n" \
					 "table {border: 1px solid black; border-collapse: collapse; line-height: 23px;}\n" \
					 "th, td {border: 1px solid black;}\nthead {background-color: #C0C0C0;}\ntbody {text-align: center;}\n" \
					 "table.left1 {position: relative; left: 36px;}\ntable.left2 {position: relative; left: 72px;}\n" \
					 "td.time {width: 80px;}\ntd.func {width: 300px;}\ntd.stat {width: 70px;}\ntd.msg {width: 400px;}" \
					 "</style>\n\t</head>\n\n<body>\n\t<h1>Logging started at %s</h1>\n", StartLogTime);
	fflush(LogFile);

	while(m_LogThreadAlive) {
		
		m_SLogData *LogData = NULL;
		while(m_LogQueue.pop(LogData)) {
			string FileData;
			FileData.reserve(1000);

			if(LogData->IsCallback == true) {
				
				CloseTable(FileData);
				if(LogData->Status == CB_BEGIN)
					SetActiveCallback(LogData->Name);
				else if(LogData->Status == CB_END)
					CloseCallback(FileData);
				
			}
			else {
				char timeform[16];
				time_t rawtime;
				time(&rawtime);
				struct tm * timeinfo;
				timeinfo = localtime(&rawtime);
				strftime(timeform, sizeof(timeform), "%X", timeinfo);
			
				switch(LogData->Status) {
					case LOG_ERROR:
						strcpy(TableColor, "RED");
						strcpy(StatusText, "ERROR");
						break;
					case LOG_WARNING:
						strcpy(TableColor, "#FF9900");
						strcpy(StatusText, "WARNING");
						break;
					case LOG_DEBUG:
						strcpy(TableColor, "#00DD00");
						strcpy(StatusText, "OK");
						break;
				}


				
				if(LogData->IsThreaded == true) {
					if(ThreadedTable == false) {
						CloseTable(FileData);
						FileData.append("\t<table class=left1>\n\t\t");
						FileData.append("<tbody>\n\t\t");
						ThreadedTable = true;
					}
				}
				else if(LogData->IsThreaded == false) {
					if(ThreadedTable == true) {
						FileData.append("\t\t</tbody>\n\t</table>\n");
						ThreadedTable = false;
						
						OpenTable(FileData, FirstInit);
						
					}
					else
						OpenTable(FileData, FirstInit);
				
				}

				
				FileData.append("\t\t\t<tr bgcolor=");
				FileData.append(TableColor);
				FileData.append(">\n\t\t\t\t<td class=time>");
				FileData.append(timeform);
				FileData.append("</td>\n\t\t\t\t<td class=func>");
				FileData.append(LogData->Name);
				FileData.append("</td>\n\t\t\t\t<td class=stat>");
				FileData.append(StatusText);
				FileData.append("</td>\n\t\t\t\t<td class=msg>");
				FileData.append(LogData->Msg);
				FileData.append("</td>\n\t\t\t</tr>\n");

				
			}
			fputs(FileData.c_str(), LogFile);
			fflush(LogFile);
			

			delete LogData;

			if(FirstInit == true)
				FirstInit = false;
		}
		SLEEP(20);
		
	}
	if(IsTableOpen == true)
		fputs("\t\t</tbody>\n\t</table>\n", LogFile);
	fputs("</body>\n\n</html>", LogFile);
	fclose(LogFile);
}

void CLog::Initialize(const char *logfile) {
	strcpy(m_LogFileName, logfile);
	SetLogType(m_LogType);
}

void CLog::SetLogType(unsigned int logtype)  {
	if(logtype != LOG_TYPE_HTML && logtype != LOG_TYPE_TEXT)
		return ;
	if(logtype == m_LogType)
		return ;
	m_LogType = logtype;
	if(logtype == LOG_TYPE_HTML) {
		if(m_LogThread == NULL)
			m_LogThread = new boost::thread(boost::bind(&CLog::ProcessLog, this));

		string FileName(m_LogFileName);
		int Pos = FileName.find_first_of(".");
		FileName.erase(Pos, FileName.size() - Pos);
		FileName.append(".html");
		strcpy(m_LogFileName, FileName.c_str());
	}
	else if(logtype == LOG_TYPE_TEXT) {
		string FileName(m_LogFileName);
		int Pos = FileName.find_first_of(".");
		FileName.erase(Pos, FileName.size() - Pos);
		FileName.append(".txt");
		strcpy(m_LogFileName, FileName.c_str());
	}
}

//void CLog::LogFunction(unsigned int status, string funcname, string msg, bool threaded) {}

void CLog::LogFunction(unsigned int status, bool threaded, char *funcname, char *msg, ...) {
	if(m_LogLevel == LOG_NONE)
		return ;
	switch(m_LogType) {
		case LOG_TYPE_HTML: {
			if (m_LogLevel & status) {
		
				m_SLogData *LogData = new m_SLogData;

				LogData->IsCallback = false;
				LogData->IsThreaded = threaded;
				LogData->Status = status;

				LogData->Msg = (char *)malloc(2048 * sizeof(char));
				va_list args;
				va_start(args, msg);
				vsprintf(LogData->Msg, msg, args);
				va_end (args);

				LogData->Name = (char *)malloc((strlen(funcname)+1) * sizeof(char));
				strcpy(LogData->Name, funcname);
		
				m_LogQueue.push(LogData);
			}
		} break;

		case LOG_TYPE_TEXT: {
			char MsgBuf[2048];
			int RealMsgLen=0;
			va_list args;
			va_start(args, msg);
			RealMsgLen = vsprintf(MsgBuf, msg, args);
			va_end (args);
			
			char *LogText = (char *)malloc((strlen(funcname) + RealMsgLen + 8) * sizeof(char));
			sprintf(LogText, "%s - %s", funcname, MsgBuf);
			TextLog(status, LogText);
			free(LogText);
		} break;
	}
}


void CLog::StartCallback(const char *cbname) {
	if(m_LogLevel == LOG_NONE)
		return ;
	if(m_LogType == LOG_TYPE_HTML) {
		m_SLogData *LogData = new m_SLogData;
		LogData->IsCallback = true;
		LogData->Name = (char *)malloc((strlen(cbname)+1) * sizeof(char));
		strcpy(LogData->Name, cbname);
		LogData->Status = CB_BEGIN;

		m_LogQueue.push(LogData);
	}
	else if(m_LogType == LOG_TYPE_TEXT) {
		char LogText[64];
		sprintf(LogText, "Calling callback \"%s\"..", cbname);
		TextLog(LOG_DEBUG, LogText);
	}
}

void CLog::EndCallback() {
	if(m_LogType != LOG_TYPE_HTML)
		return ;

	if(m_LogLevel == LOG_NONE)
		return ;
	
	m_SLogData *LogData = new m_SLogData;
	LogData->IsCallback = true;
	LogData->Status = CB_END;

	m_LogQueue.push(LogData);
}


CLog::~CLog() {
	if(m_LogThread != NULL) {
		m_LogThreadAlive = false;
		m_LogThread->join();
		delete m_LogThread;
	}
}

void CLog::TextLog(unsigned int level, char* text) {
    if (m_LogLevel & level) {
        char prefix[16];
        switch(level) {
        case LOG_ERROR:
                sprintf(prefix, "ERROR");
                break;
        case LOG_WARNING:
                sprintf(prefix, "WARNING");
                break;
        case LOG_DEBUG:
                sprintf(prefix, "DEBUG");
                break;
        }
        char timeform[16];
        time_t rawtime;
        time(&rawtime);
        struct tm * timeinfo;
        timeinfo = localtime(&rawtime);
        strftime(timeform, sizeof(timeform), "%X", timeinfo);

        FILE *file = fopen(m_LogFileName, "a");
        if(file != NULL) {
                fprintf(file, "[%s] [%s] %s\n", timeform, prefix, text);
                fclose(file);
        }
                
    }
}



void CLog::OpenCallback( string &dest )
{
	if(IsCallbackActive == true) {
		dest.append("\t<table class=left2 style=\"width: 863px;\">\n\t\t<th bgcolor=#C0C0C0 >In callback \"");
		dest.append(CallbackName);
		dest.append("\"</th>\n\t</table>\n");
		IsCallbackOpen = true;
		IsCallbackActive = false;
		ToggleHeader = true;
	}
}

void CLog::CloseCallback( string &dest )
{
	if(IsCallbackActive == false) {
		CloseTable(dest);
	}
	IsCallbackOpen = false;
	CallbackName.clear();
}

void CLog::SetActiveCallback( string cbname )
{
	CallbackName = cbname;
	IsCallbackActive = true;
}

void CLog::OpenTable( string &dest, bool header /*= false*/ )
{
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
		dest.append("<tbody>\n\t");
		IsTableOpen = true;
	}
}

void CLog::CloseTable( string &dest )
{
	if(IsTableOpen == true) {
		dest.append("\t\t</tbody>\n\t</table>\n");
		IsTableOpen = false;
	}
}

bool CLog::IsCallbackOpenF()
{
	return IsCallbackOpen;
}
