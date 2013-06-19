#pragma once

#include "CScripting.h"
#include "CMySQLHandle.h"
#include "CMySQLResult.h"
#include "CMySQLQuery.h"
#include "CCallback.h"
#include "CAmxString.h"
#include "CLog.h"

#include "misc.h"

#include <stdarg.h>
#include "malloc.h"
#include <cmath>

#include "boost/lexical_cast.hpp"


StrAmx *AMX_H;
logprintf_t logprintf;



//native cache_affected_rows(connectionHandle = 1);
cell AMX_NATIVE_CALL Native::cache_affected_rows(AMX* amx, cell* params) {
	unsigned int cID = params[1];

	if(CLog::Get()->IsLogLevel(LOG_DEBUG)) {
		char LogBuf[32];
		sprintf(LogBuf, "connection handle: %d", cID);
		CLog::Get()->LogFunction(LOG_DEBUG, "cache_affected_rows", LogBuf);	
	}

	if(!CMySQLHandle::IsValid(cID)) {
		ERROR_INVALID_CONNECTION_HANDLE("cache_affected_rows", cID);
		return 0;
	}

	CMySQLResult *Result = CMySQLHandle::GetHandle(cID)->GetResult();
	if(Result == NULL) { 
		CLog::Get()->LogFunction(LOG_WARNING, "cache_affected_rows", "no active cache");
		return 0;
	}
	
	return (cell)Result->AffectedRows();
}

//native cache_warning_count(connectionHandle = 1);
cell AMX_NATIVE_CALL Native::cache_warning_count(AMX* amx, cell* params) {
	unsigned int cID = params[1];

	if(CLog::Get()->IsLogLevel(LOG_DEBUG)) {
		char LogBuf[32];
		sprintf(LogBuf, "connection handle: %d", cID);
		CLog::Get()->LogFunction(LOG_DEBUG, "cache_warning_count", LogBuf);
	}

	if(!CMySQLHandle::IsValid(cID)) {
		ERROR_INVALID_CONNECTION_HANDLE("cache_warning_count", cID);
		return 0;
	}
	
	CMySQLResult *Result = CMySQLHandle::GetHandle(cID)->GetResult();
	if(Result == NULL) { 
		CLog::Get()->LogFunction(LOG_WARNING, "cache_warning_count", "no active cache");
		return 0;
	}
	
	return (cell)Result->GetWarningCount();
}

//native cache_insert_id(connectionHandle = 1);
cell AMX_NATIVE_CALL Native::cache_insert_id(AMX* amx, cell* params) {
	unsigned int cID = params[1];
	
	if(CLog::Get()->IsLogLevel(LOG_DEBUG)) {
		char LogBuf[32];
		sprintf(LogBuf, "connection handle: %d", cID);
		CLog::Get()->LogFunction(LOG_DEBUG, "cache_insert_id", LogBuf);
	}

	if(!CMySQLHandle::IsValid(cID)) {
		ERROR_INVALID_CONNECTION_HANDLE("cache_insert_id", cID);
		return 0;
	}

	CMySQLResult *Result = CMySQLHandle::GetHandle(cID)->GetResult();
	if(Result == NULL) { 
		CLog::Get()->LogFunction(LOG_WARNING, "cache_insert_id", "no active cache");
		return 0;
	}
	
	return (cell)Result->InsertID();
}


// native cache_save(connectionHandle = 1);
cell AMX_NATIVE_CALL Native::cache_save(AMX* amx, cell* params) {
	unsigned int cID = params[1];
	
	if(CLog::Get()->IsLogLevel(LOG_DEBUG)) {
		char LogBuf[32];
		sprintf(LogBuf, "connection handle: %d", cID);
		CLog::Get()->LogFunction(LOG_DEBUG, "cache_save", LogBuf);
	}

	if(!CMySQLHandle::IsValid(cID)) {
		ERROR_INVALID_CONNECTION_HANDLE("cache_save", cID);
		return 0;
	}
	
	int CacheID = CMySQLHandle::GetHandle(cID)->SaveActiveResult();
	if(CacheID == 0)
		CLog::Get()->LogFunction(LOG_WARNING, "cache_save", "no active cache");

	return (cell)CacheID;
}

