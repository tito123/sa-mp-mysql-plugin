#pragma once

#include "CMySQLHandler.h"

#include <cstring>
#include "malloc.h"

CMutex CMySQLQuery::MySQLMutex;
CMutex CMySQLQuery::QueryMutex;
CMutex CMySQLHandle::SQLHandleMutex;

int CMySQLQuery::QueryCounter = 0;
queue<CMySQLQuery*> CMySQLQuery::m_QueryQueue;

queue <CMySQLHandle*> CMySQLQuery::ConnectQueue;
queue <CMySQLHandle*> CMySQLQuery::DisconnectQueue;
queue <CMySQLHandle*> CMySQLQuery::ReconnectQueue;

extern bool MultiThreading;


void CMySQLResult::GetFieldName(unsigned int idx, string &dest) {
	if (idx < m_FieldNames.size()) {
		dest.assign(m_FieldNames.at(idx));
		Native::Log(LOG_DEBUG, "CMySQLResult::GetFieldName() - Result: \"%s\"", dest.c_str());
	}
	else {
		dest.assign("NULL");
		Native::Log(LOG_WARNING, "CMySQLResult::GetFieldName() - Invalid field index.");
	}
}

void CMySQLResult::GetRowData(unsigned int row, unsigned int fieldidx, string &dest) {
	if(row < m_Rows && fieldidx < m_Fields) {
		dest.assign(m_Data.at(row)->at(fieldidx));
		Native::Log(LOG_DEBUG, "CMySQLResult::GetRowData() - Result: \"%s\"", dest.c_str());
	}
	else {
		dest.assign("NULL");
		Native::Log(LOG_WARNING, "CMySQLResult::GetRowData() - Invalid row or field index.");
	}
}

void CMySQLResult::GetRowDataByName(unsigned int row, string field, string &dest) {
	if (row < m_Rows && m_Fields > 0) {
		for (unsigned int i = 0; i < m_Fields; ++i) {
			if (!strcmp(m_FieldNames.at(i), field.c_str())) {
				dest.assign(m_Data.at(row)->at(i));
				Native::Log(LOG_DEBUG, "CMySQLResult::GetRowDataByName() - Result: \"%s\"", dest.c_str());
				return ;
			}
		}
		Native::Log(LOG_WARNING, "CMySQLResult::GetRowDataByName() - Field not found.");
	}
	else
		Native::Log(LOG_WARNING, "CMySQLResult::GetRowDataByName() - Invalid row index.");
}

CMySQLResult::CMySQLResult() {
	m_Fields = 0;
	m_Rows = 0; 
	m_Data.clear();
	m_FieldNames.clear();
}

CMySQLResult::~CMySQLResult() {
	/*for(vector<char*>::iterator it = m_FieldNames.begin(), end = m_FieldNames.end(); it != end; it++)
		delete[] (*it);*/
	for(unsigned int i=0; i < m_FieldNames.size(); ++i) {
		char *val = m_FieldNames.at(i);
		//if(val == NULL)
			//Native::Log(LOG_ERROR, "Value in ~CMySQLResult has zero as pointer value!");
		delete[] val;
	}
	m_FieldNames.clear();

	for(vector<vector<char*>* >::iterator it1 = m_Data.begin(), end1 = m_Data.end(); it1 != end1; it1++) {
		for(vector<char*>::iterator it2 = (*it1)->begin(), end2 = (*it1)->end(); it2 != end2; it2++)
			delete[] (*it2);
		delete (*it1);
	} 
	m_Data.clear();

	Native::Log(LOG_DEBUG, "CMySQLResult::~CMySQLResult() - Deconstructor called.");
}



CMySQLHandle::CMySQLHandle(string host, string user, string passw, string db, size_t port) {
	m_Hostname.assign(host);
	m_Username.assign(user);
	m_Password.assign(passw);
	m_Database.assign(db);
	m_iPort = port;
	
	m_SavedResults.clear();
	m_ActiveResult = NULL;
	m_ActiveResultID = 0;
	m_ThreadConnPtr = NULL;
	m_Connected = false;


	Native::Log(LOG_DEBUG, "CMySQLHandle::CMySQLHandle() - Constructor called.");
	Native::Log(LOG_DEBUG, "CMySQLHandle::CMySQLHandle() - Connecting to \"%s\" | DB: \"%s\" | Username: \"%s\"...", m_Hostname.c_str(), m_Database.c_str(), m_Username.c_str());
}

