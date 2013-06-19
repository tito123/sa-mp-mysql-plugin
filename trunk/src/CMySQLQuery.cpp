#pragma once
#define _CRT_SECURE_NO_WARNINGS

#include "CMySQLQuery.h"
#include "CMySQLResult.h"
#include "CCallback.h"
#include "CLog.h"

#include <sstream>
using std::stringstream;

CMutex CMySQLQuery::MySQLMutex;

boost::lockfree::queue<
		CMySQLQuery*, 
		boost::lockfree::fixed_sized<true>,
		boost::lockfree::capacity<10000>
	> CMySQLQuery::m_QueryQueue;

queue <CMySQLHandle*> CMySQLQuery::ConnectQueue;
queue <CMySQLHandle*> CMySQLQuery::DisconnectQueue;
queue <CMySQLHandle*> CMySQLQuery::ReconnectQueue;


CMySQLQuery::CMySQLQuery() {
	ConnHandle = NULL;
	Result = NULL;
	Callback = NULL;
}


void CMySQLQuery::ExecuteT() {
	Result = NULL;

	if(ConnPtr != NULL) {

		MySQLMutex.Lock();
		
		int QueryErrorID = mysql_real_query(ConnPtr, Query.c_str(), Query.length());
		if (QueryErrorID == 0) {

			//Native::Log(LOG_DEBUG, "ExecuteT(%s) - Query was successful.", Callback->Name.c_str());
			if(CLog::Get()->IsLogLevel(LOG_DEBUG)) {
				char LogFuncBuf[128];
				sprintf2(LogFuncBuf, "ExecuteT[%s(%s)]", Callback->Name.c_str(), Callback->ParamFormat.c_str());
				CLog::Get()->LogFunction(LOG_DEBUG, LogFuncBuf, "query was successful", true);
			}

			MYSQL_RES *SQLResult;

			SQLResult = mysql_store_result(ConnPtr);
			MySQLMutex.Unlock();

			if ( SQLResult != NULL) {
				MYSQL_FIELD *SQLField;
				MYSQL_ROW SQLRow;

				Result = new CMySQLResult;

				Result->m_WarningCount = mysql_warning_count(ConnPtr);
				Result->m_Rows = mysql_num_rows(SQLResult);
				Result->m_Fields = mysql_num_fields(SQLResult);

				Result->m_Data.reserve((unsigned int)Result->m_Rows+1);
				Result->m_FieldNames.reserve(Result->m_Fields+1);
				Result->m_FieldDataTypes.reserve(Result->m_Fields+1);
				

				char *szField = NULL;
				while ((SQLField = mysql_fetch_field(SQLResult))) {
					szField = (char *)malloc(sizeof(char) * (SQLField->name_length+1));//new char[SQLField->name_length+1];
					//memset(szField, '\0', (SQLField->name_length + 1));
					strcpy(szField, SQLField->name);
					Result->m_FieldNames.push_back(szField);

					switch(SQLField->type) {
						case MYSQL_TYPE_LONG:
						case MYSQL_TYPE_TINY:
						case MYSQL_TYPE_SHORT:
						case MYSQL_TYPE_TIMESTAMP:
						case MYSQL_TYPE_INT24:
						case MYSQL_TYPE_LONGLONG:
						case MYSQL_TYPE_NULL:
						case MYSQL_TYPE_YEAR:
						case MYSQL_TYPE_BIT:
							Result->m_FieldDataTypes.push_back(TYPE_INT);
							break;
						case MYSQL_TYPE_FLOAT:
						case MYSQL_TYPE_DOUBLE:
						case MYSQL_TYPE_NEWDECIMAL:
						case MYSQL_TYPE_DECIMAL:
							Result->m_FieldDataTypes.push_back(TYPE_FLOAT);
							break;
						default:
							Result->m_FieldDataTypes.push_back(TYPE_STRING);
					}
				}
				
				while (SQLRow = mysql_fetch_row(SQLResult)) {
					unsigned long *lengths = mysql_fetch_lengths(SQLResult);
					vector<char*> *tempVector = new vector<char*>;
					tempVector->reserve(Result->m_Fields+1);

					char* szCurrentRow;
					for (unsigned int a = 0; a < Result->m_Fields; a++) {
						if (!SQLRow[a]) {
							szCurrentRow = (char *)malloc(sizeof(char) * (4 + 1));
							strcpy(szCurrentRow, "NULL");
						} else {
							szCurrentRow = (char *)malloc(sizeof(char) * (lengths[a]+1));
							strcpy(szCurrentRow, SQLRow[a]);
						} 
						tempVector->push_back(szCurrentRow);
					}
					Result->m_Data.push_back(tempVector);
				}

				mysql_free_result(SQLResult);
			}
			else if(mysql_field_count(ConnPtr) == 0) { //query is non-SELECT query
				Result = new CMySQLResult;
				
				Result->m_WarningCount = mysql_warning_count(ConnPtr);
				Result->m_AffectedRows = mysql_affected_rows(ConnPtr);
				Result->m_InsertID = mysql_insert_id(ConnPtr);
			}
			
			//forward Query to Callback handler
			//Native::Log(LOG_DEBUG, "ExecuteT(%s) - Data being passed to ProcessCallbacks().", Callback->Name.c_str());
			if(CLog::Get()->IsLogLevel(LOG_DEBUG)) {
				char LogFuncBuf[128];
				sprintf2(LogFuncBuf, "ExecuteT[%s(%s)]", Callback->Name.c_str(), Callback->ParamFormat.c_str());
				CLog::Get()->LogFunction(LOG_DEBUG, LogFuncBuf, "data being passed to ProcessCallbacks()", true);
			}

			CCallback::AddQueryToQueue(this);
		}
		else { //mysql_real_query failed
			
			MySQLMutex.Unlock();
			
			int ErrorID = mysql_errno(ConnPtr);
			if(ErrorID != 1065 && Callback != NULL) {
				const char *ErrorString = mysql_error(ConnPtr);
				
				//Native::Log(LOG_ERROR, "ExecuteT(%s) - %s (error ID: %d)", Callback->Name.c_str(), ErrorString, ErrorID);
				//Native::Log(LOG_DEBUG, "ExecuteT(%s) - Error will be triggered to OnQueryError().", Callback->Name.c_str());
				if(CLog::Get()->IsLogLevel(LOG_ERROR)) {
					char LogFuncBuf[128];//, LogFuncBuf2[128];
					sprintf2(LogFuncBuf, "ExecuteT[%s(%s)]", Callback->Name.c_str(), Callback->ParamFormat.c_str());
					//sprintf2(LogFuncBuf2, "ExecuteT[%s(%s)]", Callback->Name.c_str(), Callback->ParamFormat.c_str());
					//memcpy(LogFuncBuf2, LogFuncBuf, 128 * sizeof(char));

					char LogMsgBuf[512];
					sprintf2(LogMsgBuf, "(error #%d) %s", ErrorID, ErrorString);
					CLog::Get()->LogFunction(LOG_ERROR, LogFuncBuf, LogMsgBuf, true);
					
					CLog::Get()->LogFunction(LOG_DEBUG, LogFuncBuf, "error will be triggered in OnQueryError", true);
				}
				
				

				if(ErrorID == 2006) { 
					//Native::Log(LOG_WARNING, "ExecuteT() - Lost connection, reconnecting to the MySQL-server in the background thread.");
					if(CLog::Get()->IsLogLevel(LOG_WARNING)) {
						char LogFuncBuf[128];
						sprintf2(LogFuncBuf, "ExecuteT[%s(%s)]", Callback->Name.c_str(), Callback->ParamFormat.c_str());
						CLog::Get()->LogFunction(LOG_WARNING, LogFuncBuf, "lost connection, requesting reconnect", true);
					}
					
					MYSQL_RES *SQLRes;
					if ((SQLRes = mysql_store_result(ConnPtr)) != NULL)  {
							mysql_free_result(SQLRes);
					}
					PushReconnect(ConnHandle);
				}

				//forward OnQueryError(errorid, error[], callback[], query[], connectionHandle);
				CCallback *ErrorCallback = new CCallback;
				ErrorCallback->Name = "OnQueryError";
				ErrorCallback->ParamFormat = "dsssd";
				stringstream ConvBuf, ConvBuf2;
				ConvBuf << ErrorID;
				ErrorCallback->Parameters.push(ConvBuf.str());
				ErrorCallback->Parameters.push(ErrorString);
				ErrorCallback->Parameters.push(Callback->Name);
				ErrorCallback->Parameters.push(Query);
				ConvBuf2 << ConnHandle->GetID();
				ErrorCallback->Parameters.push(ConvBuf2.str());

				CMySQLQuery *ErrorCBQuery = new CMySQLQuery;
				ErrorCBQuery->Callback = ErrorCallback;
				ErrorCBQuery->Result = NULL;
				ErrorCBQuery->ConnHandle = ConnHandle;
				CCallback::AddQueryToQueue(ErrorCBQuery);

				//push query again into queue
				/*if(ErrorID != 1064) //mistake in query syntax
					CMySQLQuery::PushQuery(query);
				else*/
				//delete this;
				//query = NULL;
			}
		}
	}
}