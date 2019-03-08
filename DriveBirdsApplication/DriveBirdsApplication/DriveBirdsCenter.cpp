#include "StdAfx.h"
#include "DriveBirdsCenter.h"
#include<ctime>
#include "stdint.h"
#include "mscomm1.h"
#include "..\\Include\\CommBus\\CommBusClient.h"
#include "..\\include\\commonused.h"
using namespace commonused;
#define CONFIGURE_FILE ".\\DriveBirdsService.ini"



CDriveBirdsCenter::CDriveBirdsCenter(void)
//	
: m_pComm(NULL),m_pCommbusClient(NULL)
{
	m_tm_last_send = NULL;
}


CDriveBirdsCenter::~CDriveBirdsCenter(void)
{
}
bool CDriveBirdsCenter::run(void)
{
	m_log.InitLog();
	m_hEventExit = ::CreateEvent(NULL, TRUE, FALSE, NULL);
	if(m_hEventExit == NULL)
	{
		m_log.TraceLog(ERR, 0,"Create service event failed, error:0x%X, exit system", GetLastError());	
	
		m_log.ExitLog();

		return false;
	}

	

	char szIP[32],szRegName[32];
	memset(szIP,0,sizeof(szIP));
	GetPrivateProfileString("Server", "IP", "", szIP, sizeof(szIP), CONFIGURE_FILE);
	DWORD dwTickCount=GetTickCount();

	memset(szRegName,0,sizeof(szRegName));
	GetPrivateProfileString("Client", "Name", "", szRegName, sizeof(szRegName), CONFIGURE_FILE);
	SYSTEMTIME sysTime;
    GetLocalTime(&sysTime);
/*	sprintf(szRegName, "Client%04d%02d%02d%02d%02d%02d", sysTime.wYear,
		sysTime.wMonth, sysTime.wDay, sysTime.wHour, sysTime.wMinute, sysTime.wSecond);
*/
	if(Com_Open()==false)
	{
		m_log.TraceLog(ERR, 0,"Com_Open failed");
		return false;
	}

	m_log.TraceLog(KEY, 0,"CCommBusClient init IP:%s,RegName:%s", szIP,szRegName);	
	m_pCommbusClient = new CCommBusClient(szIP,szRegName);
	bool t_rst = m_pCommbusClient->initialize();

	if(t_rst == true)
	{
		HANDLE thread_pop_commbusmessage = (void*)_beginthreadex(NULL, 0, threadPopCommBusMessage, this, 0, NULL);
		if(thread_pop_commbusmessage<0)
		{
			m_log.TraceLog(ERR, 0,"Create threadPopCommBusMessage failed, error:0x%X, exit system", GetLastError());	

			m_log.ExitLog();

			delete m_pCommbusClient;
			m_pCommbusClient = NULL;

			return false;
		}
	}

	HANDLE thread_check_devicestate = (void*)_beginthreadex(NULL, 0, threadCheckDeviceState, this, 0, NULL);
		if(thread_check_devicestate<0)
		{
			m_log.TraceLog(ERR, 0,"Create service event failed, error:0x%X, exit system", GetLastError());	

			m_log.ExitLog();

			delete m_pCommbusClient;
			m_pCommbusClient = NULL;

			return false;
		}
	return true;
}

void CDriveBirdsCenter::stop(void)
{
	::SetEvent(m_hEventExit);
	if(m_threadCommBusMessage != NULL)
	{
		
		WaitForSingleObject(m_threadCommBusMessage, 5000);
		CloseHandle(m_threadCommBusMessage);
		m_threadCommBusMessage = NULL;
	}

	if(m_pCommbusClient)
	{
		m_pCommbusClient->release();
		delete m_pCommbusClient;
		m_pCommbusClient = NULL;
	}

	m_log.ExitLog();
}

time_t CDriveBirdsCenter::__get_time(void)
{
	time_t tm;
	time(&tm);

	return tm;
}

unsigned int _stdcall CDriveBirdsCenter::threadPopCommBusMessage(void* p)
{
	return ((CDriveBirdsCenter*)p)->popCommBusMessage();
}

unsigned int _stdcall CDriveBirdsCenter::threadStartDriveBirds(void* p)
{
	CDriveBirdsCenter* pThis = (CDriveBirdsCenter*)((paramDrive*)p)->m_pVoid;
	return pThis->StartDriveBirds(p);
}
unsigned int CDriveBirdsCenter::StartDriveBirds(void* p)
{
	char szDeviceId[32],szDeviceType[32];
	memset(szDeviceId,0,sizeof(szDeviceId));
	memset(szDeviceType,0,sizeof(szDeviceType));
	strcpy(szDeviceId,((paramDrive*)p)->m_szDeviceId);
	strcpy(szDeviceType,((paramDrive*)p)->m_szDeviceType);

	for(int i=0;i<4;i++)
	{
		Com_StartDevice(szDeviceId,"",szDeviceType);
		::Sleep(1000);
	}

	int nType = atoi(szDeviceType);
	switch(nType)
	{
	case 5:
		break;
	case 10://移动
		{
			::Sleep(2000);
			for(int i=0;i<5;i++)
			{
				Com_StopDevice(szDeviceId,"",szDeviceType);
				::Sleep(1000);
			}
		}
		break;
	case 7://定向
		{
			::Sleep(120000);
			for(int i=0;i<5;i++)
			{
				Com_StopDevice(szDeviceId,"",szDeviceType);
				::Sleep(1000);
			}
		}
		break;
	case 8://全向
		{
			::Sleep(180000);
			for(int i=0;i<5;i++)
			{
				Com_StopDevice(szDeviceId,"",szDeviceType);
				::Sleep(1000);
			}
		}
		break;
	case 6://塔式
		{
			::Sleep(5000);
			for(int i=0;i<5;i++)
			{
				Com_StopDevice(szDeviceId,"",szDeviceType);
				::Sleep(1000);
			}
		}
		break;
	default:
		m_log.TraceLog(WRN,0, "StartDriveBirds unkown device type");
		return false;
	}
	return 0;
}
unsigned int CDriveBirdsCenter::popCommBusMessage(void)
{
	int interval = 10;
    string sender_ins, message_type, message_data;

    while(WaitForSingleObject(m_hEventExit, 10) != WAIT_OBJECT_0)
    {
        if(WaitForSingleObject(m_pCommbusClient->event_link_on, 0) == WAIT_OBJECT_0)
        {
           m_log.TraceLog(KEY, 0,"CCommBusClient Link On");	
        }
        if(WaitForSingleObject(m_pCommbusClient->event_link_off, 0) == WAIT_OBJECT_0)
        {
            m_log.TraceLog(KEY, 0,"CCommBusClient Link OFF");	
        }

        if(!m_pCommbusClient->popMessage(sender_ins, message_type, message_data, interval))
        {
			int nSec1 =(int) difftime(__get_time(), m_tm_last_send);
			if(nSec1>=1)
			{
				m_pCommbusClient->pushMessage("WebSocketServer","activestandby-heartbeat","OK");
				m_tm_last_send = __get_time();
			}
            continue;
        }
		m_pCommbusClient->pushMessage("WebSocketServer","activestandby-heartbeat","OK");

		char strGBK[1024];
		memset(strGBK,0,sizeof(strGBK));
		UTF8ToGBK((unsigned char *)message_data.c_str(),(unsigned char *)strGBK,1024);
		m_log.TraceLog(DBG,0,"Receive Message(Sender:%s,Type:%s):%s\n",sender_ins.c_str(),message_type.c_str(),strGBK);  

		string strContent = message_data;
		trimLeft(strContent, '\"');
		trimRight(strContent, '\"');

		JsonObject data;
		parseJsonString(strContent, data);
		string strAction = getJsonPairValue(data, "Action");
		string strDevice = getJsonPairValue(data, "deviceId");
		string strInternal = "0";//getJsonPairValue(data, "interTime");
		string strUser = getJsonPairValue(data, "User");

		char szDeviceType[32];
		memset(szDeviceType,0,sizeof(szDeviceType));
		bool bRet = GetDeviceType(strDevice.c_str(),szDeviceType);
		if(bRet==false)
		{
			m_log.TraceLog(WRN,0,"Device:%s GetDeviceType fail",strDevice.c_str());  
			continue;
		}

		CString strActionMfc = strAction.c_str();
		if(strActionMfc.CompareNoCase("open")!=0)
		{
			m_log.TraceLog(WRN,0,"Action not open,ignore");
		}
		StartDriveBirds(strDevice.c_str(),"",szDeviceType);
		WriteOperateLog(strAction.c_str(),strDevice.c_str(),strUser.c_str(),strInternal.c_str());
    }

    return 0;
}

unsigned int _stdcall CDriveBirdsCenter::threadCheckDeviceState(void* p)
{
	return ((CDriveBirdsCenter*)p)->CheckDeviceState();
}

unsigned int _stdcall CDriveBirdsCenter::threadStopTimingDevice(void* p)
{
	return ((CDriveBirdsCenter*)p)->CheckDeviceState();
}

