#pragma once

#include "main.h"

class Natives {
public:
	static cell AMX_NATIVE_CALL n_mysql_connect(AMX* amx, cell* params);
	static cell AMX_NATIVE_CALL n_mysql_free_result(AMX* amx, cell* params);
	static cell AMX_NATIVE_CALL n_mysql_store_result(AMX* amx, cell* params);
	static cell AMX_NATIVE_CALL n_mysql_real_escape_string(AMX* amx, cell* params);
	static cell AMX_NATIVE_CALL n_mysql_close(AMX* amx, cell* params);
	static cell AMX_NATIVE_CALL n_mysql_field_count(AMX* amx, cell* params);
	static cell AMX_NATIVE_CALL n_mysql_num_rows(AMX* amx, cell* params);
	static cell AMX_NATIVE_CALL n_mysql_fetch_row_format(AMX* amx, cell* params);
	static cell AMX_NATIVE_CALL n_mysql_ping(AMX* amx, cell* params);
	static cell AMX_NATIVE_CALL n_mysql_debug(AMX* amx, cell* params); //deprecated
	static cell AMX_NATIVE_CALL n_mysql_log(AMX* amx, cell* params);
	static cell AMX_NATIVE_CALL n_mysql_fetch_field_row(AMX* amx, cell* params);
	static cell AMX_NATIVE_CALL n_mysql_fetch_field(AMX* amx, cell* params);
	static cell AMX_NATIVE_CALL n_mysql_stat(AMX* amx, cell* params);
	static cell AMX_NATIVE_CALL n_mysql_errno(AMX* amx, cell* params);
	static cell AMX_NATIVE_CALL n_mysql_warning_count(AMX* amx, cell* params);
	static cell AMX_NATIVE_CALL n_mysql_reload(AMX* amx, cell* params);
	static cell AMX_NATIVE_CALL n_mysql_num_fields(AMX* amx, cell* params);
	static cell AMX_NATIVE_CALL n_mysql_affected_rows(AMX* amx, cell* params);
	static cell AMX_NATIVE_CALL n_mysql_insert_id(AMX* amx, cell* params);
	static cell AMX_NATIVE_CALL n_mysql_reconnect(AMX* amx, cell* params);
	static cell AMX_NATIVE_CALL n_mysql_set_charset(AMX* amx, cell* params);
	static cell AMX_NATIVE_CALL n_mysql_get_charset(AMX* amx, cell* params);
	static cell AMX_NATIVE_CALL n_mysql_retrieve_row(AMX* amx, cell* params);
	static cell AMX_NATIVE_CALL n_mysql_query_callback(AMX* amx, cell* params);
	static cell AMX_NATIVE_CALL n_mysql_format(AMX* amx, cell* params);
	static cell AMX_NATIVE_CALL n_mysql_data_seek(AMX* amx, cell* params);
	// Cache functions.
	static cell AMX_NATIVE_CALL n_cache_get_row(AMX* amx, cell* params);
	static cell AMX_NATIVE_CALL n_cache_get_row_int(AMX* amx, cell* params);
	static cell AMX_NATIVE_CALL n_cache_get_row_float(AMX* amx, cell* params);
	static cell AMX_NATIVE_CALL n_cache_get_field(AMX* amx, cell* params);
	static cell AMX_NATIVE_CALL n_cache_get_data(AMX* amx, cell* params);
	static cell AMX_NATIVE_CALL n_cache_get_field_content(AMX* amx, cell* params);
	static cell AMX_NATIVE_CALL n_cache_get_field_content_int(AMX* amx, cell* params);
	static cell AMX_NATIVE_CALL n_cache_get_field_content_float(AMX* amx, cell* params);
	static cell AMX_NATIVE_CALL n_enable_mutex(AMX* amx, cell* params);

	static cell AMX_NATIVE_CALL n_cache_store(AMX* amx, cell* params);
	static cell AMX_NATIVE_CALL n_cache_free(AMX* amx, cell* params);
	static cell AMX_NATIVE_CALL n_cache_set_active(AMX* amx, cell* params);
	// Other functions.
	static void Log(unsigned int level, char* text, ...);
	static Natives* getInstance();
	~Natives();
private:
	unsigned int m_DebugLevel;

	static Natives* m_pInstance;
	Natives();
};

enum e_LogLevel {
	LOG_NONE = 0,
	LOG_ERROR = 1,
	LOG_WARNING = 2,
	LOG_DEBUG = 4
};

static const char DebugFileName[32] = "mysql_log.txt";