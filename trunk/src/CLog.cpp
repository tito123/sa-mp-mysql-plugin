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

	fprintf(LogFile, "<html>\n<head><title>MySQL Plugin log</title>\n<style>\ntable {width: 600px; border: 1px solid black; border-collapse: collapse;} \
					 \nth, td {border: 1px solid black;}\nthead {background-color: #C0C0C0;}\ntbody {text-align: center;}\n \
					 table.left1 {position: relative; left: 36px;}\ntable.left2 {position: relative; left: 72px;}\n</style>\n\t \
					 </head>\n\n<body>\n\t<h1>Logging started at %s</h1>\n", StartLogTime);
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
				FileData.append(">\n\t\t\t\t<td>");
				FileData.append(timeform);
				FileData.append("</td>\n\t\t\t\t<td>");
				FileData.append(LogData->Name);
				FileData.append("</td>\n\t\t\t\t<td>");
				FileData.append(StatusText);
				FileData.append("</td>\n\t\t\t\t<td>");
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

void CLog::LogFunction(unsigned int status, char *funcname, char *msg, bool threaded) {
	if(m_LogLevel == LOG_NONE)
		return ;
	switch(m_LogType) {
		case LOG_TYPE_HTML: {
			if (m_LogLevel & status) {
		
				m_SLogData *LogData = new m_SLogData;

				LogData->IsCallback = false;
				LogData->IsThreaded = threaded;
				LogData->Status = status;
				LogData->Msg = (char *)malloc((strlen(msg)+1) * sizeof(char));
				strcpy(LogData->Msg, msg);
				LogData->Name = (char *)malloc((strlen(funcname)+1) * sizeof(char));
				strcpy(LogData->Name, funcname);
		
				m_LogQueue.push(LogData);
			}
		} break;

		case LOG_TYPE_TEXT: {
			char *LogText = (char *)malloc((strlen(funcname) + strlen(msg) + 8) * sizeof(char));
			sprintf2(LogText, "%s - %s", funcname, msg);
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
		sprintf2(LogText, "Calling callback \"%s\"..", cbname);
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
	m_LogThreadAlive = false;
	m_LogThread->join();
	delete m_LogThread;
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




static void printchar(char **str, int c)
{
	extern int putchar(int c);
	if (str) {
		**str = c;
		++(*str);
	}
	else (void)putchar(c);
}

static int prints(char **out, const char *string, int width, int pad)
{
	register int pc = 0;

	for ( ; *string ; ++string) {
		printchar (out, *string);
		++pc;
	}
	return pc;
}

/* the following should be enough for 32 bit int */
#define PRINT_BUF_LEN 12

static int printi(char **out, int i, int b, int sg, int width, int pad, int letbase)
{
	char print_buf[PRINT_BUF_LEN];
	register char *s;
	register int t, neg = 0, pc = 0;
	register unsigned int u = i;

	if (i == 0) {
		print_buf[0] = '0';
		print_buf[1] = '\0';
		return prints (out, print_buf, width, pad);
	}

	if (sg && b == 10 && i < 0) {
		neg = 1;
		u = -i;
	}

	s = print_buf + PRINT_BUF_LEN-1;
	*s = '\0';

	while (u) {
		t = u % b;
		if( t >= 10 )
			t += letbase - '0' - 10;
		*--s = t + '0';
		u /= b;
	}

	if (neg) {
		*--s = '-';
	}

	return pc + prints (out, s, width, pad);
}

static int print2(char **out, int *varg)
{
	register int width, pad;
	register int pc = 0;
	register char *format = (char *)(*varg++);

	for (; *format != 0; ++format) {
		if (*format == '%') {
			++format;
			width = pad = 0;
			if (*format == '\0') break;
			if( *format == 's' ) {
				register char *s = *((char **)varg++);
				pc += prints (out, s?s:"(null)", width, pad);
				continue;
			}
			if( *format == 'd' ) {
				pc += printi (out, *varg++, 10, 1, width, pad, 'a');
				continue;
			}
		}
		else {
		//out:
			printchar (out, *format);
			++pc;
		}
	}
	if (out) **out = '\0';
	return pc;
}

/* assuming sizeof(void *) == sizeof(int) */
/*
int printf(const char *format, ...)
{
	register int *varg = (int *)(&format);
	return print(0, varg);
}*/

int sprintf2(char *out, const char *format, ...)
{
	register int *varg = (int *)(&format);
	return print2(&out, varg);
}