unsigned int CDriveBirdsCenter::CheckDeviceState(void)
{
//	int interval = 10000;
	::Sleep(5000);
	m_log.TraceLog(DBG,0, "begin CheckDeviceState...");
	int nInterval = 0;
    while(WaitForSingleObject(m_hEventExit, nInterval*1000) != WAIT_OBJECT_0)
    {
		nInterval = GetPrivateProfileInt(_T("Time"),_T("DetectDeviceState "),1,CONFIGURE_FILE);
		m_log.TraceLog(DBG,0, "begin Load device to check...");
		
	/*	Com_QueryDevice("A01","01","05");
		::Sleep(1000);
		continue;
	*/	

		CString strSql;
		CRecordset *pRecordSet = NULL;
		TRY
		{
			int nStartTick = GetTickCount();
			char szDSN[256];
			memset(szDSN,0,sizeof(szDSN));
			GetPrivateProfileString("Database", "DSN", "", szDSN, sizeof(szDSN), CONFIGURE_FILE);
			CDatabase db;	
			if(!db.OpenEx(szDSN,CDatabase::noOdbcDialog))
			{
				m_log.TraceLog(WRN,0,"Open Dsn:%s ERROR",szDSN);
				continue;
			}

			CString strIndex, strDeviceId, strDeviceName, strDeviceType, strResult;
			strSql.Format("SELECT id,deviceId,devicename,deviceType FROM drivebirds.t_device_list order by id asc");


			pRecordSet = new CRecordset;
			pRecordSet->m_pDatabase = &db;
			pRecordSet->Open(CRecordset::forwardOnly, strSql);
			while (!pRecordSet->IsEOF())
			{
				pRecordSet->GetFieldValue((short)0, strIndex);
				pRecordSet->GetFieldValue((short)1, strDeviceId);
				pRecordSet->GetFieldValue((short)2, strDeviceName);
				pRecordSet->GetFieldValue((short)3, strDeviceType);

				int nInterval2 = GetPrivateProfileInt(_T("Time"), _T("DetectDeviceStateInterval"), 500, CONFIGURE_FILE);
				if(nInterval2<1000)
					nInterval2 = 1000;
				for(int i=0;i<1;i++)
				{
					Com_QueryDevice(strDeviceId,strDeviceName,strDeviceType);
					::Sleep(nInterval2);
				}
				//::Sleep(1000);
				DoWarning(strDeviceId);
				pRecordSet->MoveNext();
			}
			int nCount = pRecordSet->GetRecordCount();
			pRecordSet->Close();
			delete pRecordSet;
			pRecordSet = NULL;
			db.Close();
			int nTimeUse = GetTickCount() - nStartTick;
			m_log.TraceLog(KEY,0, "Load device list Complete,use:%dms,count:%d", nTimeUse,nCount);
		}
		CATCH(CDBException, e)
		{
			if (pRecordSet != NULL)
			{
				if (pRecordSet->IsOpen())
					pRecordSet->Close();
				delete pRecordSet;
				pRecordSet = NULL;
			}
			m_log.TraceLog(ERR,0, "execute sql(%s) failed,Error:%s", strSql, e->m_strError);
			continue;
		}
		END_CATCH
	
    }
	m_log.TraceLog(DBG,0, "end CheckDeviceState...");
    return 0;
}

unsigned int CDriveBirdsCenter::StopTimingDevice(void)
{
	int interval = 1000;
	m_log.TraceLog(DBG,0, "begin StopTimingDevice...");

    while(WaitForSingleObject(m_hEventExit, interval) != WAIT_OBJECT_0)
    {
		m_log.TraceLog(DBG,0, "begin Load device to stop...");
		int nStartTick = GetTickCount();
		char szDSN[256];
		memset(szDSN,0,sizeof(szDSN));
		GetPrivateProfileString("Database", "DSN", "", szDSN, sizeof(szDSN), CONFIGURE_FILE);
		
		
		CString strIndex, strDeviceId, strDeviceName, strDeviceType, strResult, strSql;
		strSql.Format("SELECT id,deviceId,devicename,deviceType FROM drivebirds.t_device_list order by id asc");
		CRecordset *pRecordSet = NULL;

		TRY
		{
			CDatabase db;	
			if(!db.OpenEx(szDSN,CDatabase::noOdbcDialog))
			{
				m_log.TraceLog(WRN,0,"Open Dsn:%s ERROR",szDSN);
				continue;
			}
			pRecordSet = new CRecordset;
			pRecordSet->m_pDatabase = &db;
			pRecordSet->Open(CRecordset::forwardOnly, strSql);
			while (!pRecordSet->IsEOF())
			{
				pRecordSet->GetFieldValue((short)0, strIndex);
				pRecordSet->GetFieldValue((short)1, strDeviceId);
				pRecordSet->GetFieldValue((short)2, strDeviceName);
				pRecordSet->GetFieldValue((short)3, strDeviceType);

				Com_StopDevice(strDeviceId,strDeviceName,strDeviceType);
				pRecordSet->MoveNext();
			}
			int nCount = pRecordSet->GetRecordCount();
			pRecordSet->Close();
			delete pRecordSet;
			pRecordSet = NULL;
			db.Close();
			int nTimeUse = GetTickCount() - nStartTick;
			m_log.TraceLog(KEY,0, "Load device list Complete,use:%dms,count:%d", nTimeUse,nCount);
		}
		CATCH(CDBException, e)
		{
			if (pRecordSet != NULL)
			{
				if (pRecordSet->IsOpen())
					pRecordSet->Close();
				delete pRecordSet;
				pRecordSet = NULL;
			}
			m_log.TraceLog(ERR,0, "execute sql(%s) failed,Error:%s", strSql, e->m_strError);
			continue;
		}
		END_CATCH
    }

	m_log.TraceLog(DBG,0, "end StopTimingDevice...");
    return 0;
}

int CDriveBirdsCenter::UTF8ToGBK(unsigned char * lpUTF8Str,unsigned char * lpGBKStr,int nGBKStrLen)
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

bool CDriveBirdsCenter::ExecuteSql(const char* szSql)
{
	m_log.TraceLog(DBG,0, "execute sql:%s", szSql);
	char szDSN[256];
	memset(szDSN,0,sizeof(szDSN));
	GetPrivateProfileString("Database", "DSN", "", szDSN, sizeof(szDSN), CONFIGURE_FILE);
	TRY
	{
		CDatabase db;	
		if(!db.OpenEx(szDSN,CDatabase::noOdbcDialog))
		{
			m_log.TraceLog(WRN,0,"Open Dsn:%s ERROR",szDSN);
			return false;
		}
		db.ExecuteSQL(szSql);
		db.Close();
	}
	CATCH(CDBException, e)
	{
		m_log.TraceLog(ERR,0, "execute sql(%s) failed,Error:%s", szSql, e->m_strError);
		return false;
	}
	END_CATCH
		return false;
}


bool CDriveBirdsCenter::WriteOperateLog(const char* szAction, const char* szDeviceId, const char* szUser, const char* szIntervalTime)
{
	char szSQL[1024];
	memset(szSQL,0,sizeof(szSQL));
	SYSTEMTIME sysTime;
    GetLocalTime(&sysTime);
	char szTime[32];
	memset(szTime,0,sizeof(szTime));
	sprintf(szTime, "%04d-%02d-%02d %02d:%02d:%02d", sysTime.wYear,
		sysTime.wMonth, sysTime.wDay, sysTime.wHour, sysTime.wMinute, sysTime.wSecond);

	char szRemark[128];
	memset(szRemark,0,sizeof(szRemark));
	sprintf(szSQL,"insert into t_operate_log(userid,deviceid,action,operate_dt) values('%s','%s','%s','%s')",szUser,szDeviceId,szAction,szTime);

	ExecuteSql(szSQL);

	return true;
}

bool CDriveBirdsCenter::WriteDeviceStateRealtime(const char* szDeviceId,int nRunningState,int nPowerState,int nBatteryState,int nPressureState,const char* szRawData)
{
	char szSQL[1024];
	memset(szSQL,0,sizeof(szSQL));
	SYSTEMTIME sysTime;
    GetLocalTime(&sysTime);
	char szTime[32];
	memset(szTime,0,sizeof(szTime));
	sprintf(szTime, "%04d-%02d-%02d %02d:%02d:%02d", sysTime.wYear,
		sysTime.wMonth, sysTime.wDay, sysTime.wHour, sysTime.wMinute, sysTime.wSecond);
	bool bAlert = false;
	char szRemark[128];
	memset(szRemark,0,sizeof(szRemark));
	sprintf(szSQL,"update t_device_state_realtime set deviceid='%s',runningstate='%d',powerstate='%d',batterystate='%d',pressurestate='%d',rawdata='%s',alert='%d',last_modify_dt='%s'",szDeviceId,nRunningState,nPowerState,nBatteryState,nPressureState,szRawData,bAlert,szTime);

	ExecuteSql(szSQL);

	return true;
}

bool CDriveBirdsCenter::WriteDeviceStateHistory(const char* szDeviceId,int nRunningState,int nPowerState,const char* szBatteryState,const char* szPressureState,const char* szRawData)
{
	char szSQL[1024];
	memset(szSQL,0,sizeof(szSQL));
/*	SYSTEMTIME sysTime;
    GetLocalTime(&sysTime);
	char szTime[32];
	memset(szTime,0,sizeof(szTime));
	sprintf(szTime, "%04d-%02d-%02d %02d:%02d:%02d", sysTime.wYear,
		sysTime.wMonth, sysTime.wDay, sysTime.wHour, sysTime.wMinute, sysTime.wSecond);
*/
	char szRemark[128];
	memset(szRemark,0,sizeof(szRemark));
	sprintf(szSQL,"insert into t_device_state_history(deviceid,runningstate,powerstate,batterystate,pressurestate,rawdata,write_dt) values('%s','%d','%d','%s','%s','%s',now())",szDeviceId,nRunningState,nPowerState,szBatteryState,szPressureState,szRawData);

	if(IfExistInDb(szDeviceId,"t_device_state_history")==false)
		ExecuteSql(szSQL);

	return true;
}
	
bool CDriveBirdsCenter::WriteDeviceAlertLog(const char* szDeviceId,int nRunningState,int nPowerState,const char* szBatteryState,const char* szPressureState,const char* szAlertReason,const char* szRawData)
{
	char szSQL[1024];
	memset(szSQL,0,sizeof(szSQL));

	char szRemark[128];
	memset(szRemark,0,sizeof(szRemark));
	sprintf(szSQL,"insert into t_device_alert_log(deviceid,runningstate,powerstate,batterystate,pressurestate,rawdata,write_dt,alert_reasons) select deviceid,%d,%d,'%s','%s',rawdata,now(),'%s' from t_device_state_realtime where alert ='0' and deviceid='%s'",nRunningState,nPowerState,szBatteryState,szPressureState,szAlertReason,szDeviceId);

	if(IfExistInDb(szDeviceId,"t_device_alert_log")==false)
		ExecuteSql(szSQL);

	return true;
}