CMySQLHandle::~CMySQLHandle() {
	for (map<int, CMySQLResult*>::iterator it = m_SavedResults.begin(), end = m_SavedResults.end(); it != end; it++)
		delete it->second;

	Native::Log(LOG_DEBUG, "CMySQLHandle::~CMySQLHandle() - Deconstructor called.");
}

int CMySQLHandle::Create(string host, string user, string pass, string db, size_t port) {
	int ID = -1;
	SQLHandleMutex.Lock();
	if (SQLHandle.size() > 0) {
		for(map<int, CMySQLHandle*>::iterator i = SQLHandle.begin(), end = SQLHandle.end(); i != end; ++i) {
			// Code used for checking duplicate connections.
			CMySQLHandle *Handle = i->second;
			if (Handle->m_Hostname.compare(host) == 0 && Handle->m_Username.compare(user) == 0 && Handle->m_Database.compare(db) == 0 && Handle->m_Password.compare(pass) == 0) 
			{
				/*SQLHandle[i]->m_bIsConnected = false;
				SQLHandle[i]->Connect();
				match = true;*/

				Native::Log(LOG_WARNING, "CMySQLHandle::Create() - Connection already exists.");
				ID = i->first;
				break;
			}
		}
	}
	if (ID == -1) {
		Native::Log(LOG_DEBUG, "CMySQLHandle::Create() - Creating new connection..");

		CMySQLHandle *Handle = new CMySQLHandle(host, user, pass, db, port);

		if(!SQLHandle.empty()) {
			map<int, CMySQLHandle*>::iterator itHandle = SQLHandle.end();
			std::advance(itHandle, -1);
			ID = itHandle->first+1;
		}
		else
			ID = 1;

		Handle->m_CID = ID;
		SQLHandle.insert( map<int, CMySQLHandle*>::value_type(ID, Handle) );
		Native::Log(LOG_DEBUG, "CMySQLHandle::Create() - Connection created with ID %d.", ID);
	}
	SQLHandleMutex.Unlock();
	return ID;
}



bool CMySQLHandle::IsValid(int id) {
	if(SQLHandle.find(id) == SQLHandle.end() || SQLHandle.at(id) == NULL)
		return false;
	return true;
}


bool CMySQLHandle::ConnectT() {
	bool ReturnVal;
	SQLHandleMutex.Lock();

	if(m_ThreadConnPtr == NULL) {
		m_ThreadConnPtr = mysql_init(NULL);
		if (m_ThreadConnPtr == NULL)
			Native::Log(LOG_ERROR, "CMySQLHandle::ConnectT() - MySQL initialization failed.");
	}
	if (!m_Connected && !mysql_real_connect(m_ThreadConnPtr, m_Hostname.c_str(), m_Username.c_str(), m_Password.c_str(), m_Database.c_str(), m_iPort, NULL, CLIENT_COMPRESS)) {
		int ErrorID = mysql_errno(m_ThreadConnPtr);
		Native::Log(LOG_ERROR, "CMySQLHandle::ConnectT() - %s (error ID: %d).", mysql_error(m_ThreadConnPtr), ErrorID);
		ReturnVal = false;
		m_Connected = false;
	} else {
		Native::Log(LOG_DEBUG, "CMySQLHandle::ConnectT() - Connection was successful.");
		my_bool reconnect;
		mysql_options(m_ThreadConnPtr, MYSQL_OPT_RECONNECT, &reconnect);
		Native::Log(LOG_DEBUG, "CMySQLHandle::ConnectT() - Auto-reconnect has been enabled.");
		ReturnVal = true;
		m_Connected = true;
	}
	
	SQLHandleMutex.Unlock();
	return ReturnVal;
}


