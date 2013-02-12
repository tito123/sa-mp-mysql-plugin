#pragma once

#include "CMySQLHandler.h"

CMySQLHandler::CMySQLHandler(std::string host, std::string user, std::string passw, std::string db, size_t port) {
	m_Hostname.assign(host);
	m_Username.assign(user);
	m_Password.assign(passw);
	m_Database.assign(db);
	m_iPort = port;
	m_bIsConnected = false;
	m_bQueryProcessing = false;
	m_bNonThreadedQuery = false;
	m_stResult = NULL; // TODO: Set all fields to NULL.
	Natives::getInstance()->Debug("CMySQLHandler::CMySQLHandler() - constructor called.");
	Natives::getInstance()->Debug("CMySQLHandler::CMySQLHandler() - Connecting to \"%s\" | DB: \"%s\" | Username: \"%s\"...", m_Hostname.c_str(), m_Database.c_str(), m_Username.c_str());
	this->Connect();
}

CMySQLHandler::~CMySQLHandler() {
	Natives::getInstance()->Debug("CMySQLHandler::~CMySQLHandler() - deconstructor called.");
	FreeResult();
	Disconnect();
}

bool CMySQLHandler::IsValid(size_t id) {
	if (id >= SQLHandle.size()) {
		return false;
	}
	if (SQLHandle[id] == NULL) {
		return false;
	}
	return true;
}

bool CMySQLHandler::FetchField(string column) {
	if (!m_bIsConnected) {
		Natives::getInstance()->Debug("CMySQLHandler::FetchField(%s) - You cannot call this function now (connection is dead).", column.c_str());
		return 0;
	}
	if (m_szFields.empty() || m_stRow == NULL) {
		Natives::getInstance()->Debug("CMySQLHandler::FetchField(%s) - You cannot call this function now (no result).", column.c_str());
		return 0;
	}
	bool bFieldExists = false;
	for (unsigned int i = 0; i < m_dwFields; i++) {
		if (!strcmp(column.c_str(), m_szFields[i])) {
			m_szResult = (m_stRow[i] ? m_stRow[i] : "NULL");
			//m_cSStream << (m_stRow[i] ? m_stRow[i] : "NULL");
			bFieldExists = true;
			break;
		}
	}
	if (!bFieldExists) {
		Natives::getInstance()->Debug("CMySQLHandler::FetchField(\"%s\") - Field doesn't exist.", column.c_str());
		m_szResult = "NULL";
		return 0;
	}
	//getline(m_cSStream, m_szResult);
	//m_cSStream >> m_szResult;
	Natives::getInstance()->Debug("CMySQLHandler::FetchField(\"%s\") - %s.", column.c_str(), m_szResult.c_str());
	//m_cSStream.clear();
	return 1;
}

bool CMySQLHandler::Seek(size_t offset) {
	if (!m_bIsConnected) {
		Natives::getInstance()->Debug("CMySQLHandler::Seek() - You cannot call this function now (connection is dead).");
		return 0;
	}
	if (m_stResult == NULL) {
		Natives::getInstance()->Debug("CMySQLHandler::Seek() - The result is empty.");
		return 0;
	} else {
		mysql_data_seek(m_stResult, offset);
	}
	return 1;
}

string CMySQLHandler::FetchFieldName(int number) {
	if (!m_bIsConnected) {
		Natives::getInstance()->Debug("CMySQLHandler::FetchFieldName() - You cannot call this function now (connection is dead).");
		return 0;
	}
	if (m_stResult == NULL) {
		Natives::getInstance()->Debug("CMySQLHandler::FetchFieldName() - You cannot call this function now (no result).");
		return 0;
	}
	m_stField = mysql_fetch_field_direct(m_stResult, number);
	string szFieldname(m_stField->name);
	free(m_stField);
	Natives::getInstance()->Debug("CMySQLHandler::FetchFieldName(%d) - Returned: %s", number, szFieldname.c_str());
	return szFieldname;
}

string CMySQLHandler::FetchRow() {
	if (!m_bIsConnected) {
		Natives::getInstance()->Debug("CMySQLHandler::FetchRow() - You cannot call this function now (connection is dead).");
		return string("NULL");
	}
	if (m_stResult == NULL) {
		Natives::getInstance()->Debug("CMySQLHandler::FetchRow() - You cannot call this function now (no result).");
		return string("NULL");
	}
	m_dwFields = mysql_num_fields(m_stResult);
	m_stField = mysql_fetch_fields(m_stResult);
	if ((m_stRow = mysql_fetch_row(m_stResult))) {
		m_szResult.clear();
		for (string::size_type i = 0; i < m_dwFields; i++) {
			m_szResult.append(m_stRow[i] ? m_stRow[i] : "NULL");
			m_szResult.append(Delimiter);
		}
		if (m_szResult.empty()) {
			Natives::getInstance()->Debug("CMySQLHandler::FetchRow() - Result is empty.");
			return string("NULL");
		} else {
			m_szResult.erase(m_szResult.length() - 1, m_szResult.length());
			Natives::getInstance()->Debug("CMySQLHandler::FetchRow() - Return: %s.", m_szResult.c_str());
			return m_szResult;
		}
	} else {
		m_dwError = mysql_errno(m_stConnectionPtr);
		if (m_dwError > 0) {
			Natives::getInstance()->Debug("CMySQLHandler::FetchRow() - An error has occured (error: %d, %s).", m_dwError, mysql_error(m_stConnectionPtr));
		}
		return string("NULL");
	}
}

