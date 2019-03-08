#include "TraceLog.h"


CTraceLog::CTraceLog()
{
	m_hLogFileHandle = INVALID_HANDLE_VALUE;

	m_bScreenColor = true;
	m_nLogLevel = DBG;
	m_logMsgHandler = NULL;
	m_dwCurLogFileSize = 0;
	m_dwMaxBufferLen = MAX_LOG_BUFFER_SIZE;
	m_dwMaxFileLen = MAX_LOG_FILE_SIZE;
	m_uBufferExceed = -1;

	memset(m_szLogPath, NULL, sizeof(m_szLogPath));
	memset(m_szLogFileName, NULL, sizeof(m_szLogFileName));
	::InitializeCriticalSection(&m_csLog);
}

CTraceLog::~CTraceLog()
{
	::DeleteCriticalSection(&m_csLog);
}

bool CTraceLog::InitLog()
{
	if(INVALID_HANDLE_VALUE != m_hLogFileHandle)
		return false;

	if(NULL == _getcwd(m_szLogPath, sizeof(m_szLogPath)))
		return false;

	strcat(m_szLogPath,"\\Logs\\");
	//if((-1 == _mkdir(m_szLogPath)) && (EEXIST != errno))
	//	return false;

	_mkdir(m_szLogPath);

	LogFileOpen();
	return true;
}

void CTraceLog::ExitLog()
{
	LogFileClose();
}

void CTraceLog::SetCallBackMsg(LOG_MSG logMsgHandler)
{
	m_logMsgHandler = logMsgHandler;
}

void CTraceLog::LogFileOpen()
{
    SYSTEMTIME sysTime;
    GetLocalTime(&sysTime);

	sprintf(m_szLogFileName, "%s%04d%02d%02d%02d%02d%02d.log", m_szLogPath, sysTime.wYear,
		sysTime.wMonth, sysTime.wDay, sysTime.wHour, sysTime.wMinute, sysTime.wSecond);

 	if (m_hLogFileHandle != INVALID_HANDLE_VALUE)
		LogFileClose();

   m_hLogFileHandle = ::CreateFile(m_szLogFileName, GENERIC_WRITE,
	    FILE_SHARE_READ | FILE_SHARE_WRITE,
	    0, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);

    if (m_hLogFileHandle != INVALID_HANDLE_VALUE)
	{
		::SetFilePointer(m_hLogFileHandle, NULL, NULL, FILE_END);
		m_dwCurLogFileSize = ::GetFileSize(m_hLogFileHandle, NULL);
	}
}

void CTraceLog::LogFileClose()
{
    if (m_hLogFileHandle != INVALID_HANDLE_VALUE) 
    {
		CloseHandle(m_hLogFileHandle);
		m_hLogFileHandle = INVALID_HANDLE_VALUE;
		m_dwCurLogFileSize = 0;
    }
}

void CTraceLog::LogToScreen(LogLevel nTraceLevel, const char* message)
{
	char strGBK[102400];
	memset(strGBK,0,sizeof(strGBK));
	UTF8ToGBK((unsigned char *)message,(unsigned char *)strGBK,102400);
	if(m_bScreenColor)
	{
		CONSOLE_SCREEN_BUFFER_INFO csbiInfo;
		HANDLE hStdout = GetStdHandle(STD_OUTPUT_HANDLE); 
		GetConsoleScreenBufferInfo(hStdout, &csbiInfo);

		switch(nTraceLevel) 
		{
		case ERR:
			SetConsoleTextAttribute(hStdout, FOREGROUND_RED|FOREGROUND_INTENSITY);
			break;
		case WRN:
			SetConsoleTextAttribute(hStdout, FOREGROUND_RED|FOREGROUND_GREEN|FOREGROUND_INTENSITY);
			break;
		case KEY:
			SetConsoleTextAttribute(hStdout, FOREGROUND_GREEN|FOREGROUND_INTENSITY);
			break;	
		}
		printf("%s\n", strGBK);
		SetConsoleTextAttribute(hStdout, csbiInfo.wAttributes);
	}
	else
	{
		printf("%s\n", strGBK);
	}
}

