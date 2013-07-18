#pragma once
#ifndef INC_CMYSQLHANDLE_H
#define INC_CMYSQLHANDLE_H



#include <string>
#include <boost/unordered_map.hpp>

using std::string;
using boost::unordered_map;


#ifdef _WIN32
	#include <WinSock2.h>
#endif
#include "mysql_include/mysql.h"


#include "CMutex.h"



#define ERROR_INVALID_CONNECTION_HANDLE(function, id) \
	CLog::Get()->LogFunction(LOG_ERROR, false, #function, "invalid connection handle. (ID = %d).", id)


class CMySQLResult;

class CMySQLHandle {
public:
	
	CMySQLHandle(int id, string host, string user, string passw, string db, size_t port, bool reconnect);
	~CMySQLHandle();

	static bool IsValid(int id);

	int Connect(bool threaded = false);
	void Disconnect(bool threaded = false);


	static CMySQLHandle *Create(string host, string user, string pass, string db, size_t port, bool reconnect);
	void Destroy();
	static inline CMySQLHandle *GetHandle(int cid) {
		return SQLHandle.at(cid);
	}

	MYSQL *GetMySQLPointer();
	void SetMySQLPointer(MYSQL *mysqlptr);
	
	void SetNewResult(CMySQLResult *result);
	
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

	static void ClearAll();

	CMutex MySQLMutex;
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
