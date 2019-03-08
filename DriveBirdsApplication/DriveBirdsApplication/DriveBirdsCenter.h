#pragma once
#include "afxdb.h"
#include "afxwin.h"
#include "tracelog.h"

class CCommBusClient;
class CMscomm1;
struct paramDrive
{
	paramDrive(const char *pDeviceId,const char *pDeviceType,void* pVoid)
	{
		strncpy(m_szDeviceId,pDeviceId,sizeof(m_szDeviceId)-1);
		strncpy(m_szDeviceType,pDeviceType,sizeof(m_szDeviceType)-1);
		m_pVoid = pVoid;
	};
	char m_szDeviceId[32];
	char  m_szDeviceType[32];
	void* m_pVoid;
};
class CDriveBirdsCenter
{
public:
	CDriveBirdsCenter(void);
	~CDriveBirdsCenter(void);
	bool run(void);
	void stop(void);
	static unsigned int _stdcall threadPopCommBusMessage(void* p);
	static unsigned int _stdcall threadStartDriveBirds(void* p);
	unsigned int popCommBusMessage(void);
	unsigned int StartDriveBirds(void* p);
	static unsigned int _stdcall threadCheckDeviceState(void* p);
	static unsigned int _stdcall threadStopTimingDevice(void* p);
	unsigned int CheckDeviceState(void);
	unsigned int StopTimingDevice(void);
	int UTF8ToGBK(unsigned char * lpUTF8Str,unsigned char * lpGBKStr,int nGBKStrLen);
	CTraceLog m_log;
	CCommBusClient* m_pCommbusClient;
	time_t __get_time(void);
	HANDLE m_hEventExit;
	HANDLE m_threadCommBusMessage;
	time_t m_tm_last_send;
	bool ExecuteSql(const char* szSql);
	bool WriteOperateLog(const char* szAction, const char* szDeviceId, const char* szUser, const char* szIntervalTime);
	bool WriteDeviceStateRealtime(const char* szDeviceId,int nRunningState,int nPowerState,int nBatteryState,int nPressureState,const char* szRawData);
	bool WriteDeviceStateHistory(const char* szDeviceId,int nRunningState,int nPowerState,const char* szBatteryState,const char* szPressureState,const char* szRawData);
	bool WriteDeviceAlertLog(const char* szDeviceId,int nRunningState,int nPowerState,const char* szBatteryState,const char* szPressureState,const char* szAlertReason,const char* szRawData);
	unsigned char LRC_check(unsigned char *pSendBuf,unsigned char num);
	bool ParseRawData(byte* szRawData,int nLen);
	bool Com_StopDevice(const char* szDeviceId, const char* szDeviceName, const char* szDeviceType);
	bool Com_QueryDevice(const char* szDeviceId, const char* szDeviceName, const char* szDeviceType);
	bool Com_StartDevice(const char* szDeviceId, const char* szDeviceName, const char* szDeviceType);
	bool StartDriveBirds(const char* szDeviceId, const char* szDeviceName, const char* szDeviceType);
	bool SetComm(CMscomm1* pComm);
	CMscomm1* m_pComm;
	bool Com_Open(void);
	bool Com_Close(void);
	void OnCommMessage(void);
	bool String2Hex(std::string &src,unsigned char *dest);
	bool GetThresholdLevel(float* pnPressure, float* pnVoltage);
	bool RecordComMessageDt(const char* szDeviceId);
	bool SaveCurrentData(const char* szDeviceId, const char* szDeviceType,float fBatteryState,float fPressureState,int nBoardState,int nSoundState);
	bool DoWarning(const char* szDeviceId);
	bool GetDeviceType(const char* szDeviceId, char* szDeviceType);
	bool VoltageNormal(const char* szDeviceId);
	bool PressureNormal(const char* szDeviceId);
	bool SoundNormal(const char* szDeviceId);
	bool BoardNormal(const char* szDeviceId);
	bool VoltageQuanXiang(const char* szDeviceId);
	bool PressureQuanXiang(const char* szDeviceId);
	bool SoundQuanXiang(const char* szDeviceId);
	bool BoardQuanXiang(const char* szDeviceId);
	bool IfExistInDb(const char* szDeviceId, const char* szTableName);
	void printMessage(unsigned char* byteTest, int nLen);
};