bool CDriveBirdsCenter::ParseRawData(byte* szRawData,int nLen)
{
	if(nLen<8)
	{
		m_log.TraceLog(DBG,0, "ParseRawData nLen too small,return");
		return false;
	}
	if(szRawData[0]!=0x40||szRawData[1]!=0x23||szRawData[2]!=0x24)
	{
		m_log.TraceLog(DBG,0, "ParseRawData wrong data,return");
		return false;
	}
	m_log.TraceLog(DBG,0, "Parseing Data");
	if(szRawData[5]==0x08)
	{
		m_log.TraceLog(DBG,0, "this is a command request,return");
		return false;
	}
	if(szRawData[5]==0x04)
	{
		m_log.TraceLog(DBG,0, "this is a query request,return");
		return false;
	}
	if(szRawData[5]==0xFF&&szRawData[6]==0x08)
	{
		m_log.TraceLog(DBG,0, "this is a command reply,command fail");
		return false;
	}
	if(szRawData[5]==0x00&&szRawData[6]==0x08)
	{
		m_log.TraceLog(DBG,0, "this is a command reply,command success");
		if(nLen<15)
		{
			m_log.TraceLog(DBG,0, "reply nLen too small,return");
			return false;
		}
		m_log.TraceLog(DBG,0, "this is a command reply(Device:0x%02x,Command:0x%02x),command success",szRawData[3],szRawData[4]);
		return true;
	}
	if(szRawData[5]==0xFF&&szRawData[6]==0x04)
	{
		m_log.TraceLog(DBG,0, "this is a query reply,query fail");
		return false;
	}
	if(szRawData[5]==0x00&&szRawData[6]==0x04&&szRawData[7]==0xAB)
	{
		m_log.TraceLog(DBG,0, "this is a query reply,query success");
		if(nLen<15)
	{
		m_log.TraceLog(DBG,0, "reply nLen too small,return");
		return false;
	}
		if(szRawData[3]==0x05)
		{
			m_log.TraceLog(DBG,0, "Type:05 Gas");
			int nBatteryIntegerBit = szRawData[8];
			int nBatteryDecimalBit = szRawData[9];
			CString strBatteryValue;
			strBatteryValue.Format("%d.%d",nBatteryIntegerBit,nBatteryDecimalBit);
			int nPressureIntegerBit = szRawData[10];
			int nPressureDecimalBit = szRawData[11];
			CString strPressureValue;
			strPressureValue.Format("%d.%d",nPressureIntegerBit,nPressureDecimalBit);
			CString strSound;
			if(szRawData[12]==0x00)
				strSound = "1";
			else
				strSound = "0";
			m_log.TraceLog(DBG,0,"Receive State{Type:05 Gas,DeviceId:%X,BatteryValue:%s,PressureValue:%s,Sound:%s}",szRawData[4],strBatteryValue,strPressureValue,strSound);

			CString strDeviceId;
			strDeviceId.Format("A%02x",szRawData[4]);
			char szSQL[1024];
			memset(szSQL,0,sizeof(szSQL));
			sprintf(szSQL,"update t_device_state_realtime set runningstate='%d',powerstate='%d',batterystate='%s',pressurestate='%s',sound='%s',last_modify_dt=now() where deviceid='%s'",1,1,strBatteryValue,strPressureValue,strSound,strDeviceId);
			ExecuteSql(szSQL);

			float fBatteryValue = atof(strBatteryValue);
			float fPressureValue = atof(strPressureValue);
			int nSound = atoi(strSound);
			float fBatteryStateLevel =0,fPressureStateLevel = 0;
			GetThresholdLevel(&fPressureStateLevel,&fBatteryStateLevel);
			if(fBatteryValue>fBatteryStateLevel)
			{
				char szSQL[1024];
				memset(szSQL,0,sizeof(szSQL));
				sprintf(szSQL,"update t_device_state_realtime set Batterystate_avaliable_dt=now() where deviceid='%s'",strDeviceId);
				ExecuteSql(szSQL);
			}
			if(fPressureValue>fPressureStateLevel)
			{
				char szSQL[1024];
				memset(szSQL,0,sizeof(szSQL));
				sprintf(szSQL,"update t_device_state_realtime set pressurestate_avaliable_dt=now() where deviceid='%s'",strDeviceId);
				ExecuteSql(szSQL);
			}
			if(nSound==1)
			{
				char szSQL[1024];
				memset(szSQL,0,sizeof(szSQL));
				sprintf(szSQL,"update t_device_state_realtime set soundstate_avaliable_dt=now() where deviceid='%s'",strDeviceId);
				ExecuteSql(szSQL);
			}

			CString strRawData;
			RecordComMessageDt(strDeviceId);
			WriteDeviceStateHistory(strDeviceId,1,1,strBatteryValue,strPressureValue,strRawData);

		}
		else if(szRawData[3]==0x06)
		{
			m_log.TraceLog(DBG,0, "Type:06 Acoustic bird repeller");
			int nBatteryIntegerBit = szRawData[8];
			int nBatteryDecimalBit = szRawData[9];
			CString strBatteryValue;
			strBatteryValue.Format("%d.%d",nBatteryIntegerBit,nBatteryDecimalBit);
			
			CString strPressureValue;
			
			CString strSound;
			if(szRawData[12]==0x00)
				strSound = "1";
			else
				strSound = "0";
			m_log.TraceLog(DBG,0,"Receive State{Type:06 Acoustic bird repeller,DeviceId:%X,BatteryValue:%s,PressureValue:%s,Sound:%s}",szRawData[4],strBatteryValue,strPressureValue,strSound);

			CString strDeviceId;
			strDeviceId.Format("B%02x",szRawData[4]);
			char szSQL[1024];
			memset(szSQL,0,sizeof(szSQL));
			sprintf(szSQL,"update t_device_state_realtime set runningstate='%d',powerstate='%d',batterystate='%s',pressurestate='%s',sound='%s',last_modify_dt=now() where deviceid='%s'",1,1,strBatteryValue,strPressureValue,strSound,strDeviceId);
			ExecuteSql(szSQL);

			float fBatteryValue = atof(strBatteryValue);
			float fPressureValue = -1;
			
			int nSound = atoi(strSound);
			float fBatteryStateLevel =0,fPressureStateLevel = 0;
			GetThresholdLevel(&fPressureStateLevel,&fBatteryStateLevel);
			if(fBatteryValue>fBatteryStateLevel)
			{
				char szSQL[1024];
				memset(szSQL,0,sizeof(szSQL));
				sprintf(szSQL,"update t_device_state_realtime set Batterystate_avaliable_dt=now() where deviceid='%s'",strDeviceId);
				ExecuteSql(szSQL);
			}
			
			if(nSound==1)
			{
				char szSQL[1024];
				memset(szSQL,0,sizeof(szSQL));
				sprintf(szSQL,"update t_device_state_realtime set soundstate_avaliable_dt=now() where deviceid='%s'",strDeviceId);
				ExecuteSql(szSQL);
			}
			
			RecordComMessageDt(strDeviceId);
			CString strRawData;
			WriteDeviceStateHistory(strDeviceId,1,1,strBatteryValue,strPressureValue,strRawData);
		}
		else if(szRawData[3]==0x07)
		{
			m_log.TraceLog(DBG,0, "Type:07 Redirect Sonic bird repeller");

			CString strBatteryValue;
		
			CString strPressureValue;
			
			CString strSound;
			if(szRawData[12]==0x00)
				strSound = "1";
			else
				strSound = "0";
			m_log.TraceLog(DBG,0,"Receive State{Type:07 Redirect Sonic bird repeller,DeviceId:%X,BatteryValue:%s,PressureValue:%s,Sound:%s}",szRawData[4],strBatteryValue,strPressureValue,strSound);

			CString strDeviceId;
			strDeviceId.Format("C%02x",szRawData[4]);
			char szSQL[1024];
			memset(szSQL,0,sizeof(szSQL));
			sprintf(szSQL,"update t_device_state_realtime set runningstate='%d',powerstate='%d',batterystate='%s',pressurestate='%s',sound='%s',last_modify_dt=now() where deviceid='%s'",1,1,strBatteryValue,strPressureValue,strSound,strDeviceId);
			ExecuteSql(szSQL);

			float fBatteryValue = atof(strBatteryValue);
			float fPressureValue = -1;
			int nSound = atoi(strSound);
			
			if(nSound==1)
			{
				char szSQL[1024];
				memset(szSQL,0,sizeof(szSQL));
				sprintf(szSQL,"update t_device_state_realtime set soundstate_avaliable_dt=now() where deviceid='%s'",strDeviceId);
				ExecuteSql(szSQL);
			}

			CString strRawData;
			RecordComMessageDt(strDeviceId);
			WriteDeviceStateHistory(strDeviceId,1,1,strBatteryValue,strPressureValue,strRawData);

		}
		else if(szRawData[3]==0x08)
		{
			m_log.TraceLog(DBG,0, "Type:08 All Sonic bird repeller");
			int nBatteryIntegerBit = szRawData[8];
			int nBatteryDecimalBit = szRawData[9];
			CString strBatteryValue;
			strBatteryValue.Format("%d.%d",nBatteryIntegerBit,nBatteryDecimalBit);
			
			CString strPressureValue;
			
			CString strSound;
			if(szRawData[12]==0x00)
				strSound = "1";
			else
				strSound = "0";
			m_log.TraceLog(DBG,0,"Receive State{Type:08 All Sonic bird repeller,DeviceId:%X,BatteryValue:%s,PressureValue:%s,Sound:%s}",szRawData[4],strBatteryValue,strPressureValue,strSound);

			CString strDeviceId;
			strDeviceId.Format("D%02x",szRawData[4]);
			char szSQL[1024];
			memset(szSQL,0,sizeof(szSQL));
			sprintf(szSQL,"update t_device_state_realtime set runningstate='%d',powerstate='%d',batterystate='%s',pressurestate='%s',sound='%s',last_modify_dt=now() where deviceid='%s'",1,1,strBatteryValue,strPressureValue,strSound,strDeviceId);
			ExecuteSql(szSQL);

			float fBatteryValue = atof(strBatteryValue);
			float fPressureValue = -1;
			int nSound = atoi(strSound);

			float fBatteryStateLevel =0,fPressureStateLevel = 0;
			GetThresholdLevel(&fPressureStateLevel,&fBatteryStateLevel);
			if(fBatteryValue>fBatteryStateLevel)
			{
				char szSQL[1024];
				memset(szSQL,0,sizeof(szSQL));
				sprintf(szSQL,"update t_device_state_realtime set Batterystate_avaliable_dt=now() where deviceid='%s'",strDeviceId);
				ExecuteSql(szSQL);
			}
			
			if(nSound==1)
			{
				char szSQL[1024];
				memset(szSQL,0,sizeof(szSQL));
				sprintf(szSQL,"update t_device_state_realtime set soundstate_avaliable_dt=now() where deviceid='%s'",strDeviceId);
				ExecuteSql(szSQL);
			}
			RecordComMessageDt(strDeviceId);
			CString strRawData;
			WriteDeviceStateHistory(strDeviceId,1,1,strBatteryValue,strPressureValue,strRawData);
		}
		else if(szRawData[3]==0x09)
		{
			m_log.TraceLog(DBG,0, "Type:09 Tower-type Sonic bird repeller");
			int nBatteryIntegerBit = szRawData[8];
			int nBatteryDecimalBit = szRawData[9];
			CString strBatteryValue;
			strBatteryValue.Format("%d.%d",nBatteryIntegerBit,nBatteryDecimalBit);
			
			CString strPressureValue;
			
			CString strSound;
			if(szRawData[12]==0x00)
				strSound = "1";
			else
				strSound = "0";
			m_log.TraceLog(DBG,0,"Receive State{Type:09 Tower-type Sonic bird repeller,DeviceId:%X,BatteryValue:%s,PressureValue:%s,Sound:%s}",szRawData[4],strBatteryValue,strPressureValue,strSound);

			CString strDeviceId;
			strDeviceId.Format("E%02x",szRawData[4]);
			char szSQL[1024];
			memset(szSQL,0,sizeof(szSQL));
			sprintf(szSQL,"update t_device_state_realtime set runningstate='%d',powerstate='%d',batterystate='%s',pressurestate='%s',sound='%s',last_modify_dt=now() where deviceid='%s'",1,1,strBatteryValue,strPressureValue,strSound,strDeviceId);
			ExecuteSql(szSQL);

			float fBatteryValue = atof(strBatteryValue);
			float fPressureValue = -1;
			int nSound = atoi(strSound);
			
			float fBatteryStateLevel =0,fPressureStateLevel = 0;
			GetThresholdLevel(&fPressureStateLevel,&fBatteryStateLevel);
			if(fBatteryValue>fBatteryStateLevel)
			{
				char szSQL[1024];
				memset(szSQL,0,sizeof(szSQL));
				sprintf(szSQL,"update t_device_state_realtime set Batterystate_avaliable_dt=now() where deviceid='%s'",strDeviceId);
				ExecuteSql(szSQL);
			}
			
			if(nSound==1)
			{
				char szSQL[1024];
				memset(szSQL,0,sizeof(szSQL));
				sprintf(szSQL,"update t_device_state_realtime set soundstate_avaliable_dt=now() where deviceid='%s'",strDeviceId);
				ExecuteSql(szSQL);
			}
			RecordComMessageDt(strDeviceId);
			CString strRawData;
			WriteDeviceStateHistory(strDeviceId,1,1,strBatteryValue,strPressureValue,strRawData);
		}
		else
		{
			m_log.TraceLog(DBG,0, "Unknow Device");
		}
		return true;
	}


	return true;
}

