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


class CMySQLQuery;

class CCallback {
public:
	friend class CMySQLQuery;

	static void ProcessCallbacks();
	
	static void AddQueryToQueue(CMySQLQuery *cb) {
		CallbackMutex.Lock();
		CallbackQueue.push(cb);
		CallbackMutex.Unlock();
	}

	static CMySQLQuery *GetNextQuery() {
		CallbackMutex.Lock();
		CMySQLQuery *NextQuery = NULL;
		if(!CallbackQueue.empty()) {
			NextQuery = CallbackQueue.front();
			CallbackQueue.pop();
		}
		CallbackMutex.Unlock();
		return NextQuery;
	}

	stack<string> Parameters;
	string Name;
	string ParamFormat;

private:
	static CMutex CallbackMutex;
	static queue<CMySQLQuery*> CallbackQueue;
	
};







#endif
