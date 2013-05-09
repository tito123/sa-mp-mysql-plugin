#pragma once

#include "main.h"
#include "malloc.h"

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

PLUGIN_EXPORT unsigned int PLUGIN_CALL Supports() {
	return SUPPORTS_VERSION | SUPPORTS_AMX_NATIVES | SUPPORTS_PROCESS_TICK; 
}

PLUGIN_EXPORT bool PLUGIN_CALL Load(void **ppData) {
	pAMXFunctions = ppData[PLUGIN_DATA_AMX_EXPORTS];
	logprintf = (logprintf_t)ppData[PLUGIN_DATA_LOGPRINTF];
	if (mysql_library_init(0, NULL, NULL)) {
		logprintf("plugin.mysql: Couldn't initialize MySQL library.");
		Natives::Log(LOG_ERROR, "Plugin failed to load due to unitialized MySQL library (libmysql probably missing).");
		exit(0);
		return 0;
	}
	logprintf("plugin.mysql: R20 successfully loaded.");
	Natives::Log(LOG_DEBUG, "Plugin succesfully loaded!");
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
	// Initializing the mutex.
	Mutex::getInstance();
	return 1;
}

PLUGIN_EXPORT void PLUGIN_CALL Unload() {
	Natives::Log(LOG_DEBUG, "Unloading Plugin");
	for (map<int, CMySQLHandler *>::iterator it = SQLHandle.begin(); it != SQLHandle.end(); ++it)
		delete it->second;
	SQLHandle.clear();
	p_Amx.clear();
	delete Mutex::getInstance();
	delete Natives::getInstance();
	logprintf("plugin.mysql: Plugin unloaded.");
}

PLUGIN_EXPORT void PLUGIN_CALL ProcessTick() {
	Mutex::getInstance()->_lockMutex();
	if (SQLHandle.size() > 0) {
		for (map<int, CMySQLHandler *>::iterator it = SQLHandle.begin(), end = SQLHandle.end(); it != end; it++) {
			

			CMySQLHandler *cHandle = it->second;

			if (!cHandle->errorCallback.empty()) {
				CMySQLHandler::errorInfo tempData = cHandle->errorCallback.front();
				cell amx_Address[3], amx_Ret, *phys_addr;
				int amx_Idx;
				//OnQueryError(errorid, error[], callback[], query[], connectionHandle)
				for (list<AMX *>::iterator a = p_Amx.begin(); a != p_Amx.end(); a++) {
					if (amx_FindPublic(* a, "OnQueryError", &amx_Idx) == AMX_ERR_NONE) {
						Natives::Log(LOG_DEBUG, "OnQueryError() - Callback has been called.");
						amx_Push(* a, (cell)it->first);
						amx_PushString(* a, &amx_Address[0], &phys_addr, tempData.m_szQuery.c_str(), 0, 0);
						amx_PushString(* a, &amx_Address[1], &phys_addr, tempData.m_szCallback.c_str(), 0, 0);
						amx_PushString(* a, &amx_Address[2], &phys_addr, tempData.m_szError.c_str(), 0, 0);
						amx_Push(* a, tempData.m_uiErrorID);
						amx_Exec(* a, &amx_Ret, amx_Idx);
						amx_Release(* a, amx_Address[0]);
						amx_Release(* a, amx_Address[1]);
						amx_Release(* a, amx_Address[2]);
					}
				}
				
				tempData.m_szQuery.clear();
				tempData.m_szCallback.clear();
				tempData.m_szError.clear();
				cHandle->errorCallback.pop();
			}
			if (!cHandle->m_sCallbackData.empty()) {
				s_aFormat tempData = cHandle->m_sCallbackData.front();
				int amx_Idx, idx_format = strlen(tempData.szFormat) - 1;

				for (list<AMX *>::iterator a = p_Amx.begin(); a != p_Amx.end(); a++) {
					cell amx_Ret, *phys_addr;
					cell amx_Address = -1;
					if (amx_FindPublic(* a, tempData.szCallback, &amx_Idx) == AMX_ERR_NONE) {
						
						Natives::Log(LOG_DEBUG, "%s(%s) - Callback is being called...", tempData.szCallback, tempData.szFormat);
						char* revFormat = strrev(tempData.szFormat);
						
						while (*revFormat) {
							if (*revFormat == 'i' || *revFormat == 'd') {
								cell b = (cell)atoi(tempData.arrElements[idx_format].c_str());
								amx_Push(* a, b);
							} else if (*revFormat == 'f') {
								float floatParam = (float)atof(tempData.arrElements[idx_format].c_str());
								amx_Push(* a, amx_ftoc(floatParam));
							} else {
								cell tmp;
								amx_PushString(* a, &tmp, &phys_addr, tempData.arrElements[idx_format].c_str(), 0, 0);
								if (amx_Address < NULL) {
									amx_Address = tmp;
								}
							}
							idx_format--;
							*revFormat++;
						}
						amx_Exec(* a, &amx_Ret, amx_Idx);
						if (amx_Address >= NULL) {
							amx_Release(* a, amx_Address);
						}
						if (tempData.bCache) {
							// Clear the cache
							delete cHandle->m_pActiveCache;
							cHandle->m_pActiveCache = NULL;
							cHandle->m_bActiveCacheStored = 0;
							Natives::Log(LOG_DEBUG, "ProcessTick() - The cache has been cleared.");
						} else {
							if (cHandle->m_stResult != NULL) {
								Natives::Log(LOG_ERROR, "ProcessTick() - The result wasn't freed!");
								//TODO: free result automatically?
								//Natives::Log(LOG_WARNING, "ProcessTick() - The result wasn't free'd! Freeing automatically...");
								//cHandle->FreeResult();
							}
						}
						free(tempData.szCallback);
						free(tempData.szFormat);
						free(tempData.szQuery);
					}
				}
				cHandle->m_sCallbackData.pop();
				cHandle->m_bQueryProcessing = false;
			}
			
		}
	}
	Mutex::getInstance()->_unlockMutex();
}

