#pragma once

#include "main.h"
#include "malloc.h"
//#include "misc.h"

list<AMX *> p_Amx;
void **ppPluginData;
extern void	*pAMXFunctions;
extern logprintf_t logprintf;
#ifdef WIN32
	HANDLE threadHandle;
	DWORD __stdcall ProcessQueryThread(LPVOID lpParam);
#else
	void * ProcessQueryThread(void *lpParam);
#endif

PLUGIN_EXPORT unsigned int PLUGIN_CALL Supports() 
{
	return SUPPORTS_VERSION | SUPPORTS_AMX_NATIVES | SUPPORTS_PROCESS_TICK;
}

PLUGIN_EXPORT bool PLUGIN_CALL Load( void **ppData ) 
{
	pAMXFunctions = ppData[PLUGIN_DATA_AMX_EXPORTS];
	logprintf = (logprintf_t)ppData[PLUGIN_DATA_LOGPRINTF];
	if(mysql_library_init(0, NULL, NULL))
	{
		logprintf("Couldn't initialize MySQL library.");
		Natives::getInstance()->Debug("Can't start SQL-plugin due to missing library. (mysqllib)");
		exit(0);
		return 0;
	}
	logprintf( "\n\n  > MySQL plugin R8 (google code Revision 5) successfully loaded.\n");
#ifdef WIN32
	DWORD dwThreadId = 0;
	threadHandle = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)ProcessQueryThread, NULL, 0, &dwThreadId);
	CloseHandle(threadHandle);
#else
	pthread_t threadHandle;
	pthread_attr_t attr;
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
	int error = pthread_create(&threadHandle, &attr, &ProcessQueryThread, NULL);
#endif
	// initialize the mutex
	Mutex::getInstance();
	return 1;
}

PLUGIN_EXPORT void PLUGIN_CALL Unload()
{
	Natives::getInstance()->Debug("Unloading Plugin");
	for(vector<CMySQLHandler *>::iterator i = SQLHandle.begin();i != SQLHandle.end();++i) {
		delete * i;
		SQLHandle.erase(i);
	}
	SQLHandle.clear();
	p_Amx.clear();
	delete Mutex::getInstance();
	delete Natives::getInstance();
	logprintf("\n\n  > MySQL plugin unloaded.\n");
}