void CTraceLog::LogToFile(const char* message, unsigned short& nMsgLen)
{
	if (m_hLogFileHandle != INVALID_HANDLE_VALUE)
	{
		unsigned long nNumberOfBytesToWrite;
		WriteFile(m_hLogFileHandle, message, nMsgLen, &nNumberOfBytesToWrite, NULL);
		m_dwCurLogFileSize += nNumberOfBytesToWrite;
		WriteFile(m_hLogFileHandle, "\r\n", 2, &nNumberOfBytesToWrite, NULL);
		m_dwCurLogFileSize += nNumberOfBytesToWrite;

		if (m_dwCurLogFileSize > m_dwMaxFileLen)
		{
			LogFileClose();
			LogFileOpen();
		}
	}
}

void CTraceLog::SetLogLevels(LogLevel nTraceLevel)
{
	m_nLogLevel = nTraceLevel;
}

void CTraceLog::SetScreenColorful(bool bColored)
{
	m_bScreenColor = bColored;
}

LogLevel CTraceLog::GetLogLevels(void)
{
	return m_nLogLevel;
}

int CTraceLog::UTF8ToGBK(unsigned char * lpUTF8Str,unsigned char * lpGBKStr,int nGBKStrLen)
{
 wchar_t * lpUnicodeStr = NULL;
 int nRetLen = 0;

 if(!lpUTF8Str)  //如果UTF8字符串为NULL则出错退出
  return 0;

 nRetLen = ::MultiByteToWideChar(CP_UTF8,0,(char *)lpUTF8Str,-1,NULL,NULL);  //获取转换到Unicode编码后所需要的字符空间长度
 lpUnicodeStr = new WCHAR[nRetLen + 1];  //为Unicode字符串空间
 nRetLen = ::MultiByteToWideChar(CP_UTF8,0,(char *)lpUTF8Str,-1,lpUnicodeStr,nRetLen);  //转换到Unicode编码
 if(!nRetLen)  //转换失败则出错退出
  return 0;

 nRetLen = ::WideCharToMultiByte(CP_ACP,0,lpUnicodeStr,-1,NULL,NULL,NULL,NULL);  //获取转换到GBK编码后所需要的字符空间长度

 if(!lpGBKStr)  //输出缓冲区为空则返回转换后需要的空间大小
 {
  if(lpUnicodeStr)
   delete []lpUnicodeStr;
  return nRetLen;
 }

 if(nGBKStrLen < nRetLen)  //如果输出缓冲区长度不够则退出
 {
  if(lpUnicodeStr)
   delete []lpUnicodeStr;
  return 0;
 }
 nRetLen = ::WideCharToMultiByte(CP_ACP,0,lpUnicodeStr,-1,(char *)lpGBKStr,nRetLen,NULL,NULL);  //转换到GBK编码

 if(lpUnicodeStr)
  delete []lpUnicodeStr;

 return nRetLen;
}

void CTraceLog::TraceLog(LogLevel nTraceLevel, unsigned short wChannel, const char *message, ...)
{
	if(m_hLogFileHandle == INVALID_HANDLE_VALUE)
		return;

	if(nTraceLevel < 0 || nTraceLevel > m_nLogLevel)
		return;

	if (message == NULL)
		return;

	if(::IsBadStringPtrA(message, -1))
		return;

	SYSTEMTIME sysTime;
	GetLocalTime(&sysTime);

	unsigned short nPreFix, nMsgLen;
	::EnterCriticalSection(&m_csLog);
	char* szData = new char[m_dwMaxBufferLen];
	memset(szData, NULL, m_dwMaxBufferLen);
	nPreFix = sprintf(szData, "%04d-%02d-%02d %02d:%02d:%02d:%03d %03d %s ",//32 length
						sysTime.wYear, sysTime.wMonth, sysTime.wDay, sysTime.wHour,
						sysTime.wMinute, sysTime.wSecond, sysTime.wMilliseconds, wChannel, LevelStrings[nTraceLevel]);

	va_list argList;
	va_start(argList, message);
	nMsgLen = _vsnprintf(szData+nPreFix, m_dwMaxBufferLen-nPreFix-1, message, argList);
	va_end(argList);

	if(m_uBufferExceed == nMsgLen)//buffer not enough
		nMsgLen = m_dwMaxBufferLen;
	else
		nMsgLen = nMsgLen+nPreFix+1;
	LogToScreen(nTraceLevel, szData);
	LogToFile(szData, nMsgLen);
	if(NULL != m_logMsgHandler)
		m_logMsgHandler(szData, nMsgLen);
	delete []szData;
	::LeaveCriticalSection(&m_csLog);
}

