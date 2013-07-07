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

	char *RowData = NULL;
	
	if(Result->GetRowData(params[1], params[2], &RowData) != TYPE_INT) {
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

	char *RowData = NULL;
	
	if(Result->GetRowData(params[1], params[2], &RowData) != TYPE_FLOAT) {
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
	
	char *FieldName = NULL;
	char *FieldData = NULL;
	
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
	

	char *FieldName = NULL;
	char *FieldData = NULL;

	amx_StrParam(amx, params[2], FieldName);


	if(Result->GetRowDataByName(params[1], FieldName, &FieldData) != TYPE_INT) {
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
	

	char *FieldName = NULL;
	char *FieldData = NULL;

	amx_StrParam(amx, params[2], FieldName);

	if(Result->GetRowDataByName(params[1], FieldName, &FieldData) != TYPE_FLOAT) {
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
	if(Query->Callback->Name.find("FJ37DH3JG") != -1) {
		Query->Callback->IsInline = true;
		CLog::Get()->LogFunction(LOG_DEBUG, "mysql_tquery", "inline function detected");
	}
	
	
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
	
	if(CLog::Get()->IsLogLevel(LOG_DEBUG)) {
		char LogBuf[512];
		sprintf(LogBuf, "pushing query \"%s\"..", Query->Query.c_str());
		CLog::Get()->LogFunction(LOG_DEBUG, "mysql_tquery", LogBuf);
	}
	
	CMySQLQuery::PushQuery(Query);
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

	char *Output = (char *)malloc(sizeof(char) * DestLen * 2); //*2 just for safety
	char *OrgOutput = Output;
	memset(Output, 0, sizeof(char) * DestLen * 2);

	const unsigned int FirstParam = 5;
	unsigned int NumArgs = (params[0] / sizeof(cell))-4; //-4 because of params[0]
	unsigned int ParamCounter = 0;
	

	for( ; *Format != '\0'; ++Format) {
		
		if(strlen(OrgOutput) >= DestLen) {
			CLog::Get()->LogFunction(LOG_ERROR, "mysql_format", "destination length is too low");
			break;
		}
		
		if(*Format == '%') {
			++Format;


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

			//printf("\nSpaceWidth: %s\nWidth: %d\nPrecision: %d\n", SpaceWidth == true ? "yes" : "no", Width, Precision);

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
					for(size_t c=0, len = strlen(StrBuf); c < len; ++c) {
						*Output = StrBuf[c];
						++Output;
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
					
					sprintf(Output, FloatBuf);
					Output += strlen(FloatBuf);
					break;
				}
				case 'e': 
				case 'E':
				{
					char *StrBuf = NULL;
					amx_StrParam(amx, params[FirstParam + ParamCounter], StrBuf);

					size_t StrBufLen = strlen(StrBuf);
					char *EscapeBuf = (char *)alloca(sizeof(char) * (StrBufLen*2) + 1);

					mysql_real_escape_string(ConnPtr, EscapeBuf, StrBuf, StrBufLen);

					sprintf(Output, EscapeBuf);
					Output += strlen(EscapeBuf);

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

					sprintf(Output, HexBuf);
					Output += strlen(HexBuf);

					break;
				}
				case 'b':
				case 'B':
				{
					char BinBuf[32];
					memset(BinBuf, 0, 32);
					itoa(*AddressPtr, BinBuf, 2);

					sprintf(Output, BinBuf);
					Output += strlen(BinBuf);

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
	AMX_H->SetCString(amx, params[2], OrgOutput, DestLen);
	free(OrgOutput);
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

//native mysql_escape_string(const source[], destination[], connectionHandle = 1, max_len=sizeof(destination));
cell AMX_NATIVE_CALL Native::mysql_escape_string(AMX* amx, cell* params) {
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

	char *Source = NULL;
	amx_StrParam(amx, params[1], Source);

	char *StrBuffer = (char *)malloc(DestLength*2+1);
	memset(StrBuffer, 0, DestLength*2 + 1);

	cell StringLen = (cell)mysql_real_escape_string(ConnPtr, StrBuffer, Source, strlen(Source));
	
	AMX_H->SetCString(amx, params[2], StrBuffer, params[4]);
	free(StrBuffer);
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
