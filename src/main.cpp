#pragma once

#include "main.h"
#include "CMySQLHandler.h"
#include "CScripting.h"

#include "malloc.h"
#include <cstring>

#include "boost/thread.hpp"
#include "boost/threadpool.hpp"



#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include "Windows.h"

#define SLEEP(x) { Sleep(x); }
#else
#include "pthread.h"
#include <unistd.h>

#define SLEEP(x) { usleep(x * 1000); }
#endif

 

list<AMX *> p_Amx;
void **ppPluginData;  
extern void	*pAMXFunctions;
extern logprintf_t logprintf;

bool 
	MultiThreading = false,
	ThreadRunning = true;

 

boost::threadpool::pool *QueryTPool = NULL;


PLUGIN_EXPORT unsigned int PLUGIN_CALL Supports() {
	return SUPPORTS_VERSION | SUPPORTS_AMX_NATIVES | SUPPORTS_PROCESS_TICK; 
}

PLUGIN_EXPORT bool PLUGIN_CALL Load(void **ppData) {
	pAMXFunctions = ppData[PLUGIN_DATA_AMX_EXPORTS];
	logprintf = (logprintf_t)ppData[PLUGIN_DATA_LOGPRINTF];
	
	boost::thread QueryThread(CMySQLQuery::ProcessQueryT);
	QueryThread.detach();
	
	std::ios_base::sync_with_stdio(false);

	logprintf(" >> plugin.mysql: R24 successfully loaded.");
	Native::Log(LOG_DEBUG, "Plugin succesfully loaded!");
	return 1;
}

PLUGIN_EXPORT void PLUGIN_CALL Unload() {
	Native::Log(LOG_DEBUG, "Unloading Plugin");
	logprintf("plugin.mysql: Unloading plugin...");

	ThreadRunning = false;
	while(ThreadRunning == false) { SLEEP(30); }
	p_Amx.clear();
	delete QueryTPool;

	logprintf("plugin.mysql: Plugin unloaded."); 
}

PLUGIN_EXPORT void PLUGIN_CALL ProcessTick() {
	CCallback::ProcessCallbacks();
}

 
void CMySQLQuery::ProcessQueryT()
{
	if (mysql_library_init(0, NULL, NULL)) {
		logprintf("plugin.mysql: Couldn't initialize MySQL library.");
		Native::Log(LOG_ERROR, "Plugin failed to load due to unitialized MySQL library (libmysql probably missing).");
		exit(0);
		return ;
	}
	
	while(ThreadRunning) {

		//Connect queue
		CMySQLHandle::SQLHandleMutex.Lock();
		CMySQLHandle *OldHandle = NULL;
		while(!ReconnectQueue.empty()) {
			CMySQLHandle *Handle = ReconnectQueue.front();
			ReconnectQueue.pop();
			if(Handle != OldHandle) {
				Handle->DisconnectT();
				Handle->ConnectT();
			}
			OldHandle = Handle;
		}
		
		//reconnect queue
		OldHandle = NULL;
		while(!ConnectQueue.empty()) {
			CMySQLHandle *Handle = ConnectQueue.front();
			ConnectQueue.pop();
			if(Handle != OldHandle) {
				Handle->ConnectT();
			}
			OldHandle = Handle;
		}
		CMySQLHandle::SQLHandleMutex.Unlock();

		//executing queries
		CMySQLQuery *Query = NULL;
		while( (Query = GetNextQuery()) != NULL) {
			if(MultiThreading == true)
				QueryTPool->schedule(boost::bind(&ExecuteT, Query));
			else
				Query->ExecuteT();
		}

		if(MultiThreading == true)
			QueryTPool->wait(0);
		
		//disconnect queue
		CMySQLHandle::SQLHandleMutex.Lock();
		CCallback::CallbackMutex.Lock();
		while(!DisconnectQueue.empty()) {
			CMySQLHandle *Handle = DisconnectQueue.front();
			DisconnectQueue.pop();


			Handle->DisconnectT();
			CMySQLHandle::SQLHandle.erase(Handle->m_CID);
			delete Handle;
		}
		CMySQLHandle::SQLHandleMutex.Unlock();
		CCallback::CallbackMutex.Unlock();
		
		SLEEP(10);
	}


	//Delete/unload all things
	if(MultiThreading == true) {
		int PendingQueries = QueryTPool->pending() + QueryTPool->active();
		if(PendingQueries > 0) {
			logprintf("plugin.mysql: There are still %d queries left, waiting for execution...", PendingQueries);
			QueryTPool->wait(0);
			logprintf("plugin.mysql: All queries were executed.");
		}
	}


	CMySQLQuery::QueryMutex.Lock(); 
	CMySQLHandle::SQLHandleMutex.Lock();
	CCallback::CallbackMutex.Lock();

	for(map<int, CMySQLHandle *>::iterator i = CMySQLHandle::SQLHandle.begin(); i != CMySQLHandle::SQLHandle.end(); ++i) {
		i->second->DisconnectT();
		delete i->second;
	}
	CMySQLHandle::SQLHandle.clear();

	while(!CMySQLQuery::m_QueryQueue.empty()) {
		delete CMySQLQuery::m_QueryQueue.front();
		CMySQLQuery::m_QueryQueue.pop();
	}
	
	while(!CCallback::CallbackQueue.empty()) {
		delete CCallback::CallbackQueue.front();
		CCallback::CallbackQueue.pop();
	}
	
	/*
	CMySQLQuery::QueryMutex.Unlock();
	CMySQLHandle::SQLHandleMutex.Unlock();
	CCallback::CallbackMutex.Unlock();
	*/
	mysql_library_end();
	ThreadRunning = true;
}

