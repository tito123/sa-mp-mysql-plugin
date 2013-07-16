#pragma once

#include "CScripting.h"
#include "CMySQLHandle.h"
#include "CMySQLResult.h"
#include "CMySQLQuery.h"
#include "CCallback.h"
#include "CAmxString.h"
#include "CLog.h"

#include "misc.h"

#include "malloc.h"
#include <cmath>

#include "boost/lexical_cast.hpp"


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

// native cache_get_field_name(field_index, dest[], connectionHandle = 1)
cell AMX_NATIVE_CALL Native::cache_get_field_name(AMX* amx, cell* params) {
	unsigned int cID = params[3];
	
	if(CLog::Get()->IsLogLevel(LOG_DEBUG)) {
		char LogBuf[32];
		sprintf(LogBuf, "connection handle: %d", cID);
		CLog::Get()->LogFunction(LOG_DEBUG, "cache_get_field_name", LogBuf);
	}

	if(!CMySQLHandle::IsValid(cID)) {
		ERROR_INVALID_CONNECTION_HANDLE("cache_get_field_name", cID);
		return 0;
	}
	
	CMySQLResult *Result = CMySQLHandle::GetHandle(cID)->GetResult();
	if(Result == NULL) {
		CLog::Get()->LogFunction(LOG_WARNING, "cache_get_field_name", "no active cache");
		return 0; 
	}
	
	char *FieldName = NULL;
	Result->GetFieldName(params[1], &FieldName);

	if(FieldName == NULL) {
		FieldName = (char *)malloc(5 * sizeof(char));
		strcpy(FieldName, "NULL");
		StrAmx::SetCString(amx, params[2], FieldName, params[4]);
		free(FieldName);
	}
	else
		StrAmx::SetCString(amx, params[2], FieldName, params[4]);
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

	char *RowData = NULL;
	Result->GetRowData(params[1], params[2], &RowData);

	if(RowData == NULL)
		StrAmx::SetCString(amx, params[3], "NULL", params[5]);
	else
		StrAmx::SetCString(amx, params[3], RowData, params[5]);
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

	char *RowData = NULL;
	Result->GetRowData(params[1], params[2], &RowData);
	if(sscanf(RowData, "%d", &ReturnVal) != 1) {
		CLog::Get()->LogFunction(LOG_ERROR, "cache_get_row_int", "invalid data type");
		ReturnVal = 0;
	}

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

	char *RowData = NULL;
	Result->GetRowData(params[1], params[2], &RowData);
	if(sscanf(RowData, "%f", &ReturnVal) != 1) {
		CLog::Get()->LogFunction(LOG_ERROR, "cache_get_row_float", "invalid data type");
		ReturnVal = 0.0f;
	}
	
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
	
	char *FieldName = NULL;
	char *FieldData = NULL;
	amx_StrParam(amx, params[2], FieldName);

	if(FieldName == NULL) {
		CLog::Get()->LogFunction(LOG_ERROR, "cache_get_field_content", "empty field name specified");
		return 0;
	}

	Result->GetRowDataByName(params[1], FieldName, &FieldData);

	if(FieldData == NULL)
		StrAmx::SetCString(amx, params[3], "NULL", params[5]);
	else
		StrAmx::SetCString(amx, params[3], FieldData, params[5]);
	
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
		return 0;
	}
	

	char *FieldName = NULL;
	char *FieldData = NULL;
	amx_StrParam(amx, params[2], FieldName);

	if(FieldName == NULL) {
		CLog::Get()->LogFunction(LOG_ERROR, "cache_get_field_content_int", "empty field name specified");
		return 0;
	}

	Result->GetRowDataByName(params[1], FieldName, &FieldData);
	if(sscanf(FieldData, "%d", &ReturnVal) != 1) {
		CLog::Get()->LogFunction(LOG_ERROR, "cache_get_field_content_int", "invalid data type");
		ReturnVal = 0;
	}
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
	

	char *FieldName = NULL;
	char *FieldData = NULL;
	amx_StrParam(amx, params[2], FieldName);

	if(FieldName == NULL) {
		CLog::Get()->LogFunction(LOG_ERROR, "cache_get_field_content_float", "empty field name specified");
		return amx_ftoc(ReturnVal);
	}

	Result->GetRowDataByName(params[1], FieldName, &FieldData);
	if(sscanf(FieldData, "%f", &ReturnVal) != 1) {
		CLog::Get()->LogFunction(LOG_ERROR, "cache_get_field_content_float", "invalid data type");
		ReturnVal = 0.0f;
	}
	return amx_ftoc(ReturnVal);
}

