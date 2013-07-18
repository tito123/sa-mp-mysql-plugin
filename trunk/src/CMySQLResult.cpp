#pragma once

#include "CLog.h"
#include "CMySQLResult.h"


void CMySQLResult::GetFieldName(unsigned int idx, char **dest) {
	if (idx < m_Fields) {
		(*dest) = const_cast<char*>(m_FieldNames.at(idx).c_str());
		CLog::Get()->LogFunction(LOG_DEBUG, false, "CMySQLResult::GetFieldName", "index: '%d', name: \"%s\"", idx, *dest);
	}
	else 
		CLog::Get()->LogFunction(LOG_WARNING, false, "CMySQLResult::GetFieldName", "invalid field index ('%d')", idx);
}

void CMySQLResult::GetRowData(unsigned int row, unsigned int fieldidx, char **dest) {
	if(row < m_Rows && fieldidx < m_Fields) {
		(*dest) = const_cast<char*>(m_Data.at(row).at(fieldidx).c_str());

		if(CLog::Get()->IsLogLevel(LOG_DEBUG)) {
			string ShortenDest(*dest);
			ShortenDest.resize(1024);
			CLog::Get()->LogFunction(LOG_DEBUG, false, "CMySQLResult::GetRowData", "row: '%d', field: '%d', data: \"%s\"", row, fieldidx, ShortenDest.c_str());
		}
	}
	else 
		CLog::Get()->LogFunction(LOG_WARNING, false, "CMySQLResult::GetRowData", "invalid row ('%d') or field index ('%d')", row, fieldidx);
}

void CMySQLResult::GetRowDataByName(unsigned int row, const char *field, char **dest) {
	if (row < m_Rows && m_Fields > 0) {
		for (unsigned int i = 0; i < m_Fields; ++i) {
			if(m_FieldNames.at(i).compare(field) == 0) {
				(*dest) = const_cast<char*>(m_Data.at(row).at(i).c_str());

				if(CLog::Get()->IsLogLevel(LOG_DEBUG)) {
					string ShortenDest(*dest);
					ShortenDest.resize(1024);
					CLog::Get()->LogFunction(LOG_DEBUG, false, "CMySQLResult::GetRowDataByName", "row: '%d', field: \"%s\", data: \"%s\"", row, field, ShortenDest.c_str());
				}

				return ;
			}
		}
		CLog::Get()->LogFunction(LOG_WARNING, false, "CMySQLResult::GetRowDataByName", "field not found (\"%s\")", field);
	}
	else 
		CLog::Get()->LogFunction(LOG_WARNING, false, "CMySQLResult::GetRowDataByName()", "invalid row index ('%d')", row);
}

CMySQLResult::CMySQLResult() {
	m_Fields = 0;
	m_Rows = 0; 
	m_InsertID = 0;
	m_AffectedRows = 0;
	m_WarningCount = 0;
}

CMySQLResult::~CMySQLResult() {
	CLog::Get()->LogFunction(LOG_DEBUG, false, "CMySQLResult::~CMySQLResult()", "deconstructor called");
}