void CMySQLHandle::DisconnectT() {
	SQLHandleMutex.Lock(); 
	
	if (m_ThreadConnPtr == NULL) {
		Native::Log(LOG_WARNING, "CMySQLHandle::DisconnectT() - There is no connection opened.");
	} else {
		mysql_close(m_ThreadConnPtr);
		m_ThreadConnPtr = NULL;
		Native::Log(LOG_DEBUG, "CMySQLHandle::DisconnectT() - Connection was closed.");
	}
	m_Connected = false;
	
	SQLHandleMutex.Unlock();
}

int CMySQLHandle::SaveActiveResult() {
	if(m_ActiveResult != NULL) {
		if(m_ActiveResultID != 0) { //if active cache was already saved
			Native::Log(LOG_WARNING, "CMySQLHandle::SaveActiveResult() - Active cache was already saved.");
			return m_ActiveResultID; //return the ID of already saved cache
		}
		else {
			int ID = 1;
			
			if(!m_SavedResults.empty()) {
				map<int, CMySQLResult*>::iterator itHandle = m_SavedResults.end();
				std::advance(itHandle, -1);
				ID = itHandle->first+1;
			}
			m_ActiveResultID = ID;
			m_SavedResults.insert( std::map<int, CMySQLResult*>::value_type(ID, m_ActiveResult) );
			Native::Log(LOG_DEBUG, "CMySQLHandle::SaveActiveResult() - Cache saved with ID %d.", ID);
			return ID; 
		}
	}
	
	return 0;
}

bool CMySQLHandle::DeleteSavedResult(int resultid) {
	if(resultid > 0) {
		if(m_SavedResults.find(resultid) != m_SavedResults.end()) {
			CMySQLResult *ResultHandle = m_SavedResults.at(resultid);
			if(m_ActiveResult == ResultHandle) {
				m_ActiveResult = NULL;
				m_ActiveResultID = 0;
			}
			delete ResultHandle;
			m_SavedResults.erase(resultid);
			Native::Log(LOG_DEBUG, "CMySQLHandle::DeleteSavedResult() - Result deleted.");
			return true;
		}
	}
	Native::Log(LOG_WARNING, "CMySQLHandle::DeleteSavedResult() - Invalid result ID.");
	return false;
}

bool CMySQLHandle::SetActiveResult(int resultid) {
	if(resultid > 0) {
		if(m_SavedResults.find(resultid) != m_SavedResults.end()) {
			CMySQLResult *cResult = m_SavedResults.at(resultid);
			if(cResult != NULL) {
				if(m_ActiveResult != NULL)
					if(m_ActiveResultID == 0) //if cache not saved
						delete m_ActiveResult; //delete unsaved cache
				
				m_ActiveResult = cResult; //set new active cache
				m_ActiveResultID = resultid; //new active cache was stored previously
				Native::Log(LOG_DEBUG, "CMySQLHandle::SetActiveResult() - Specified result is now active.");
			}
		}
		else
			Native::Log(LOG_WARNING, "CMySQLHandle::SetActiveResult() - Result not found.");
	}
	else {
		if(m_ActiveResultID == 0) //if cache not saved
			delete m_ActiveResult; //delete unsaved cache
		m_ActiveResult = NULL;
		m_ActiveResultID = 0;
		Native::Log(LOG_DEBUG, "CMySQLHandle::SetActiveResult() - Invalid result ID specified, setting active result to zero.");
	}
	return true;
}


CMySQLQuery::CMySQLQuery() {
	ConnHandle = NULL;
	Result = NULL;
	Callback = NULL;
}

