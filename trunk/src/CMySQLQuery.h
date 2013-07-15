#pragma once
#ifndef INC_CMYSQLQUERY_H
#define INC_CMYSQLQUERY_H


#include <queue>
#include <string>

#define BOOST_THREAD_DONT_USE_CHRONO
#include "boost/threadpool.hpp"

using std::queue;
using std::string;


#include "CMySQLHandle.h"

class CCallback;
class CMySQLResult;

class CMySQLQuery {
public:
	CMySQLQuery() :
		ConnHandle(NULL),
		Result(NULL),
		Callback(NULL)
	{ }
	~CMySQLQuery() {}
	
	string Query;

	CMySQLHandle *ConnHandle;
	CMySQLResult *Result;
	CCallback *Callback;

	static inline void ScheduleQuery(CMySQLQuery *query) {
		QueryThreadPool->schedule(boost::bind(&Execute, query));
	}

	void Execute();

	static void InitializeThreadPool(size_t numthreads) {
		if(QueryThreadPool == NULL && numthreads > 0)
			QueryThreadPool = new boost::threadpool::pool(numthreads);
	}
	static void DeleteThreadPool() {
		if(QueryThreadPool != NULL) {
			QueryThreadPool->wait(0);
			delete QueryThreadPool;
		}
	}
	static bool IsThreadPoolInitialized() {
		return QueryThreadPool == NULL ? false : true;
	}
	static void WaitForThreadPool() {
		if(QueryThreadPool != NULL)
			QueryThreadPool->wait(0);
	}
private:
	static boost::threadpool::pool *QueryThreadPool;
};

#endif