PLUGIN_EXPORT void PLUGIN_CALL ProcessTick()
{
	if(SQLHandle.size() > 0) {
		for(unsigned int i = 0;i < SQLHandle.size();i++) {
			//Mutex::getInstance()->_lockMutex();
			if(!SQLHandle[i]->errorCallback.empty()) {
				CMySQLHandler::errorInfo tempData = SQLHandle[i]->errorCallback.front();
				cell amx_Address[3], amx_Ret, *phys_addr; 
				int amx_Idx;
				//OnQueryError(errorid, error[], callback[], query[], connectionHandle)
				for (list<AMX *>::iterator a = p_Amx.begin(); a != p_Amx.end(); a++) {
					if(amx_FindPublic(* a, "OnQueryError", &amx_Idx) == AMX_ERR_NONE) {
						Natives::getInstance()->Debug("OnQueryError() - Called.");
						amx_Push(* a,(cell)i+1);
						amx_PushString(* a, &amx_Address[0], &phys_addr, tempData.m_szQuery.c_str(), 0, 0);
						amx_PushString(* a, &amx_Address[1], &phys_addr, tempData.m_szCallback.c_str(), 0, 0);
						amx_PushString(* a, &amx_Address[2], &phys_addr, tempData.m_szError.c_str(), 0, 0);
						amx_Push(* a, tempData.m_uiErrorID);
						amx_Exec(* a, &amx_Ret, amx_Idx);
						amx_Release(* a, amx_Address[0]);
						amx_Release(* a, amx_Address[1]);
						amx_Release(* a, amx_Address[2]);
						/*if(amx_Ret == 1) {
							Natives::getInstance()->Debug("OnQueryError() - Resending query.");
						}*/
					}
				}
				SQLHandle[i]->errorCallback.pop();
			}
			if(!SQLHandle[i]->m_sCallbackData.empty()) {
				s_aFormat tempData = SQLHandle[i]->m_sCallbackData.front();
				cell amx_Address, amx_Ret, *phys_addr; 
				int amx_Idx, idx_format = strlen(tempData.szFormat)-1;
				for (list<AMX *>::iterator a = p_Amx.begin(); a != p_Amx.end(); a++) {
					if(amx_FindPublic(* a,tempData.szCallback, &amx_Idx) == AMX_ERR_NONE) {
						Natives::getInstance()->Debug("%s(%s) - Threaded function called.", tempData.szCallback, tempData.szFormat);
						char* revFormat = strrev(tempData.szFormat);
						while(*revFormat) {
							if(*revFormat == 'i' || *revFormat == 'd') {
								cell b = (cell)atoi(tempData.arrElements[idx_format].c_str());
								amx_Push(* a, b);
							} else if(*revFormat == 'f') {
								float floatParam = (float)atof(tempData.arrElements[idx_format].c_str());
								amx_Push(* a, amx_ftoc(floatParam));
							} else {
								cell tmp;
								amx_PushString(* a, &tmp, &phys_addr, tempData.arrElements[idx_format].c_str(), 0, 0);
								if (amx_Address == NULL) {
									amx_Address = tmp;
								}
							}
							idx_format--;
							*revFormat++;
						}
						amx_Exec(* a, &amx_Ret, amx_Idx);
						if(amx_Address != NULL) amx_Release(* a, amx_Address);
						if(/*amx_Ret && */tempData.bCache) {
							// public function returned 1 so lets clear the cache
							for(unsigned int x = 0, s = SQLHandle[i]->m_sCache.size();x < s;x++) {
								for(unsigned int y = 0, z = SQLHandle[i]->m_dwCacheFields;y < z;y++) {
									if(SQLHandle[i]->m_sCache[x][y] != NULL) free(SQLHandle[i]->m_sCache[x][y]);
								}
							}
							for(unsigned int x = 0, s = SQLHandle[i]->m_szCacheFields.size();x < s;x++) {
								if(SQLHandle[i]->m_szCacheFields[x] != NULL) free(SQLHandle[i]->m_szCacheFields[x]);
							}
							SQLHandle[i]->m_dwCacheFields = NULL;
							SQLHandle[i]->m_dwCacheRows = NULL;
							SQLHandle[i]->m_sCache.clear();
							SQLHandle[i]->m_szCacheFields.clear();
							Natives::getInstance()->Debug("ProcessTick() - The cache has been cleared.");
						}
						free(tempData.szCallback);
						free(tempData.szFormat);
						free(tempData.szQuery);
					}
				}
				SQLHandle[i]->m_sCallbackData.pop();
				SQLHandle[i]->m_bQueryProcessing = false;
			}
			//Mutex::getInstance()->_lockMutex();
		}
	}
}