#ifdef WIN32 
DWORD __stdcall ProcessQueryThread(LPVOID lpParam)
#else
void * ProcessQueryThread(void *lpParam)
#endif
{
	// The main thread which handles MySQL connection.
	mysql_thread_init();
	while (true) {
		Mutex::getInstance()->_lockMutex();
		if (SQLHandle.size() > 0) {
			for (map<int, CMySQLHandler *>::iterator it = SQLHandle.begin(), end = SQLHandle.end(); it != end; it++) {
				CMySQLHandler *cHandle = it->second;
				if (!cHandle->m_sQueryData.empty() && cHandle->m_bIsConnected && !cHandle->m_bQueryProcessing) {

					if (mysql_ping(cHandle->m_stConnectionPtr) == 0) {
						s_aFormat cQueue = cHandle->m_sQueryData.front();
						cHandle->m_bQueryProcessing = true;
						Natives::Log(LOG_DEBUG, "ProcessQueryThread(%s) - Executing query %s...", cQueue.szCallback, cQueue.szQuery);
						if (mysql_real_query(cHandle->m_stConnectionPtr, cQueue.szQuery, strlen(cQueue.szQuery)) == 0) {
							Natives::Log(LOG_DEBUG, "ProcessQueryThread(%s) - Query was successful.", cQueue.szCallback);
							
							if (cQueue.bCache) { 
								Natives::Log(LOG_DEBUG, "ProcessQueryThread(%s) - Data caching enabled.", cQueue.szCallback);
								if (cHandle->StoreResult()) {
									//Create an instance of result class to store the whole cached result
									CMySQLResult *pResult = new CMySQLResult;

									pResult->m_dwCacheRows = mysql_num_rows(cHandle->m_stResult);
									pResult->m_dwCacheFields = mysql_num_fields(cHandle->m_stResult);
									unsigned int iFields = 0;
									char* szField;
									while ((cHandle->m_stField = mysql_fetch_field(cHandle->m_stResult))) {
										szField = (char*)malloc(cHandle->m_stField->name_length * sizeof(char) + 1);
										memset(szField, '\0', (cHandle->m_stField->name_length + 1));
										strcpy(szField, cHandle->m_stField->name);
										pResult->m_szCacheFields.push_back(szField);
									}
									while (cHandle->m_stRow = mysql_fetch_row(cHandle->m_stResult)) {
										unsigned long *lengths = mysql_fetch_lengths(cHandle->m_stResult);
										std::vector<char*> tempVector;
										char* szCurrentRow;
										for (unsigned int a = 0; a < pResult->m_dwCacheFields; a++) {
											if (!cHandle->m_stRow[a]) {
												szCurrentRow = (char*)malloc((sizeof(char) * 4) + 1);
												memset(szCurrentRow, '\0', 4 + 1);
												strcpy(szCurrentRow, "NULL");
											} else {
												szCurrentRow = (char*)malloc((sizeof(char) * lengths[a]) + 1);
												memset(szCurrentRow, '\0', (lengths[a] + 1));
												strcpy(szCurrentRow, cHandle->m_stRow[a]);
											}
											tempVector.push_back(szCurrentRow);
										}
										pResult->m_sCache.push_back(tempVector);
										tempVector.clear();
									}

									//Assign the store-instance to the mysql handle as active cache
									cHandle->m_pActiveCache = pResult;
									cHandle->m_bActiveCacheStored = 0;

									cHandle->FreeResult();
								}
							}

							Natives::Log(LOG_DEBUG, "ProcessQueryThread(%s) - Data being passed to ProcessTick().", cQueue.szCallback);
							cHandle->m_sCallbackData.push(cQueue);
							cHandle->m_dwError = 0;
						} else {
							CMySQLHandler::errorInfo cError;
							cError.m_szQuery = cQueue.szQuery;
							cError.m_uiErrorID = mysql_errno(cHandle->m_stConnectionPtr);
							cHandle->m_dwError = cError.m_uiErrorID;
							cError.m_szCallback = cQueue.szCallback;
							cError.m_szError = mysql_error(cHandle->m_stConnectionPtr);
							cHandle->errorCallback.push(cError);
							cHandle->m_bQueryProcessing = false;
							Natives::Log(LOG_ERROR, "ProcessQueryThread(%s) - %s (error ID: %d)", cQueue.szCallback, cError.m_szError.c_str(), cError.m_uiErrorID);
							Natives::Log(LOG_DEBUG, "ProcessQueryThread(%s) - Error will be triggered to OnQueryError().", cQueue.szCallback);
						}
						cHandle->m_sQueryData.pop();
					} else {
						Natives::Log(LOG_WARNING, "ProcessQueryThread() - Lost connection, reconnecting to the MySQL-server in the background thread.");
						cHandle->m_bIsConnected = false;
						if ((cHandle->m_stResult = mysql_store_result(cHandle->m_stConnectionPtr)) != NULL)  {
							mysql_free_result(cHandle->m_stResult);
							cHandle->m_stResult = NULL;
						}
						cHandle->Connect();
					}
				}
			}
		}
		Mutex::getInstance()->_unlockMutex();
		// Sleeping in order to avoid high resource usage.
		SLEEP(5);
	}
	mysql_thread_end();
}