int CMySQLHandler::RetrieveRow() {
	if (!m_bIsConnected) {
		Natives::getInstance()->Debug("CMySQLHandler::RetrieveRow() - You cannot call this function now (connection is dead).");
		return 0;
	}
	if (m_stResult == NULL) {
		Natives::getInstance()->Debug("CMySQLHandler::RetrieveRow() - You cannot call this function now (no result).");
		return 0;
	}
	m_dwFields = mysql_num_fields(m_stResult);
	if ((m_stRow = mysql_fetch_row(m_stResult))) {
		if (m_szCacheFields.empty()) {
			char* szField;
			while ((m_stField = mysql_fetch_field(m_stResult))) {
				szField = (char*)malloc(m_stField->name_length * sizeof(char) + 1);
				memset(szField, '\0', (m_stField->name_length + 1));
				strcpy(szField, m_stField->name);
				m_szFields.push_back(szField);
			}
		}
		return 1;
	}
	return 0;
}

bool CMySQLHandler::Connect() {
	if (m_bIsConnected) {
		return 0;
	}
	m_stConnectionPtr = mysql_init(NULL);
	if (m_stConnectionPtr == NULL) {
		Natives::getInstance()->Debug("CMySQLHandler::Connect() - MySQL initialization failed.");
	}
	if (!mysql_real_connect(m_stConnectionPtr, m_Hostname.c_str(), m_Username.c_str(), m_Password.c_str(), m_Database.c_str(), m_iPort, NULL, CLIENT_COMPRESS)) {
		m_dwError = mysql_errno(m_stConnectionPtr);
		m_bIsConnected = false;
		Natives::getInstance()->Debug("CMySQLHandler::Connect() - %s (error ID: %d).", mysql_error(m_stConnectionPtr), m_dwError);
		return 0;
	} else {
		m_bIsConnected = true;
		Natives::getInstance()->Debug("CMySQLHandler::Connect() - Connection was successful.");
		my_bool reconnect;
		mysql_options(m_stConnectionPtr, MYSQL_OPT_RECONNECT, &reconnect);
		Natives::getInstance()->Debug("CMySQLHandler::Connect() - Auto-reconnect has been enabled.");
		return 1;
	}
	return 1;
}

void CMySQLHandler::Disconnect() {
	if (!m_bIsConnected) {
		Natives::getInstance()->Debug("CMySQLHandler::Disconnect() - You cannot call this function now (connection is dead).");
	} else {
		if (m_stConnectionPtr == NULL) {
			Natives::getInstance()->Debug("CMySQLHandler::Disconnect() - There is no connection opened.");
		} else {
			mysql_close(m_stConnectionPtr);
			m_stConnectionPtr = NULL;
			Natives::getInstance()->Debug("CMySQLHandler::Disconnect() - Connection was closed.");
		}
		m_bIsConnected = false;
	}
}

int CMySQLHandler::SetCharset(string charsetname) {
	if (!m_bIsConnected) {
		Natives::getInstance()->Debug("CMySQLHandler::SetCharset() - You cannot call this function now (connection is dead).");
		return (-1);
	}
	return mysql_set_character_set(m_stConnectionPtr, charsetname.c_str());
}

string CMySQLHandler::GetCharset() {
	if (!m_bIsConnected) {
		Natives::getInstance()->Debug("CMySQLHandler::GetCharset() - You cannot call this function now (connection is dead).");
		return string("NULL");
	}
	return mysql_character_set_name(m_stConnectionPtr);
}

bool CMySQLHandler::FreeResult() {
	if (!m_bIsConnected) {
		Natives::getInstance()->Debug("CMySQLHandler::FreeResult() - There is nothing to free (connection is dead).");
		return 0;
	}
	if (m_stResult == NULL) {
		Natives::getInstance()->Debug("CMySQLHandler::FreeResult() - The result is already empty.");
		return 0;
	}
	mysql_free_result(m_stResult);
	m_stResult = NULL;
	m_stRow = NULL;
	int size = m_szFields.size();
	if (size > 0) {
		for (unsigned int x = 0, s = size; x < s; x++) {
			free(m_szFields[x]);
		}
		m_szFields.clear();
	}
	Natives::getInstance()->Debug("CMySQLHandler::FreeResult() - Result was successfully freed.");
	return 1;
}