#if defined __cplusplus
extern "C"
#endif
const AMX_NATIVE_INFO MySQLNatives[] = {
	{"mysql_mt",						Native::mysql_mt},
	{"mysql_log",						Native::mysql_log}, 
	{"mysql_connect",					Native::mysql_connect},
	{"mysql_close",						Native::mysql_close},
	{"mysql_reconnect",					Native::mysql_reconnect},
	
	{"mysql_real_escape_string",		Native::mysql_real_escape_string},
	{"mysql_format",					Native::mysql_format},
	{"mysql_tquery",					Native::mysql_tquery},
	{"mysql_function_query",			Native::mysql_function_query},

	{"mysql_stat",						Native::mysql_stat},
	{"mysql_get_charset",				Native::mysql_get_charset},
	{"mysql_set_charset",				Native::mysql_set_charset},

	{"cache_get_data",					Native::cache_get_data},
	{"cache_get_field",					Native::cache_get_field},
	{"cache_get_row",					Native::cache_get_row},
	{"cache_get_row_int",				Native::cache_get_row_int},
	{"cache_get_row_float",				Native::cache_get_row_float},
	{"cache_get_field_content",			Native::cache_get_field_content},
	{"cache_get_field_content_int",		Native::cache_get_field_content_int},
	{"cache_get_field_content_float",	Native::cache_get_field_content_float},
	{"cache_save",						Native::cache_save},
	{"cache_delete",					Native::cache_delete},
	{"cache_set_active",				Native::cache_set_active},
	{"cache_affected_rows",				Native::cache_affected_rows},
	{"cache_insert_id",					Native::cache_insert_id},
	{"cache_warning_count",				Native::cache_warning_count},
	{NULL, NULL}
};

PLUGIN_EXPORT int PLUGIN_CALL AmxLoad(AMX *amx) {
	p_Amx.push_back(amx);
	return amx_Register(amx, MySQLNatives, -1);
}

PLUGIN_EXPORT int PLUGIN_CALL AmxUnload(AMX *amx) {
	for (list<AMX *>::iterator i = p_Amx.begin(); i != p_Amx.end(); i++) {
		if (* i == amx) {
			p_Amx.erase(i);
			break;
		}
	}
	return AMX_ERR_NONE;
}

