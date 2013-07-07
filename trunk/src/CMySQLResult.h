#pragma once
#ifndef INC_CMYSQLRESULT_H
#define INC_CMYSQLRESULT_H


#ifdef _WIN32
#include <WinSock2.h>
#endif
#include "mysql_include/mysql.h"


#include <vector>
#include <string>
using std::vector;
using std::string;



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

	inline my_ulonglong GetRowCount() const {
		return m_Rows;
	}

	inline unsigned int GetFieldCount() const {
		return m_Fields;
	}

	unsigned short GetFieldName(unsigned int idx, char **dest);
	unsigned short GetRowData(unsigned int row, unsigned int fieldidx, char **dest);
	unsigned short GetRowDataByName(unsigned int row, const char *field, char **dest);


	inline my_ulonglong InsertID() const {
		return m_InsertID;
	}

	inline my_ulonglong AffectedRows() const {
		return m_AffectedRows;
	}

	inline unsigned int GetWarningCount() const {
		return m_WarningCount;
	}

private:
	unsigned int m_Fields;
	my_ulonglong m_Rows;

	vector< vector<string> > m_Data;
	vector<string> m_FieldNames;
	vector<unsigned short> m_FieldDataTypes;

	my_ulonglong m_InsertID, m_AffectedRows;

	unsigned int m_WarningCount;
};


#endif
