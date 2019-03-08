/************************************************************************/
/* Copyright  2005-2100,  Peter Inc. - All Rights Reserved. 			*/
/* Create		:   2005-11-26   				                        */
/* Last Update	:   2006-09-18   				                        */
/* Purpose	    :   Implement define common infomation			        */
/* FileName     :   CommonDefine.h		                                */
/* Author 	    :   Peter 		 										*/
/************************************************************************/
#ifndef COMMONDEFINE_H
#define COMMONDEFINE_H

#pragma once
/**********************Define Log Level**************************************/
//ERR: error message,high level
//WRN: waring message
//KEY: key message
//INF: infomation message
//DBG: debug message low level,trace all message
enum LogLevel 
{ 
	ERR = 0, 
	WRN,
	KEY,
	INF, 
	DBG,
	LEVEL_QTY
};
//callback funtion
//Parameter Input: log level
//Parameter Input: return log message
typedef void (__stdcall *LOG_EVENT)(LogLevel nTraceLevel, const char *szMessage);
typedef void (__stdcall *LOG_EVTEX)(LogLevel nTraceLevel, const char *szMessage, char* argList);
typedef void (__stdcall *LOG_CHNEVT)(LogLevel nTraceLevel, unsigned short wChannel, const char *szMessage, char* argList);
typedef void (__stdcall *LOG_MSG)(const char *szMessage, unsigned short& nMsgLen);
typedef void (__stdcall *USER_MSG)(unsigned short uMsgType, char *szMessage, unsigned long& nMsgLen, unsigned long dwInstance);

#endif