unsigned char CDriveBirdsCenter::LRC_check(unsigned char *pSendBuf,unsigned char num)
{
	int8_t bylrc = 0,i = 0;
	
	for(i = 0; i < num; i++)
	{
		bylrc += pSendBuf[i];
	}
	bylrc = ~bylrc;
	bylrc++;
	
	return bylrc;
}

bool CDriveBirdsCenter::Com_StartDevice(const char* szDeviceId, const char* szDeviceName, const char* szDeviceType)
{
	m_log.TraceLog(KEY,0, "Com_StartDevice(Id:%s,Name:%s,Type:%s)", szDeviceId,szDeviceName,szDeviceType);
//	return false;

	if(strlen(szDeviceId)==0)
		return false;
	if(strlen(szDeviceType)==0||strlen(szDeviceType)!=2)
		return false;

	int nType = atoi(szDeviceType);
	char cType;
	switch(nType)
	{
	case 5:
		cType = 0x05;
		break;
	case 10:
		cType = 0x06;
		break;
	case 7:
		cType = 0x07;
		break;
	case 8:
		cType = 0x08;
		break;
	case 6:
		cType = 0x09;
		break;
	default:
		m_log.TraceLog(WRN,0, "unkown device type");
		return false;
	}
//	int nDeviceId = atoi(szDeviceId);
//	char cDeviceId = nDeviceId;
	CString strDeviceId = szDeviceId;
	int nDeviceId = atoi(strDeviceId.Right(2));
	unsigned char cDeviceId = BYTE(nDeviceId);
	string strTmp = strDeviceId.Right(2);
//	string strTmp ="0A";
	String2Hex(strTmp,&cDeviceId);
	m_log.TraceLog(DBG,0, "DeviceId:%s %d %02x",strDeviceId.Right(2),nDeviceId,cDeviceId);

	unsigned char data[9]={0x40,0x23,0x24,0x00,0x00,0x08,0x55,0xB3,0x00};
	data[3] = cType;
	data[4] = cDeviceId;
	unsigned char crc=LRC_check(data,sizeof(data));
	unsigned char senddata[10];
	CByteArray hexdata;
	for(int i=0;i<9;i++)
	{
		senddata[i]=data[i];
		hexdata.Add(data[i]);
	}
	senddata[9] = crc;
	hexdata.Add(crc);

	try
	{
	m_pComm->put_OutBufferCount(0);//清空发送缓冲区
	m_pComm->put_Output(COleVariant(hexdata));//发送十六进制数据
	}
	catch( CException *e)  
{
    m_log.TraceLog(WRN,0, "Catch Send Com Message");
    e->Delete();
	exit(0);
}
	return true;
}

bool CDriveBirdsCenter::Com_StopDevice(const char* szDeviceId, const char* szDeviceName, const char* szDeviceType)
{
	m_log.TraceLog(KEY,0, "Com_StopDevice(Id:%s,Name:%s,Type:%s)", szDeviceId,szDeviceName,szDeviceType);
//	return false;

	if(strlen(szDeviceId)==0)
		return false;
	if(strlen(szDeviceType)==0||strlen(szDeviceType)!=2)
		return false;

	int nType = atoi(szDeviceType);
	char cType;
	switch(nType)
	{
	case 5:
		cType = 0x05;
		break;
	case 10:
		cType = 0x06;
		break;
	case 7:
		cType = 0x07;
		break;
	case 8:
		cType = 0x08;
		break;
	case 6:
		cType = 0x09;
		break;
	default:
		m_log.TraceLog(WRN,0, "unkown device type");
		return false;
	}
//	int nDeviceId = atoi(szDeviceId);
//	char cDeviceId = nDeviceId;
	CString strDeviceId = szDeviceId;
	int nDeviceId = atoi(strDeviceId.Right(2));
	unsigned char cDeviceId = BYTE(nDeviceId);
	string strTmp = strDeviceId.Right(2);
//	string strTmp ="0A";
	String2Hex(strTmp,&cDeviceId);
	m_log.TraceLog(DBG,0, "DeviceId:%s %d %02x",strDeviceId.Right(2),nDeviceId,cDeviceId);

	unsigned char data[9]={0x40,0x23,0x24,0x00,0x00,0x08,0xAA,0xB3,0x00};
	data[3] = cType;
	data[4] = cDeviceId;
	unsigned char crc=LRC_check(data,sizeof(data));
	unsigned char senddata[10];
	CByteArray hexdata;
	for(int i=0;i<9;i++)
	{
		senddata[i]=data[i];
		hexdata.Add(data[i]);
	}
	senddata[9] = crc;
	hexdata.Add(crc);
	try
	{
	m_pComm->put_OutBufferCount(0);//清空发送缓冲区
	m_pComm->put_Output(COleVariant(hexdata));//发送十六进制数据
	}
	catch( CException *e)  
{
    m_log.TraceLog(WRN,0, "Catch Send Com Message");
    e->Delete();
	exit(0);
}
	return true;
}

bool CDriveBirdsCenter::String2Hex(std::string &src,unsigned char *dest)  
{  
    unsigned char hb;  
    unsigned char lb;  
  
    if(src.size()%2!=0)  
        return false;  
  
    transform(src.begin(), src.end(), src.begin(), toupper);  
  
    for(int i=0, j=0;i<src.size();i++)  
    {  
        hb=src[i];  
        if( hb>='A' && hb<='F' )  
            hb = hb - 'A' + 10;  
        else if( hb>='0' && hb<='9' )  
            hb = hb - '0';  
        else  
            return false;  
  
        i++;  
        lb=src[i];  
        if( lb>='A' && lb<='F' )  
            lb = lb - 'A' + 10;  
        else if( lb>='0' && lb<='9' )  
            lb = lb - '0';  
        else  
            return false;  
  
        dest[j++]=(hb<<4)|(lb);  
    }  
    return true;  
}  
bool CDriveBirdsCenter::Com_QueryDevice(const char* szDeviceId, const char* szDeviceName, const char* szDeviceType)
{
	m_log.TraceLog(KEY,0, "Com_QueryDevice(Id:%s,Name:%s,Type:%s)", szDeviceId,szDeviceName,szDeviceType);
	if(strlen(szDeviceId)==0||strlen(szDeviceId)<=2)
	{
		m_log.TraceLog(DBG,0,"DeviceId is wrong,return");
		return false;
	}
	if(strlen(szDeviceType)==0||strlen(szDeviceType)!=2)
	{
		m_log.TraceLog(DBG,0,"DeviceType is wrong,return");
		return false;
	}

	int nType = atoi(szDeviceType);
	char cType;
	switch(nType)
	{
	case 5:
		cType = 0x05;
		break;
	case 10:
		cType = 0x06;
		break;
	case 7:
		cType = 0x07;
		break;
	case 8:
		cType = 0x08;
		break;
	case 6:
		cType = 0x09;
		break;
	case 9:
		return false;
	default:
		m_log.TraceLog(WRN,0, "unkown device type");
		return false;
	}
	CString strDeviceId = szDeviceId;
	int nDeviceId = atoi(strDeviceId.Right(2));
	unsigned char cDeviceId = BYTE(nDeviceId);
	string strTmp = strDeviceId.Right(2);
//	string strTmp ="0A";
	String2Hex(strTmp,&cDeviceId);
	m_log.TraceLog(DBG,0, "DeviceId:%s %d %02x",strDeviceId.Right(2),nDeviceId,cDeviceId);

	unsigned char data[9]={0x40,0x23,0x24,0x00,0x00,0x04,0xAB,0x00,0x00};
	data[3] = cType;
	data[4] = cDeviceId;
	unsigned char crc=LRC_check(data,sizeof(data));
	unsigned char senddata[10];
	CByteArray hexdata;
	CString strBuffer,strTemp;
	for(int i=0;i<9;i++)
	{
		senddata[i]=data[i];
		hexdata.Add(data[i]);

		strTemp.Format(_T("0x%02x "),data[i]);  
		strBuffer += strTemp;
	}
	senddata[9] = crc;
	strTemp.Format(_T("0x%02x "),senddata[9]);  
	strBuffer += strTemp;
	hexdata.Add(crc);
	try
	{
	m_pComm->put_OutBufferCount(0);//清空发送缓冲区
	m_pComm->put_Output(COleVariant(hexdata));//发送十六进制数据
	m_log.TraceLog(KEY,0, "Send Com Message:%s",strBuffer);
	}
	catch( CException *e)  
{
    m_log.TraceLog(WRN,0, "Catch Send Com Message");
    e->Delete();
	exit(0);
}

	return true;
}


