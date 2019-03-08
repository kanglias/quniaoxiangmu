
#pragma once

#include ".\includes.h"

using namespace std;

namespace commonused
{
	//  ����Ӳ����Ϣ������GUID
	string genarateGUID();

	// filesystem related
	string getModuleFullPath();
	string getModuleFilePath();
	string getModuleFileName();
	string getModuleFileExt();
	string getFilePath(string fullpath);
	string getFileName(string fullpath);
	string getFileExt(string fullpath);
	string getSystemTempDirectory();
	bool createFile(string file, bool overwrite = false);
	void deleteFile(string file);
	bool createFolder(string folder);
	bool deleteFolder(string folder);
	bool isFileExist(string file);
	bool isFolderExist(string folder);
	string findFirstFileWithSpecifiedExtension(string path, string ext);
	int getFileSize(string file);
	time_t getFileModifiedTime(string file);
	bool moveFile(string src, string dest);
	string getWindowsAppDataFolder();

	// socket related
	string getIPString(int ip);
	unsigned long getIPAddress(string ip);
	string getHostName();
	unsigned long getFirstIP();
	string getFirstIPString();
	void getAllIPs(vector<unsigned long>& ips);
	void getAllIPsString(vector<string>& ipss);
	unsigned long getIPOfURL(string url);
	string getIPStringOfURL(string url);

	// machine related
	string getUniqueIDOfLocalMachine();
	string getFirstMacAddress();
	string getFirstPhysicalDriveID();
	string getCPUID();
	void getAllMacAddresses(vector<string>& macs);
	string getScreenResolution();

	/****************************************************************************
	 *   Unicode��Utf8�����ַ�������ת��
	 *****************************************************************************/
	char* UnicodeToUTF8(const WCHAR* source);
	WCHAR* UTF8ToUnicode(const char* source);

	/****************************************************************************
	 *   Unicode��ANSI�����ַ�����ת��
	 *****************************************************************************/
	char*  UnicodeToAnsi(const WCHAR *source);
	WCHAR* AnsiToUnicode(const char *source);

	// ��ȡϵͳĿ¼����ȡӦ�ó�������Ŀ¼��
	void GetAppDataPath(CString &path);

	//  д��־�ļ���ʱ�䣺�¼�
	void LogFile(char *data);
	//  д��־�ļ���ʱ�䣺�¼�
	void LogFile(WCHAR *data);

	//  ͨ��Mac��ַ�㲥����
	int MacPowerOn(char *macAdress);

	/****************************************************************************
	 *   URL�������뺯�������ڴ��������ַ��ĺ��� URL���룺 
	 *  # ������־�ض����ĵ�λ�� %23 
	 *	% �������ַ����б��� %25 
	 *	& �ָ���ͬ�ı���ֵ�� %26 
	 *	+ �ڱ���ֵ�б�ʾ�ո� %2B 
	 *	\ ��ʾĿ¼·�� %2F 
	 *	= �������Ӽ���ֵ %3D 
	 *	? ��ʾ��ѯ�ַ����Ŀ�ʼ %3F 
	 *   �ں��������б��о������⹦�ܵ������ַ����ַ�������Ϊ������GET��ʽ����ʱ��ֻ��Ҫ��URLENCODE��������һ�¾Ϳ�������
	 *   �������������߿�������HTML TAG���ַ����ľ���ֵֵ�ǣ����ǿ�����HTMLENCODE����һ�¡� 
	 *****************************************************************************/
	string UrlEncode(CString& szToEncode);
	string UrlDecode(CString& szToDecode);
	string UrlStringEncode(const string& szToEncode);
	string UrlStringDecode(const string& szToDecode);

	// process related
	bool invokeProcess(string command_line);
	bool killProcess(string process);
	bool isProcessRunning(string process);
	bool isProcessWindowRunning(string window);

	// time related
	time_t	__get_time();
	char* __format_time(time_t t, const char* format, char *& buf, int max_size);
	time_t __to_datetime(char* buf);
	int __get_year();
	int	__get_month();
	int	__get_day();
	int	__get_hour();
	int	__get_minute();
	int	__get_second();
	int	__get_timediff(time_t start, time_t end);
	string __get_localtime_string(time_t);
	string __get_gmtime_string(time_t t);

	// string related
	char* _strDup(char * pszString);
	int _strGetCharCount(char *pszString, char c);
	char ** _strTokenize(char *pszString, char *pszTokenizer);
	int _strStringsCount(char **ppszStrings);
	void _strFreeStrings(char **ppszStrings);
	bool strTokenize(string source, string tokenizer, vector<string>& v);
	string trim(string& source, char quot);
	string trimLeft(string& source, char quot);
	string trimRight(string& source, char quot);

	// key1=value1;key2=value2...... string related
	bool parseKeyValueString(string source, vector<string>& keys, vector<string>& values);
	bool getKeyValue(string source, string key, string& value);

	// json related
	class JsonPair
	{
	public:
		JsonPair(string n, string v){name = n; value = v;};
		string name;
		string value;
	};

	typedef vector<JsonPair> JsonObject;

	bool parseJsonString(string json_string, JsonObject& json_object);
	string getJsonPairValue(JsonObject& json_object, string pair_name);
	string getJsonPairValue(string json_string, string pair_name);
	bool createJsonString(JsonObject& json_object, string& json_string);
	void eraseJsonPair(JsonObject& json_object, string pair_name);

	// �ַ�������
	string encodeURLToUTF8(char * pIn);

};