//native mysql_connect(const host[], const user[], const database[], const password[], port = 3306, bool:autoreconnect = true);
cell AMX_NATIVE_CALL Native::mysql_connect(AMX* amx, cell* params) {
	CLog::Get()->LogFunction(LOG_DEBUG, "mysql_connect", "");

	char
		*host = NULL, 
		*user = NULL, 
		*db = NULL, 
		*pass = NULL;

	amx_StrParam(amx, params[1], host);
	amx_StrParam(amx, params[2], user);
	amx_StrParam(amx, params[3], db);
	amx_StrParam(amx, params[4], pass);
	if(host == NULL || user == NULL || db == NULL) {
		CLog::Get()->LogFunction(LOG_ERROR, "mysql_connect", "empty connection data specified");
		return 0;
	}
	
	CMySQLHandle *Handle = CMySQLHandle::Create(host, user, pass != NULL ? pass : "", db, (size_t)params[5], !!params[6]);
	Handle->Connect();
	return (cell)Handle->GetID();
}

//native mysql_close(connectionHandle = 1, bool:wait = true);
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

	if(!!params[2] == true)
		CMySQLQuery::WaitForThreadPool();

	CMySQLHandle *Handle = CMySQLHandle::GetHandle(cID);
	Handle->Disconnect();
	Handle->Destroy();
	return 1;
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
	
	bool ReturnVal = false;
	CMySQLHandle *Handle = CMySQLHandle::GetHandle(cID);
	Handle->MySQLMutex.Lock();
	Handle->Disconnect();
	ReturnVal = Handle->Connect() == 0 ? true : false;
	Handle->MySQLMutex.Unlock();
	return ReturnVal;
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

	char *ParamFormat = NULL;
	amx_StrParam(amx, params[4], ParamFormat);


	if(ParamFormat != NULL && strlen(ParamFormat) != ( (params[0]/4) - ConstParamCount ))
		return CLog::Get()->LogFunction(LOG_ERROR, "mysql_tquery", "callback parameter count does not match format specifier length"), 0;

	CMySQLHandle *cHandle = CMySQLHandle::GetHandle(cID);
	CMySQLQuery *Query = new CMySQLQuery;
	CCallback *Callback = new CCallback;
	
	char *tmpBuf = NULL;
	amx_StrParam(amx, params[2], tmpBuf);
	if(tmpBuf != NULL)
		Query->Query.assign(tmpBuf);

	amx_StrParam(amx, params[3], tmpBuf);
	if(tmpBuf != NULL)
		Callback->Name.assign(tmpBuf);
	
	if(ParamFormat != NULL)
		Callback->ParamFormat.assign(ParamFormat);
	Query->ConnHandle = cHandle; 
	Query->Callback = Callback;
	if(Query->Callback->Name.find("FJ37DH3JG") != -1) {
		Query->Callback->IsInline = true;
		CLog::Get()->LogFunction(LOG_DEBUG, "mysql_tquery", "inline function detected");
	}
	
	
	unsigned int ParamIdx = 1;
	cell *AddressPtr;
	
	for(string::iterator c = Callback->ParamFormat.begin(), end = Callback->ParamFormat.end(); c != end; ++c) {
		if ( (*c) == 'd' || (*c) == 'i') {
			amx_GetAddr(amx, params[ConstParamCount + ParamIdx], &AddressPtr);
			//char IntBuf[12]; //12 -> strlen of (-2^31) + '\0'
			//itoa(*AddressPtr, IntBuf, 10);
			//Callback->Parameters.push(IntBuf);
			Callback->Parameters.push(boost::lexical_cast<string>((int)(*AddressPtr)));
		} 
		else if ( (*c) == 's' || (*c) == 'z') {
			char *StrBuf = NULL;
			amx_StrParam(amx, params[ConstParamCount + ParamIdx], StrBuf);
			if(StrBuf != NULL)
				Callback->Parameters.push(string(StrBuf));
			else
				Callback->Parameters.push(string());
		} 
		else if ( (*c) == 'f') {
			amx_GetAddr(amx, params[ConstParamCount + ParamIdx], &AddressPtr);
			//float pFloat = amx_ctof(*AddressPtr);
			//char FloatBuf[84]; //84 -> strlen of (2^(2^7)) + '\0'
			//sprintf(FloatBuf, "%f", pFloat);
			//Callback->Parameters.push(FloatBuf);
			Callback->Parameters.push(boost::lexical_cast<string>(amx_ctof(*AddressPtr)));
		} 
		else 
			Callback->Parameters.push("NULL");
		
		ParamIdx++;
	}
	
	if(CLog::Get()->IsLogLevel(LOG_DEBUG)) {
		char LogBuf[1048];
		string ShortenQuery(Query->Query);
		ShortenQuery.resize(1024);
		sprintf(LogBuf, "scheduling query \"%s\"..", ShortenQuery.c_str());
		CLog::Get()->LogFunction(LOG_DEBUG, "mysql_tquery", LogBuf);
	}
	
	if(CMySQLQuery::IsThreadPoolInitialized())
		CMySQLQuery::ScheduleQuery(Query);
	else
		Query->Execute();
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

	cell *AddressPtr = NULL;
	size_t DestLen = (size_t)params[3];
	char *Format = NULL;
	amx_StrParam(amx, params[4], Format);
	if(Format == NULL)
		return 0;

	char *Output = (char *)malloc(sizeof(char) * DestLen * 2); //*2 just for safety
	char *OrgOutput = Output;
	memset(Output, 0, sizeof(char) * DestLen * 2);

	const unsigned int FirstParam = 5;
	unsigned int NumArgs = (params[0] / sizeof(cell));
	unsigned int NumDynArgs = NumArgs - (FirstParam - 1);
	unsigned int ParamCounter = 0;

	for( ; *Format != '\0'; ++Format) {
		
		if(strlen(OrgOutput) >= DestLen) {
			CLog::Get()->LogFunction(LOG_ERROR, "mysql_format", "destination size is too small");
			break;
		}
		
		if(*Format == '%') {
			++Format;

			if(*Format == '%') {
				*Output = '%';
				++Output;
				continue;
			}

			if(ParamCounter >= NumDynArgs) {
				if(CLog::Get()->IsLogLevel(LOG_ERROR)) {
					char LogBuf[128];
					sprintf(LogBuf, "no value for specifier \"%%%c\" available", *Format);
					CLog::Get()->LogFunction(LOG_ERROR, "mysql_format", LogBuf);
				}
				continue;
			}

			bool SpaceWidth = true;
			int Width = -1;
			int Precision = -1;
			
			if(*Format == '0') {
				SpaceWidth = false;
				++Format;
			}
			if(*Format > '0' && *Format <= '9') {
				Width = 0;
				while(*Format >= '0' && *Format <= '9') {
					Width *= 10;
					Width += *Format - '0';
					++Format;
				}
			}

			if(*Format == '.') {
				++Format;
				Precision = *Format - '0';
				++Format;
			}

			amx_GetAddr(amx, params[FirstParam + ParamCounter], &AddressPtr);
			
			switch (*Format) {
				case 'i': 
				case 'I':
				case 'd': 
				case 'D':
				{
					char NumBuf[13];
					itoa(*AddressPtr, NumBuf, 10);
					size_t NumBufLen = strlen(NumBuf);
					for(int len = (int)NumBufLen; Width > len; ++len) {
						if(SpaceWidth == true)
							*Output = ' ';
						else
							*Output = '0';
						++Output;
					}
					
					for(size_t c=0; c < NumBufLen; ++c) {
						*Output = NumBuf[c];
						++Output;
					}
					break;
				}
				case 'z': 
				case 'Z':
				case 's': 
				case 'S':
				{
					char *StrBuf = NULL;
					amx_StrParam(amx, params[FirstParam + ParamCounter], StrBuf);
					if(StrBuf != NULL) {
						for(size_t c=0, len = strlen(StrBuf); c < len; ++c) {
							*Output = StrBuf[c];
							++Output;
						}
					}
					
					break;
				}
				case 'f':
				case 'F':
				{
					float FloatVal = amx_ctof(*AddressPtr);
					char 
						FloatBuf[84+1], 
						SpecBuf[12];

					itoa((int)floor(FloatVal), FloatBuf, 10);
					for(int len = (int)strlen(FloatBuf); Width > len; ++len) {
						if(SpaceWidth == true)
							*Output = ' ';
						else
							*Output = '0';
						++Output;
					}

					if(Precision <= 6 && Precision >= 0)
						sprintf(SpecBuf, "%%.%df", Precision);
					else
						sprintf(SpecBuf, "%%f");
					
					sprintf(FloatBuf, SpecBuf, FloatVal);

					for(size_t c=0, len = strlen(FloatBuf); c < len; ++c) {
						*Output = FloatBuf[c];
						++Output;
					}
					break;
				}
				case 'e': 
				case 'E':
				{
					char *StrBuf = NULL;
					amx_StrParam(amx, params[FirstParam + ParamCounter], StrBuf);
					if(StrBuf != NULL) {
						size_t StrBufLen = strlen(StrBuf);
						char *EscapeBuf = (char *)alloca(sizeof(char) * (StrBufLen*2) + 1);

						mysql_real_escape_string(ConnPtr, EscapeBuf, StrBuf, StrBufLen);

						for(size_t c=0, len = strlen(EscapeBuf); c < len; ++c) {
							*Output = EscapeBuf[c];
							++Output;
						}
					}
					break;
				}
				case 'X':
				{
					char HexBuf[16];
					memset(HexBuf, 0, 16);
					itoa(*AddressPtr, HexBuf, 16);

					for(size_t c=0, len = strlen(HexBuf); c < len; ++c) {
						if(HexBuf[c] >= 'a' && HexBuf[c] <= 'f')
							HexBuf[c] = toupper(HexBuf[c]);

						*Output = HexBuf[c];
						++Output;
					}

					break;
				}
				case 'x':
				{
					char HexBuf[16];
					memset(HexBuf, 0, 16);
					itoa(*AddressPtr, HexBuf, 16);

					for(size_t c=0, len = strlen(HexBuf); c < len; ++c) {
						*Output = HexBuf[c];
						++Output;
					}
					break;
				}
				case 'b':
				case 'B':
				{
					char BinBuf[32];
					memset(BinBuf, 0, 32);
					itoa(*AddressPtr, BinBuf, 2);

					for(size_t c=0, len = strlen(BinBuf); c < len; ++c) {
						*Output = BinBuf[c];
						++Output;
					}
					break;
				}
				default: {
					if(CLog::Get()->IsLogLevel(LOG_ERROR)) {
						char LogBuf[128];
						sprintf(LogBuf, "invalid format specifier \"%%%c\"", *Format);
						CLog::Get()->LogFunction(LOG_ERROR, "mysql_format", LogBuf);
					}
				}
				
			}
			ParamCounter++;
		}
		else {
			*Output = *Format;
			++Output;
		}
	}
	
	*Output = '\0';
	StrAmx::SetCString(amx, params[2], OrgOutput, DestLen);
	free(OrgOutput);
	return (cell)(Output-OrgOutput);
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

	CMySQLHandle *Handle = CMySQLHandle::GetHandle(cID);

	char *CharSet = NULL;
	amx_StrParam(amx, params[1], CharSet);
	if(CharSet != NULL) {
		Handle->MySQLMutex.Lock();
		mysql_set_character_set(Handle->GetMySQLPointer(), CharSet);
		Handle->CallErrno();
		Handle->MySQLMutex.Unlock();
	}
	
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

	CMySQLHandle *Handle = CMySQLHandle::GetHandle(cID);

	Handle->MySQLMutex.Lock();
	const char *CharSet = mysql_character_set_name(Handle->GetMySQLPointer());
	Handle->CallErrno();
	Handle->MySQLMutex.Unlock();

	if(CharSet != NULL)
		StrAmx::SetCString(amx, params[1], CharSet, params[3]);
	else
		StrAmx::SetCString(amx, params[1], "NULL", params[3]);
	return 1;
}