bool CDriveBirdsCenter::SetComm(CMscomm1* pComm)
{
	if(pComm==NULL)
	{
		m_log.TraceLog(KEY,0, "pComm is NULL");
		return false;
	}
	m_pComm = pComm;
	return true;
}


bool CDriveBirdsCenter::Com_Open(void)
{
	try
	{
	if(m_pComm==NULL)
	{
		m_log.TraceLog(KEY,0, "m_pComm is NULL");
		return false;
	}
	int nCom = GetPrivateProfileInt(_T("System"),_T("Com"),-1,CONFIGURE_FILE);

	if(m_pComm->get_PortOpen())//检测串口是否已经打开  
    {  
        m_pComm->put_PortOpen(FALSE);//若串口已经打开，则关闭串口  
    }  
    m_pComm->put_CommPort(nCom);//打开串口1  
    m_pComm->put_InBufferSize(1024);//串口接收缓冲区大小  
    m_pComm->put_OutBufferSize(1024);//串口发送缓冲去大小  
    m_pComm->put_InputLen(0);//数据发送长度  
    m_pComm->put_InputMode(1);//数据发送模式  
    m_pComm->put_RThreshold(1);//  
    m_pComm->put_Settings(_T("38400,n,8,1"));//串口设置  
	m_pComm->put_OutBufferCount(0);//清空输出缓冲区
    if(!m_pComm->get_PortOpen())//检测串口是否打开，若关闭则打开串口  
    {  
        m_pComm->put_PortOpen(TRUE);  
        m_log.TraceLog(KEY,0, "Com Open Success");  
    }  
    else  
    {  
        m_pComm->put_OutBufferCount(0);//清空输出缓冲区  
        m_log.TraceLog(ERR,0, "Com Open Fail"); 
		return false;
    }  
	}
	catch(...)
	{
		m_log.TraceLog(ERR,0, "Com Open Fail"); 
		return false;
	}
	return true;
}


bool CDriveBirdsCenter::Com_Close(void)
{
	if(m_pComm==NULL)
	{
		m_log.TraceLog(KEY,0, "m_pComm is NULL");
		return false;
	}

	if(m_pComm->get_PortOpen())//检测串口是否已经打开  
    {  
        m_pComm->put_PortOpen(FALSE);//若串口已经打开，则关闭串口  
    }
	return true;
}


void CDriveBirdsCenter::OnCommMessage(void)
{
	if(m_pComm==NULL)
	{
		m_log.TraceLog(KEY,0, "m_pComm is NULL");
		return;
	}
	static unsigned int cnt = 0;  
    VARIANT variant_inp;  
    COleSafeArray safearryay_inp;  
    long len,k;  
    unsigned int data[1024]={0};  
    byte rxdata[1024];  
    CString stremp,strBuffer;  
    if(m_pComm->get_CommEvent() == 2)  
    {  
		::Sleep(500);
        cnt++;  
        variant_inp = m_pComm->get_Input();  
		//m_pComm->put_OutBufferCount(0);//清空输出缓冲区
        safearryay_inp = variant_inp;  
        len = safearryay_inp.GetOneDimSize();  
		if(len>=1024)
		{
			m_log.TraceLog(WRN,0,"GetOneDimSize too big,ignore this message,size:%d",len);
			return;
		}
        for(k=0;k<len;k++)  
        {  
            safearryay_inp.GetElement(&k,rxdata+k);  
        }  
		m_log.TraceLog(INF,0, "Receive Original Message");
		printMessage(rxdata, len);
		ParseRawData(rxdata, len);
	/*	vector<int> vecTest;

		for (int i = 0; i < len - 2; i++)
		{
			if (rxdata[i] == 0x40 && rxdata[i + 1] == 0x23 && rxdata[i + 2] == 0x24)
			{
				vecTest.push_back(i);
			}
		}

		m_log.TraceLog(INF,0, "Message Count:%d", vecTest.size());
		if(vecTest.size()==0)
			return;
		int j;
		int nPos0;
		for (j = 1; j < vecTest.size(); j++)
		{
			m_log.TraceLog(INF, 0, "begin Parse Message %d:", j);
			nPos0 = vecTest[j - 1];
			printMessage(&rxdata[nPos0], vecTest[j] - vecTest[j - 1]);
			ParseRawData(&rxdata[nPos0], vecTest[j] - vecTest[j - 1]);
		}
		m_log.TraceLog(INF, 0, "begin Parse Message %d:", j);
		nPos0 = vecTest[j - 1];
		printMessage(&rxdata[nPos0], len - vecTest[j - 1]);
		ParseRawData(&rxdata[nPos0], len - vecTest[j - 1]);
	*/
    }  
}


bool CDriveBirdsCenter::GetThresholdLevel(float* pnPressure, float* pnVoltage)
{
	m_log.TraceLog(DBG,0, "begin GetThresholdLevel...");

	char szDSN[256];
	memset(szDSN,0,sizeof(szDSN));
	GetPrivateProfileString("Database", "DSN", "", szDSN, sizeof(szDSN), CONFIGURE_FILE);
	CDatabase db;	
	

	CString strIndex, strPressure, strVoltage, strSql;
	strSql.Format("SELECT id,pressure,voltage FROM t_threshold_level");
	CRecordset *pRecordSet = NULL;

	TRY
	{
		if(!db.OpenEx(szDSN,CDatabase::noOdbcDialog))
	{
		m_log.TraceLog(WRN,0,"Open Dsn:%s ERROR",szDSN);
		return false;
	}

		pRecordSet = new CRecordset;
		pRecordSet->m_pDatabase = &db;
		pRecordSet->Open(CRecordset::forwardOnly, strSql);
		if (!pRecordSet->IsEOF())
		{
			pRecordSet->GetFieldValue((short)0, strIndex);
			pRecordSet->GetFieldValue((short)1, strPressure);
			pRecordSet->GetFieldValue((short)2, strVoltage);
			*pnPressure= atof(strPressure);
			*pnVoltage = atof(strVoltage);
		}
		pRecordSet->Close();
		delete pRecordSet;
		pRecordSet = NULL;
		db.Close();
	}
	CATCH(CDBException, e)
	{
		if (pRecordSet != NULL)
		{
			if (pRecordSet->IsOpen())
				pRecordSet->Close();
			delete pRecordSet;
			pRecordSet = NULL;
		}
		m_log.TraceLog(ERR,0, "execute sql(%s) failed,Error:%s", strSql, e->m_strError);
		return false;
	}
	END_CATCH

	m_log.TraceLog(DBG,0, "end GetThresholdLevel(Pressure:%f,Voltage:%f)...",*pnPressure,*pnVoltage);
	return true;
}

bool CDriveBirdsCenter::RecordComMessageDt(const char* szDeviceId)
{
	CString strSql;
	strSql.Format("update t_device_state_realtime set get_message_dt=now() where deviceid='%s'",szDeviceId);
	ExecuteSql(strSql);

	return true;
}

bool CDriveBirdsCenter::SaveCurrentData(const char* szDeviceId, const char* szDeviceType,float fBatteryState,float fPressureState,int nBoardState,int nSoundState)
{
	return true;
}