#ifdef WIN32
DWORD __stdcall ProcessQueryThread(LPVOID lpParam)
#else
void * ProcessQueryThread(void *lpParam)
#endif
{
	//This is the main thread which executes all threaded SQL queries
	while(true) {
		Mutex::getInstance()->_lockMutex();
		if(SQLHandle.size() > 0) {
			for(unsigned int i = 0;i < SQLHandle.size();i++) {
				if(!SQLHandle[i]->m_sQueryData.empty() && SQLHandle[i]->m_bIsConnected && !SQLHandle[i]->m_bQueryProcessing) {
					if(!mysql_ping(SQLHandle[i]->m_stConnectionPtr)) {
						s_aFormat cQueue = SQLHandle[i]->m_sQueryData.front();
						SQLHandle[i]->m_bQueryProcessing = true;
						Natives::getInstance()->Debug("Passing query %s | %s", cQueue.szQuery, cQueue.szFormat);
						if(!mysql_real_query(SQLHandle[i]->m_stConnectionPtr, cQueue.szQuery, strlen(cQueue.szQuery))) {
							Natives::getInstance()->Debug("ProcessQueryThread(%s) - Query was successful. (%s)", cQueue.szCallback, cQueue.szQuery);
							if(cQueue.bCache) {
								Natives::getInstance()->Debug("ProcessQueryThread(%s) - Data caching enabled.", cQueue.szCallback);
								if(SQLHandle[i]->StoreResult()) {
									SQLHandle[i]->m_dwCacheRows = mysql_num_rows(SQLHandle[i]->m_stResult);
									SQLHandle[i]->m_dwCacheFields = mysql_num_fields(SQLHandle[i]->m_stResult); 
									unsigned int iFields = 0; char* szField;
									while((SQLHandle[i]->m_stField = mysql_fetch_field(SQLHandle[i]->m_stResult))) {
										szField = (char*)malloc(SQLHandle[i]->m_stField->name_length * sizeof(char)+1);
										memset(szField, '\0', (SQLHandle[i]->m_stField->name_length+1));
										strcpy(szField, SQLHandle[i]->m_stField->name);
										SQLHandle[i]->m_szCacheFields.push_back(szField);
									}
									while(SQLHandle[i]->m_stRow = mysql_fetch_row(SQLHandle[i]->m_stResult)) {
										unsigned long *lengths = mysql_fetch_lengths(SQLHandle[i]->m_stResult);
										std::vector<char*> tempVector;
										char* szCurrentRow;
										for(unsigned int a = 0;a < SQLHandle[i]->m_dwCacheFields;a++) {
											//Natives::getInstance()->Debug("Row = %s", SQLHandle[i]->m_stRow[a] ? SQLHandle[i]->m_stRow[a] : "NULL");
											if(!SQLHandle[i]->m_stRow[a]) {
												szCurrentRow = (char*)malloc((sizeof(char)*4)+1);
												memset(szCurrentRow, '\0', 4 + 1);
												strcpy(szCurrentRow, "NULL");
											} else {
												szCurrentRow = (char*)malloc((sizeof(char)*lengths[a])+1);
												memset(szCurrentRow, '\0', (lengths[a]+1));
												strcpy(szCurrentRow, SQLHandle[i]->m_stRow[a]);
											}
											tempVector.push_back(szCurrentRow);
										}
										SQLHandle[i]->m_sCache.push_back(tempVector);
										tempVector.clear();
									}
									SQLHandle[i]->FreeResult();
								}
							}
							Natives::getInstance()->Debug("CMySQLHandler::ProcessQueryThread() - Data is getting passed to ->ProcessTick()");
							SQLHandle[i]->m_sCallbackData.push(cQueue);
							SQLHandle[i]->m_dwError = 0;
						} else {
							Natives::getInstance()->Debug("CMySQLHandler::ProcessQueryThread() - Error will be triggered to OnQueryError()");
							CMySQLHandler::errorInfo cError;
							cError.m_szQuery = cQueue.szQuery;
							cError.m_uiErrorID = mysql_errno(SQLHandle[i]->m_stConnectionPtr);
							SQLHandle[i]->m_dwError = cError.m_uiErrorID;
							cError.m_szCallback = cQueue.szCallback;
							cError.m_szError = mysql_error(SQLHandle[i]->m_stConnectionPtr);
							SQLHandle[i]->errorCallback.push(cError);
							SQLHandle[i]->m_bQueryProcessing = false;
						}
						SQLHandle[i]->m_sQueryData.pop();
					} else {
						Natives::getInstance()->Debug("CMySQLHandler::ProcessQueryThread() - Reconnection attempt in background thread.");
						SQLHandle[i]->m_bIsConnected = false;
						if((SQLHandle[i]->m_stResult = mysql_store_result(SQLHandle[i]->m_stConnectionPtr)) != NULL)  {
							mysql_free_result(SQLHandle[i]->m_stResult);
							SQLHandle[i]->m_stResult = NULL;
						}
						SQLHandle[i]->Connect();
					}
				}
			}
		}
		Mutex::getInstance()->_unlockMutex();
		//we use sleep to avoid memory leaks etc.
		SLEEP(5);
	}
}

