#pragma once

#include "CScripting.h"
#include "../misc.h"

vector<CMySQLHandler*> SQLHandle;
StrAmx *AMX_H;

logprintf_t logprintf;
unsigned int ID = 0;
bool Debugging = true;
extern list<AMX *> p_Amx;
Natives* Natives::m_pInstance = NULL;

Natives::Natives() 
{
	// constructor
}

Natives::~Natives() 
{
	// deconstructor
}

Natives* Natives::getInstance() 
{
	// based on the singleton structure
	if(m_pInstance == NULL) {
		m_pInstance = new Natives();
	}
	return m_pInstance;
}

// native enable_mutex(bool:enable); is disabled by default
cell AMX_NATIVE_CALL Natives::n_enable_mutex( AMX* amx, cell* params )
{
	if(params[1])
		Mutex::getInstance()->m_gEnable = true;
	else
		Mutex::getInstance()->m_gEnable = false;
	Debug(">> enable_mutex()");
	Debug(">> The mutex method has been %s", (params[1] != 0 ? "enabled" : "disabled"));
	return 1;
}

// native cache_get_data(&num_rows, &num_fields, connectionHandle = 1);
cell AMX_NATIVE_CALL Natives::n_cache_get_data( AMX* amx, cell* params )
{
	unsigned int cID = params[3]-1;
	Mutex::getInstance()->_lockMutex();
	Debug(">> cache_get_data( Connection handle: %d )", cID+1);
	VALID_CONNECTION_HANDLE("cache_get_data", cID);
	CMySQLHandler *cHandle = SQLHandle[cID];
	cell *ptr;
	amx_GetAddr(amx, params[1], &ptr);
	*ptr = (cell)cHandle->m_dwCacheRows;
	amx_GetAddr(amx, params[2], &ptr);
	*ptr = cHandle->m_dwCacheFields;
	Mutex::getInstance()->_unlockMutex();
	return 1;
}

// native cache_get_field(field_index, dest[], connectionHandle = 1)
cell AMX_NATIVE_CALL Natives::n_cache_get_field( AMX* amx, cell* params )
{
	unsigned int cID = params[3]-1;
	Mutex::getInstance()->_lockMutex();
	Debug(">> cache_get_field( Connection handle: %d )", cID+1);
	VALID_CONNECTION_HANDLE("cache_get_field", cID);
	CMySQLHandler *cHandle = SQLHandle[cID];
	unsigned int a = params[1];
	if(a < cHandle->m_szCacheFields.size())
		AMX_H->SetCString(amx, params[2], cHandle->m_szCacheFields[a], params[4]);
	else
		AMX_H->SetCString(amx, params[2], "NULL", params[4]);
	Mutex::getInstance()->_unlockMutex();
	return 1;
}

// native cache_get_row(row, idx, dest[], connectionHandle = 1);
cell AMX_NATIVE_CALL Natives::n_cache_get_row( AMX* amx, cell* params )
{
	unsigned int cID = params[4]-1;
	Mutex::getInstance()->_lockMutex();
	//Debug(">> cache_get_row( Connection handle: %d )", cID+1);
	VALID_CONNECTION_HANDLE("cache_get_row", cID);
	CMySQLHandler *cHandle = SQLHandle[cID];
	unsigned int a = params[1], b = params[2];
	if(a < cHandle->m_sCache.size())
		AMX_H->SetCString(amx, params[3], cHandle->m_sCache[a][b], params[5]);
	else
		AMX_H->SetCString(amx, params[3], "NULL", params[5]);
	Mutex::getInstance()->_unlockMutex();
	return 1;
}

