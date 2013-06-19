#pragma once
#ifndef INC_CMYSQLQUERY_H
#define INC_CMYSQLQUERY_H


#include <queue>
#include <string>
#include "boost/lockfree/queue.hpp"

using std::queue;
using std::string;


//#include "CMySQLResult.h"
#include "CMySQLHandle.h"

class CCallback;
class CMySQLResult;

class CMySQLQuery {
public:
	static void ProcessQueryT();
	
	CMySQLQuery();
	~CMySQLQuery() {}
	CMySQLQuery(const CMySQLQuery &rhs) {
		Query = rhs.Query;
		ConnHandle = rhs.ConnHandle;
		ConnPtr = rhs.ConnPtr;
		Result = rhs.Result;
		Callback = rhs.Callback;
	}
	void operator=(const CMySQLQuery &rhs) {
		Query = rhs.Query;
		ConnHandle = rhs.ConnHandle;
		ConnPtr = rhs.ConnPtr;
		Result = rhs.Result;
		Callback = rhs.Callback;
	}
	
	string Query;
	
	CMySQLHandle *ConnHandle;
	MYSQL *ConnPtr;
	CMySQLResult *Result;
	CCallback *Callback;

	static void PushQuery(CMySQLQuery *query) {
		//QueryMutex.Lock();
		m_QueryQueue.push(query);
		//QueryMutex.Unlock();
	}

	static CMySQLQuery *GetNextQuery() {
		/*QueryMutex.Lock();
		CMySQLQuery *Query = NULL;
		if(!m_QueryQueue.empty()) {
			Query = m_QueryQueue.front();
			m_QueryQueue.pop();
		}
		QueryMutex.Unlock();*/
		CMySQLQuery *Query = NULL;
		m_QueryQueue.pop(Query);
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
	//static CMutex QueryMutex;
	//static queue<CMySQLQuery*> m_QueryQueue;
	static boost::lockfree::queue<
			CMySQLQuery*, 
			boost::lockfree::fixed_sized<true>,
			boost::lockfree::capacity<10000>
		> m_QueryQueue;
	

	static queue <CMySQLHandle*> ConnectQueue;
	static queue <CMySQLHandle*> DisconnectQueue;
	static queue <CMySQLHandle*> ReconnectQueue;
};

#endif