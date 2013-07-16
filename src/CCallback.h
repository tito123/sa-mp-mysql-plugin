#pragma once

#ifndef INC_CCALLBACK_H
#define INC_CCALLBACK_H

#include <list>
#include <stack>
#include <string>

using std::list;
using std::stack;
using std::string;

#include "boost/lockfree/queue.hpp"

#include "main.h"


class CMySQLQuery;

class CCallback {
public:

	static void ProcessCallbacks();
	
	static void AddQueryToQueue(CMySQLQuery *cb);
	static CMySQLQuery *GetNextQuery();

	static void AddAmx(AMX *amx);
	static void EraseAmx(AMX *amx);

	static void ClearAll();


	stack<string> Parameters;
	string Name;
	string ParamFormat;
	bool IsInline;


	CCallback() :
		IsInline(false)
	{}
	~CCallback() {}
	CCallback(const CCallback &rhs);
	void operator=(const CCallback &rhs);

private:
	static boost::lockfree::queue<
			CMySQLQuery*, 
			boost::lockfree::fixed_sized<true>,
			boost::lockfree::capacity<10000>
		> CallbackQueue;

	static list<AMX *> AmxList;
};


#endif
