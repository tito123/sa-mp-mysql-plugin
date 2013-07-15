#pragma once

#include "CMySQLQuery.h"
#include "CMySQLResult.h"
#include "CCallback.h"
#include "CLog.h"

#include <boost/lexical_cast.hpp>


boost::threadpool::pool *CMySQLQuery::QueryThreadPool = NULL;

void CMySQLQuery::Execute() {
	if(CLog::Get()->IsLogLevel(LOG_DEBUG)) {
		char LogFuncBuf[128];
		sprintf2(LogFuncBuf, "ExecuteT[%s(%s)]", Callback->Name.c_str(), Callback->ParamFormat.c_str());
		CLog::Get()->LogFunction(LOG_DEBUG, LogFuncBuf, "starting query execution", true);
	}

	Result = NULL;
	MYSQL *ConnPtr = ConnHandle->GetMySQLPointer();
	
	if(ConnPtr != NULL) {

		ConnHandle->MySQLMutex.Lock();

		int QueryErrorID = mysql_real_query(ConnPtr, Query.c_str(), Query.length());
		if (QueryErrorID == 0) {
			
			if(CLog::Get()->IsLogLevel(LOG_DEBUG)) {
				char LogFuncBuf[128];
				sprintf2(LogFuncBuf, "ExecuteT[%s(%s)]", Callback->Name.c_str(), Callback->ParamFormat.c_str());
				CLog::Get()->LogFunction(LOG_DEBUG, LogFuncBuf, "query was successful", true);
			}

			MYSQL_RES *SQLResult;
			SQLResult = mysql_store_result(ConnPtr);

			ConnHandle->MySQLMutex.Unlock();

			if(Callback->Name.length() != 0) { //why should we process the result if it won't and can't be used?
				if (SQLResult != NULL) {
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
						Result->m_FieldNames.push_back(SQLField->name);

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
							case MYSQL_TYPE_ENUM:
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
						std::vector< vector<string> >::iterator It = Result->m_Data.insert(Result->m_Data.end(), vector<string>());
						It->reserve(Result->m_Fields+1);
					
						for (unsigned int a = 0; a < Result->m_Fields; ++a) {
							if (!SQLRow[a])
								It->push_back("NULL");
							else
								It->push_back(SQLRow[a]);
						}
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
				if(CLog::Get()->IsLogLevel(LOG_DEBUG)) {
					char LogFuncBuf[128];
					sprintf2(LogFuncBuf, "ExecuteT[%s(%s)]", Callback->Name.c_str(), Callback->ParamFormat.c_str());
					CLog::Get()->LogFunction(LOG_DEBUG, LogFuncBuf, "data being passed to ProcessCallbacks()", true);
				}
			
				CCallback::AddQueryToQueue(this);
			}
			else { //no callback was specified
				if(CLog::Get()->IsLogLevel(LOG_DEBUG)) {
					char LogFuncBuf[128];
					sprintf2(LogFuncBuf, "ExecuteT[%s(%s)]", Callback->Name.c_str(), Callback->ParamFormat.c_str());
					CLog::Get()->LogFunction(LOG_DEBUG, LogFuncBuf, "no callback specified, skipping result saving", true);
				}
				delete Callback;
				delete this;
			}
		}
		else { //mysql_real_query failed

			int ErrorID = mysql_errno(ConnPtr);
			string ErrorString(mysql_error(ConnPtr));
			ConnHandle->MySQLMutex.Unlock();

			if(ErrorID != 1065 && Callback != NULL) {
				
				if(CLog::Get()->IsLogLevel(LOG_ERROR)) {
					char LogFuncBuf[128];
					sprintf2(LogFuncBuf, "ExecuteT[%s(%s)]", Callback->Name.c_str(), Callback->ParamFormat.c_str());

					char LogMsgBuf[2048];
					sprintf2(LogMsgBuf, "(error #%d) %s", ErrorID, ErrorString.c_str());
					CLog::Get()->LogFunction(LOG_ERROR, LogFuncBuf, LogMsgBuf, true);
					
					CLog::Get()->LogFunction(LOG_DEBUG, LogFuncBuf, "error will be triggered in OnQueryError", true);
				}
				
				if(ConnHandle->IsAutoReconnectEnabled() && ErrorID == 2006) { 
					if(CLog::Get()->IsLogLevel(LOG_WARNING)) {
						char LogFuncBuf[128];
						sprintf2(LogFuncBuf, "ExecuteT[%s(%s)]", Callback->Name.c_str(), Callback->ParamFormat.c_str());
						CLog::Get()->LogFunction(LOG_WARNING, LogFuncBuf, "lost connection, reconnecting..", true);
					}

					MYSQL_RES *SQLRes;
					if ((SQLRes = mysql_store_result(ConnPtr)) != NULL)  {
						mysql_free_result(SQLRes);
					}
					
					ConnHandle->Disconnect(true);
					ConnHandle->Connect(true);
				}

				//forward OnQueryError(errorid, error[], callback[], query[], connectionHandle);
				//recycle these structures, change some data
				while(Callback->Parameters.size() > 0)
					Callback->Parameters.pop(); //why the hell isn't there a .clear() function?!
				Callback->Parameters.push(boost::lexical_cast<string>(ErrorID));
				Callback->Parameters.push(ErrorString);
				Callback->Parameters.push(Callback->Name);
				Callback->Parameters.push(Query);
				Callback->Parameters.push(boost::lexical_cast<string>(ConnHandle->GetID()));

				Callback->Name = "OnQueryError";
				Callback->ParamFormat = "dsssd";

				Result = NULL;

				CCallback::AddQueryToQueue(this);
			}
		}
	}
}