// native cache_delete(id, connectionHandle = 1);
cell AMX_NATIVE_CALL Native::cache_delete(AMX* amx, cell* params) {
	unsigned int cID = params[2];
	
	if(CLog::Get()->IsLogLevel(LOG_DEBUG)) {
		char LogBuf[32];
		sprintf(LogBuf, "connection handle: %d", cID);
		CLog::Get()->LogFunction(LOG_DEBUG, "cache_delete", LogBuf);
	}

	if(!CMySQLHandle::IsValid(cID)) {
		ERROR_INVALID_CONNECTION_HANDLE("cache_delete", cID);
		return 0;
	}

	return (cell)CMySQLHandle::GetHandle(cID)->DeleteSavedResult(params[1]);
}

// native cache_set_active(id, connectionHandle = 1);
cell AMX_NATIVE_CALL Native::cache_set_active(AMX* amx, cell* params) {
	unsigned int cID = params[2];
	
	if(CLog::Get()->IsLogLevel(LOG_DEBUG)) {
		char LogBuf[32];
		sprintf(LogBuf, "connection handle: %d", cID);
		CLog::Get()->LogFunction(LOG_DEBUG, "cache_set_active", LogBuf);
	}

	if(!CMySQLHandle::IsValid(cID)) {
		ERROR_INVALID_CONNECTION_HANDLE("cache_set_active", cID);
		return 0;
	}

	return CMySQLHandle::GetHandle(cID)->SetActiveResult((int)params[1]);
}

// native cache_get_data(&num_rows, &num_fields, connectionHandle = 1);
cell AMX_NATIVE_CALL Native::cache_get_data(AMX* amx, cell* params) {
	unsigned int cID = params[3];
	
	if(CLog::Get()->IsLogLevel(LOG_DEBUG)) {
		char LogBuf[32];
		sprintf(LogBuf, "connection handle: %d", cID);
		CLog::Get()->LogFunction(LOG_DEBUG, "cache_get_data", LogBuf);
	}

	if(!CMySQLHandle::IsValid(cID)) {
		ERROR_INVALID_CONNECTION_HANDLE("cache_get_data", cID);
		return 0;
	}
	
	CMySQLResult *Result = CMySQLHandle::GetHandle(cID)->GetResult();
	if(Result == NULL) { 
		CLog::Get()->LogFunction(LOG_WARNING, "cache_get_data", "no active cache");
		return 0;
	}

	cell *AddressPtr;
	amx_GetAddr(amx, params[1], &AddressPtr);
	(*AddressPtr) = (cell)Result->GetRowCount();
	amx_GetAddr(amx, params[2], &AddressPtr);
	(*AddressPtr) = (cell)Result->GetFieldCount();
	return 1;
}

// native cache_get_field(field_index, dest[], connectionHandle = 1)
cell AMX_NATIVE_CALL Native::cache_get_field(AMX* amx, cell* params) {
	unsigned int cID = params[3];
	
	if(CLog::Get()->IsLogLevel(LOG_DEBUG)) {
		char LogBuf[32];
		sprintf(LogBuf, "connection handle: %d", cID);
		CLog::Get()->LogFunction(LOG_DEBUG, "cache_get_field", LogBuf);
	}

	if(!CMySQLHandle::IsValid(cID)) {
		ERROR_INVALID_CONNECTION_HANDLE("cache_get_field", cID);
		return 0;
	}
	
	CMySQLResult *Result = CMySQLHandle::GetHandle(cID)->GetResult();
	if(Result == NULL) {
		CLog::Get()->LogFunction(LOG_WARNING, "cache_get_field", "no active cache");
		return 0; 
	}
	
	//string FieldName;
	//char FieldName[128];
	char *FieldName = NULL;

	Result->GetFieldName(params[1], &FieldName);

	if(FieldName == NULL) {
		FieldName = (char *)malloc(5 * sizeof(char));
		strcpy(FieldName, "NULL");
		AMX_H->SetCString(amx, params[2], FieldName, params[4]);
		free(FieldName);
	}
	else
		AMX_H->SetCString(amx, params[2], FieldName, params[4]);
	return 1;
}