bool CMySQLHandler::StoreResult() {
	if (!m_bIsConnected) {
		Natives::getInstance()->Debug("CMySQLHandler::StoreResult() - There is nothing to store (connection is dead).");
		return 0;
	}
	if (!(m_stResult = mysql_store_result(m_stConnectionPtr))) { // Prevents the server from crashing.
		Natives::getInstance()->Debug("CMySQLHandler::StoreResult() - No data to store.");
		return 0;
	}
	Natives::getInstance()->Debug("CMySQLHandler::StoreResult() - Result was stored.");
	return 1;
}

my_ulonglong CMySQLHandler::InsertId() {
	if (!m_bIsConnected) {
		Natives::getInstance()->Debug("CMySQLHandler::InsertId() - You cannot call this function now (connection is dead).");
		return 0;
	}
	return mysql_insert_id(m_stConnectionPtr);
}

int CMySQLHandler::Reload() {
	if (!m_bIsConnected) {
		Natives::getInstance()->Debug("CMySQLHandler::InsertId() - You cannot call this function now (connection is dead).");
		return 0;
	}
	return mysql_reload(m_stConnectionPtr);;
}

my_ulonglong CMySQLHandler::AffectedRows() {
	if (!m_bIsConnected) {
		Natives::getInstance()->Debug("CMySQLHandler::AffectedRows() - You cannot call this function now (connection is dead).");
		return 0;
	}
	my_ulonglong ullAffected = mysql_affected_rows(m_stConnectionPtr);
	Natives::getInstance()->Debug("CMySQLHandler::NumRows() - Returned %d affected rows(s)", ullAffected);
	return ullAffected;
}

int CMySQLHandler::Ping() {
	if (!m_bIsConnected) {
		Natives::getInstance()->Debug("CMySQLHandler::Ping() - You cannot call this function now (connection is dead).");
		return 1;
	} else if (mysql_ping(m_stConnectionPtr) != 0) {
		m_dwError = mysql_errno(m_stConnectionPtr);
		Natives::getInstance()->Debug("CMySQLHandler::Ping() - An error has occured (error: %d, %s).", m_dwError, mysql_error(m_stConnectionPtr));
		return 1;
	} else {
		Natives::getInstance()->Debug("CMySQLHandler::Ping() - Connection is still alive.");
		return 0;
	}
}

my_ulonglong CMySQLHandler::NumRows() {
	if (!m_bIsConnected) {
		Natives::getInstance()->Debug("CMySQLHandler::NumRows() - You cannot call this function now (connection is dead).");
		return (-1);
	}
	if (m_stResult == NULL) {
		Natives::getInstance()->Debug("CMySQLHandler::NumRows() - You cannot call this function now (no result).");
		return (-1);
	}
	my_ulonglong ullRows = mysql_num_rows(m_stResult);
	Natives::getInstance()->Debug("CMySQLHandler::NumRows() - Returned %d row(s).", ullRows);
	return ullRows;
}

unsigned int CMySQLHandler::NumFields() {
	if (!m_bIsConnected) {
		Natives::getInstance()->Debug("CMySQLHandler::NumFields() - You cannot call this function now (connection is dead).");
		return (-1);
	}
	if (m_stResult == NULL) {
		Natives::getInstance()->Debug("CMySQLHandler::NumFields() - You cannot call this function now (no result).");
		return (-1);
	}
	unsigned int uiNumFields = mysql_num_fields(m_stResult);
	Natives::getInstance()->Debug("CMySQLHandler::NumFields() - Returned %d field(s).", uiNumFields);
	return uiNumFields;
}

unsigned int CMySQLHandler::FieldCount() {
	if (!m_bIsConnected) {
		Natives::getInstance()->Debug("CMySQLHandler::NumFields() - You cannot call this function now (connection is dead).");
		return (-1);
	}
	return mysql_field_count(m_stConnectionPtr);
}

unsigned int CMySQLHandler::WarningCount() {
	if (!m_bIsConnected) {
		Natives::getInstance()->Debug("CMySQLHandler::WarningCount() - You cannot call this function now (connection is dead).");
		return (-1);
	}
	unsigned int uiWarnings = mysql_warning_count(m_stConnectionPtr);
	Natives::getInstance()->Debug("CMySQLHandler::WarningCount() - Returned %d warning(s).", uiWarnings);
	return uiWarnings;
}

string CMySQLHandler::Statistics() {
	if (!m_bIsConnected) {
		Natives::getInstance()->Debug("CMySQLHandler::Statistics() - You cannot call this function now (connection is dead).");
		return 0;
	}
	return string(mysql_stat(m_stConnectionPtr));
}

int CMySQLHandler::EscapeStr(string source, char *to) {
	if (!m_bIsConnected) {
		Natives::getInstance()->Debug("CMySQLHandler::EscapeString() - You cannot call this function now (connection is dead).");;
		return 0;
	}
	int length = mysql_real_escape_string(m_stConnectionPtr, to, source.c_str(), source.length());
	Natives::getInstance()->Debug("CMySQLHandler::EscapeString(%s) - Escaped %u characters to %s.", source.c_str(), length, to);
	return length;
}