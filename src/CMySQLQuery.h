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

	void Execute();


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

	static void InitializeThreadPool(size_t numthreads);
	static void DeleteThreadPool();
	static bool IsThreadPoolInitialized();
	static void WaitForThreadPool();
private:
	static boost::threadpool::pool *QueryThreadPool;
};

#endif