#pragma once
#ifndef INC_CMYSQLHANDLE_H
#define INC_CMYSQLHANDLE_H


//#include <sstream>
#include <string>
#include <map>

#ifdef _WIN32
#include <WinSock2.h>
#endif
#include "mysql_include/mysql.h"

//#include "CScripting.h"
#include "CMutex.h"
//#include "CCallback.h"


//using std::stringstream;
using std::string;
using std::map;


#define ERROR_INVALID_CONNECTION_HANDLE(function, id) \
	do { \
		if(CLog::Get()->IsLogLevel(LOG_ERROR)) { \
			char InvConHandleBuf[64]; \
			sprintf(InvConHandleBuf, "invalid connection handle. (ID = %d).", id); \
			CLog::Get()->LogFunction(LOG_ERROR, #function, InvConHandleBuf); \
		} \
	} while(false)


class CMySQLResult;

class CMySQLHandle {
public:
	friend class CMySQLQuery;
	
	CMySQLHandle(string host, string user, string passw, string db, size_t port);
	~CMySQLHandle();

	static bool IsValid(int id);

	bool ConnectT(); //should only be called in the thread
	void DisconnectT(); //should only be called in the thread


	static int Create(string host, string user, string pass, string db, size_t port);

	static CMySQLHandle *GetHandle(int cid) {
		SQLHandleMutex.Lock();
		CMySQLHandle *Result = NULL;
		//if(IsValid(cid))
		Result = SQLHandle.at(cid);
		SQLHandleMutex.Unlock();
		return Result;
	}

	MYSQL *GetMySQLPointer() {
		SQLHandleMutex.Lock();
		MYSQL *ptr = m_ThreadConnPtr;
		SQLHandleMutex.Unlock();
		return ptr;
	}

	void SetMySQLPointer(MYSQL *mysqlptr) {
		SQLHandleMutex.Lock();
		m_ThreadConnPtr = mysqlptr;
		SQLHandleMutex.Unlock();
	}
	
	void SetNewResult(CMySQLResult *result) {
		m_ActiveResult = result;
		m_ActiveResultID = 0;
	}
	
	int SaveActiveResult();
	bool DeleteSavedResult(int resultid);
	bool SetActiveResult(int resultid);

	inline CMySQLResult *GetResult() {
		return m_ActiveResult;
	}

	inline bool IsActiveResultSaved() {
		return m_ActiveResultID > 0 ? true : false;
	}

	inline int GetID() {
		return m_CID;
	}
private:
	static CMutex SQLHandleMutex;
	static map<int, CMySQLHandle *> SQLHandle;
	
	map<int, CMySQLResult*> m_SavedResults;

	CMySQLResult *m_ActiveResult;
	int m_ActiveResultID; //ID of stored result; 0 if not stored yet
	

	bool m_Connected;
	int m_CID;

	string m_Hostname, m_Username, m_Password, m_Database;
	size_t m_iPort;

	MYSQL *m_ThreadConnPtr;
};


#endif