// native cache_get_field_content(row, const field_name[], dest[], connectionHandle = 1);
cell AMX_NATIVE_CALL Natives::n_cache_get_field_content( AMX* amx, cell* params )
{
	unsigned int cID = params[4]-1;
	Mutex::getInstance()->_lockMutex();
	//Debug(">> cache_get_field_content( Field[] = %s )", szField);
	VALID_CONNECTION_HANDLE("cache_get_field_content", cID);
	CMySQLHandler *cHandle = SQLHandle[cID];
	unsigned int a = params[1];
	char* szField;
	AMX_H->GetCString(amx, params[2], szField);
	AMX_H->SetCString(amx, params[3], "NULL");
	if(a < cHandle->m_sCache.size() && cHandle->m_dwCacheFields > 0) {
		for(unsigned int i = 0;i < cHandle->m_dwCacheFields;i++) {
			if(!strcmp(szField, cHandle->m_szCacheFields[i])) {
				AMX_H->SetCString(amx, params[3], cHandle->m_sCache[a][i], params[5]);
				break;
			}
		}
	}
	free(szField);
	Mutex::getInstance()->_unlockMutex();
	return 1;
}

cell AMX_NATIVE_CALL Natives::n_mysql_connect( AMX* amx, cell* params )
{
	bool match = false;
	string 
		host = AMX_H->GetString(amx,params[1]),
		user = AMX_H->GetString(amx,params[2]),
		db = AMX_H->GetString(amx,params[3]),
		pass = AMX_H->GetString(amx,params[4]);
	unsigned int i = 0, port;
	if(params[0]/4 >= 5)
		port = params[5];
	else
		port = 3306;
	Mutex::getInstance()->_lockMutex();
	Debug(">> mysql_connect(%s, %s, %s, ******) on port %d", host.c_str(), user.c_str(), db.c_str(), port);
	if(SQLHandle.size() > 0) {
		while(i != SQLHandle.size()) {
			//this code is used to avoid double instances of the same connection
			//I recommend using mysql_reconnect() to reconnect instead of mysql_connect(), because the plugin saves the data of a connection handle
			if(!SQLHandle[i]->m_Hostname.compare(host) && !SQLHandle[i]->m_Username.compare(user) && !SQLHandle[i]->m_Database.compare(db) && !SQLHandle[i]->m_Password.compare(pass)) {
				SQLHandle[i]->m_bIsConnected = false;
				SQLHandle[i]->Connect();
				match = true;
				break;
			}
			i++;
		}
	}
	if(!match) {
		CMySQLHandler *cHandle = new CMySQLHandler(host, user, pass, db, port);
		SQLHandle.push_back(cHandle);
		ID = (unsigned int)(SQLHandle.size());
		Mutex::getInstance()->_unlockMutex();
		return (cell)ID;
	}
	Mutex::getInstance()->_unlockMutex();
	return (cell)i+1;
}

cell AMX_NATIVE_CALL Natives::n_mysql_data_seek( AMX* amx, cell* params )
{
	unsigned int cID = params[5]-1;
	Mutex::getInstance()->_lockMutex();
	Debug(">> mysql_data_seek( Connection handle: %d )", cID+1);
	VALID_CONNECTION_HANDLE("mysql_data_seek", cID);
	CMySQLHandler *cHandle = SQLHandle[cID];
	cell ret_val = (cell)cHandle->Seek(params[1]);
	Mutex::getInstance()->_unlockMutex();
	return ret_val;
}

cell AMX_NATIVE_CALL Natives::n_mysql_reconnect(AMX* amx, cell* params)
{
	unsigned int cID = params[1]-1;
	Mutex::getInstance()->_lockMutex();
	Debug(">> mysql_reconnect( Connection handle: %d )", cID+1);
	VALID_CONNECTION_HANDLE("mysql_reconnect", cID);
	CMySQLHandler *cHandle = SQLHandle[cID];
	cHandle->Disconnect();
	cHandle->m_bIsConnected = false;
	cHandle->Connect();
	Mutex::getInstance()->_unlockMutex();
	return 1;
}

//mysql_query_callback(conhandle, query[], bool:cache, callback[], format[], {Float,_}:...);