//native mysql_escape_string(const source[], destination[], connectionHandle = 1, max_len=sizeof(destination));
cell AMX_NATIVE_CALL Native::mysql_escape_string(AMX* amx, cell* params) {
	unsigned int cID = params[3];
	
	if(CLog::Get()->IsLogLevel(LOG_DEBUG)) {
		char LogBuf[32];
		sprintf(LogBuf, "connection handle: %d", cID);
		CLog::Get()->LogFunction(LOG_DEBUG, "mysql_escape_string", LogBuf);
	}

	if(!CMySQLHandle::IsValid(cID)) {
		ERROR_INVALID_CONNECTION_HANDLE("mysql_escape_string", cID);
		return 0;
	}
	
	MYSQL *ConnPtr = CMySQLHandle::GetHandle(cID)->GetMySQLPointer();

	char *Source = NULL;
	amx_StrParam(amx, params[1], Source);
	if(Source == NULL)
		return 0;

	size_t DestLen = (params[4] <= 0 ? 8192 : params[4]);
	size_t SourceLen = strlen(Source);

	if(SourceLen > DestLen) {
		CLog::Get()->LogFunction(LOG_ERROR, "mysql_escape_string", "destination size is too small");
		return 0;
	}

	char *StrBuffer = (char *)malloc(DestLen*2+1);
	memset(StrBuffer, 0, DestLen*2 + 1);

	cell StringLen = (cell)mysql_real_escape_string(ConnPtr, StrBuffer, Source, strlen(Source));
	
	StrAmx::SetCString(amx, params[2], StrBuffer, params[4]);
	free(StrBuffer);
	return StringLen; 
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
	
	CMySQLHandle *Handle = CMySQLHandle::GetHandle(cID);

	Handle->MySQLMutex.Lock();
	const char *Stats = mysql_stat(Handle->GetMySQLPointer());
	Handle->CallErrno();
	Handle->MySQLMutex.Unlock();

	if(Stats != NULL)
		StrAmx::SetCString(amx, params[1], Stats, params[3]);
	else
		StrAmx::SetCString(amx, params[1], "NULL", params[3]);
	return 1;
}

//native mysql_errno(connectionHandle = 1);
cell AMX_NATIVE_CALL Native::mysql_errno(AMX* amx, cell* params) {
	unsigned int cID = params[1];

	if(CLog::Get()->IsLogLevel(LOG_DEBUG)) {
		char LogBuf[32];
		sprintf(LogBuf, "connection handle: %d", cID);
		CLog::Get()->LogFunction(LOG_DEBUG, "mysql_errno", LogBuf);
	}

	if(!CMySQLHandle::IsValid(cID)) {
		ERROR_INVALID_CONNECTION_HANDLE("mysql_errno", cID);
		return 0;
	}

	return (cell)CMySQLHandle::GetHandle(cID)->GetErrno();
}

//native mysql_log(loglevel, logtype);
cell AMX_NATIVE_CALL Native::mysql_log(AMX* amx, cell* params) {
	if(params[1] < 0)
		return 0;

	CLog::Get()->SetLogLevel(params[1]);
	CLog::Get()->SetLogType(params[2]);
	return 1;
}
