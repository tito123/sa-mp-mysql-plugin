#pragma once

#include "CLog.h"

#include "CMySQLHandle.h"
#include "CMySQLResult.h"

//#include <cstring>
//#include "malloc.h"


unordered_map<int, CMySQLHandle *> CMySQLHandle::SQLHandle;

CMySQLHandle::CMySQLHandle(int id, string host, string user, string passw, string db, size_t port, bool reconnect) {
	m_CID = id;
	m_Hostname.assign(host);
	m_Username.assign(user);
	m_Password.assign(passw);
	m_Database.assign(db);
	m_iPort = port;
	m_AutoReconnect = reconnect;
	
	m_SavedResults.clear();
	m_ActiveResult = NULL;
	m_ActiveResultID = 0;
	
	m_Connected = false;
	m_ErrnoVal = 0;

	m_MySQLConnPtr = NULL;


	CLog::Get()->LogFunction(LOG_DEBUG, "CMySQLHandle::CMySQLHandle", "constructor called");
}

CMySQLHandle::~CMySQLHandle() {
	for (unordered_map<int, CMySQLResult*>::iterator it = m_SavedResults.begin(), end = m_SavedResults.end(); it != end; it++)
		delete it->second;

	CLog::Get()->LogFunction(LOG_DEBUG, "CMySQLHandle::~CMySQLHandle", "deconstructor called");
}

CMySQLHandle *CMySQLHandle::Create(string host, string user, string pass, string db, size_t port, bool reconnect) {
	//int ID = -1;
	CMySQLHandle *handle = NULL;
	if (SQLHandle.size() > 0) {
		// Code used for checking duplicate connections.
		for(unordered_map<int, CMySQLHandle*>::iterator i = SQLHandle.begin(), end = SQLHandle.end(); i != end; ++i) {
			CMySQLHandle *Handle = i->second;
			if (Handle->m_Hostname.compare(host) == 0 && Handle->m_Username.compare(user) == 0 && Handle->m_Database.compare(db) == 0 && Handle->m_Password.compare(pass) == 0) 
			{
				CLog::Get()->LogFunction(LOG_WARNING, "CMySQLHandle::Create", "connection already exists");
				//ID = i->first;
				handle = i->second;
				break;
			}
		}
	}
	//if (ID == -1) {
	if(handle == NULL) {
		CLog::Get()->LogFunction(LOG_DEBUG, "CMySQLHandle::Create", "creating new connection..");

		int ID = 1;
		if(!SQLHandle.empty()) {
			unordered_map<int, CMySQLHandle*>::iterator itHandle = SQLHandle.begin();
			do {
				ID = itHandle->first+1;
				++itHandle;
			} while(SQLHandle.find(ID) != SQLHandle.end());
		}


		handle = new CMySQLHandle(ID, host, user, pass, db, port, reconnect);

		SQLHandle.insert( unordered_map<int, CMySQLHandle*>::value_type(ID, handle) );

		if(CLog::Get()->IsLogLevel(LOG_DEBUG)) {
			char LogBufMsg[128];
			sprintf2(LogBufMsg, "connection created with ID = %d", ID);
			CLog::Get()->LogFunction(LOG_DEBUG, "CMySQLHandle::Create", LogBufMsg);
		}
	}
	return handle;
}

void CMySQLHandle::Destroy() {
	if(m_Connected == true)
		Disconnect();
	SQLHandle.erase(m_CID);
	delete this;
}

bool CMySQLHandle::IsValid(int id) {
	if(SQLHandle.find(id) == SQLHandle.end()/* || SQLHandle.at(id) == NULL*/)
		return false;
	return true;
}



int CMySQLHandle::Connect(bool threaded) {
	int ReturnVal = 0;
	MySQLMutex.Lock();

	if(m_MySQLConnPtr == NULL) {
		m_MySQLConnPtr = mysql_init(NULL);
		if (m_MySQLConnPtr == NULL)
			CLog::Get()->LogFunction(LOG_ERROR, "CMySQLHandle::Connect", "MySQL initialization failed", threaded);
	}
	if (!m_Connected && !mysql_real_connect(m_MySQLConnPtr, m_Hostname.c_str(), m_Username.c_str(), m_Password.c_str(), m_Database.c_str(), m_iPort, NULL, NULL)) {
		int ErrorID = CallErrno();

		if(CLog::Get()->IsLogLevel(LOG_ERROR)) {
			char LogBufMsg[512];
			sprintf2(LogBufMsg, "(error #%d) %s", ErrorID, mysql_error(m_MySQLConnPtr));
			CLog::Get()->LogFunction(LOG_ERROR, "CMySQLHandle::Connect", LogBufMsg, threaded);
		}

		ReturnVal = ErrorID;
		m_Connected = false;
	} else {
		CLog::Get()->LogFunction(LOG_DEBUG, "CMySQLHandle::Connect", "connection was successful", threaded);
		my_bool reconnect = m_AutoReconnect;
		mysql_options(m_MySQLConnPtr, MYSQL_OPT_RECONNECT, &reconnect);
		if(CLog::Get()->IsLogLevel(LOG_DEBUG)) {
			char LogBufMsg[128];
			sprintf2(LogBufMsg, "auto-reconnect has been %s", m_AutoReconnect == true ? "enabled" : "disabled");
			CLog::Get()->LogFunction(LOG_DEBUG, "CMySQLHandle::Connect", LogBufMsg, threaded);
		}
		ReturnVal = 0;
		m_Connected = true;
	}
	
	MySQLMutex.Unlock();
	return ReturnVal;
}