cell AMX_NATIVE_CALL Natives::n_mysql_query_callback( AMX* amx, cell* params )
{
	unsigned int cID = params[1]-1;
	Mutex::getInstance()->_lockMutex();
	Debug(">> mysql_query_callback( Connection handle: %d )", cID+1);
	VALID_CONNECTION_HANDLE("mysql_query_callback", cID);
	CMySQLHandler *cHandle = SQLHandle[cID];
	cell *ptr;
	s_aFormat qData;
	qData.bCache = params[3];
	AMX_H->GetCString(amx, params[2], qData.szQuery);
	AMX_H->GetCString(amx, params[4], qData.szCallback);
	AMX_H->GetCString(amx, params[5], qData.szFormat);
	int iParamCount = 6, idx = 0;
	char *szArg;
	while(*qData.szFormat) {
		if((iParamCount + idx) > params[0] || idx >= 20) break;
		if(*qData.szFormat == 'd' || *qData.szFormat == 'i') {
			amx_GetAddr(amx, params[iParamCount + idx], &ptr);
			szArg = (char*)malloc(sizeof(char)+1);
			itoa(*ptr, szArg, 10);
		} else if(*qData.szFormat == 's' || *qData.szFormat == 'z') {
			AMX_H->GetCString(amx, params[iParamCount + idx], szArg);
		} else if(*qData.szFormat == 'f') {
			amx_GetAddr(amx, params[iParamCount + idx], &ptr);
			float pFloat = amx_ctof(*ptr);
			szArg = (char*)malloc(sizeof(float)*4);
			sprintf(szArg, "%f", pFloat);
		} else {
			szArg = (char*)malloc(sizeof(char)*5);
			strcpy(szArg, "NULL"); // avoid crashing for invalid formatting characters
		}
		qData.arrElements[idx].assign(szArg);
		free(szArg);
		*qData.szFormat++;
		idx++;
	}
	// set the pointer to it's start address
	while(idx > 0) { *qData.szFormat--; idx--; }
	//qData.szFormat = qData.szFormat-idx;
	/*qData.iAddr = (cell*)malloc(i * sizeof(cell));
	memcpy(qData.iAddr, &iArgs, i * sizeof(cell));*/
	cHandle->m_sQueryData.push(qData);
	Mutex::getInstance()->_unlockMutex();
	return 1;
}