#if defined __cplusplus
extern "C"
#endif
const AMX_NATIVE_INFO MySQLNatives[] = {
	{"enable_mutex",					Natives::getInstance()->n_enable_mutex},
	{"mysql_debug",						Natives::getInstance()->n_mysql_debug},
	{"mysql_log",						Natives::getInstance()->n_mysql_log},
	{"mysql_connect",					Natives::getInstance()->n_mysql_connect},
	{"mysql_reconnect",					Natives::getInstance()->n_mysql_reconnect},
	{"mysql_close",						Natives::getInstance()->n_mysql_close},
	{"mysql_ping",						Natives::getInstance()->n_mysql_ping},
	{"mysql_stat",						Natives::getInstance()->n_mysql_stat},
	{"mysql_reload",					Natives::getInstance()->n_mysql_reload},
	{"mysql_get_charset",				Natives::getInstance()->n_mysql_get_charset},
	{"mysql_set_charset",				Natives::getInstance()->n_mysql_set_charset},
	{"mysql_real_escape_string",		Natives::getInstance()->n_mysql_real_escape_string},
	{"mysql_format",					Natives::getInstance()->n_mysql_format},
	{"mysql_warning_count",				Natives::getInstance()->n_mysql_warning_count},
	{"mysql_errno",						Natives::getInstance()->n_mysql_errno},
	{"mysql_function_query",			Natives::getInstance()->n_mysql_query_callback},
	{"mysql_affected_rows",				Natives::getInstance()->n_mysql_affected_rows},
	{"mysql_insert_id",					Natives::getInstance()->n_mysql_insert_id},
	{"mysql_store_result",				Natives::getInstance()->n_mysql_store_result},
	{"mysql_free_result",				Natives::getInstance()->n_mysql_free_result},
	{"mysql_num_rows",					Natives::getInstance()->n_mysql_num_rows},
	{"mysql_num_fields",				Natives::getInstance()->n_mysql_num_fields},
	{"mysql_field_count",				Natives::getInstance()->n_mysql_field_count},
	{"mysql_retrieve_row",				Natives::getInstance()->n_mysql_retrieve_row},
	{"mysql_data_seek",					Natives::getInstance()->n_mysql_data_seek},
	{"mysql_fetch_field",				Natives::getInstance()->n_mysql_fetch_field},
	{"mysql_fetch_field_row",			Natives::getInstance()->n_mysql_fetch_field_row},
	{"mysql_fetch_row_format",			Natives::getInstance()->n_mysql_fetch_row_format},
	{"cache_get_data",					Natives::getInstance()->n_cache_get_data},
	{"cache_get_field",					Natives::getInstance()->n_cache_get_field},
	{"cache_get_row",					Natives::getInstance()->n_cache_get_row},
	{"cache_get_row_int",				Natives::getInstance()->n_cache_get_row_int},
	{"cache_get_row_float",				Natives::getInstance()->n_cache_get_row_float},
	{"cache_get_field_content",			Natives::getInstance()->n_cache_get_field_content},
	{"cache_get_field_content_int",		Natives::getInstance()->n_cache_get_field_content_int},
	{"cache_get_field_content_float",	Natives::getInstance()->n_cache_get_field_content_float},
	{"cache_save",						Natives::getInstance()->n_cache_save},
	{"cache_delete",					Natives::getInstance()->n_cache_delete},
	{"cache_set_active",				Natives::getInstance()->n_cache_set_active},
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