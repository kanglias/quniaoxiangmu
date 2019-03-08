
#ifndef TRACELOG_H
#define TRACELOG_H

#pragma once

#include "WindowsExtend.h"
#include <windows.h>
#include "CommonDefine.h"

#define MAX_LOG_BUFFER_SIZE	1024*2+32
#define MAX_LOG_FILE_SIZE	1024 * 1024 * 4

const char LevelStrings[][4] =
{
	{"ERR"},
	{"WRN"},
	{"KEY"},
	{"INF"},
	{"DBG"}
};

enum LogSubSystem
{
	UNK,
	SYS,
	IVR,
	SCR,				//script
	STC
};

const char SubSystemStrings[][4] = 
{
	{"UNK"},
	{"SYS"},
	{"IVR"},
	{"SCR"},
	{"STC"}
};

class CTraceLog
{
public:
	CTraceLog();
	~CTraceLog();

	bool InitLog();
	void ExitLog();

	LogLevel GetLogLevels(void);
	void SetLogLevels(LogLevel nTraceLevel);
	void SetScreenColorful(bool bColored);
	void SetCallBackMsg(LOG_MSG logMsgHandler = NULL);

	void TraceLog(LogLevel nTraceLevel, unsigned short wChannel, const char *message, ...);
	void TraceLogV(LogLevel nTraceLevel, unsigned short wChannel, const char *message, va_list argList);
	void TraceLog(const char *message, ...);
	void TraceLogV(const char *message, va_list argList);
private:
	void LogFileOpen();
	void LogFileClose();
	void LogToScreen(LogLevel nTraceLevel, const char* message);
	void LogToFile(const char* message, unsigned short& nMsgLen);
	int  UTF8ToGBK(unsigned char * lpUTF8Str,unsigned char * lpGBKStr,int nGBKStrLen);

private:
	CRITICAL_SECTION	m_csLog;
	HANDLE				m_hLogFileHandle;

	char				m_szLogPath[_MAX_PATH];
	char				m_szLogFileName[MAX_PATH];
	unsigned long		m_dwCurLogFileSize;

	unsigned long		m_dwMaxBufferLen;
	unsigned long		m_dwMaxFileLen;
	unsigned short		m_uBufferExceed;

	bool				m_bScreenColor;
	LogLevel			m_nLogLevel;
	LOG_MSG				m_logMsgHandler;
};

#endif