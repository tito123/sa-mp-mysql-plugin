#pragma once

#include "CMySQLQuery.h"
#include "CMySQLResult.h"
#include "CCallback.h"
#include "CLog.h"

#include <boost/lexical_cast.hpp>


boost::threadpool::pool *CMySQLQuery::QueryThreadPool = NULL;

void CMySQLQuery::Execute() {
	char LogFuncBuf[128];
	sprintf(LogFuncBuf, "CMySQLQuery::Execute[%s(%s)]", Callback->Name.c_str(), Callback->ParamFormat.c_str());
	
	CLog::Get()->LogFunction(LOG_DEBUG, true, LogFuncBuf, "starting query execution");
	

	Result = NULL;
	MYSQL *ConnPtr = ConnHandle->GetMySQLPointer();
	
	if(ConnPtr != NULL) {

		ConnHandle->MySQLMutex.Lock();

		int QueryErrorID = mysql_real_query(ConnPtr, Query.c_str(), Query.length());
		if (QueryErrorID == 0) {
			
			CLog::Get()->LogFunction(LOG_DEBUG, true, LogFuncBuf, "query was successful");


			MYSQL_RES *SQLResult;
			SQLResult = mysql_store_result(ConnPtr);

			ConnHandle->MySQLMutex.Unlock();

			if(Callback->Name.length() > 0) { //why should we process the result if it won't and can't be used?
				if (SQLResult != NULL) {
					MYSQL_FIELD *SQLField;
					MYSQL_ROW SQLRow;

					Result = new CMySQLResult;

					Result->m_WarningCount = mysql_warning_count(ConnPtr);
					Result->m_Rows = mysql_num_rows(SQLResult);
					Result->m_Fields = mysql_num_fields(SQLResult);

					Result->m_Data.reserve((unsigned int)Result->m_Rows+1);
					Result->m_FieldNames.reserve(Result->m_Fields+1);


					while ((SQLField = mysql_fetch_field(SQLResult))) {
						Result->m_FieldNames.push_back(SQLField->name);
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
			
				//forward Query to callback handler
				CLog::Get()->LogFunction(LOG_DEBUG, true, LogFuncBuf, "data being passed to ProcessCallbacks()");
			
				CCallback::AddQueryToQueue(this);
			}
			else { //no callback was specified
				CLog::Get()->LogFunction(LOG_DEBUG, true, LogFuncBuf, "no callback specified, skipping result saving");

				delete Callback;
				delete this;
			}
		}
		else { //mysql_real_query failed

			int ErrorID = mysql_errno(ConnPtr);
			string ErrorString(mysql_error(ConnPtr));
			ConnHandle->MySQLMutex.Unlock();

			CLog::Get()->LogFunction(LOG_ERROR, true, LogFuncBuf, "(error #%d) %s", ErrorID, ErrorString.c_str());
			
			
			if(ConnHandle->IsAutoReconnectEnabled() && ErrorID == 2006) { 
				CLog::Get()->LogFunction(LOG_WARNING, true, LogFuncBuf, "lost connection, reconnecting..");

				ConnHandle->MySQLMutex.Lock();

				MYSQL_RES *SQLRes;
				if ((SQLRes = mysql_store_result(ConnPtr)) != NULL)  {
					mysql_free_result(SQLRes);
				}
				
				ConnHandle->Disconnect(true);
				ConnHandle->Connect(true);

				ConnHandle->MySQLMutex.Unlock();
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

			CLog::Get()->LogFunction(LOG_DEBUG, true, LogFuncBuf, "error will be triggered in OnQueryError");

			CCallback::AddQueryToQueue(this);
		}
	}
}


void CMySQLQuery::InitializeThreadPool( size_t numthreads )
{
	if(QueryThreadPool == NULL && numthreads > 0)
		QueryThreadPool = new boost::threadpool::pool(numthreads);
}

void CMySQLQuery::DeleteThreadPool()
{
	if(QueryThreadPool != NULL) {
		QueryThreadPool->wait(0);
		delete QueryThreadPool;
	}
}

bool CMySQLQuery::IsThreadPoolInitialized()
{
	return QueryThreadPool == NULL ? false : true;
}

void CMySQLQuery::WaitForThreadPool()
{
	if(QueryThreadPool != NULL)
		QueryThreadPool->wait(0);
} 
