#pragma once
#ifndef INC_CMYSQLRESULT_H
#define INC_CMYSQLRESULT_H


#ifdef _WIN32
#include <WinSock2.h>
#endif
#include "mysql_include/mysql.h"


#include <vector>
using std::vector;



enum eFieldDataType {
	TYPE_NONE,
	TYPE_INT,
	TYPE_FLOAT,
	TYPE_STRING
};


class CMySQLResult {
public:
	friend class CMySQLQuery;
	
	CMySQLResult();
	~CMySQLResult();

	my_ulonglong GetRowCount() {
		return m_Rows;
	}

	unsigned int GetFieldCount() {
		return m_Fields;
	}

	//unsigned int GetFieldName(unsigned int idx, string &dest);
	unsigned int GetFieldName(unsigned int idx, char **dest);
	//unsigned int GetRowData(unsigned int row, unsigned int fieldidx, string &dest);
	unsigned int GetRowData(unsigned int row, unsigned int fieldidx, char **dest);
	//unsigned int GetRowDataByName(unsigned int row, string field, string &dest);
	unsigned int GetRowDataByName(unsigned int row, const char *field, char **dest);


	my_ulonglong InsertID() {
		return m_InsertID;
	}

	my_ulonglong AffectedRows() {
		return m_AffectedRows;
	}

	unsigned int GetWarningCount() {
		return m_WarningCount;
	}

private:
	unsigned int m_Fields;
	my_ulonglong m_Rows;

	vector<vector<char*>* > m_Data;
	vector<char*> m_FieldNames;
	vector<unsigned int> m_FieldDataTypes;

	my_ulonglong m_InsertID, m_AffectedRows;

	unsigned int m_WarningCount;
};


#endif
