#pragma once

#ifndef INC_CMYSQLHANDLER_H
#define INC_CMYSQLHANDLER_H


#include <sstream>
#include <vector>
#include <queue>
#include <map>
#include <string>

#ifdef _WIN32
#include <WinSock2.h>
#endif
#include "mysql_include/mysql.h"
#include "CScripting.h"
#include "CMutex.h"
#include "CCallback.h"

using std::map;
using std::vector;
using std::queue;
using std::stringstream;
using std::string;

#define ERROR_INVALID_CONNECTION_HANDLE(function, id) \
	Log(LOG_ERROR, ">> %s() - Invalid connection handle. (ID = %d).", function, id)

enum eFieldDataType {
	TYPE_INT = 1,
	TYPE_FLOAT,
	TYPE_STRING
};

class CMySQLResult {
public:
	friend class CMySQLQuery;
	
	CMySQLResult();
	~CMySQLResult();

	my_ulonglong GetRowCount() {
		return m_Rows;
	}

	unsigned int GetFieldCount() {
		return m_Fields;
	}

	unsigned int GetFieldName(unsigned int idx, string &dest);
	unsigned int GetRowData(unsigned int row, unsigned int fieldidx, string &dest);
	unsigned int GetRowDataByName(unsigned int row, string field, string &dest);

	my_ulonglong InsertID() {
		return m_InsertID;
	}

	my_ulonglong AffectedRows() {
		return m_AffectedRows;
	}

	unsigned int GetWarningCount() {
		return m_WarningCount;
	}

private:
	unsigned int m_Fields;
	my_ulonglong m_Rows;

	vector<vector<char*>* > m_Data;
	vector<char*> m_FieldNames;
	vector<unsigned int> m_FieldDataTypes;

	my_ulonglong m_InsertID, m_AffectedRows;

	unsigned int m_WarningCount;
};


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
		if(IsValid(cid))
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

	CMySQLResult *GetResult() {
		return m_ActiveResult;
	}

	bool IsActiveResultSaved() {
		return m_ActiveResultID > 0 ? true : false;
	}

	int GetID() {
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


class CMySQLQuery {
public:
	static void ProcessQueryT();
	
	CMySQLQuery();
	~CMySQLQuery() {}
	
	string Query;
	
	CMySQLHandle *ConnHandle;
	MYSQL *ConnPtr;
	CMySQLResult *Result;
	CCallback *Callback;

	static void PushQuery(CMySQLQuery *query) {
		QueryMutex.Lock();
		m_QueryQueue.push(query);
		QueryMutex.Unlock();
	}

	static CMySQLQuery *GetNextQuery() {
		QueryMutex.Lock();
		CMySQLQuery *Query = NULL;
		if(!m_QueryQueue.empty()) {
			Query = m_QueryQueue.front();
			m_QueryQueue.pop();
		}
		QueryMutex.Unlock();
		return Query;
	}

	void ExecuteT();


	static void PushConnect(CMySQLHandle *handle) {
		CMySQLHandle::SQLHandleMutex.Lock();
		ConnectQueue.push(handle);
		CMySQLHandle::SQLHandleMutex.Unlock();
	}
	static void PushDisconnect(CMySQLHandle *handle) {
		CMySQLHandle::SQLHandleMutex.Lock();
		DisconnectQueue.push(handle);
		CMySQLHandle::SQLHandleMutex.Unlock();
	}
	static void PushReconnect(CMySQLHandle *handle) {
		CMySQLHandle::SQLHandleMutex.Lock();
		ReconnectQueue.push(handle);
		CMySQLHandle::SQLHandleMutex.Unlock();
	}
private:
	static CMutex MySQLMutex;
	static CMutex QueryMutex;
	static queue<CMySQLQuery*> m_QueryQueue;

	static int QueryCounter;

	static queue <CMySQLHandle*> ConnectQueue;
	static queue <CMySQLHandle*> DisconnectQueue;
	static queue <CMySQLHandle*> ReconnectQueue;
};

#endif