// native cache_get_row(row, idx, dest[], connectionHandle = 1);
cell AMX_NATIVE_CALL Native::cache_get_row(AMX* amx, cell* params) {
	unsigned int cID = params[4];
	
	if(CLog::Get()->IsLogLevel(LOG_DEBUG)) {
		char LogBuf[32];
		sprintf(LogBuf, "connection handle: %d", cID);
		CLog::Get()->LogFunction(LOG_DEBUG, "cache_get_row", LogBuf);
	}

	if(!CMySQLHandle::IsValid(cID)) {
		ERROR_INVALID_CONNECTION_HANDLE("cache_get_row", cID);
		return 0;
	}

	CMySQLResult *Result = CMySQLHandle::GetHandle(cID)->GetResult();
	if(Result == NULL) {
		CLog::Get()->LogFunction(LOG_WARNING, "cache_get_row", "no active cache");
		return 0;
	}

	//string RowData;
	//char *RowData = (char *)malloc( (params[5]+1) * sizeof(char));
	char *RowData = NULL;

	Result->GetRowData(params[1], params[2], &RowData);

	if(RowData == NULL) {
		RowData = (char *)malloc(5 * sizeof(char));
		strcpy(RowData, "NULL");
		AMX_H->SetCString(amx, params[3], RowData, params[5]);
		free(RowData);
	}
	else
		AMX_H->SetCString(amx, params[3], RowData, params[5]);
	return 1;
}

// native cache_get_row_int(row, idx, connectionHandle = 1);
cell AMX_NATIVE_CALL Native::cache_get_row_int(AMX* amx, cell* params) {
	unsigned int cID = params[3];
	
	if(CLog::Get()->IsLogLevel(LOG_DEBUG)) {
		char LogBuf[32];
		sprintf(LogBuf, "connection handle: %d", cID);
		CLog::Get()->LogFunction(LOG_DEBUG, "cache_get_row_int", LogBuf);
	}

	if(!CMySQLHandle::IsValid(cID)) {
		ERROR_INVALID_CONNECTION_HANDLE("cache_get_row_int", cID);
		return 0;
	}

	int ReturnVal = 0;
	CMySQLResult *Result = CMySQLHandle::GetHandle(cID)->GetResult();
	if(Result == NULL) {
		CLog::Get()->LogFunction(LOG_WARNING, "cache_get_row_int", "no active cache");
		return 0;
	}

	//string RowData;
	//char RowData[12];
	char *RowData = NULL;
	
	if(Result->GetRowData(params[1], params[2], &RowData) != TYPE_INT/* || sscanf(RowData, "%d", &ReturnVal) != 1*/) {
		CLog::Get()->LogFunction(LOG_ERROR, "cache_get_row_int", "invalid data type");
		ReturnVal = 0;
	}
	else
		ReturnVal = boost::lexical_cast<int>(RowData);
	return ReturnVal;
}

// native Float:cache_get_row_float(row, idx, connectionHandle = 1);
cell AMX_NATIVE_CALL Native::cache_get_row_float(AMX* amx, cell* params) {
	unsigned int cID = params[3];
	
	if(CLog::Get()->IsLogLevel(LOG_DEBUG)) {
		char LogBuf[32];
		sprintf(LogBuf, "connection handle: %d", cID);
		CLog::Get()->LogFunction(LOG_DEBUG, "cache_get_row_float", LogBuf);
	}

	if(!CMySQLHandle::IsValid(cID)) {
		ERROR_INVALID_CONNECTION_HANDLE("cache_get_row_float", cID);
		return 0;
	}
	
	
	float ReturnVal = 0.0f;
	CMySQLResult *Result = CMySQLHandle::GetHandle(cID)->GetResult();
	if(Result == NULL) {
		CLog::Get()->LogFunction(LOG_WARNING, "cache_get_row_float", "no active cache");
		return amx_ftoc(ReturnVal);
	}

	//string RowData;
	//char RowData[84];
	char *RowData = NULL;
	
	if(Result->GetRowData(params[1], params[2], &RowData) != TYPE_FLOAT/* ||sscanf(RowData, "%f", &ReturnVal) != 1*/) {
		CLog::Get()->LogFunction(LOG_ERROR, "cache_get_row_float", "invalid data type");
		ReturnVal = 0.0f;
	}
	else
		ReturnVal = boost::lexical_cast<float>(RowData);
	
	return amx_ftoc(ReturnVal);
}