void CMySQLQuery::ExecuteT(CMySQLQuery *query, MYSQL *connptr) {
	CMySQLResult *Result = NULL;

	if(connptr != NULL) {

		MySQLMutex.Lock();
		
		int QueryErrorID = mysql_real_query(connptr, query->Query.c_str(), query->Query.length());
		if (QueryErrorID == 0) {

			Native::Log(LOG_DEBUG, "ExecuteT(%s) - Query was successful.", query->Callback->Name.c_str());

			MYSQL_RES *SQLResult;
			MYSQL_FIELD * SQLField;
			MYSQL_ROW SQLRow;

			SQLResult = mysql_store_result(connptr);
			MySQLMutex.Unlock();

			if ( SQLResult != NULL) {
				Result = new CMySQLResult;

				Result->m_Rows = mysql_num_rows(SQLResult);
				Result->m_Fields = mysql_num_fields(SQLResult);
				Result->m_Data.reserve((unsigned int)Result->m_Rows+1);

				char *szField = NULL;
				while ((SQLField = mysql_fetch_field(SQLResult))) {
					szField = new char[SQLField->name_length+1];
					memset(szField, '\0', (SQLField->name_length + 1));
					strcpy(szField, SQLField->name);
					Result->m_FieldNames.push_back(szField);
				}

				while (SQLRow = mysql_fetch_row(SQLResult)) {
					unsigned long *lengths = mysql_fetch_lengths(SQLResult);
					vector<char*> *tempVector = new vector<char*>;
					tempVector->reserve(Result->m_Fields+1);
					char* szCurrentRow;
					for (unsigned int a = 0; a < Result->m_Fields; a++) {
						if (!SQLRow[a]) {
							szCurrentRow = new char[4 + 1];
							memset(szCurrentRow, '\0', 4 + 1);
							strcpy(szCurrentRow, "NULL");
						} else {
							szCurrentRow = new char[lengths[a]+1];
							memset(szCurrentRow, '\0', (lengths[a] + 1));
							strcpy(szCurrentRow, SQLRow[a]);
						}
						tempVector->push_back(szCurrentRow);
					}
					Result->m_Data.push_back(tempVector);
				}

				mysql_free_result(SQLResult);
			}
			
			//forward Query to Callback handler
			Native::Log(LOG_DEBUG, "ExecuteT(%s) - Data being passed to ProcessCallbacks().", query->Callback->Name.c_str());
			query->Result = Result;
			CCallback::AddQueryToQueue(query);
		}
		else { //mysql_real_query failed
			
			MySQLMutex.Unlock();
			
			int ErrorID = mysql_errno(connptr);
			if(ErrorID != 1065 && query->Callback != NULL) {
				const char *ErrorString = mysql_error(connptr);
						
				Native::Log(LOG_ERROR, "ExecuteT(%s) - %s (error ID: %d)", query->Callback->Name.c_str(), ErrorString, ErrorID);
				Native::Log(LOG_DEBUG, "ExecuteT(%s) - Error will be triggered to OnQueryError().", query->Callback->Name.c_str());

				if(ErrorID == 2006) {
					Native::Log(LOG_WARNING, "ExecuteT() - Lost connection, reconnecting to the MySQL-server in the background thread.");
					MYSQL_RES *SQLRes;
					if ((SQLRes = mysql_store_result(connptr)) != NULL)  {
							mysql_free_result(SQLRes);
					}
					PushReconnect(query->ConnHandle);
				}

				//forward OnQueryError(errorid, error[], callback[], query[], connectionHandle);
				CCallback *ErrorCallback = new CCallback;
				ErrorCallback->Name = "OnQueryError";
				ErrorCallback->ParamFormat = "dsssd";
				stringstream ConvBuf, ConvBuf2;
				ConvBuf << ErrorID;
				ErrorCallback->Parameters.push(ConvBuf.str());
				ErrorCallback->Parameters.push(ErrorString);
				ErrorCallback->Parameters.push(query->Callback->Name);
				ErrorCallback->Parameters.push(query->Query);
				ConvBuf2 << query->ConnHandle->GetID();
				ErrorCallback->Parameters.push(ConvBuf2.str());

				CMySQLQuery *ErrorCBQuery = new CMySQLQuery;
				ErrorCBQuery->Callback = ErrorCallback;
				ErrorCBQuery->Result = NULL;
				ErrorCBQuery->ConnHandle = query->ConnHandle;
				CCallback::AddQueryToQueue(ErrorCBQuery);

				//push query again into queue
				/*if(ErrorID != 1064) //mistake in query syntax
					CMySQLQuery::PushQuery(query);
				else*/
				delete query;
				query = NULL;
			}
		}
	}
	if(MultiThreading == true) {
		MySQLMutex.Lock();
		QueryCounter--;
		MySQLMutex.Unlock();
	}
}