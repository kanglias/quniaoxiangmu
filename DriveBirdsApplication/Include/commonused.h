
#pragma once

#include ".\includes.h"

using namespace std;

namespace commonused
{
	//  根据硬件信息，生成GUID
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
	 *   Unicode与Utf8编码字符串互相转换
	 *****************************************************************************/
	char* UnicodeToUTF8(const WCHAR* source);
	WCHAR* UTF8ToUnicode(const char* source);

	/****************************************************************************
	 *   Unicode与ANSI编码字符互相转换
	 *****************************************************************************/
	char*  UnicodeToAnsi(const WCHAR *source);
	WCHAR* AnsiToUnicode(const char *source);

	// 获取系统目录（获取应用程序数据目录）
	void GetAppDataPath(CString &path);

	//  写日志文件，时间：事件
	void LogFile(char *data);
	//  写日志文件，时间：事件
	void LogFile(WCHAR *data);

	//  通过Mac地址广播开机
	int MacPowerOn(char *macAdress);

	/****************************************************************************
	 *   URL请求编解码函数，用于处理特殊字符的含义 URL编码： 
	 *  # 用来标志特定的文档位置 %23 
	 *	% 对特殊字符进行编码 %25 
	 *	& 分隔不同的变量值对 %26 
	 *	+ 在变量值中表示空格 %2B 
	 *	\ 表示目录路径 %2F 
	 *	= 用来连接键和值 %3D 
	 *	? 表示查询字符串的开始 %3F 
	 *   在含有以上列表中具有特殊功能的特殊字符的字符串，作为参数用GET方式传递时，只需要用URLENCODE方法处理一下就可以拉。
	 *   如果不想让浏览者看到含后HTML TAG的字符串的具体值值是，我们可以用HTMLENCODE处理一下。 
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

	// 字符串编码
	string encodeURLToUTF8(char * pIn);

};