// native cache_get_field_content(row, const field_name[], dest[], connectionHandle = 1);
cell AMX_NATIVE_CALL Native::cache_get_field_content(AMX* amx, cell* params) {
	unsigned int cID = params[4];
	
	if(CLog::Get()->IsLogLevel(LOG_DEBUG)) {
		char LogBuf[32];
		sprintf(LogBuf, "connection handle: %d", cID);
		CLog::Get()->LogFunction(LOG_DEBUG, "cache_get_field_content", LogBuf);
	}

	if(!CMySQLHandle::IsValid(cID)) {
		ERROR_INVALID_CONNECTION_HANDLE("cache_get_field_content", cID);
		return 0;
	}
	
	CMySQLResult *Result = CMySQLHandle::GetHandle(cID)->GetResult();
	if(Result == NULL) {
		CLog::Get()->LogFunction(LOG_WARNING, "cache_get_field_content", "no active cache");
		return 0;
	}
	
	//string 
		//FieldName,
		//FieldData;
	char *FieldName = NULL;
	//char *FieldData = (char *)malloc((params[5]+1) * sizeof(char));
	char *FieldData = NULL;
	//char FieldData[256];
	
	//AMX_H->GetString(amx, params[2], FieldName);
	amx_StrParam(amx, params[2], FieldName);
	Result->GetRowDataByName(params[1], FieldName, &FieldData);
	if(FieldData == NULL) {
		FieldData = (char *)malloc(5 * sizeof(char));
		strcpy(FieldData, "NULL");
		AMX_H->SetCString(amx, params[3], FieldData, params[5]);
		free(FieldData);
	}
	else
		AMX_H->SetCString(amx, params[3], FieldData, params[5]);
	
	return 1;
}

// native cache_get_field_content_int(row, const field_name[], connectionHandle = 1);
cell AMX_NATIVE_CALL Native::cache_get_field_content_int(AMX* amx, cell* params) {
	unsigned int cID = params[3];
	
	if(CLog::Get()->IsLogLevel(LOG_DEBUG)) {
		char LogBuf[32];
		sprintf(LogBuf, "connection handle: %d", cID);
		CLog::Get()->LogFunction(LOG_DEBUG, "cache_get_field_content_int", LogBuf);
	}

	if(!CMySQLHandle::IsValid(cID)) {
		ERROR_INVALID_CONNECTION_HANDLE("cache_get_field_content_int", cID);
		return 0;
	}
	
	int ReturnVal = 0;
	CMySQLResult *Result = CMySQLHandle::GetHandle(cID)->GetResult();

	if(Result == NULL) {
		CLog::Get()->LogFunction(LOG_WARNING, "cache_get_field_content_int", "no active cache");
		return amx_ftoc(ReturnVal);
	}
	

	//string 
		//FieldName,
		//FieldData;
	char *FieldName = NULL;
	char *FieldData = NULL;
	//char FieldData[12];

	//AMX_H->GetString(amx, params[2], FieldName);
	amx_StrParam(amx, params[2], FieldName);


	if(Result->GetRowDataByName(params[1], FieldName, &FieldData) != TYPE_INT/* || sscanf(FieldData, "%d", &ReturnVal) != 1*/) {
		CLog::Get()->LogFunction(LOG_ERROR, "cache_get_field_content_int", "invalid data type");
		ReturnVal = 0;
	}
	else
		ReturnVal = boost::lexical_cast<int>(FieldData);
	return ReturnVal;
}

// native Float:cache_get_field_content_float(row, const field_name[], connectionHandle = 1);
cell AMX_NATIVE_CALL Native::cache_get_field_content_float(AMX* amx, cell* params) {
	unsigned int cID = params[3];
	
	if(CLog::Get()->IsLogLevel(LOG_DEBUG)) {
		char LogBuf[32];
		sprintf(LogBuf, "connection handle: %d", cID);
		CLog::Get()->LogFunction(LOG_DEBUG, "cache_get_field_content_float", LogBuf);
	}
	if(!CMySQLHandle::IsValid(cID)) {
		ERROR_INVALID_CONNECTION_HANDLE("cache_get_field_content_float", cID);
		return 0;
	}
	
	float ReturnVal = 0.0f;
	CMySQLResult *Result = CMySQLHandle::GetHandle(cID)->GetResult();

	if(Result == NULL) {
		CLog::Get()->LogFunction(LOG_WARNING, "cache_get_field_content_float", "no active cache");
		return amx_ftoc(ReturnVal);
	}
	

	//string 
		//FieldName,
		//FieldData;
	char *FieldName = NULL;
	char *FieldData = NULL;
	//char FieldData[84];

	//AMX_H->GetString(amx, params[2], FieldName);
	amx_StrParam(amx, params[2], FieldName);

	if(Result->GetRowDataByName(params[1], FieldName, &FieldData) != TYPE_FLOAT/* || sscanf(FieldData, "%f", &ReturnVal) != 1*/) {
		CLog::Get()->LogFunction(LOG_ERROR, "cache_get_field_content_float", "invalid data type");
		ReturnVal = 0.0f;
	}
	else
		ReturnVal = boost::lexical_cast<float>(FieldData);
	return amx_ftoc(ReturnVal);
}

