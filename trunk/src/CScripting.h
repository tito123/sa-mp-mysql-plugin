#pragma once

#ifndef INC_CSCRIPTING_H
#define INC_CSCRIPTING_H
#define is_string_char(c) (c == 'z' || c == 's' || c == 'e')

#include "main.h"
#include <fstream>
#include <ctime>


namespace Native {
	cell AMX_NATIVE_CALL mysql_mt(AMX* amx, cell *params);
	cell AMX_NATIVE_CALL mysql_log(AMX* amx, cell* params);
	cell AMX_NATIVE_CALL mysql_connect(AMX* amx, cell* params);
	cell AMX_NATIVE_CALL mysql_close(AMX* amx, cell* params);
	cell AMX_NATIVE_CALL mysql_reconnect(AMX* amx, cell* params);
	
	cell AMX_NATIVE_CALL mysql_real_escape_string(AMX* amx, cell* params);
	cell AMX_NATIVE_CALL mysql_format(AMX* amx, cell* params);
	cell AMX_NATIVE_CALL mysql_tquery(AMX* amx, cell* params);
	cell AMX_NATIVE_CALL mysql_function_query(AMX* amx, cell* params); //wrapper for mysql_tquery
	
	
	//ASync functions
	cell AMX_NATIVE_CALL mysql_stat(AMX* amx, cell* params);
	cell AMX_NATIVE_CALL mysql_set_charset(AMX* amx, cell* params);
	cell AMX_NATIVE_CALL mysql_get_charset(AMX* amx, cell* params);
	

	//Cache functions
	cell AMX_NATIVE_CALL cache_get_data(AMX* amx, cell* params);
	cell AMX_NATIVE_CALL cache_get_field(AMX* amx, cell* params);
	cell AMX_NATIVE_CALL cache_get_row(AMX* amx, cell* params);
	cell AMX_NATIVE_CALL cache_get_row_int(AMX* amx, cell* params);
	cell AMX_NATIVE_CALL cache_get_row_float(AMX* amx, cell* params);
	cell AMX_NATIVE_CALL cache_get_field_content(AMX* amx, cell* params);
	cell AMX_NATIVE_CALL cache_get_field_content_int(AMX* amx, cell* params);
	cell AMX_NATIVE_CALL cache_get_field_content_float(AMX* amx, cell* params);
	cell AMX_NATIVE_CALL cache_save(AMX* amx, cell* params);
	cell AMX_NATIVE_CALL cache_delete(AMX* amx, cell* params);
	cell AMX_NATIVE_CALL cache_set_active(AMX* amx, cell* params);
	
	cell AMX_NATIVE_CALL cache_affected_rows(AMX* amx, cell* params);
	cell AMX_NATIVE_CALL cache_insert_id(AMX* amx, cell* params);
	cell AMX_NATIVE_CALL cache_warning_count(AMX* amx, cell* params);

	//Other functions
	void Log(unsigned int level, char* text, ...);
};

enum e_LogLevel {
	LOG_NONE = 0,
	LOG_ERROR = 1,
	LOG_WARNING = 2,
	LOG_DEBUG = 4
};


#endif