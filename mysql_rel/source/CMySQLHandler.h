#pragma once

#include "../main.h"

struct s_aFormat {
	char* szCallback;
	char* szFormat;
	char* szQuery;
	int bCache;
	std::string arrElements[20];
};

class CMySQLHandler
{
public:
	CMySQLHandler(std::string host, std::string user, std::string passw, std::string db, size_t port);
	~CMySQLHandler();
	int	Ping();
	int RetrieveRow();
	int SetCharset(std::string charsetname);
	int EscapeStr(std::string source, char *to);
	int Reload();
	bool Connect();
	bool Seek(size_t offset);
	bool FreeResult();
	bool StoreResult();
	bool FetchField(std::string column);
	bool m_bIsConnected;
	bool m_bNonThreadedQuery;
	bool m_bQueryProcessing;
	my_ulonglong InsertId();
	my_ulonglong NumRows();
	my_ulonglong AffectedRows();
	void Disconnect();

	unsigned int NumFields();
	unsigned int WarningCount();
	unsigned int FieldCount();

	unsigned int m_dwError, m_dwFields;

	struct errorInfo {
		std::string m_szQuery;
		std::string m_szError;
		int m_uiErrorID;
		std::string m_szCallback;
	};

	// cache variables
	unsigned int m_dwCacheFields;
	my_ulonglong m_dwCacheRows;
	std::vector<std::vector<char*> > m_sCache;
	std::vector<char*> m_szCacheFields;
	std::vector<char*> m_szFields;

	std::queue<s_aFormat> m_sQueryData;
	std::queue<s_aFormat> m_sCallbackData;
	std::queue<errorInfo> errorCallback;

	std::string FetchRow();
	std::string Statistics();
	std::string GetCharset();
	std::string FetchFieldName(int number);
	std::string
			m_Hostname,m_Username,m_Password,m_Database,Delimiter,m_szResult;

	size_t m_iPort;

	std::stringstream m_cSStream;
	MYSQL * m_stConnectionPtr;
	MYSQL_ROW m_stRow;
	MYSQL_RES * m_stResult;
	MYSQL_FIELD * m_stField;
};

extern std::vector<CMySQLHandler *> SQLHandle;
