#pragma once

#ifndef INC_CCALLBACK_H
#define INC_CCALLBACK_H

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
		CallbackQueue.push(cb);
	}

	static CMySQLQuery *GetNextQuery() {
		CMySQLQuery *NextQuery = NULL;
		CallbackQueue.pop(NextQuery);
		return NextQuery;
	}

	stack<string> Parameters;
	string Name;
	string ParamFormat;
	bool IsInline;

	CCallback() :
		IsInline(false)
	{}
	~CCallback() {}
	CCallback(const CCallback &rhs) {
		Parameters = rhs.Parameters;
		Name = rhs.Name;
		ParamFormat = rhs.ParamFormat;
		IsInline = rhs.IsInline;
	}
	void operator=(const CCallback &rhs) {
		Parameters = rhs.Parameters;
		Name = rhs.Name;
		ParamFormat = rhs.ParamFormat;
		IsInline = rhs.IsInline;
	}

private:
	static boost::lockfree::queue<
			CMySQLQuery*, 
			boost::lockfree::fixed_sized<true>,
			boost::lockfree::capacity<10000>
		> CallbackQueue;
};







#endif
