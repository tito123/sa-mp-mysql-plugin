#pragma once
#ifndef INC_CMYSQLHANDLE_H
#define INC_CMYSQLHANDLE_H



#include <string>
#include <boost/unordered_map.hpp>

#ifdef _WIN32
#include <WinSock2.h>
#endif
#include "mysql_include/mysql.h"

#include "CMutex.h"


using std::string;
using boost::unordered_map;


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
	
	CMySQLHandle(string host, string user, string passw, string db, size_t port, bool reconnect);
	~CMySQLHandle();

	static bool IsValid(int id);

	int Connect(bool threaded = false);
	void Disconnect(bool threaded = false);


	static int Create(string host, string user, string pass, string db, size_t port, bool reconnect);
	void Destroy();

	static CMySQLHandle *GetHandle(int cid) {
		CMySQLHandle *Result = NULL;
		Result = SQLHandle.at(cid);
		return Result;
	}

	MYSQL *GetMySQLPointer() {
		MySQLMutex.Lock();
		MYSQL *ptr = m_MySQLConnPtr;
		MySQLMutex.Unlock();
		return ptr;
	}

	void SetMySQLPointer(MYSQL *mysqlptr) {
		MySQLMutex.Lock();
		m_MySQLConnPtr = mysqlptr;
		MySQLMutex.Unlock();
	}
	
	void SetNewResult(CMySQLResult *result) {
		m_ActiveResult = result;
		m_ActiveResultID = 0;
	}
	
	int SaveActiveResult();
	bool DeleteSavedResult(int resultid);
	bool SetActiveResult(int resultid);

	inline CMySQLResult *GetResult() const {
		return m_ActiveResult;
	}

	inline bool IsActiveResultSaved() const {
		return m_ActiveResultID > 0 ? true : false;
	}

	inline int GetID() const {
		return m_CID;
	}

	inline bool IsAutoReconnectEnabled() const {
		return m_AutoReconnect;
	}

	int CallErrno() {
		m_ErrnoVal = mysql_errno(m_MySQLConnPtr);
		return m_ErrnoVal;
	}
	inline int GetErrno() const {
		return m_ErrnoVal;
	}

	CMutex MySQLMutex;
	static void ClearAll();
private:
	static unordered_map<int, CMySQLHandle *> SQLHandle;
	
	unordered_map<int, CMySQLResult*> m_SavedResults;

	CMySQLResult *m_ActiveResult;
	int m_ActiveResultID; //ID of stored result; 0 if not stored yet
	

	bool m_Connected;
	bool m_AutoReconnect;
	int m_CID;
	int m_ErrnoVal;

	string m_Hostname, m_Username, m_Password, m_Database;
	size_t m_iPort;

	MYSQL *m_MySQLConnPtr;
};


#endif