//native mysql_connect(const host[], const user[], const database[], const password[], port = 3306);
cell AMX_NATIVE_CALL Native::mysql_connect(AMX* amx, cell* params) {
	bool match = false;
	string 
		host, user, db, pass;
	unsigned int 
		port = params[5];

	AMX_H->GetString(amx, params[1], host);
	AMX_H->GetString(amx, params[2], user);
	AMX_H->GetString(amx, params[3], db);
	AMX_H->GetString(amx, params[4], pass);
	
	CLog::Get()->LogFunction(LOG_DEBUG, "mysql_connect", "-");

	int CID = CMySQLHandle::Create(host, user, pass, db, port);

	CMySQLQuery::PushConnect(CMySQLHandle::GetHandle(CID));
	while(CMySQLHandle::GetHandle(CID)->GetMySQLPointer() == NULL) { }

	return (cell)CID;
}

//native mysql_reconnect(connectionHandle = 1);
cell AMX_NATIVE_CALL Native::mysql_reconnect(AMX* amx, cell* params) {
	unsigned int cID = params[1];
	
	if(CLog::Get()->IsLogLevel(LOG_DEBUG)) {
		char LogBuf[32];
		sprintf(LogBuf, "connection handle: %d", cID);
		CLog::Get()->LogFunction(LOG_DEBUG, "mysql_reconnect", LogBuf);
	}

	if(!CMySQLHandle::IsValid(cID)) {
		ERROR_INVALID_CONNECTION_HANDLE("mysql_reconnect", cID);
		return 0;
	}
	
	CMySQLQuery::PushReconnect(CMySQLHandle::GetHandle(cID));
	return 1;
}

//native mysql_tquery(conhandle, query[], callback[], format[], {Float,_}:...);
cell AMX_NATIVE_CALL Native::mysql_tquery(AMX* amx, cell* params) {
	static const int ConstParamCount = 4;
	unsigned int cID = params[1];
	
	if(CLog::Get()->IsLogLevel(LOG_DEBUG)) {
		char LogBuf[32];
		sprintf(LogBuf, "connection handle: %d", cID);
		CLog::Get()->LogFunction(LOG_DEBUG, "mysql_tquery", LogBuf);
	}

	if(!CMySQLHandle::IsValid(cID)) {
		ERROR_INVALID_CONNECTION_HANDLE("mysql_tquery", cID);
		return 0;
	}

	string ParamFormat;
	AMX_H->GetString(amx, params[4], ParamFormat);


	if(ParamFormat.length() != ( (params[0]/4) - ConstParamCount ))
		return CLog::Get()->LogFunction(LOG_ERROR, "mysql_tquery", "callback parameter count does not match format specifier length"), 0;

	CMySQLHandle *cHandle = CMySQLHandle::GetHandle(cID);
	CMySQLQuery *Query = new CMySQLQuery;
	CCallback *Callback = new CCallback;
	
	AMX_H->GetString(amx, params[2], Query->Query);
	AMX_H->GetString(amx, params[3], Callback->Name);
	
	Callback->ParamFormat = ParamFormat;
	Query->ConnHandle = cHandle; 
	Query->ConnPtr = cHandle->GetMySQLPointer();
	Query->Callback = Callback;
	
	
	int idx = 1;
	char *szArg;
	cell *AddressPtr;
	
	for(string::iterator c = Callback->ParamFormat.begin(), end = Callback->ParamFormat.end(); c != end; ++c) {
		if ( (*c) == 'd' || (*c) == 'i') {
			amx_GetAddr(amx, params[ConstParamCount + idx], &AddressPtr);
			szArg = (char*)malloc(sizeof(char) * 12); // strlen of (-2^31) + '\0'
			if(szArg != NULL)
				itoa(*AddressPtr, szArg, 10);
		} else if ( (*c) == 's' || (*c) == 'z') {
			AMX_H->GetCString(amx, params[ConstParamCount + idx], szArg);
		} else if ( (*c) == 'f') {
			amx_GetAddr(amx, params[ConstParamCount + idx], &AddressPtr);
			float pFloat = amx_ctof(*AddressPtr);
			szArg = (char*)malloc(sizeof(char) * 84); // strlen of (2^(2^7)) + '\0'
			if(szArg != NULL)
				sprintf(szArg, "%f", pFloat);
		} else {
			szArg = (char*)malloc(sizeof(char) * 5); // "NULL" + '\0'
			if(szArg != NULL)
				strcpy(szArg, "NULL"); // Avoids crashes caused by invalid formatting characters.
		}
		Callback->Parameters.push(string(szArg));
		free(szArg);
		
		idx++;
	}
	

	CMySQLQuery::PushQuery(Query); 
	return 1;
}

