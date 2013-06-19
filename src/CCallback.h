#pragma once

#ifndef INC_CCALLBACK_H
#define INC_CCALLBACK_H

#include "CMutex.h"

#include <queue>
#include <stack>
#include <string>

using std::queue;
using std::stack;
using std::string;

#include "boost/lockfree/queue.hpp"


class CMySQLQuery;

class CCallback {
public:
	friend class CMySQLQuery;

	static void ProcessCallbacks();
	
	static void AddQueryToQueue(CMySQLQuery *cb) {
		//CallbackMutex.Lock();
		CallbackQueue.push(cb);
		//CallbackMutex.Unlock();
	}

	static CMySQLQuery *GetNextQuery() {
		/*CallbackMutex.Lock();
		CMySQLQuery *NextQuery = NULL;
		if(!CallbackQueue.empty()) {
			NextQuery = CallbackQueue.front();
			CallbackQueue.pop();
		}
		CallbackMutex.Unlock();*/
		CMySQLQuery *NextQuery = NULL;
		CallbackQueue.pop(NextQuery);
		return NextQuery;
	}

	stack<string> Parameters;
	string Name;
	string ParamFormat;

	CCallback() {};
	~CCallback() {};
	CCallback(const CCallback &rhs) {
		Parameters = rhs.Parameters;
		Name = rhs.Name;
		ParamFormat = rhs.ParamFormat;
	}
	void operator=(const CCallback &rhs) {
		Parameters = rhs.Parameters;
		Name = rhs.Name;
		ParamFormat = rhs.ParamFormat;
	}

private:
	//static CMutex CallbackMutex;
	//static queue<CMySQLQuery*> CallbackQueue;
	static boost::lockfree::queue<
			CMySQLQuery*, 
			boost::lockfree::fixed_sized<true>,
			boost::lockfree::capacity<10000>
		> CallbackQueue;
};







#endif