cell AMX_NATIVE_CALL Natives::n_mysql_format( AMX* amx, cell* params )
{
	unsigned int cID = params[1]-1;
	Mutex::getInstance()->_lockMutex();
	Debug(">> mysql_format( Connection handle: %d )", cID+1);
	VALID_CONNECTION_HANDLE("mysql_format", cID);
	CMySQLHandler *cHandle = SQLHandle[cID];
	cell *sPtr; float fData; 
	char *szFormat, *szParam, *szResult;
	AMX_H->GetCString(amx, params[3], szFormat);
	unsigned int iParam = 4, iArgs = (params[0]/sizeof(cell))-3, precision = NULL;
	char format_data[20];
	memset(format_data, '\0', 12);
	for(unsigned int i = 0, l = strlen(szFormat);i < l;i++) {
		char num_str[4];
		char cChar = szFormat[i];
		if(cChar != '%' || (iParam-4) >= iArgs)
			continue;
		if(szFormat[i + 1] == '.') {
			//precision = atoi(&szFormat[i + 2]);
			int a = 0;
			while((szFormat[i + 2 + a] >= '0' && szFormat[i + 2 + a] <= '9') && a <= 3) {
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
		switch(cChar) {
			case 'i':
			case 'd':
				char number[32];
				unsigned int numLen;
				amx_GetAddr(amx, params[iParam], &sPtr);
				memset(number, '\0', 32);
				itoa(*sPtr, number, 10);
				numLen = strlen(number);
				if(precision > numLen) {
					szParam = (char*)malloc(numLen+(precision-numLen));
					memset(szParam, '0', (precision-numLen)); // append '0' x times
					memcpy(szParam+(precision-numLen), number, numLen);
					szParam[numLen+(precision-numLen)] = '\0';
				} else {
					szParam = (char*)malloc(numLen+1);
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
				while((float_val /= 10) > 0) 
					float_len++;
				if(precision != NULL && precision <= 6)
					memcpy(format_data, szFormat+(i-1), strlen(num_str)+3); // precision len + '%' + '.' + 'f'
				else
					memcpy(format_data, "%f", 2);
				szParam = (char*)malloc((precision != NULL ? (float_len+precision) : (float_len+6))+1);
				sprintf(szParam, format_data, fData);
				goto alloc_format;
			case 'e':
				char szBuffer[8192];
				memset(szBuffer, '\0', 8192+1);
				cHandle->EscapeStr(AMX_H->GetString(amx, params[iParam]), szBuffer);
				szParam = (char*)malloc(strlen(szBuffer)+1);
				strcpy(szParam, szBuffer);
				goto alloc_format;
			case 'x':
				char hex[16];
				amx_GetAddr(amx, params[iParam], &sPtr);
				memset(hex, '\0', 16);
				itoa(*sPtr, hex, 16);
				szParam = (char*)malloc(strlen(hex)+1);
				strcpy(szParam, hex);
				goto alloc_format;
			case 'b':
				char binary[32];
				amx_GetAddr(amx, params[iParam], &sPtr);
				memset(binary, '\0', 32);
				itoa(*sPtr, binary, 2);
				szParam = (char*)malloc(strlen(binary)+1);
				strcpy(szParam, binary);
				goto alloc_format;
			default:
				continue;
		}
alloc_format:
		if(*szParam != '\0') {
			unsigned int iFormatLen = strlen(szFormat), iParamLen = strlen(szParam);
			szResult = (char*)malloc(iFormatLen+iParamLen+1);
			memcpy(szResult, szFormat, (i-1));
			if(precision != NULL && is_string_char(cChar) && precision <= iParamLen)
				memcpy(szResult+(i-1), szParam, precision);
			else
				memcpy(szResult+(i-1), szParam, iParamLen);
			if(precision != NULL && is_string_char(cChar)) {
				memcpy(szResult+(i-1)+precision, szFormat+strlen(num_str)+i+2, iFormatLen-i);
			} else if(precision != NULL && (cChar == 'f' || cChar == 'd')) {
				memcpy(szResult+(i-1)+iParamLen, szFormat+strlen(num_str)+i+2, iFormatLen-i);
			} else {
				memcpy(szResult+(i-1)+iParamLen, szFormat+i+1, iFormatLen-i);
			}
			free(szFormat);
			free(szParam);
			szFormat = (char*)malloc(strlen(szResult)+1);
			strcpy(szFormat, szResult);
			free(szResult);
			l = strlen(szFormat);
			szFormat[l] = '\0';
			iParam++;
		}
	}
	Mutex::getInstance()->_unlockMutex();
	// native mysql_format(connectionHandle, output[], format[], {Float,_}:...);
	AMX_H->SetCString(amx, params[2], szFormat);
	unsigned int iLen = strlen(szFormat);
	free(szFormat);
	return (cell)iLen;
}


cell AMX_NATIVE_CALL Natives::n_mysql_set_charset( AMX* amx, cell* params )
{
	unsigned int cID = params[2]-1;
	Mutex::getInstance()->_lockMutex();
	Debug(">> mysql_set_charset( Connection handle: %d )", cID+1);
	VALID_CONNECTION_HANDLE("mysql_set_charset", cID);
	CMySQLHandler *cHandle = SQLHandle[cID];
	cHandle->SetCharset(AMX_H->GetString(amx,params[1]));
	Mutex::getInstance()->_unlockMutex();
	return 0;
}

cell AMX_NATIVE_CALL Natives::n_mysql_get_charset( AMX* amx, cell* params )
{
	unsigned int cID = params[2]-1;
	Mutex::getInstance()->_lockMutex();
	Debug(">> mysql_get_charset( Connection handle: %d )", cID+1);
	VALID_CONNECTION_HANDLE("mysql_get_charset", cID);
	CMySQLHandler *cHandle = SQLHandle[cID];
	AMX_H->SetString(amx, params[1], cHandle->GetCharset(), params[3]);
	Mutex::getInstance()->_unlockMutex();
	return 0;
}

cell AMX_NATIVE_CALL Natives::n_mysql_insert_id( AMX* amx,cell* params )
{
	unsigned int cID = params[1]-1;
	Mutex::getInstance()->_lockMutex();
	Debug(">> mysql_insert_id( Connection handle: %d )", cID+1);
	VALID_CONNECTION_HANDLE("mysql_insert_id", cID);
	CMySQLHandler *cHandle = SQLHandle[cID];
	cell ret_val = (cell)cHandle->InsertId();
	Mutex::getInstance()->_unlockMutex();
	return ret_val;
}

cell AMX_NATIVE_CALL Natives::n_mysql_free_result( AMX* amx, cell* params )
{
	unsigned int cID = params[1]-1;
	Mutex::getInstance()->_lockMutex();
	Debug(">> mysql_free_result( Connection handle: %d )", cID+1);
	VALID_CONNECTION_HANDLE("mysql_free_result", cID);
	CMySQLHandler *cHandle = SQLHandle[cID];
	cHandle->FreeResult();
	Mutex::getInstance()->_unlockMutex();
	return 1;
	//return (cell)cHandle->FreeResult();
}

cell AMX_NATIVE_CALL Natives::n_mysql_store_result( AMX* amx, cell* params )
{
	unsigned int cID = params[1]-1;
	Mutex::getInstance()->_lockMutex();
	Debug(">> mysql_store_result( Connection handle: %d )", cID+1);
	VALID_CONNECTION_HANDLE("mysql_store_result", cID);
	CMySQLHandler *cHandle = SQLHandle[cID];
	cHandle->StoreResult();
	Mutex::getInstance()->_unlockMutex();
	return 1;
	//return (cell)cHandle->StoreResult();
}

cell AMX_NATIVE_CALL Natives::n_mysql_real_escape_string( AMX* amx, cell* params )
{
	unsigned int cID = params[3]-1;
	Mutex::getInstance()->_lockMutex();
	Debug(">> mysql_real_escape_string( Connection handle: %d )", cID+1);
	VALID_CONNECTION_HANDLE("mysql_real_escape_string", cID);
	CMySQLHandler *cHandle = SQLHandle[cID];
	char tmp_buffer[8192+1]; // allocate a return string
	memset(tmp_buffer,0,8192+1); // init the return string to 0
	int ret_len = cHandle->EscapeStr(AMX_H->GetString(amx,params[1]), tmp_buffer);
	AMX_H->SetString(amx, params[2], tmp_buffer, params[4]);
	Mutex::getInstance()->_unlockMutex();
	return ret_len;
}

cell AMX_NATIVE_CALL Natives::n_mysql_field_count( AMX* amx, cell* params )
{
	unsigned int cID = params[1]-1;
	Mutex::getInstance()->_lockMutex();
	Debug(">> mysql_field_count( Connection handle: %d )", cID+1);
	VALID_CONNECTION_HANDLE("mysql_field_count", cID);
	CMySQLHandler *cHandle = SQLHandle[cID];
	cell ret_val = cHandle->FieldCount();
	Mutex::getInstance()->_unlockMutex();
	return ret_val;
}

cell AMX_NATIVE_CALL Natives::n_mysql_reload( AMX* amx, cell* params )
{
	unsigned int cID = params[1]-1;
	Debug(">> mysql_reload( Connection handle: %d )", cID+1);
	VALID_CONNECTION_HANDLE("mysql_reload", cID);
	CMySQLHandler *cHandle = SQLHandle[cID];
	return (cell)cHandle->Reload();
}

cell AMX_NATIVE_CALL Natives::n_mysql_close( AMX* amx, cell* params )
{
	unsigned int cID = params[1]-1;
	Mutex::getInstance()->_lockMutex();
	Debug(">> mysql_close( Connection handle: %d )", cID+1);
	VALID_CONNECTION_HANDLE("mysql_close", cID);
	delete SQLHandle[cID]; //delete the class instance and call the deconstructor
	SQLHandle.erase(SQLHandle.begin()+cID);
	Mutex::getInstance()->_unlockMutex();
	return 1;
}

// this function should be updated at some point
cell AMX_NATIVE_CALL Natives::n_mysql_fetch_row_format( AMX* amx, cell* params )
{
	unsigned int cID = params[3]-1;
	Debug(">> mysql_fetch_row_format( Connection handle: %d )", cID+1);
	VALID_CONNECTION_HANDLE("mysql_fetch_row_format", cID);
	CMySQLHandler *cHandle = SQLHandle[cID];
	cHandle->Delimiter = AMX_H->GetString(amx,params[2]);
	string fRow = cHandle->FetchRow();
	if(fRow.compare("NULL") != 0) {
		AMX_H->SetString(amx,params[1], fRow, params[4]);
		cHandle->m_szResult.clear();
		return 1;
	}
	return 0;
}


// this function should be updated at some point
cell AMX_NATIVE_CALL Natives::n_mysql_fetch_field_row( AMX* amx, cell* params )
{
	unsigned int cID = params[3]-1;
	Mutex::getInstance()->_lockMutex();
	Debug(">> mysql_fetch_field_row( Connection handle: %d )", cID+1);
	VALID_CONNECTION_HANDLE("mysql_fetch_field_row", cID);
	CMySQLHandler *cHandle = SQLHandle[cID];
	string szField = AMX_H->GetString(amx, params[2]);
	cHandle->FetchField(szField);
	AMX_H->SetString(amx, params[1], cHandle->m_szResult, params[4]);
	cell ret_val = cHandle->m_szResult.length();
	cHandle->m_szResult.clear();
	Mutex::getInstance()->_unlockMutex();
	return ret_val;
}

cell AMX_NATIVE_CALL Natives::n_mysql_retrieve_row( AMX* amx, cell* params )
{
	unsigned int cID = params[1]-1;
	Mutex::getInstance()->_lockMutex();
	Debug(">> mysql_retrieve_row( Connection handle: %d )", cID+1);
	VALID_CONNECTION_HANDLE("mysql_retrieve_row", cID);
	CMySQLHandler *cHandle = SQLHandle[cID];
	cell ret_val = cHandle->RetrieveRow();
	Mutex::getInstance()->_unlockMutex();
	return ret_val;
	//return (cell)cHandle->RetrieveRow();
}

cell AMX_NATIVE_CALL Natives::n_mysql_ping (AMX* amx, cell* params )
{
	unsigned int cID = params[1]-1;
	Debug(">> mysql_ping( Connection handle: %d )", cID+1);
	VALID_CONNECTION_HANDLE("mysql_ping", cID);
	CMySQLHandler *cHandle = SQLHandle[cID];
	Mutex::getInstance()->_lockMutex();
	#if defined JernejL
		cell ret_val = cHandle->Ping();
	#else
		cell ret_val = ((cHandle->Ping() == 0) ? 1 : (-1));
	#endif
	Mutex::getInstance()->_unlockMutex();
	return ret_val;
}

cell AMX_NATIVE_CALL Natives::n_mysql_num_rows( AMX* amx, cell* params )
{
	unsigned int cID = params[1]-1;
	Mutex::getInstance()->_lockMutex();
	Debug(">> mysql_num_rows( Connection handle: %d )", cID+1);
	VALID_CONNECTION_HANDLE("mysql_num_rows", cID);
	CMySQLHandler *cHandle = SQLHandle[cID];
	cell ret_val = (cell)cHandle->NumRows();
	Mutex::getInstance()->_unlockMutex();
	return ret_val;
}

cell AMX_NATIVE_CALL Natives::n_mysql_num_fields( AMX* amx, cell* params )
{
	unsigned int cID = params[1]-1;
	Mutex::getInstance()->_lockMutex();
	Debug(">> mysql_num_fields( Connection handle: %d )", cID+1);
	VALID_CONNECTION_HANDLE("mysql_num_fields", cID);
	CMySQLHandler *cHandle = SQLHandle[cID];
	cell ret_val = cHandle->NumFields();
	Mutex::getInstance()->_unlockMutex();
	return ret_val;
}

cell AMX_NATIVE_CALL Natives::n_mysql_affected_rows( AMX* amx, cell* params )
{
	unsigned int cID = params[1]-1;
	Mutex::getInstance()->_lockMutex();
	Debug(">> mysql_affected_rows( Connection handle: %d )", cID+1);
	VALID_CONNECTION_HANDLE("mysql_stat", cID);
	CMySQLHandler *cHandle = SQLHandle[cID];
	cell ret_val = (cell)cHandle->AffectedRows();
	Mutex::getInstance()->_unlockMutex();
	return ret_val;
}

cell AMX_NATIVE_CALL Natives::n_mysql_stat( AMX* amx, cell* params )
{
	unsigned int cID = params[2]-1;
	Mutex::getInstance()->_lockMutex();
	Debug(">> mysql_stat( Connection handle: %d )", cID+1);
	VALID_CONNECTION_HANDLE("mysql_stat", cID);
	CMySQLHandler *cHandle = SQLHandle[cID];
	AMX_H->SetString(amx, params[1], cHandle->Statistics(), params[3]);
	Mutex::getInstance()->_unlockMutex();
	return 1;
}

cell AMX_NATIVE_CALL Natives::n_mysql_warning_count( AMX* amx, cell* params )
{
	unsigned int cID = params[1]-1;
	Mutex::getInstance()->_lockMutex();
	Debug(">> mysql_warning_count( Connection handle: %d )", cID+1);
	VALID_CONNECTION_HANDLE("mysql_warning_count", cID);
	CMySQLHandler *cHandle = SQLHandle[cID];
	cell ret_val = cHandle->WarningCount();
	Mutex::getInstance()->_unlockMutex();
	return ret_val;
}

cell AMX_NATIVE_CALL Natives::n_mysql_errno( AMX* amx, cell* params )
{
	unsigned int cID = params[1]-1;
	Mutex::getInstance()->_lockMutex();
	Debug(">> mysql_errno( Connection handle: %d )", cID+1);
	VALID_CONNECTION_HANDLE("mysql_errno", cID);
	CMySQLHandler *cHandle = SQLHandle[cID];
	cell ret_val = cHandle->m_dwError;
	Mutex::getInstance()->_unlockMutex();
	return ret_val;
}

cell AMX_NATIVE_CALL Natives::n_mysql_fetch_field( AMX* amx, cell* params )
{
	unsigned int cID = params[3]-1;
	Mutex::getInstance()->_lockMutex();
	Debug(">> mysql_fetch_field( Connection handle: %d )", cID+1);
	VALID_CONNECTION_HANDLE("mysql_fetch_field", cID);
	CMySQLHandler *cHandle = SQLHandle[cID];
	AMX_H->SetString(amx, params[2], cHandle->FetchFieldName(params[1]), params[4]);
	Mutex::getInstance()->_unlockMutex();
	return 1;
}

cell AMX_NATIVE_CALL Natives::n_mysql_debug( AMX* amx, cell* params )
{
	if(params[1]) {
		time_t rawtime;
		struct tm *timeinfo;
		char timeform[10];
		time(&rawtime);
		timeinfo = localtime(&rawtime);
		strftime(timeform, 10, "%x", timeinfo);
		Debugging = true;
		Debug(" ");
		Debug(" ** MySQL Debugging enabled (%s)", timeform);
		Debug(" ");
	}
	if(!params[1]) {
		Debug(" ");
		Debug(" ** MySQL Debugging disabled");
		Debug(" ");
		Debugging = false;
	}
	return 1;
}

void Natives::Debug(char *text,...)
{
	if(Debugging) {
		/*char buffer[256];
		va_list args;
		va_start(args, text);
		vsprintf(buffer, text, args);
		va_end(args);
		time_t rawtime;
		struct tm *timeinfo;
		char timeform[10];
		time(&rawtime);
		timeinfo = localtime(&rawtime);
		strftime(timeform, 10, "%X", timeinfo);
		int len = strlen(buffer)+strlen(timeform)+5;
		char szData[1024];
		memset(szData, '\0', 1024);
		sprintf(szData, "[%s] %s\n", timeform, buffer);
		FILE *pFile = fopen("mysql_log.txt", "a+");
		fputs (szData,pFile);
		free(szData);
		fclose(pFile);*/
		time_t rawtime;
		struct tm *timeinfo;
		char timeform[10];
		time(&rawtime);
		timeinfo = localtime(&rawtime);
		strftime(timeform, 10, "%X", timeinfo);
		va_list args;
		va_start(args, text);
		string buffer = stringvprintf(text, args);
		va_end(args);
		ofstream logfile;
		logfile.open("mysql_log.txt", std::ios_base::app);
		if (logfile.is_open()) {
			logfile << "[" << timeform << "] " << buffer << "\n";
			buffer.clear();
			logfile.flush();
			logfile.close();
		}
	}
}