#pragma once

#include "CLog.h"

#include "CMySQLHandle.h"
#include "CMySQLResult.h"

#include <cstring>
#include "malloc.h"


 

CMutex CMySQLHandle::SQLHandleMutex;

map<int, CMySQLHandle *> CMySQLHandle::SQLHandle;



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


	//Native::Log(LOG_DEBUG, "CMySQLHandle::CMySQLHandle() - Constructor called.");
	CLog::Get()->LogFunction(LOG_DEBUG, "CMySQLHandle::CMySQLHandle", "constructor called");
}

CMySQLHandle::~CMySQLHandle() {
	for (map<int, CMySQLResult*>::iterator it = m_SavedResults.begin(), end = m_SavedResults.end(); it != end; it++)
		delete it->second;

	//Native::Log(LOG_DEBUG, "CMySQLHandle::~CMySQLHandle() - Deconstructor called.");
	CLog::Get()->LogFunction(LOG_DEBUG, "CMySQLHandle::~CMySQLHandle", "deconstructor called");
}

int CMySQLHandle::Create(string host, string user, string pass, string db, size_t port) {
	int ID = -1;
	SQLHandleMutex.Lock();
	if (SQLHandle.size() > 0) {
		// Code used for checking duplicate connections.
		for(map<int, CMySQLHandle*>::iterator i = SQLHandle.begin(), end = SQLHandle.end(); i != end; ++i) {
			CMySQLHandle *Handle = i->second;
			if (Handle->m_Hostname.compare(host) == 0 && Handle->m_Username.compare(user) == 0 && Handle->m_Database.compare(db) == 0 && Handle->m_Password.compare(pass) == 0) 
			{
				/*SQLHandle[i]->m_bIsConnected = false;
				SQLHandle[i]->Connect();
				match = true;*/

				//Native::Log(LOG_WARNING, "CMySQLHandle::Create() - Connection already exists.");
				CLog::Get()->LogFunction(LOG_WARNING, "CMySQLHandle::Create", "connection already exists");
				ID = i->first;
				break;
			}
		}
	}
	if (ID == -1) {
		//Native::Log(LOG_DEBUG, "CMySQLHandle::Create() - Creating new connection..");
		CLog::Get()->LogFunction(LOG_DEBUG, "CMySQLHandle::Create", "creating new connection..");

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

		//Native::Log(LOG_DEBUG, "CMySQLHandle::Create() - Connection created with ID %d.", ID);
		if(CLog::Get()->IsLogLevel(LOG_DEBUG)) {
			char LogBufMsg[128];
			sprintf2(LogBufMsg, "connection created with ID = %d", ID);
			CLog::Get()->LogFunction(LOG_DEBUG, "CMySQLHandle::Create", LogBufMsg);
		}
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
			CLog::Get()->LogFunction(LOG_ERROR, "CMySQLHandle::ConnectT", "MySQL initialization failed", true);
			//Native::Log(LOG_ERROR, "CMySQLHandle::ConnectT() - MySQL initialization failed.");
	}
	if (!m_Connected && !mysql_real_connect(m_ThreadConnPtr, m_Hostname.c_str(), m_Username.c_str(), m_Password.c_str(), m_Database.c_str(), m_iPort, NULL, /*CLIENT_COMPRESS*/NULL)) {
		int ErrorID = mysql_errno(m_ThreadConnPtr);

		//Native::Log(LOG_ERROR, "CMySQLHandle::ConnectT() - %s (error ID: %d).", mysql_error(m_ThreadConnPtr), ErrorID);  
		if(CLog::Get()->IsLogLevel(LOG_ERROR)) {
			char LogBufMsg[512];
			sprintf2(LogBufMsg, "(error #%d) %s", ErrorID, mysql_error(m_ThreadConnPtr));
			CLog::Get()->LogFunction(LOG_ERROR, "CMySQLHandle::ConnectT", LogBufMsg, true);
		}

		ReturnVal = false;
		m_Connected = false;
	} else {
		//Native::Log(LOG_DEBUG, "CMySQLHandle::ConnectT() - Connection was successful.");
		CLog::Get()->LogFunction(LOG_DEBUG, "CMySQLHandle::ConnectT", "connection was successful", true);
		my_bool reconnect = 1;
		mysql_options(m_ThreadConnPtr, MYSQL_OPT_RECONNECT, &reconnect);
		//Native::Log(LOG_DEBUG, "CMySQLHandle::ConnectT() - Auto-reconnect has been enabled.");
		CLog::Get()->LogFunction(LOG_DEBUG, "CMySQLHandle::ConnectT", "auto-reconnect has been enabled", true);
		ReturnVal = true;
		m_Connected = true;
	}
	
	SQLHandleMutex.Unlock();
	return ReturnVal;
}


void CMySQLHandle::DisconnectT() {
	SQLHandleMutex.Lock(); 
	
	if (m_ThreadConnPtr == NULL) {
		CLog::Get()->LogFunction(LOG_WARNING, "CMySQLHandle::DisconnectT", "no connection available", true);
		//Native::Log(LOG_WARNING, "CMySQLHandle::DisconnectT() - There is no connection opened.");
	} else {
		mysql_close(m_ThreadConnPtr);
		m_ThreadConnPtr = NULL;
		CLog::Get()->LogFunction(LOG_DEBUG, "CMySQLHandle::DisconnectT", "connection was closed", true);
		//Native::Log(LOG_DEBUG, "CMySQLHandle::DisconnectT() - Connection was closed.");
	}
	m_Connected = false;
	
	SQLHandleMutex.Unlock();
}

int CMySQLHandle::SaveActiveResult() {
	if(m_ActiveResult != NULL) {
		if(m_ActiveResultID != 0) { //if active cache was already saved
			CLog::Get()->LogFunction(LOG_WARNING, "CMySQLHandle::SaveActiveResult", "active cache was already saved");
			//Native::Log(LOG_WARNING, "CMySQLHandle::SaveActiveResult() - Active cache was already saved.");
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
			
			//Native::Log(LOG_DEBUG, "CMySQLHandle::SaveActiveResult() - Cache saved with ID %d.", ID);
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
			//Native::Log(LOG_DEBUG, "CMySQLHandle::DeleteSavedResult() - Result deleted.");
			return true;
		}
	}
	
	//Native::Log(LOG_WARNING, "CMySQLHandle::DeleteSavedResult() - Invalid result ID.");
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
				//Native::Log(LOG_DEBUG, "CMySQLHandle::SetActiveResult() - Specified result is now active.");
			}
		}
		else
			CLog::Get()->LogFunction(LOG_ERROR, "CMySQLHandle::SetActiveResult", "result not found");
			//Native::Log(LOG_WARNING, "CMySQLHandle::SetActiveResult() - Result not found.");
	}
	else {
		if(m_ActiveResultID == 0) //if cache not saved
			delete m_ActiveResult; //delete unsaved cache
		m_ActiveResult = NULL;
		m_ActiveResultID = 0;
		CLog::Get()->LogFunction(LOG_DEBUG, "CMySQLHandle::SetActiveResult", "invalid result ID specified, setting active result to zero");
		//Native::Log(LOG_DEBUG, "CMySQLHandle::SetActiveResult() - Invalid result ID specified, setting active result to zero.");
	}
	return true;
}