void CTraceLog::TraceLogV(LogLevel nTraceLevel, unsigned short wChannel, const char *message, va_list argList)
{
	if(m_hLogFileHandle == INVALID_HANDLE_VALUE)
		return;

	if(nTraceLevel < 0 || nTraceLevel > m_nLogLevel)
		return;

	if (message == NULL)
		return;

	if(::IsBadStringPtrA(message, -1))
		return;

	SYSTEMTIME sysTime;
	GetLocalTime(&sysTime);

	unsigned short nPreFix, nMsgLen;
	::EnterCriticalSection(&m_csLog);
	char* szData = new char[m_dwMaxBufferLen];
	memset(szData, NULL, m_dwMaxBufferLen);
	nPreFix = sprintf(szData, "%04d-%02d-%02d %02d:%02d:%02d:%03d %03d %s ",//32 length
						sysTime.wYear, sysTime.wMonth, sysTime.wDay, sysTime.wHour,
						sysTime.wMinute, sysTime.wSecond, sysTime.wMilliseconds, wChannel, LevelStrings[nTraceLevel]);

	nMsgLen = _vsnprintf(szData+nPreFix, m_dwMaxBufferLen-nPreFix-1, message, argList);

	if(m_uBufferExceed == nMsgLen)//buffer not enough
		nMsgLen = m_dwMaxBufferLen;
	else
		nMsgLen = nMsgLen+nPreFix+1;
	LogToScreen(nTraceLevel, szData);
	LogToFile(szData, nMsgLen);
	if(NULL != m_logMsgHandler)
		m_logMsgHandler(szData, nMsgLen);
	delete []szData;
	::LeaveCriticalSection(&m_csLog);
}

void CTraceLog::TraceLog(const char *message, ...)
{
	if(m_hLogFileHandle == INVALID_HANDLE_VALUE)
		return;

	if (message == NULL)
		return;

	if(::IsBadStringPtrA(message, -1))
		return;

	unsigned short nMsgLen;
	::EnterCriticalSection(&m_csLog);
	char* szData = new char[m_dwMaxBufferLen];
	memset(szData, NULL, m_dwMaxBufferLen);

	va_list argList;
	va_start( argList, message);
	nMsgLen = _vsnprintf( szData, m_dwMaxBufferLen-1, message, argList); 
	va_end(argList);

	if(m_uBufferExceed == nMsgLen)//buffer not enough
		nMsgLen = m_dwMaxBufferLen;
	else
		nMsgLen = nMsgLen+1;

	LogToScreen(INF, szData);
	LogToFile(szData, nMsgLen);
	if(NULL != m_logMsgHandler)
		m_logMsgHandler(szData, nMsgLen);
	delete []szData;
	::LeaveCriticalSection(&m_csLog);
}

void CTraceLog::TraceLogV(const char *message, va_list argList)
{
	if(m_hLogFileHandle == INVALID_HANDLE_VALUE)
		return;

	if (message == NULL)
		return;

	if(::IsBadStringPtrA(message, -1))
		return;

	unsigned short nMsgLen;
	::EnterCriticalSection(&m_csLog);
	char* szData = new char[m_dwMaxBufferLen];
	memset(szData, NULL, m_dwMaxBufferLen);
	nMsgLen = _vsnprintf(szData, m_dwMaxBufferLen-1, message, argList); 

	if(m_uBufferExceed == nMsgLen)//buffer not enough
		nMsgLen = m_dwMaxBufferLen;
	else
		nMsgLen = nMsgLen+1;

	LogToScreen(INF, szData);
	LogToFile(szData, nMsgLen);
	if(NULL != m_logMsgHandler)
		m_logMsgHandler(szData, nMsgLen);
	delete []szData;
	::LeaveCriticalSection(&m_csLog);
}
////////////////////////////////////////////////////////////////////////////////////