bool CDriveBirdsCenter::DoWarning(const char* szDeviceId)
{
	int nStartTime = GetPrivateProfileInt(_T("Time"),_T("StartTime"),7,CONFIGURE_FILE);
	int nStopTime = GetPrivateProfileInt(_T("Time"),_T("StopTime"),22,CONFIGURE_FILE);
	SYSTEMTIME sysTime;
    GetLocalTime(&sysTime);
	int nTime = sysTime.wHour;
	if(nTime<nStartTime||nTime>nStopTime)
	{
		m_log.TraceLog(DBG,0,"SleepTime(%d-%d)",nStartTime,nStopTime);
//		::Sleep(60000);
		return true;
	}

	char szDeviceType[32];
	memset(szDeviceType,0,sizeof(szDeviceType));
	bool bRet = GetDeviceType(szDeviceId,szDeviceType);
	if(bRet==false)
	{
		m_log.TraceLog(WRN,0,"Device:%s GetDeviceType fail",szDeviceId);  
		return false;
	}


	int nType = atoi(szDeviceType);
	switch(nType)
	{
	case 5://煤气炮
		{
			CString strAlertReason;
			bool bWarning = false;
			bool bVoltageNormal = VoltageNormal(szDeviceId);
			bool bPressureNormal = PressureNormal(szDeviceId);
			bool bSoundNormal = SoundNormal(szDeviceId);
			bool bBoardNormal = BoardNormal(szDeviceId);
			if(bVoltageNormal==false)
			{
				if(bWarning)
					strAlertReason += ",3";
				else
					strAlertReason += "3";

				bWarning = true;
			}
			if(bPressureNormal==false)
			{
				if(bWarning)
					strAlertReason += ",1";
				else
					strAlertReason += "1";

				bWarning = true;
			}
			if(bSoundNormal==false)
			{
				if(bWarning)
					strAlertReason += ",4";
				else
					strAlertReason += "4";

				bWarning = true;
			}
			if(bBoardNormal==0)
			{
				strAlertReason = "2,4";

				bWarning = true;
			}
			if(bWarning)
			{
				WriteDeviceAlertLog(szDeviceId,bBoardNormal,bBoardNormal,"","",strAlertReason,"");
				CString strSql;
				strSql.Format("update t_device_state_realtime set alert='1',alert_reasons='%s',warning_flag_dt=now() where deviceid='%s' and alert<>'2'",strAlertReason,szDeviceId);
				ExecuteSql(strSql);
				strSql.Format("update t_device_state_realtime set alert_reasons='%s',warning_flag_dt=now() where deviceid='%s' and alert='2'",strAlertReason,szDeviceId);
				ExecuteSql(strSql);
				if(bBoardNormal==0)
				{
					//strSql.Format("update t_device_state_realtime set alert='1',alert_reasons='%s',warning_flag_dt=now() where deviceid='%s' and alert<>'2'",strAlertReason,szDeviceId);
					strSql.Format("update t_device_state_realtime set runningstate='0',last_modify_dt=now() where deviceid='%s'",szDeviceId);
					ExecuteSql(strSql);
				}
				
			}
			else
			{
				CString strSql;
				strSql.Format("update t_device_state_realtime set alert='0',alert_reasons='',Last_modify_dt =now(),warning_flag_dt=now() where deviceid='%s' and alert<>'0'",szDeviceId);
				ExecuteSql(strSql);
			}
		}
		break;
	case 10://移动声学
		{
			CString strAlertReason;
			bool bWarning = false;
			bool bVoltageNormal = VoltageNormal(szDeviceId);
			bool bSoundNormal = SoundNormal(szDeviceId);
			bool bBoardNormal = BoardNormal(szDeviceId);
			if(bVoltageNormal==false)
			{
				if(bWarning)
					strAlertReason += ",3";
				else
					strAlertReason += "3";

				bWarning = true;
			}
			if(bSoundNormal==false)
			{
				if(bWarning)
					strAlertReason += ",4";
				else
					strAlertReason += "4";

				bWarning = true;
			}
			if(bBoardNormal==0)
			{
				strAlertReason = "2,4";

				bWarning = true;
			}
			if(bWarning)
			{
				WriteDeviceAlertLog(szDeviceId,bBoardNormal,bBoardNormal,"","",strAlertReason,"");
				CString strSql;
				strSql.Format("update t_device_state_realtime set alert='1',alert_reasons='%s',warning_flag_dt=now() where deviceid='%s' and alert<>'2'",strAlertReason,szDeviceId);
				ExecuteSql(strSql);
				strSql.Format("update t_device_state_realtime set alert_reasons='%s',warning_flag_dt=now() where deviceid='%s' and alert='2'",strAlertReason,szDeviceId);
				ExecuteSql(strSql);
				if(bBoardNormal==0)
				{
					//strSql.Format("update t_device_state_realtime set alert='1',alert_reasons='%s',warning_flag_dt=now() where deviceid='%s' and alert<>'2'",strAlertReason,szDeviceId);
					strSql.Format("update t_device_state_realtime set runningstate='0',last_modify_dt=now() where deviceid='%s'",szDeviceId);
					ExecuteSql(strSql);
				}
			}
			else
			{
				CString strSql;
				strSql.Format("update t_device_state_realtime set alert='0',alert_reasons='',Last_modify_dt =now(),warning_flag_dt=now() where deviceid='%s' and alert<>'0'",szDeviceId);
				ExecuteSql(strSql);
			}
		}
		break;
	case 7://定向
		{
			CString strAlertReason;
			bool bWarning = false;
			bool bSoundNormal = SoundNormal(szDeviceId);
			bool bBoardNormal = BoardNormal(szDeviceId);

			if(bSoundNormal==false)
			{
				if(bWarning)
					strAlertReason = ",4";
				else
					strAlertReason += "4";

				bWarning = true;
			}
			if(bBoardNormal==0)
			{
				strAlertReason = "2,4";

				bWarning = true;
			}
			if(bWarning)
			{
				WriteDeviceAlertLog(szDeviceId,bBoardNormal,bBoardNormal,"","",strAlertReason,"");
				CString strSql;
				strSql.Format("update t_device_state_realtime set alert='1',alert_reasons='%s',warning_flag_dt=now() where deviceid='%s' and alert<>'2'",strAlertReason,szDeviceId);
				ExecuteSql(strSql);
				strSql.Format("update t_device_state_realtime set alert_reasons='%s',warning_flag_dt=now() where deviceid='%s' and alert='2'",strAlertReason,szDeviceId);
				ExecuteSql(strSql);
				if(bBoardNormal==0)
				{
					//strSql.Format("update t_device_state_realtime set alert='1',alert_reasons='%s',warning_flag_dt=now() where deviceid='%s' and alert<>'2'",strAlertReason,szDeviceId);
					strSql.Format("update t_device_state_realtime set runningstate='0',last_modify_dt=now() where deviceid='%s'",szDeviceId);
					ExecuteSql(strSql);
				}
			}
			else
			{
				CString strSql;
				strSql.Format("update t_device_state_realtime set alert='0',alert_reasons='',Last_modify_dt =now(),warning_flag_dt=now() where deviceid='%s' and alert<>'0'",szDeviceId);
				ExecuteSql(strSql);
			}
		}
		break;
	case 8://全向
		{
			CString strAlertReason;
			bool bWarning = false;
			bool bVoltageNormal = VoltageQuanXiang(szDeviceId);
			bool bSoundNormal = SoundQuanXiang(szDeviceId);
			bool bBoardNormal = BoardQuanXiang(szDeviceId);
			if(bVoltageNormal==false)
			{
				if(bWarning)
					strAlertReason += ",3";
				else
					strAlertReason += "3";

				bWarning = true;
			}

			if(bSoundNormal==false)
			{
				if(bWarning)
					strAlertReason += ",4";
				else
					strAlertReason += "4";

				bWarning = true;
			}
			if(bBoardNormal==0)
			{
				strAlertReason = "2,4";

				bWarning = true;
			}
			if(bWarning)
			{
				WriteDeviceAlertLog(szDeviceId,bBoardNormal,bBoardNormal,"","",strAlertReason,"");
				CString strSql;
				strSql.Format("update t_device_state_realtime set alert='1',alert_reasons='%s',warning_flag_dt=now() where deviceid='%s' and alert<>'2'",strAlertReason,szDeviceId);
				ExecuteSql(strSql);
				strSql.Format("update t_device_state_realtime set alert_reasons='%s',warning_flag_dt=now() where deviceid='%s' and alert='2'",strAlertReason,szDeviceId);
				ExecuteSql(strSql);
				if(bBoardNormal==0)
				{
					//strSql.Format("update t_device_state_realtime set alert='1',alert_reasons='%s',warning_flag_dt=now() where deviceid='%s' and alert<>'2'",strAlertReason,szDeviceId);
					strSql.Format("update t_device_state_realtime set runningstate='0',last_modify_dt=now() where deviceid='%s'",szDeviceId);
					ExecuteSql(strSql);
				}
			}
			else
			{
				CString strSql;
				strSql.Format("update t_device_state_realtime set alert='0',alert_reasons='',Last_modify_dt =now(),warning_flag_dt=now() where deviceid='%s' and alert<>'0'",szDeviceId);
				ExecuteSql(strSql);
			}
		}
		break;
	case 6://塔式
		{
			CString strAlertReason;
			bool bWarning = false;
			bool bVoltageNormal = VoltageNormal(szDeviceId);
			bool bSoundNormal = SoundNormal(szDeviceId);
			bool bBoardNormal = BoardNormal(szDeviceId);
			if(bVoltageNormal==false)
			{
				if(bWarning)
					strAlertReason += ",3";
				else
					strAlertReason += "3";

				bWarning = true;
			}
			if(bSoundNormal==false)
			{
				if(bWarning)
					strAlertReason += ",4";
				else
					strAlertReason += "4";

				bWarning = true;
			}
			if(bBoardNormal==0)
			{
				strAlertReason = "2,4";

				bWarning = true;
			}
			if(bWarning)
			{
				WriteDeviceAlertLog(szDeviceId,bBoardNormal,bBoardNormal,"","",strAlertReason,"");
				CString strSql;
				strSql.Format("update t_device_state_realtime set alert='1',alert_reasons='%s',warning_flag_dt=now() where deviceid='%s' and alert<>'2'",strAlertReason,szDeviceId);
				ExecuteSql(strSql);
				strSql.Format("update t_device_state_realtime set alert_reasons='%s',warning_flag_dt=now() where deviceid='%s' and alert='2'",strAlertReason,szDeviceId);
				ExecuteSql(strSql);
				if(bBoardNormal==0)
				{
					//strSql.Format("update t_device_state_realtime set alert='1',alert_reasons='%s',warning_flag_dt=now() where deviceid='%s' and alert<>'2'",strAlertReason,szDeviceId);
					strSql.Format("update t_device_state_realtime set runningstate='0',last_modify_dt=now() where deviceid='%s'",szDeviceId);
					ExecuteSql(strSql);
				}
			}
			else
			{
				CString strSql;
				strSql.Format("update t_device_state_realtime set alert='0',alert_reasons='',Last_modify_dt =now(),warning_flag_dt=now() where deviceid='%s' and alert<>'0'",szDeviceId);
				ExecuteSql(strSql);
			}
		}
		break;
	default:
		m_log.TraceLog(WRN,0, "warning3 unkown device type");
		return false;
	}

	return true;
}
bool CDriveBirdsCenter::GetDeviceType(const char* szDeviceId, char* szDeviceType)
{
	m_log.TraceLog(DBG,0, "begin GetDeviceType...");
	strcpy(szDeviceType,"");
	bool bRet = false;

	char szDSN[256];
	memset(szDSN,0,sizeof(szDSN));
	GetPrivateProfileString("Database", "DSN", "", szDSN, sizeof(szDSN), CONFIGURE_FILE);
	CDatabase db;	
	

	CString strIndex, strDeviceType, strSql;
	strSql.Format("SELECT id,devicetype FROM t_device_list where deviceid='%s'",szDeviceId);
	CRecordset *pRecordSet = NULL;

	TRY
	{
		if(!db.OpenEx(szDSN,CDatabase::noOdbcDialog))
	{
		m_log.TraceLog(WRN,0,"Open Dsn:%s ERROR",szDSN);
		return false;
	}
		pRecordSet = new CRecordset;
		pRecordSet->m_pDatabase = &db;
		pRecordSet->Open(CRecordset::forwardOnly, strSql);
		if (!pRecordSet->IsEOF())
		{
			pRecordSet->GetFieldValue((short)0, strIndex);
			pRecordSet->GetFieldValue((short)1, strDeviceType);
			strcpy(szDeviceType,strDeviceType);
			bRet = true;
		}
		pRecordSet->Close();
		delete pRecordSet;
		pRecordSet = NULL;
		db.Close();
	}
	CATCH(CDBException, e)
	{
		if (pRecordSet != NULL)
		{
			if (pRecordSet->IsOpen())
				pRecordSet->Close();
			delete pRecordSet;
			pRecordSet = NULL;
		}
		m_log.TraceLog(ERR,0, "execute sql(%s) failed,Error:%s", strSql, e->m_strError);
		return false;
	}
	END_CATCH

	m_log.TraceLog(DBG,0, "end GetDeviceType(DeviceId:%s,DeviceType:%s)...",szDeviceId,szDeviceType);
	return bRet;
}