void CMySQLHandle::Disconnect(bool threaded) {
	MySQLMutex.Lock();
	
	if (m_MySQLConnPtr == NULL) {
		CLog::Get()->LogFunction(LOG_WARNING, "CMySQLHandle::Disconnect", "no connection available", threaded);
	} else {
		mysql_close(m_MySQLConnPtr);
		m_MySQLConnPtr = NULL;
		CallErrno();
		CLog::Get()->LogFunction(LOG_DEBUG, "CMySQLHandle::Disconnect", "connection was closed", threaded);
	}
	m_Connected = false;
	
	MySQLMutex.Unlock();
}

int CMySQLHandle::SaveActiveResult() {
	if(m_ActiveResult != NULL) {
		if(m_ActiveResultID != 0) { //if active cache was already saved
			CLog::Get()->LogFunction(LOG_WARNING, "CMySQLHandle::SaveActiveResult", "active cache was already saved");
			return m_ActiveResultID; //return the ID of already saved cache
		}
		else {
			int ID = 1;
			
			if(!m_SavedResults.empty()) {
				unordered_map<int, CMySQLResult*>::iterator itHandle = m_SavedResults.begin();
				do {
					ID = itHandle->first+1;
					++itHandle;
				} while(m_SavedResults.find(ID) != m_SavedResults.end());
			}
			m_ActiveResultID = ID;
			m_SavedResults.insert( std::map<int, CMySQLResult*>::value_type(ID, m_ActiveResult) );
			
			if(CLog::Get()->IsLogLevel(LOG_DEBUG)) {
				char LogBufMsg[128];
				sprintf2(LogBufMsg, "cache saved with ID = %d", ID);
				CLog::Get()->LogFunction(LOG_DEBUG, "CMySQLHandle::SaveActiveResult", LogBufMsg);
			}
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
			CLog::Get()->LogFunction(LOG_DEBUG, "CMySQLHandle::DeleteSavedResult", "result deleted");
			return true;
		}
	}
	
	if(CLog::Get()->IsLogLevel(LOG_WARNING)) {
		char LogBufMsg[128];
		sprintf2(LogBufMsg, "invalid result ID ('%d')", resultid);
		CLog::Get()->LogFunction(LOG_WARNING, "CMySQLHandle::DeleteSavedResult", LogBufMsg);
	}
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
				CLog::Get()->LogFunction(LOG_DEBUG, "CMySQLHandle::SetActiveResult", "result is now active");
			}
		}
		else
			CLog::Get()->LogFunction(LOG_ERROR, "CMySQLHandle::SetActiveResult", "result not found");
	}
	else {
		if(m_ActiveResultID == 0) //if cache not saved
			delete m_ActiveResult; //delete unsaved cache
		m_ActiveResult = NULL;
		m_ActiveResultID = 0;
		CLog::Get()->LogFunction(LOG_DEBUG, "CMySQLHandle::SetActiveResult", "invalid result ID specified, setting active result to zero");
	}
	return true;
}

void CMySQLHandle::ClearAll()
{
	for(unordered_map<int, CMySQLHandle *>::iterator i = SQLHandle.begin(); i != SQLHandle.end(); ++i) {
		i->second->Disconnect();
		delete i->second;
	}
	SQLHandle.clear();
}

MYSQL * CMySQLHandle::GetMySQLPointer()
{
	MySQLMutex.Lock();
	MYSQL *ptr = m_MySQLConnPtr;
	MySQLMutex.Unlock();
	return ptr;
}

void CMySQLHandle::SetMySQLPointer( MYSQL *mysqlptr )
{
	MySQLMutex.Lock();
	m_MySQLConnPtr = mysqlptr;
	MySQLMutex.Unlock();
}

void CMySQLHandle::SetNewResult( CMySQLResult *result )
{
	m_ActiveResult = result;
	m_ActiveResultID = 0;
}