//native mysql_function_query(conhandle, query[], bool:cache, callback[], format[], {Float,_}:...);
cell AMX_NATIVE_CALL Native::mysql_function_query(AMX* amx, cell* params) {
	cell *NewParams = new cell[params[0]/4];
	NewParams[0] = params[0]-4;
	NewParams[1] = params[1];
	NewParams[2] = params[2];
	NewParams[3] = params[4];
	NewParams[4] = params[5];
	for(int i=6; i <= (params[0]/4); ++i)
		NewParams[i-1] = params[i];
	mysql_tquery(amx, NewParams);
	delete NewParams;
	return 1;
}

// native mysql_format(connectionHandle, output[], len, format[], {Float,_}:...);
cell AMX_NATIVE_CALL Native::mysql_format(AMX* amx, cell* params) {
	unsigned int cID = params[1];
	
	if(CLog::Get()->IsLogLevel(LOG_DEBUG)) {
		char LogBuf[32];
		sprintf(LogBuf, "connection handle: %d", cID);
		CLog::Get()->LogFunction(LOG_DEBUG, "mysql_format", LogBuf);
	}

	if(!CMySQLHandle::IsValid(cID)) {
		ERROR_INVALID_CONNECTION_HANDLE("mysql_format", cID);
		return 0;
	}
	
	MYSQL *ConnPtr = CMySQLHandle::GetHandle(cID)->GetMySQLPointer();

	cell *sPtr;
	float fData;
	char 
		*szFormat = NULL, 
		*szParam = NULL, 
		*szResult = NULL;
	AMX_H->GetCString(amx, params[4], szFormat);
	//amx_StrParam(amx, params[4], szFormat);
	unsigned int iParam = 5, iArgs = (params[0] / sizeof(cell)) - 4, precision = NULL;
	char format_data[20];
	memset(format_data, '\0', 20);
	for (unsigned int i = 0, l = strlen(szFormat); i < l; i++) {
		char num_str[4];
		char cChar = szFormat[i];
		if (cChar != '%' || (iParam - 5) >= iArgs) {
			continue;
		}
		if (szFormat[i + 1] == '.') {
			int a = 0;
			while ((szFormat[i + 2 + a] >= '0' && szFormat[i + 2 + a] <= '9') && a <= 3) {
				num_str[a] = szFormat[i + 2 + a];
				a++;
			}
			num_str[a] = '\0';
			precision = atoi(num_str);
			cChar = (char)szFormat[i + 2 + strlen(num_str)];
			i++;
		} else {
			cChar = (char)szFormat[++i];
			precision = NULL;
		}
		switch (cChar) {
			case 'i':
			case 'd':
				char number[32];
				unsigned int numLen;
				amx_GetAddr(amx, params[iParam], &sPtr);
				memset(number, '\0', 32);
				itoa(*sPtr, number, 10);
				numLen = strlen(number);
				if (precision > numLen) {
					szParam = (char*)malloc(numLen + (precision - numLen) + 1);
					memset(szParam, '0', (precision - numLen)); // Appending '0' x times.
					memcpy(szParam + (precision - numLen), number, numLen);
					szParam[numLen + (precision - numLen)] = '\0';
				} else {
					szParam = (char*)malloc(numLen + 1);
					strcpy(szParam, number);
					szParam[numLen] = '\0';
				}
				goto alloc_format;
			case 'z':
			case 's':
				AMX_H->GetCString(amx, params[iParam], szParam);
				goto alloc_format;
			case 'f':
				int float_val, float_len;
				float_len = 2;
				amx_GetAddr(amx, params[iParam], &sPtr);
				fData = amx_ctof(*sPtr);
				float_val = (int)floor(fData);
				while ((float_val /= 10) > 0) {
					float_len++;
				}
				if (precision != NULL && precision <= 6) {
					memcpy(format_data, szFormat + (i - 1), strlen(num_str) + 3); // Precision length + '%' + '.' + 'f'
				} else {
					memcpy(format_data, "%f", 2);
				}
				szParam = (char*)malloc((precision != NULL ? (float_len + precision) : (float_len + 6)) + 1);
				sprintf(szParam, format_data, fData);
				goto alloc_format;
			case 'e': {
				char szBuffer[8192];
				memset(szBuffer, '\0', 8192);
				
				string Source;
				AMX_H->GetString(amx, params[iParam], Source);
				mysql_real_escape_string(ConnPtr, szBuffer, Source.c_str(), Source.length());
				
				szParam = (char*)malloc(strlen(szBuffer) + 1);
				strcpy(szParam, szBuffer);
				goto alloc_format;
			}
			case 'x':
				char hex[16];
				amx_GetAddr(amx, params[iParam], &sPtr);
				memset(hex, '\0', 16);
				//if(*sPtr < 0) *sPtr = -(*sPtr); // TODO: this?!
				itoa(*sPtr, hex, 16);
				szParam = (char*)malloc(strlen(hex) + 1);
				strcpy(szParam, hex);
				goto alloc_format;
			case 'b':
				char binary[32];
				amx_GetAddr(amx, params[iParam], &sPtr);
				memset(binary, '\0', 32);
				itoa(*sPtr, binary, 2);
				szParam = (char*)malloc(strlen(binary) + 1);
				strcpy(szParam, binary);
				goto alloc_format;
			default:
				continue;
		}
alloc_format:
		unsigned int iFormatLen = strlen(szFormat), iParamLen = strlen(szParam);
		szResult = (char*)malloc(iFormatLen + iParamLen + 1);
		memcpy(szResult, szFormat, (i - 1));
		if (precision != NULL && is_string_char(cChar) && precision <= iParamLen) {
			memcpy(szResult + (i - 1), szParam, precision);
		} else {
			memcpy(szResult + (i - 1), szParam, iParamLen);
		}
		if (precision != NULL && is_string_char(cChar)) {
			if(iParamLen < precision)
				memset(szResult + (i - 1) + iParamLen, ' ', precision - iParamLen);
			memcpy(szResult + (i - 1) + precision, szFormat + strlen(num_str) + i + 2, iFormatLen - i);
		} else if (precision != NULL && (cChar == 'f' || cChar == 'd')) {
			memcpy(szResult + (i - 1) + iParamLen, szFormat + strlen(num_str) + i + 2, iFormatLen - i);
		} else {
			memcpy(szResult + (i - 1) + iParamLen, szFormat + i + 1, iFormatLen - i);
		}
		free(szFormat);
		free(szParam);
		szFormat = (char*)malloc(strlen(szResult) + 1);
		strcpy(szFormat, szResult);
		free(szResult);
		l = strlen(szFormat);
		szFormat[l] = '\0';
		iParam++;
	}
	
	AMX_H->SetCString(amx, params[2], szFormat, params[3]);
	free(szFormat);
	return 1;
}