bool CDriveBirdsCenter::StartDriveBirds(const char* szDeviceId, const char* szDeviceName, const char* szDeviceType)
{
	m_log.TraceLog(DBG,0, "begin StartDriveBirds(DeviceId:%s,DeviceType:%s)...",szDeviceId,szDeviceType);
	
	paramDrive *pParamDrive = new paramDrive(szDeviceId,szDeviceType,this);
	HANDLE tDriveBirds = (void*)_beginthreadex(NULL, 0, threadStartDriveBirds, pParamDrive, 0, NULL);

	::CloseHandle(tDriveBirds);
	m_log.TraceLog(DBG,0, "end StartDriveBirds(DeviceId:%s,DeviceType:%s)...",szDeviceId,szDeviceType);
	return true;
}

bool CDriveBirdsCenter::VoltageNormal(const char* szDeviceId)
{
	int nInterval = GetPrivateProfileInt(_T("Time"),_T("VoltageWarningInterval"),600,CONFIGURE_FILE);

	char szDSN[256];
	memset(szDSN,0,sizeof(szDSN));
	GetPrivateProfileString("Database", "DSN", "", szDSN, sizeof(szDSN), CONFIGURE_FILE);
	CDatabase db;	
	
	CString strIndex, strSql;
//	strSql.Format("select id from t_device_state_realtime where deviceid='%s' and time_to_sec(now())-time_to_sec(Batterystate_avaliable_dt)<%d",szDeviceId,nInterval);
	strSql.Format("select id from t_device_state_realtime where deviceid='%s' and TIMESTAMPDIFF(second,Batterystate_avaliable_dt,now())<%d",szDeviceId,nInterval);
//	TIMESTAMPDIFF(second,get_message_dt,now())
	CRecordset *pRecordSet = NULL;
	bool bRet = true;
	TRY
	{
		if(!db.OpenEx(szDSN,CDatabase::noOdbcDialog))
	{
		m_log.TraceLog(WRN,0,"Open Dsn:%s ERROR",szDSN);
		return true;
	}
		pRecordSet = new CRecordset;
		pRecordSet->m_pDatabase = &db;
		pRecordSet->Open(CRecordset::forwardOnly, strSql);
		if (!pRecordSet->IsEOF())
		{
			pRecordSet->GetFieldValue((short)0, strIndex);
			bRet = true;
		}
		else
		{
			bRet = false;
		}
		pRecordSet->Close();
		delete pRecordSet;
		pRecordSet = NULL;
		db.Close();
	}
	CATCH(CDBException, e)
	{
		if (pRecordSet != NULL)
		{
			if (pRecordSet->IsOpen())
				pRecordSet->Close();
			delete pRecordSet;
			pRecordSet = NULL;
		}
		m_log.TraceLog(ERR,0, "execute sql(%s) failed,Error:%s", strSql, e->m_strError);
		return false;
	}
	END_CATCH
	return bRet;
}

bool CDriveBirdsCenter::PressureNormal(const char* szDeviceId)
{
	int nInterval = GetPrivateProfileInt(_T("Time"),_T("PressureWarningInterval"),600,CONFIGURE_FILE);

	char szDSN[256];
	memset(szDSN,0,sizeof(szDSN));
	GetPrivateProfileString("Database", "DSN", "", szDSN, sizeof(szDSN), CONFIGURE_FILE);
	CDatabase db;	
	
	CString strIndex, strSql;
//	strSql.Format("select id from t_device_state_realtime where deviceid='%s' and time_to_sec(now())-time_to_sec(pressurestate_avaliable_dt)<%d",szDeviceId,nInterval);
	strSql.Format("select id from t_device_state_realtime where deviceid='%s' and TIMESTAMPDIFF(second,pressurestate_avaliable_dt,now())<%d",szDeviceId,nInterval);
//	TIMESTAMPDIFF(second,get_message_dt,now())
	CRecordset *pRecordSet = NULL;
	bool bRet = true;
	TRY
	{
		if(!db.OpenEx(szDSN,CDatabase::noOdbcDialog))
	{
		m_log.TraceLog(WRN,0,"Open Dsn:%s ERROR",szDSN);
		return true;
	}
		pRecordSet = new CRecordset;
		pRecordSet->m_pDatabase = &db;
		pRecordSet->Open(CRecordset::forwardOnly, strSql);
		if (!pRecordSet->IsEOF())
		{
			pRecordSet->GetFieldValue((short)0, strIndex);
			bRet = true;
		}
		else
		{
			bRet = false;
		}
		pRecordSet->Close();
		delete pRecordSet;
		pRecordSet = NULL;
		db.Close();
	}
	CATCH(CDBException, e)
	{
		if (pRecordSet != NULL)
		{
			if (pRecordSet->IsOpen())
				pRecordSet->Close();
			delete pRecordSet;
			pRecordSet = NULL;
		}
		m_log.TraceLog(ERR,0, "execute sql(%s) failed,Error:%s", strSql, e->m_strError);
		return false;
	}
	END_CATCH
	return bRet;
}

bool CDriveBirdsCenter::SoundNormal(const char* szDeviceId)
{
	int nInterval = GetPrivateProfileInt(_T("Time"),_T("SoundWarningInterval"),300,CONFIGURE_FILE);

	char szDSN[256];
	memset(szDSN,0,sizeof(szDSN));
	GetPrivateProfileString("Database", "DSN", "", szDSN, sizeof(szDSN), CONFIGURE_FILE);
	CDatabase db;	
	
	CString strIndex, strSql;
//	strSql.Format("select id from t_device_state_realtime where deviceid='%s' and time_to_sec(now())-time_to_sec(soundstate_avaliable_dt)<%d",szDeviceId,nInterval);
	strSql.Format("select id from t_device_state_realtime where deviceid='%s' and TIMESTAMPDIFF(second,soundstate_avaliable_dt,now())<%d",szDeviceId,nInterval);
//	TIMESTAMPDIFF(second,get_message_dt,now())
	CRecordset *pRecordSet = NULL;
	bool bRet = true;
	TRY
	{
		if(!db.OpenEx(szDSN,CDatabase::noOdbcDialog))
	{
		m_log.TraceLog(WRN,0,"Open Dsn:%s ERROR",szDSN);
		return true;
	}
		pRecordSet = new CRecordset;
		pRecordSet->m_pDatabase = &db;
		pRecordSet->Open(CRecordset::forwardOnly, strSql);
		if (!pRecordSet->IsEOF())
		{
			pRecordSet->GetFieldValue((short)0, strIndex);
			bRet = true;
		}
		else
		{
			bRet = false;
		}
		pRecordSet->Close();
		delete pRecordSet;
		pRecordSet = NULL;
		db.Close();
	}
	CATCH(CDBException, e)
	{
		if (pRecordSet != NULL)
		{
			if (pRecordSet->IsOpen())
				pRecordSet->Close();
			delete pRecordSet;
			pRecordSet = NULL;
		}
		m_log.TraceLog(ERR,0, "execute sql(%s) failed,Error:%s", strSql, e->m_strError);
		return false;
	}
	END_CATCH
	return bRet;
}

bool CDriveBirdsCenter::BoardNormal(const char* szDeviceId)
{
	int nInterval = GetPrivateProfileInt(_T("Time"),_T("ComWarningInterval"),300,CONFIGURE_FILE);

	char szDSN[256];
	memset(szDSN,0,sizeof(szDSN));
	GetPrivateProfileString("Database", "DSN", "", szDSN, sizeof(szDSN), CONFIGURE_FILE);
	CDatabase db;	
	
	CString strIndex, strSql;
//	strSql.Format("select id from t_device_state_realtime where deviceid='%s' and time_to_sec(now())-time_to_sec(get_message_dt)<%d",szDeviceId,nInterval);
	strSql.Format("select id from t_device_state_realtime where deviceid='%s' and TIMESTAMPDIFF(second,get_message_dt,now())<%d",szDeviceId,nInterval);
//	TIMESTAMPDIFF(second,get_message_dt,now())
	CRecordset *pRecordSet = NULL;
	bool bRet = true;
	TRY
	{
		if(!db.OpenEx(szDSN,CDatabase::noOdbcDialog))
	{
		m_log.TraceLog(WRN,0,"Open Dsn:%s ERROR",szDSN);
		return true;
	}
		pRecordSet = new CRecordset;
		pRecordSet->m_pDatabase = &db;
		pRecordSet->Open(CRecordset::forwardOnly, strSql);
		if (!pRecordSet->IsEOF())
		{
			pRecordSet->GetFieldValue((short)0, strIndex);
			bRet = true;
		}
		else
		{
			bRet = false;
		}
		pRecordSet->Close();
		delete pRecordSet;
		pRecordSet = NULL;
		db.Close();
	}
	CATCH(CDBException, e)
	{
		if (pRecordSet != NULL)
		{
			if (pRecordSet->IsOpen())
				pRecordSet->Close();
			delete pRecordSet;
			pRecordSet = NULL;
		}
		m_log.TraceLog(ERR,0, "execute sql(%s) failed,Error:%s", strSql, e->m_strError);
		return false;
	}
	END_CATCH
	return bRet;
}