#if defined __cplusplus
	extern "C"
#endif
const AMX_NATIVE_INFO MySQLNatives[] = 
{
	{"mysql_connect",			Natives::getInstance()->n_mysql_connect},
	{"mysql_get_charset",		Natives::getInstance()->n_mysql_get_charset},
	{"mysql_free_result",		Natives::getInstance()->n_mysql_free_result},
	{"mysql_store_result",		Natives::getInstance()->n_mysql_store_result},
	{"mysql_real_escape_string",Natives::getInstance()->n_mysql_real_escape_string},
	{"mysql_reload",			Natives::getInstance()->n_mysql_reload},
	{"mysql_retrieve_row",		Natives::getInstance()->n_mysql_retrieve_row},
	{"mysql_field_count",		Natives::getInstance()->n_mysql_field_count},
	{"mysql_format",			Natives::getInstance()->n_mysql_format},
	{"mysql_close",				Natives::getInstance()->n_mysql_close},
	{"mysql_ping",				Natives::getInstance()->n_mysql_ping},
	{"mysql_num_rows",			Natives::getInstance()->n_mysql_num_rows},
	{"mysql_num_fields",		Natives::getInstance()->n_mysql_num_fields},
	{"mysql_stat",				Natives::getInstance()->n_mysql_stat},
	{"mysql_set_charset",		Natives::getInstance()->n_mysql_set_charset},
	{"mysql_affected_rows",		Natives::getInstance()->n_mysql_affected_rows},
	{"mysql_warning_count",		Natives::getInstance()->n_mysql_warning_count},
	{"mysql_errno",				Natives::getInstance()->n_mysql_errno},
	{"mysql_fetch_field",		Natives::getInstance()->n_mysql_fetch_field},
	{"mysql_fetch_field_row",	Natives::getInstance()->n_mysql_fetch_field_row},
	{"mysql_fetch_row_format",	Natives::getInstance()->n_mysql_fetch_row_format},
	{"mysql_debug",				Natives::getInstance()->n_mysql_debug},
	{"mysql_insert_id",			Natives::getInstance()->n_mysql_insert_id},
	{"mysql_reconnect",			Natives::getInstance()->n_mysql_reconnect},
	{"mysql_function_query",	Natives::getInstance()->n_mysql_query_callback},
	{"mysql_data_seek",			Natives::getInstance()->n_mysql_data_seek},
	{"cache_get_row",			Natives::getInstance()->n_cache_get_row},
	{"cache_get_field",			Natives::getInstance()->n_cache_get_field},
	{"cache_get_data",			Natives::getInstance()->n_cache_get_data},
	{"cache_get_field_content", Natives::getInstance()->n_cache_get_field_content},
	{"enable_mutex",			Natives::getInstance()->n_enable_mutex},
	{NULL, NULL}
};

PLUGIN_EXPORT int PLUGIN_CALL AmxLoad( AMX *amx )
{
	p_Amx.push_back(amx);
	return amx_Register(amx, MySQLNatives, -1);
}

PLUGIN_EXPORT int PLUGIN_CALL AmxUnload( AMX *amx ) 
{
	for(list<AMX *>::iterator i = p_Amx.begin();i != p_Amx.end();i++) {
		if(* i == amx) {
			p_Amx.erase(i);
			break;
		}
	}
	return AMX_ERR_NONE;
}