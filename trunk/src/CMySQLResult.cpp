#pragma once

#include "CLog.h"
#include "CMySQLResult.h"


void CMySQLResult::GetFieldName(unsigned int idx, char **dest) {
	if (idx < m_Fields) {
		(*dest) = const_cast<char*>(m_FieldNames.at(idx).c_str());

		if(CLog::Get()->IsLogLevel(LOG_DEBUG)) {
			char LogMsgBuf[256];
			sprintf2(LogMsgBuf, "index: '%d', name: \"%s\"", idx, *dest);
			CLog::Get()->LogFunction(LOG_DEBUG, "CMySQLResult::GetFieldName", LogMsgBuf);
		}
	}
	else {

		if(CLog::Get()->IsLogLevel(LOG_WARNING)) {
			char LogMsgBuf[128];
			sprintf2(LogMsgBuf, "invalid field index ('%d')", idx);
			CLog::Get()->LogFunction(LOG_WARNING, "CMySQLResult::GetFieldName", LogMsgBuf);
		}
	}
}

void CMySQLResult::GetRowData(unsigned int row, unsigned int fieldidx, char **dest) {
	if(row < m_Rows && fieldidx < m_Fields) {
		(*dest) = const_cast<char*>(m_Data.at(row).at(fieldidx).c_str());

		if(CLog::Get()->IsLogLevel(LOG_DEBUG)) {
			char LogMsgBuf[1096];
			string ShortenDest(*dest);
			ShortenDest.resize(1024);
			sprintf2(LogMsgBuf, "row: '%d', field: '%d', data: \"%s\"", row, fieldidx, ShortenDest.c_str());
			CLog::Get()->LogFunction(LOG_DEBUG, "CMySQLResult::GetRowData", LogMsgBuf);
		}
	}
	else {
		if(CLog::Get()->IsLogLevel(LOG_WARNING)) {
			char LogMsgBuf[128];
			sprintf2(LogMsgBuf, "invalid row ('%d') or field index ('%d')", row, fieldidx);
			CLog::Get()->LogFunction(LOG_WARNING, "CMySQLResult::GetRowData", LogMsgBuf);
		}
	}
}

void CMySQLResult::GetRowDataByName(unsigned int row, const char *field, char **dest) {
	if (row < m_Rows && m_Fields > 0) {
		for (unsigned int i = 0; i < m_Fields; ++i) {
			if(m_FieldNames.at(i).compare(field) == 0) {
				(*dest) = const_cast<char*>(m_Data.at(row).at(i).c_str());

				if(CLog::Get()->IsLogLevel(LOG_DEBUG)) {
					char LogMsgBuf[1296];
					string ShortenDest(*dest);
					ShortenDest.resize(1024);
					sprintf2(LogMsgBuf, "row: '%d', field: \"%s\", data: \"%s\"", row, field, ShortenDest.c_str());
					CLog::Get()->LogFunction(LOG_DEBUG, "CMySQLResult::GetRowDataByName", LogMsgBuf);
				}

				return ;
			}
		}

		if(CLog::Get()->IsLogLevel(LOG_WARNING)) {
			char LogMsgBuf[256];
			sprintf2(LogMsgBuf, "field not found (\"%s\")", field);
			CLog::Get()->LogFunction(LOG_WARNING, "CMySQLResult::GetRowDataByName", LogMsgBuf);
		}

	}
	else {
		if(CLog::Get()->IsLogLevel(LOG_WARNING)) {
			char LogMsgBuf[128];
			sprintf2(LogMsgBuf, "invalid row index ('%d')", row);
			CLog::Get()->LogFunction(LOG_WARNING, "CMySQLResult::GetRowDataByName()", LogMsgBuf);
		}
	}
}

CMySQLResult::CMySQLResult() {
	m_Fields = 0;
	m_Rows = 0; 
	m_InsertID = 0;
	m_AffectedRows = 0;
	m_WarningCount = 0;
}

CMySQLResult::~CMySQLResult() {
	CLog::Get()->LogFunction(LOG_DEBUG, "CMySQLResult::~CMySQLResult()", "deconstructor called");
}