bool CDriveBirdsCenter::IfExistInDb(const char* szDeviceId, const char* szTableName)
{
	char szDSN[256];
	memset(szDSN,0,sizeof(szDSN));
	GetPrivateProfileString("Database", "DSN", "", szDSN, sizeof(szDSN), CONFIGURE_FILE);
	CDatabase db;	
	
	int nMin = GetPrivateProfileInt(_T("Time"),_T("MinAlertInterval"),60,CONFIGURE_FILE);
	CString strIndex, strSql;
	strSql.Format("select id from %s where deviceid='%s' and TIMESTAMPDIFF(second,write_dt,now())<%d",szTableName,szDeviceId,nMin);
	CRecordset *pRecordSet = NULL;
	bool bRet = true;
	TRY
	{
		if(!db.OpenEx(szDSN,CDatabase::noOdbcDialog))
	{
		m_log.TraceLog(WRN,0,"Open Dsn:%s ERROR",szDSN);
		return true;
	}
		pRecordSet = new CRecordset;
		pRecordSet->m_pDatabase = &db;
		pRecordSet->Open(CRecordset::forwardOnly, strSql);
		if (!pRecordSet->IsEOF())
		{
			pRecordSet->GetFieldValue((short)0, strIndex);
			bRet = true;
		}
		else
		{
			bRet = false;
		}
		pRecordSet->Close();
		delete pRecordSet;
		pRecordSet = NULL;
		db.Close();
	}
	CATCH(CDBException, e)
	{
		if (pRecordSet != NULL)
		{
			if (pRecordSet->IsOpen())
				pRecordSet->Close();
			delete pRecordSet;
			pRecordSet = NULL;
		}
		m_log.TraceLog(ERR,0, "execute sql(%s) failed,Error:%s", strSql, e->m_strError);
		return false;
	}
	END_CATCH
	return bRet;
}

bool CDriveBirdsCenter::VoltageQuanXiang(const char* szDeviceId)
{
	int nInterval = GetPrivateProfileInt(_T("Time"),_T("VoltageWarningInterval"),600,CONFIGURE_FILE);

	char szDSN[256];
	memset(szDSN,0,sizeof(szDSN));
	GetPrivateProfileString("Database", "DSN", "", szDSN, sizeof(szDSN), CONFIGURE_FILE);
	CDatabase db;	
	
	CString strIndex, strSql;
//	strSql.Format("select id from t_device_state_realtime where deviceid='%s' and time_to_sec(now())-time_to_sec(Batterystate_avaliable_dt)<%d",szDeviceId,nInterval);
	strSql.Format("select id from t_device_state_realtime where deviceid='%s' and TIMESTAMPDIFF(second,Batterystate_avaliable_dt,now())<%d",szDeviceId,nInterval);
//	TIMESTAMPDIFF(second,get_message_dt,now())
	CRecordset *pRecordSet = NULL;
	bool bRet = true;
	TRY
	{
		if(!db.OpenEx(szDSN,CDatabase::noOdbcDialog))
	{
		m_log.TraceLog(WRN,0,"Open Dsn:%s ERROR",szDSN);
		return true;
	}
		pRecordSet = new CRecordset;
		pRecordSet->m_pDatabase = &db;
		pRecordSet->Open(CRecordset::forwardOnly, strSql);
		if (!pRecordSet->IsEOF())
		{
			pRecordSet->GetFieldValue((short)0, strIndex);
			bRet = true;
		}
		else
		{
			bRet = false;
		}
		pRecordSet->Close();
		delete pRecordSet;
		pRecordSet = NULL;
		db.Close();
	}
	CATCH(CDBException, e)
	{
		if (pRecordSet != NULL)
		{
			if (pRecordSet->IsOpen())
				pRecordSet->Close();
			delete pRecordSet;
			pRecordSet = NULL;
		}
		m_log.TraceLog(ERR,0, "execute sql(%s) failed,Error:%s", strSql, e->m_strError);
		return false;
	}
	END_CATCH
	return bRet;
}

bool CDriveBirdsCenter::PressureQuanXiang(const char* szDeviceId)
{
	int nInterval = GetPrivateProfileInt(_T("Time"),_T("PressureWarningInterval"),600,CONFIGURE_FILE);

	char szDSN[256];
	memset(szDSN,0,sizeof(szDSN));
	GetPrivateProfileString("Database", "DSN", "", szDSN, sizeof(szDSN), CONFIGURE_FILE);
	CDatabase db;	
	
	CString strIndex, strSql;
//	strSql.Format("select id from t_device_state_realtime where deviceid='%s' and time_to_sec(now())-time_to_sec(pressurestate_avaliable_dt)<%d",szDeviceId,nInterval);
	strSql.Format("select id from t_device_state_realtime where deviceid='%s' and TIMESTAMPDIFF(second,pressurestate_avaliable_dt,now())<%d",szDeviceId,nInterval);
//	TIMESTAMPDIFF(second,get_message_dt,now())
	CRecordset *pRecordSet = NULL;
	bool bRet = true;
	TRY
	{
		if(!db.OpenEx(szDSN,CDatabase::noOdbcDialog))
	{
		m_log.TraceLog(WRN,0,"Open Dsn:%s ERROR",szDSN);
		return true;
	}
		pRecordSet = new CRecordset;
		pRecordSet->m_pDatabase = &db;
		pRecordSet->Open(CRecordset::forwardOnly, strSql);
		if (!pRecordSet->IsEOF())
		{
			pRecordSet->GetFieldValue((short)0, strIndex);
			bRet = true;
		}
		else
		{
			bRet = false;
		}
		pRecordSet->Close();
		delete pRecordSet;
		pRecordSet = NULL;
		db.Close();
	}
	CATCH(CDBException, e)
	{
		if (pRecordSet != NULL)
		{
			if (pRecordSet->IsOpen())
				pRecordSet->Close();
			delete pRecordSet;
			pRecordSet = NULL;
		}
		m_log.TraceLog(ERR,0, "execute sql(%s) failed,Error:%s", strSql, e->m_strError);
		return false;
	}
	END_CATCH
	return bRet;
}

bool CDriveBirdsCenter::SoundQuanXiang(const char* szDeviceId)
{
	int nInterval = GetPrivateProfileInt(_T("Time"),_T("SoundWarningInterval"),5400,CONFIGURE_FILE);
/*	bool bTest = GetPrivateProfileInt(_T("SoundTest"),szDeviceId,0,CONFIGURE_FILE);
	if(bTest==true)
	{
		return true;
	}
*/
	char szDSN[256];
	memset(szDSN,0,sizeof(szDSN));
	GetPrivateProfileString("Database", "DSN", "", szDSN, sizeof(szDSN), CONFIGURE_FILE);
	CDatabase db;	
	
	CString strIndex, strSql;
//	strSql.Format("select id from t_device_state_realtime where deviceid='%s' and time_to_sec(now())-time_to_sec(soundstate_avaliable_dt)<%d",szDeviceId,nInterval);
	strSql.Format("select id from t_device_state_realtime where deviceid='%s' and TIMESTAMPDIFF(second,soundstate_avaliable_dt,now())<%d",szDeviceId,nInterval);
//	TIMESTAMPDIFF(second,get_message_dt,now())
	CRecordset *pRecordSet = NULL;
	bool bRet = true;
	TRY
	{
		if(!db.OpenEx(szDSN,CDatabase::noOdbcDialog))
	{
		m_log.TraceLog(WRN,0,"Open Dsn:%s ERROR",szDSN);
		return true;
	}
		pRecordSet = new CRecordset;
		pRecordSet->m_pDatabase = &db;
		pRecordSet->Open(CRecordset::forwardOnly, strSql);
		if (!pRecordSet->IsEOF())
		{
			pRecordSet->GetFieldValue((short)0, strIndex);
			bRet = true;
		}
		else
		{
			bRet = false;
		}
		pRecordSet->Close();
		delete pRecordSet;
		pRecordSet = NULL;
		db.Close();
	}
	CATCH(CDBException, e)
	{
		if (pRecordSet != NULL)
		{
			if (pRecordSet->IsOpen())
				pRecordSet->Close();
			delete pRecordSet;
			pRecordSet = NULL;
		}
		m_log.TraceLog(ERR,0, "execute sql(%s) failed,Error:%s", strSql, e->m_strError);
		return false;
	}
	END_CATCH
	return bRet;
}

bool CDriveBirdsCenter::BoardQuanXiang(const char* szDeviceId)
{
	int nInterval = GetPrivateProfileInt(_T("Time"),_T("ComWarningInterval"),5400,CONFIGURE_FILE);

	char szDSN[256];
	memset(szDSN,0,sizeof(szDSN));
	GetPrivateProfileString("Database", "DSN", "", szDSN, sizeof(szDSN), CONFIGURE_FILE);
	CDatabase db;	
	
	CString strIndex, strSql;
//	strSql.Format("select id from t_device_state_realtime where deviceid='%s' and time_to_sec(now())-time_to_sec(get_message_dt)<%d",szDeviceId,nInterval);
	strSql.Format("select id from t_device_state_realtime where deviceid='%s' and TIMESTAMPDIFF(second,get_message_dt,now())<%d",szDeviceId,nInterval);
//	TIMESTAMPDIFF(second,get_message_dt,now())
	CRecordset *pRecordSet = NULL;
	bool bRet = true;
	TRY
	{
		if(!db.OpenEx(szDSN,CDatabase::noOdbcDialog))
	{
		m_log.TraceLog(WRN,0,"Open Dsn:%s ERROR",szDSN);
		return true;
	}
		pRecordSet = new CRecordset;
		pRecordSet->m_pDatabase = &db;
		pRecordSet->Open(CRecordset::forwardOnly, strSql);
		if (!pRecordSet->IsEOF())
		{
			pRecordSet->GetFieldValue((short)0, strIndex);
			bRet = true;
		}
		else
		{
			bRet = false;
		}
		pRecordSet->Close();
		delete pRecordSet;
		pRecordSet = NULL;
		db.Close();
	}
	CATCH(CDBException, e)
	{
		if (pRecordSet != NULL)
		{
			if (pRecordSet->IsOpen())
				pRecordSet->Close();
			delete pRecordSet;
			pRecordSet = NULL;
		}
		m_log.TraceLog(ERR,0, "execute sql(%s) failed,Error:%s", strSql, e->m_strError);
		return false;
	}
	END_CATCH
	return bRet;
}

void CDriveBirdsCenter::printMessage(unsigned char* byteTest, int nLen)
{
	CString strBuffer;
	for (int k = 0; k<nLen; k++)
	{
		BYTE bt = *(byteTest + k);
		CString strTemp;
		strTemp.Format(_T("0x%02x "), bt);
		strBuffer += strTemp;
	}
	m_log.TraceLog(INF, 0, "Com Data:%s\r\n", strBuffer.GetBuffer(0));
}