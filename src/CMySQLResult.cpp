#pragma once

#include "CLog.h"
#include "CMySQLResult.h"



unsigned int CMySQLResult::GetFieldName(unsigned int idx, char **dest) {
	if (idx < m_FieldNames.size()) {
		(*dest) = m_FieldNames.at(idx);

		//Native::Log(LOG_DEBUG, "CMySQLResult::GetFieldName() - Result: \"%s\"", dest);
		if(CLog::Get()->IsLogLevel(LOG_DEBUG)) {
			char LogMsgBuf[128];
			sprintf2(LogMsgBuf, "index: '%d', name: \"%s\"", idx, *dest);
			CLog::Get()->LogFunction(LOG_DEBUG, "CMySQLResult::GetFieldName", LogMsgBuf);
		}
		
		return m_FieldDataTypes.at(idx);
	}
	else {

		//Native::Log(LOG_WARNING, "CMySQLResult::GetFieldName() - Invalid field index.");
		if(CLog::Get()->IsLogLevel(LOG_WARNING)) {
			char LogMsgBuf[128];
			sprintf2(LogMsgBuf, "invalid field index ('%d')", idx);
			CLog::Get()->LogFunction(LOG_WARNING, "CMySQLResult::GetFieldName", LogMsgBuf);
		}

		return TYPE_NONE;
	}
}

unsigned int CMySQLResult::GetRowData(unsigned int row, unsigned int fieldidx,char **dest) {
	if(row < m_Rows && fieldidx < m_Fields) {
		(*dest) = m_Data.at(row)->at(fieldidx);

		//Native::Log(LOG_DEBUG, "CMySQLResult::GetRowData() - Result: \"%s\"", *dest);
		if(CLog::Get()->IsLogLevel(LOG_DEBUG)) {
			char LogMsgBuf[512];
			sprintf2(LogMsgBuf, "row: '%d', field: '%d', data: \"%s\"", row, fieldidx, *dest);
			CLog::Get()->LogFunction(LOG_DEBUG, "CMySQLResult::GetRowData", LogMsgBuf);
		}

		return m_FieldDataTypes.at(fieldidx);
	}
	else {
		//Native::Log(LOG_WARNING, "CMySQLResult::GetRowData() - Invalid row or field index.");
		if(CLog::Get()->IsLogLevel(LOG_WARNING)) {
			char LogMsgBuf[128];
			sprintf2(LogMsgBuf, "invalid row ('%d') or field index ('%d')", row, fieldidx);
			CLog::Get()->LogFunction(LOG_WARNING, "CMySQLResult::GetRowData", LogMsgBuf);
		}

		return TYPE_NONE;
	}
}

unsigned int CMySQLResult::GetRowDataByName(unsigned int row, const char *field, char **dest) {
	if (row < m_Rows && m_Fields > 0) {
		for (unsigned int i = 0; i < m_Fields; ++i) {
			if (!strcmp(m_FieldNames.at(i), field)) {
				(*dest) = m_Data.at(row)->at(i);

				//Native::Log(LOG_DEBUG, "CMySQLResult::GetRowDataByName() - Result: \"%s\"", *dest);
				if(CLog::Get()->IsLogLevel(LOG_DEBUG)) {
					char LogMsgBuf[512];
					sprintf2(LogMsgBuf, "row: '%d', field: \"%s\", data: \"%s\"", row, field, *dest);
					CLog::Get()->LogFunction(LOG_DEBUG, "CMySQLResult::GetRowDataByName", LogMsgBuf);
				}

				return m_FieldDataTypes.at(i);
			}
		}

		//Native::Log(LOG_WARNING, "CMySQLResult::GetRowDataByName() - Field not found.");
		if(CLog::Get()->IsLogLevel(LOG_WARNING)) {
			char LogMsgBuf[128];
			sprintf2(LogMsgBuf, "field not found (\"%s\")", field);
			CLog::Get()->LogFunction(LOG_WARNING, "CMySQLResult::GetRowDataByName", LogMsgBuf);
		}

	}
	else {
		//Native::Log(LOG_WARNING, "CMySQLResult::GetRowDataByName() - Invalid row index.");
		if(CLog::Get()->IsLogLevel(LOG_WARNING)) {
			char LogMsgBuf[128];
			sprintf2(LogMsgBuf, "invalid row index ('%d')", row);
			CLog::Get()->LogFunction(LOG_WARNING, "CMySQLResult::GetRowDataByName()", LogMsgBuf);
		}
	}
	return TYPE_NONE;
}

CMySQLResult::CMySQLResult() {
	m_Fields = 0;
	m_Rows = 0; 
	m_Data.clear();
	m_FieldNames.clear();
	m_InsertID = 0;
	m_AffectedRows = 0;
	m_WarningCount = 0;
}

CMySQLResult::~CMySQLResult() {
	for(unsigned int i=0; i < m_FieldNames.size(); ++i) {
		free(m_FieldNames.at(i));
	}
	m_FieldNames.clear();

	for(vector<vector<char*>* >::iterator it1 = m_Data.begin(), end1 = m_Data.end(); it1 != end1; it1++) {
		for(vector<char*>::iterator it2 = (*it1)->begin(), end2 = (*it1)->end(); it2 != end2; it2++)
			free(*it2);
		delete (*it1);
	} 
	m_Data.clear();

	//Native::Log(LOG_DEBUG, "CMySQLResult::~CMySQLResult() - Deconstructor called.");
	CLog::Get()->LogFunction(LOG_DEBUG, "CMySQLResult::~CMySQLResult()", "deconstructor called");
}