//native mysql_set_charset(charset[], connectionHandle = 1);
cell AMX_NATIVE_CALL Native::mysql_set_charset(AMX* amx, cell* params) {
	unsigned int cID = params[2];
	
	if(CLog::Get()->IsLogLevel(LOG_DEBUG)) {
		char LogBuf[32];
		sprintf(LogBuf, "connection handle: %d", cID);
		CLog::Get()->LogFunction(LOG_DEBUG, "mysql_set_charset", LogBuf);
	}

	if(!CMySQLHandle::IsValid(cID)) {
		ERROR_INVALID_CONNECTION_HANDLE("mysql_set_charset", cID);
		return 0;
	}

	MYSQL *ConnPtr = CMySQLHandle::GetHandle(cID)->GetMySQLPointer();
	string CharSet;
	AMX_H->GetString(amx, params[1], CharSet);
	mysql_set_character_set(ConnPtr, CharSet.c_str());
	
	return 1;
}

//native mysql_get_charset(destination[], connectionHandle = 1, max_len=sizeof(destination));
cell AMX_NATIVE_CALL Native::mysql_get_charset(AMX* amx, cell* params) {
	unsigned int cID = params[2];
	
	if(CLog::Get()->IsLogLevel(LOG_DEBUG)) {
		char LogBuf[32];
		sprintf(LogBuf, "connection handle: %d", cID);
		CLog::Get()->LogFunction(LOG_DEBUG, "mysql_get_charset", LogBuf);
	}

	if(!CMySQLHandle::IsValid(cID)) {
		ERROR_INVALID_CONNECTION_HANDLE("mysql_get_charset", cID);
		return 0;
	}

	MYSQL *ConnPtr = CMySQLHandle::GetHandle(cID)->GetMySQLPointer();
	string CharSet = mysql_character_set_name(ConnPtr);
	AMX_H->SetString(amx, params[1], CharSet, params[3]);
	
	return 1;
}

//native mysql_real_escape_string(const source[], destination[], connectionHandle = 1, max_len=sizeof(destination));
cell AMX_NATIVE_CALL Native::mysql_real_escape_string(AMX* amx, cell* params) {
	unsigned int cID = params[3];
	
	if(CLog::Get()->IsLogLevel(LOG_DEBUG)) {
		char LogBuf[32];
		sprintf(LogBuf, "connection handle: %d", cID);
		CLog::Get()->LogFunction(LOG_DEBUG, "mysql_real_escape_string", LogBuf);
	}

	if(!CMySQLHandle::IsValid(cID)) {
		ERROR_INVALID_CONNECTION_HANDLE("mysql_real_escape_string", cID);
		return 0;
	}
	
	MYSQL *ConnPtr = CMySQLHandle::GetHandle(cID)->GetMySQLPointer();
	size_t DestLength = (params[4] <= 0 ? 8192 : params[4]);

	string Source;
	AMX_H->GetString(amx, params[1], Source);
	char *StrBuffer = new char[Source.length()*2+1];
	memset(StrBuffer, '\0', Source.length()*2 + 1);

	cell StringLen = (cell)mysql_real_escape_string(ConnPtr, StrBuffer, Source.c_str(), Source.length());
	string EscapedString(StrBuffer);
	AMX_H->SetString(amx, params[2], EscapedString, params[4]);
	
	delete[] StrBuffer; 
	return StringLen; 
}

cell AMX_NATIVE_CALL Native::mysql_close(AMX* amx, cell* params) {
	unsigned int cID = params[1];
	
	if(CLog::Get()->IsLogLevel(LOG_DEBUG)) {
		char LogBuf[32];
		sprintf(LogBuf, "connection handle: %d", cID);
		CLog::Get()->LogFunction(LOG_DEBUG, "mysql_close", LogBuf);
	}

	if(!CMySQLHandle::IsValid(cID)) {
		ERROR_INVALID_CONNECTION_HANDLE("mysql_close", cID);
		return 0;
	}
	
	CMySQLQuery::PushDisconnect(CMySQLHandle::GetHandle(cID));
	return 1;
}

//native mysql_stat(destination[], connectionHandle = 1, max_len=sizeof(destination));
cell AMX_NATIVE_CALL Native::mysql_stat(AMX* amx, cell* params) {
	unsigned int cID = params[2];
	
	if(CLog::Get()->IsLogLevel(LOG_DEBUG)) {
		char LogBuf[32];
		sprintf(LogBuf, "connection handle: %d", cID);
		CLog::Get()->LogFunction(LOG_DEBUG, "mysql_stat", LogBuf);
	}

	if(!CMySQLHandle::IsValid(cID)) {
		ERROR_INVALID_CONNECTION_HANDLE("mysql_stat", cID);
		return 0;
	}
	
	MYSQL *ConnPtr = CMySQLHandle::GetHandle(cID)->GetMySQLPointer();
	string Stats = string(mysql_stat(ConnPtr));
	AMX_H->SetString(amx, params[1], Stats, params[3]);
	
	return 1;
}

//native mysql_log(loglevel, logtype);
cell AMX_NATIVE_CALL Native::mysql_log(AMX* amx, cell* params) {
	if(params[1] < 0)
		return 0;

	CLog::Get()->SetLogLevel(params[1]);
	CLog::Get()->SetLogType(params[2]);
	return 1;
}
