
#include ".\commonused.h"

#pragma comment(lib, "shell32.lib")
#include <shlobj.h>

 //获取系统目录（获取应用程序数据目录）
void commonused::GetAppDataPath(CString &path) 
{
	LPITEMIDLIST  pidl;
	LPMALLOC      pShellMalloc;
	char          szDir[MAX_PATH];

	if(SUCCEEDED(SHGetMalloc(&pShellMalloc)))
	{
		if(SUCCEEDED(SHGetSpecialFolderLocation(NULL,CSIDL_APPDATA,&pidl)))
		{
			// 如果成功返回true
			if(SHGetPathFromIDList(pidl, szDir))
			{
				path = szDir;
			}
			pShellMalloc->Free(pidl);
		}
		pShellMalloc->Release();
	}
}

// 日志文件
void commonused::LogFile(char *data)
{
	//  获取系统时间，精确到毫秒
	SYSTEMTIME sys;          
	GetLocalTime( &sys );     
	 
	// 获取应用程序数据目录
	CString t_AppDataPath;
	GetAppDataPath(t_AppDataPath);
	t_AppDataPath.Append(_T("\\DaemonLog.txt"));
//	char *t_AppDataPathChar = UnicodeToAnsi(t_AppDataPath);

	FILE *fp = fopen(t_AppDataPath,_T("a+"));
//	delete t_AppDataPathChar;
	if(fp!= NULL)
	{
		fprintf(fp,"%4d/%02d/%02d %02d:%02d:%02d.%03d 星期%1d %s\n",sys.wYear, sys.wMonth, sys.wDay, sys.wHour, sys.wMinute, sys.wSecond, sys.wMilliseconds, sys.wDayOfWeek, data);
		fclose(fp);
	}
}

//  写日志文件，时间：事件
void commonused::LogFile(WCHAR *data)
{
	//  获取系统时间，精确到毫秒
	SYSTEMTIME sys;          
	GetLocalTime( &sys );     

	// 获取应用程序数据目录
	CString t_AppDataPath;
	GetAppDataPath(t_AppDataPath);
	t_AppDataPath.Append(_T("\\DaemonLog.txt"));
//	char *t_AppDataPathChar = UnicodeToAnsi(t_AppDataPath);

	FILE *fp = fopen(t_AppDataPath,"a+");
//	delete t_AppDataPathChar;
	if(fp!= NULL)
	{
		char* t_char = UnicodeToAnsi(data);
		fprintf(fp,"%4d/%02d/%02d %02d:%02d:%02d.%03d 星期%1d %s\n",sys.wYear, sys.wMonth, sys.wDay, sys.wHour, sys.wMinute, sys.wSecond, sys.wMilliseconds, sys.wDayOfWeek, t_char);
		fclose(fp);
		delete t_char;
	}
}

int commonused::MacPowerOn(char *macAdress) 
{ 
	//检查MAC地址是否正确 
	for(char * a=macAdress; *a; a++) 
		if(*a!= '-' && !isxdigit(*a)) 
		{ 
			LogFile("PowerOnCall:MAC Adresse must be like this:00-D0-4C-BF-52-BA!");
			return 1; 
		} 

		int dstaddr[6]; 
		int i=sscanf(macAdress,"%2x-%2x-%2x-%2x-%2x-%2x ",&dstaddr[0], &dstaddr[1], &dstaddr[2], &dstaddr[3], &dstaddr[4], &dstaddr[5]); 
		if(i!=6) 
		{ 
			fprintf(stderr,"Invalid MAC Adresse! "); 
			return 1; 
		} 

		unsigned char ether_addr[6]; 
		for (i=0; i<6; i++) 
			ether_addr[i]=dstaddr[i]; 

		//构造Magic Packet 
		unsigned char magicpacket[200]; 
		memset(magicpacket, 0xff, 6); 
		int packetsize=6; 
		for(i=0; i<16; i++) 
		{ 
			memcpy(magicpacket+packetsize, ether_addr, 6); 
			packetsize+=6; 
		} 

		//启动WSA 
		WSADATA WSAData; 
		if(WSAStartup( MAKEWORD(2,0), &WSAData)!=0) 
		{ 
			fprintf(stderr, "WSAStartup failed:%d\n ", GetLastError()); 
			return 1; 
		} 
		else
			LogFile("PowerOnCall:WSAStartup Ok");

		//创建socket 
		SOCKET sock=socket(AF_INET, SOCK_DGRAM, 0); 
		if(sock==INVALID_SOCKET) 
		{ 
			fprintf(stderr, "Socket create error: %d\n ", GetLastError()); 
			return 1; 
		} 
		else
			LogFile("PowerOnCall:Socket create Ok");

		//设置为广播发送 
		BOOL bOptVal=TRUE; 
		int iOptLen=sizeof(BOOL); 
		if(setsockopt(sock, SOL_SOCKET, SO_BROADCAST, (char*)&bOptVal, iOptLen)==SOCKET_ERROR) 
		{ 
			fprintf(stderr, "setsockopt error: %d\n", WSAGetLastError()); 
			closesocket(sock); 
			WSACleanup(); 
			return 1; 
		} 

		sockaddr_in to; 
		to.sin_family=AF_INET; 
		to.sin_port=htons(0); 
		to.sin_addr.s_addr=htonl(INADDR_BROADCAST); 

		//发送Magic Packet 
		if(sendto(sock, (const char *)magicpacket, packetsize, 0, (const struct sockaddr *)&to, sizeof(to))==SOCKET_ERROR)   
			fprintf(stderr, "Magic packet send error: %d ", WSAGetLastError()); 
		else 
		{
			LogFile("PowerOnCall:Magic packet send!");
		}
		closesocket(sock); 
		WSACleanup(); 
		return   0; 
} 

// Unicode下Unicode转换为ANSI
char* commonused::UnicodeToAnsi(const WCHAR *source)
{
	int byteLen = WideCharToMultiByte(CP_ACP,0,source,-1,NULL,0,NULL,NULL); //预转换，得到所需空间的大小（单位为字节） 
	char* destination = new char[byteLen+1];
	memset(destination, 0, byteLen+1);
	WideCharToMultiByte(CP_ACP,0,source,-1,destination,byteLen,NULL,NULL);	//宽字节编码转换成多字节编码
	return destination;
}

// Unicode下ANSI转换为Unicode
WCHAR * commonused::AnsiToUnicode(const char *source)
{
	int byteLen = ::MultiByteToWideChar(CP_ACP, NULL, source, -1, NULL, 0); 
	wchar_t* destination = new wchar_t[byteLen + 1]; 
	memset(destination, 0, (byteLen+1)*sizeof(WCHAR));  
	::MultiByteToWideChar(CP_ACP, NULL, source, -1, destination, byteLen);	//多字节编码转换成宽字节编码
	return destination; 
}

char* commonused::UnicodeToUTF8(const WCHAR* source) 
{
	int byteLen = ::WideCharToMultiByte(CP_UTF8, NULL, source, -1, NULL, 0, NULL, NULL); 	//预转换，得到所需空间的大小（单位为字节） 
	char* destination = new char[byteLen + 1]; 	//UTF8虽然是Unicode的压缩形式，但也是多字节字符串，所以可以以char的形式保存 
	memset(destination, 0, byteLen + 1);  
	::WideCharToMultiByte(CP_UTF8, NULL, source, -1, destination, byteLen, NULL, NULL); 
	return destination;
}

WCHAR* commonused::UTF8ToUnicode(const char* source)  
{  
	int byteLen = ::MultiByteToWideChar(CP_UTF8,0,source,-1,NULL,0);  //预转换，得到所需空间的大小（单位为字节）
	WCHAR * destination = new WCHAR[byteLen+1];  
	memset(destination, 0, (byteLen+1)*sizeof(WCHAR));  
	MultiByteToWideChar(CP_UTF8, 0, source, -1, destination, byteLen);    
	return destination;   
} 

string commonused::UrlEncode(CString& szToEncode)
{
//	char *t_szToEncode = commonused::UnicodeToAnsi(szToEncode);
	string str1= szToEncode;
	string t_result = commonused::UrlStringEncode(str1);
//	delete t_szToEncode;
	return t_result;
}

string commonused::UrlStringEncode(const string& szToEncode)
{
	string src = szToEncode;
	char hex[] = "0123456789ABCDEF";
	std::string dst;

	for (size_t i = 0; i < src.size(); ++i)
	{
		unsigned char cc = src[i];
		if (isascii(cc))
		{
			if (cc == ' ')
			{
				dst += "%20";
			}
			else
				dst += cc;
		}
		else
		{
			unsigned char c = static_cast<unsigned char>(src[i]);
			dst += '%';
			dst += hex[c / 16];
			dst += hex[c % 16];
		}
	}
	return dst;
}

string commonused::UrlDecode(CString& szToDecode)
{
//	char *t_szToDecode = commonused::UnicodeToAnsi(szToDecode);
	string str1 = szToDecode;
	string t_result = commonused::UrlStringDecode(str1);
//	delete t_szToDecode;
	return t_result;
}

string commonused::UrlStringDecode(const string& szToDecode)
{
	string result;
	int hex = 0;
	for (size_t i = 0; i < szToDecode.length(); ++i)
	{
		switch (szToDecode[i])
		{
		case '+':
			result += ' ';
			break;
		case '%':
			if (isxdigit(szToDecode[i + 1]) && isxdigit(szToDecode[i + 2]))
			{
				std::string hexStr = szToDecode.substr(i + 1, 2);
				hex = strtol(hexStr.c_str(), 0, 16);
				//字母和数字[0-9a-zA-Z]、一些特殊符号[$-_.+!*'(),] 、以及某些保留字[$&+,/:;=?@]
				//可以不经过编码直接用于URL
				if (!((hex >= 48 && hex <= 57) ||	//0-9
					(hex >=97 && hex <= 122) ||	//a-z
					(hex >=65 && hex <= 90) ||	//A-Z
					//一些特殊符号及保留字[$-_.+!*'(),]  [$&+,/:;=?@]
					hex == 0x21 || hex == 0x24 || hex == 0x26 || hex == 0x27 || hex == 0x28 || hex == 0x29
					|| hex == 0x2a || hex == 0x2b|| hex == 0x2c || hex == 0x2d || hex == 0x2e || hex == 0x2f
					|| hex == 0x3A || hex == 0x3B|| hex == 0x3D || hex == 0x3f || hex == 0x40 || hex == 0x5f
					))
				{
					result += char(hex);
					i += 2;
				}
				else result += '%';
			}else {
				result += '%';
			}
			break;
		default:
			result += szToDecode[i];
			break;
		}
	}
	return result;
}

string commonused::getModuleFullPath()
{
	char buffer[522] = {0};
	TCHAR szFilePath[MAX_PATH + 1] = {0};
	
	GetModuleFileName(NULL, szFilePath, MAX_PATH);    

#ifdef UNICODE
	WideCharToMultiByte(CP_ACP, 0, szFilePath, -1, buffer, 522, NULL, NULL);
#endif

	return buffer;
}

string commonused::getModuleFilePath()
{
	char buffer[522] = {0};
	TCHAR szFilePath[MAX_PATH + 1] = {0};    
	
	GetModuleFileName(NULL, szFilePath, MAX_PATH);    
	(_tcsrchr(szFilePath, _T('\\')))[1] = 0;

#ifdef UNICODE
	WideCharToMultiByte(CP_ACP, 0, szFilePath, -1, buffer, 522, NULL, NULL);
#endif

	return buffer;
};

string commonused::getModuleFileName()
{
	char buffer[522]={0};
	TCHAR szFilePath[MAX_PATH + 1]={0};
	char szDrive[MAX_PATH + 1]={0}, szDir[MAX_PATH + 1]={0}, szFilename[MAX_PATH + 1]={0}, szExt[MAX_PATH + 1]={0};    
	
	GetModuleFileName(NULL, szFilePath, MAX_PATH);    

#ifdef UNICODE
	WideCharToMultiByte(CP_ACP, 0, szFilePath, -1, buffer, 522, NULL, NULL);
#endif
	 _splitpath((const char*)buffer, szDrive, szDir, szFilename, szExt);

	return szFilename;
}

string commonused::getModuleFileExt()
{
	char buffer[522]={0};
	TCHAR szFilePath[MAX_PATH + 1]={0};
	char szDrive[MAX_PATH + 1]={0}, szDir[MAX_PATH + 1]={0}, szFilename[MAX_PATH + 1]={0}, szExt[MAX_PATH + 1]={0};    
	
	GetModuleFileName(NULL, szFilePath, MAX_PATH);    
#ifdef UNICODE
	WideCharToMultiByte(CP_ACP, 0, szFilePath, -1, buffer, 522, NULL, NULL);
#endif
	 _splitpath((const char*)buffer, szDrive, szDir, szFilename, szExt);

	return szExt;
}

string commonused::getFilePath(string fullpath)
{
	char buffer[522]={0};
	char szDrive[MAX_PATH + 1]={0}, szDir[MAX_PATH + 1]={0}, szFilename[MAX_PATH + 1]={0}, szExt[MAX_PATH + 1]={0};    
	_splitpath(fullpath.c_str(), szDrive, szDir, szFilename, szExt);

	string path = szDrive;
	path += szDir;

	return path;
}

string commonused::getFileName(string fullpath)
{
	char buffer[522]={0};
	char szDrive[MAX_PATH + 1]={0}, szDir[MAX_PATH + 1]={0}, szFilename[MAX_PATH + 1]={0}, szExt[MAX_PATH + 1]={0};    
	_splitpath(fullpath.c_str(), szDrive, szDir, szFilename, szExt);

	return szFilename;
}

string commonused::getFileExt(string fullpath)
{
	char buffer[522]={0};
	char szDrive[MAX_PATH + 1]={0}, szDir[MAX_PATH + 1]={0}, szFilename[MAX_PATH + 1]={0}, szExt[MAX_PATH + 1]={0};    
	_splitpath(fullpath.c_str(), szDrive, szDir, szFilename, szExt);

	return szExt;
}

string commonused::getSystemTempDirectory()
{
	char buffer[522]={0};

	TCHAR szTempDirectory[MAX_PATH + 1]={0};
	GetTempPath(MAX_PATH, szTempDirectory); 

#ifdef UNICODE
	WideCharToMultiByte(CP_ACP, 0, szTempDirectory, -1, buffer, 522, NULL, NULL);
#endif

	return buffer;
}

string commonused::getWindowsAppDataFolder()
{
	char buffer[522]={0};

	TCHAR szAppDataPath[MAX_PATH] = {0};
	SHGetSpecialFolderPath(NULL, szAppDataPath, CSIDL_APPDATA, FALSE);

#ifdef UNICODE
	WideCharToMultiByte(CP_ACP, 0, szAppDataPath, -1, buffer, 522, NULL, NULL);
#endif

	return buffer;
}

int commonused::getFileSize(string file)
{
	struct _stat buf;    
	if(_stat(file.c_str(), &buf) == 0)    
		return buf.st_size;

	return 0;
}

unsigned long commonused::getIPOfURL(string url)
{
	hostent* hostinfo = NULL;
	char host_name[32] = "", first_ip[32] = "";

	hostinfo = gethostbyname(url.c_str());
	if(hostinfo == NULL)
		return 0;

	struct in_addr addr;
	memcpy(&addr, hostinfo->h_addr_list[0], sizeof(struct in_addr));

	return addr.S_un.S_addr;
}

string commonused::getIPStringOfURL(string url)
{
	return getIPString(getIPOfURL(url));
}

string commonused::getIPString(int ip)
{
	in_addr addr;
	addr.S_un.S_addr = ip;

	return inet_ntoa(addr);
}

string commonused::getHostName()
{
	char host_name[32];
	gethostname(host_name, 32);
	
	return host_name;
}

void commonused::getAllIPs(vector<unsigned long>& ips)
{
	hostent* hostinfo = NULL;
	char host_name[32] = "", first_ip[32] = "";

	gethostname(host_name, 32);
	hostinfo = gethostbyname(host_name);

	if(hostinfo == NULL)
		return;

	for (int ii = 0; hostinfo->h_addr_list[ii] != 0; ++ii) 
	{
		struct in_addr addr;
		memcpy(&addr, hostinfo->h_addr_list[ii], sizeof(struct in_addr));
		ips.push_back(addr.S_un.S_addr);
	}
}

void getAllIPsString(vector<string>& ipss)
{
	hostent* hostinfo = NULL;
	char host_name[32];//, //first_ip[32];

	gethostname(host_name, 32);
	hostinfo = gethostbyname(host_name);

	if(hostinfo == NULL)
		return;

	for (int ii = 0; hostinfo->h_addr_list[ii] != 0; ++ii) 
	{
		struct in_addr addr;
		memcpy(&addr, hostinfo->h_addr_list[ii], sizeof(struct in_addr));
		ipss.push_back(inet_ntoa(addr));
	}
}


string commonused::getFirstIPString()
{
	hostent* hostent = NULL;
	char host_name[32], first_ip[32];

	gethostname(host_name, 32);
	hostent = gethostbyname(host_name);

	if(hostent == NULL)
		return "";

	sprintf(first_ip, "%u.%u.%u.%u", (unsigned char)hostent->h_addr_list[0][0], (unsigned char)hostent->h_addr_list[0][1], (unsigned char)hostent->h_addr_list[0][2], (unsigned char)hostent->h_addr_list[0][3]);

	return first_ip;
}

unsigned long commonused::getFirstIP()
{
	hostent* hostent = NULL;
	char host_name[32], first_ip[32];

	gethostname(host_name, 32);
	hostent = gethostbyname(host_name);

	if(hostent == NULL)
		return 0;

	sprintf(first_ip, "%u.%u.%u.%u", (unsigned char)hostent->h_addr_list[0][0], (unsigned char)hostent->h_addr_list[0][1], (unsigned char)hostent->h_addr_list[0][2], (unsigned char)hostent->h_addr_list[0][3]);

	return inet_addr(first_ip);
}

unsigned long commonused::getIPAddress(string ip)
{
	return inet_addr(ip.c_str());
}

string commonused::getFirstMacAddress()
{
	NCB ncb;   

	typedef struct _ASTAT_   
	{  
		ADAPTER_STATUS   adapt;   
		NAME_BUFFER   NameBuff   [30];       
	}ASTAT, *PASTAT;  
	
	ASTAT Adapter;     

	typedef struct _LANA_ENUM   
	{  
		UCHAR length;   
		UCHAR lana[MAX_LANA];  
	}LANA_ENUM;       

	LANA_ENUM lana_enum;     

	UCHAR uRetCode;       
	memset(&ncb, 0, sizeof(ncb));
	memset(&lana_enum, 0, sizeof(lana_enum));

	ncb.ncb_command = NCBENUM;  
	ncb.ncb_buffer = (unsigned char *)&lana_enum;  
	ncb.ncb_length = sizeof(LANA_ENUM);     

	uRetCode = Netbios(&ncb); //调用netbois(ncb)获取网卡序列号       
	if(uRetCode != NRC_GOODRET)       
		return "";       

	for(int lana = 0; lana < lana_enum.length; lana ++)       
	{  
		ncb.ncb_command = NCBRESET;   //对网卡发送NCBRESET命令，进行初始化   
		ncb.ncb_lana_num = lana_enum.lana[lana];   
		uRetCode = Netbios(&ncb);     
	}   

	if(uRetCode != NRC_GOODRET)  
		return "";       

	memset(&ncb, 0, sizeof(ncb));   
	ncb.ncb_command = NCBASTAT; 
	ncb.ncb_lana_num = lana_enum.lana[0];     //指定网卡号，这里仅仅指定第一块网卡    
	strcpy((char*)ncb.ncb_callname, "*");  
	ncb.ncb_buffer = (unsigned char *)&Adapter;   
	ncb.ncb_length = sizeof(Adapter);  

	uRetCode = Netbios(&ncb);   
	if(uRetCode != NRC_GOODRET)     
		return "";     

	char mac_address[256] = {0};
	sprintf(mac_address,"%02X-%02X-%02X-%02X-%02X-%02X", Adapter.adapt.adapter_address[0], Adapter.adapt.adapter_address[1], Adapter.adapt.adapter_address[2], Adapter.adapt.adapter_address[3], Adapter.adapt.adapter_address[4], Adapter.adapt.adapter_address[5]);   

	return mac_address;     
}

void commonused::getAllMacAddresses(vector<string>& macs)
{
	NCB ncb;   

	typedef struct _ASTAT_   
	{  
		ADAPTER_STATUS   adapt;   
		NAME_BUFFER   NameBuff   [30];       
	}ASTAT, *PASTAT;  
	
	ASTAT Adapter;     

	typedef struct _LANA_ENUM   
	{  
		UCHAR length;   
		UCHAR lana[MAX_LANA];  
	}LANA_ENUM;       

	LANA_ENUM lana_enum;     

	UCHAR uRetCode;       
	memset(&ncb, 0, sizeof(ncb));
	memset(&lana_enum, 0, sizeof(lana_enum));

	ncb.ncb_command = NCBENUM;  
	ncb.ncb_buffer = (unsigned char *)&lana_enum;  
	ncb.ncb_length = sizeof(LANA_ENUM);     

	uRetCode = Netbios(&ncb); //调用netbois(ncb)获取网卡序列号       
	if(uRetCode != NRC_GOODRET)       
		return;       

	for(int lana = 0; lana < lana_enum.length; lana ++)       
	{  
		ncb.ncb_command = NCBRESET;   //对网卡发送NCBRESET命令，进行初始化   
		ncb.ncb_lana_num = lana_enum.lana[lana];   
		uRetCode = Netbios(&ncb);     
	}   

	if(uRetCode != NRC_GOODRET)  
		return;       

	for(int lana = 0; lana < lana_enum.length; lana ++)
	{
		memset(&ncb, 0, sizeof(ncb));   
		ncb.ncb_command = NCBASTAT; 
		ncb.ncb_lana_num = lana_enum.lana[lana];    
		strcpy((char*)ncb.ncb_callname, "*");  
		ncb.ncb_buffer = (unsigned char *)&Adapter;   
		ncb.ncb_length = sizeof(Adapter);  

		uRetCode = Netbios(&ncb);   
		if(uRetCode != NRC_GOODRET)     
			return;     

		char mac_address[256] = {0};
		sprintf(mac_address,"%02X-%02X-%02X-%02X-%02X-%02X", Adapter.adapt.adapter_address[0], Adapter.adapt.adapter_address[1], Adapter.adapt.adapter_address[2], Adapter.adapt.adapter_address[3], Adapter.adapt.adapter_address[4], Adapter.adapt.adapter_address[5]);  

		macs.push_back(mac_address);
	}
}

char *__fastcall convertDwordToString(DWORD dwDiskData[256], int nFirstIndex, int nLastIndex)
{
    static char szResBuf[1024];
    char ss[256];
    int nIndex = 0;
    int nPosition = 0;

    for(nIndex = nFirstIndex; nIndex <= nLastIndex; nIndex++)
    {
        ss[nPosition] = (char)(dwDiskData[nIndex] / 256);
        nPosition++;

        // Get low BYTE for 2nd character
        ss[nPosition] = (char)(dwDiskData[nIndex] % 256);
        nPosition++;
    }

    // End the string
    ss[nPosition] = '\0';

    int i, index=0;
    for(i=0; i<nPosition; i++)
    {
        if(ss[i]==0 || ss[i]==32)    continue;
        szResBuf[index]=ss[i];
        index++;
    }
    szResBuf[index]=0;

    return szResBuf;
}

string commonused::getFirstPhysicalDriveID()
{
	char drive_id[256] = {0};
	char serial_number[64] = {0}, model_number[64] = {0};

	HANDLE hDevice = ::CreateFileA("\\\\.\\PHYSICALDRIVE0", GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
	if(!hDevice) 
		hDevice=::CreateFileA("\\\\.\\Scsi0",GENERIC_READ | GENERIC_WRITE,FILE_SHARE_READ,NULL,OPEN_EXISTING,NULL,NULL);

	if(hDevice == NULL)
		return "";

	DWORD dwBytesReturned;
	GETVERSIONINPARAMS gvopVersionParams;

	DeviceIoControl(hDevice, SMART_GET_VERSION, NULL, 0, &gvopVersionParams, sizeof(gvopVersionParams), &dwBytesReturned, NULL);
	if(dwBytesReturned == 0 || gvopVersionParams.bIDEDeviceMap <= 0)   
	{
		::CloseHandle(hDevice); 
		return ""; 
	}

	SENDCMDINPARAMS scip;     
    memset(&scip, 0, sizeof(SENDCMDINPARAMS)); 
    scip.cBufferSize = IDENTIFY_BUFFER_SIZE; 
    scip.irDriveRegs.bSectorCountReg = 1; 
    scip.irDriveRegs.bSectorNumberReg = 1; 
    scip.irDriveRegs.bDriveHeadReg = 0xA0; 
    scip.irDriveRegs.bCommandReg = 0xEC; 
     
    BYTE btBuffer[1024]; 
    memset(btBuffer,0,1024); 
     
    if(!DeviceIoControl(hDevice, SMART_RCV_DRIVE_DATA, &scip, sizeof(SENDCMDINPARAMS), btBuffer, 1024, &dwBytesReturned, NULL)) 
    { 
        ::CloseHandle(hDevice); 
        return ""; 
    } 

     ::CloseHandle(hDevice); 

	DWORD dwDiskData[256];
	USHORT *pIDSector = (USHORT*)((SENDCMDOUTPARAMS*)btBuffer)->bBuffer;

	for(int ii = 0; ii < 256; ii ++)    
		dwDiskData[ii] = pIDSector[ii];

	// 取系列号
	strcpy(serial_number, convertDwordToString(dwDiskData, 10, 19));

	// 取模型号
	strcpy(model_number, convertDwordToString(dwDiskData, 27, 46));

	sprintf(drive_id, "%s-%s", serial_number, model_number);

	return drive_id;
}

string commonused::getCPUID()
{
	char cpu_id[64] = {0};

	unsigned char szCPUSN[12];
	memset(szCPUSN, 0, 12);
	int  CPUInfo[4] = {-1};

	__cpuid(CPUInfo, 1);
	memcpy(&szCPUSN[8], &CPUInfo[0], 4); // eax为最高位的两个WORD

	__cpuid(CPUInfo, 3);
	//获取序列号的前4个WORD
	memcpy(&szCPUSN[0], &CPUInfo[2], 4);
	memcpy(&szCPUSN[4], &CPUInfo[3], 4);

	char szTemp[10];
	for(int i = 0;i < 12;i ++)
	{
		sprintf(szTemp, "%02X", szCPUSN[i]);
		if(i == 0)
			strcpy(cpu_id, szTemp);
		else
		{
			if(i%2 == 0)
				strcat(cpu_id, "-");
			strcat(cpu_id, szTemp);
		}
	}

	return cpu_id;
}

string commonused::getUniqueIDOfLocalMachine()
{
	char unique_id[256] = {0};

	string drive_id = getFirstPhysicalDriveID();
	string cpu_id = getCPUID();

	sprintf(unique_id, "h-%s-%s", drive_id.c_str(), cpu_id.c_str());
	
	return unique_id; 
}

string commonused::getScreenResolution()
{
	return "1080P";
}

string commonused::genarateGUID()
{
	char message_id[64];
	memset(message_id, 0, 64);

	GUID guid;

	if(S_OK == ::CoCreateGuid(&guid))
	{
		_snprintf_s(message_id, 
				sizeof(message_id), 
				"%08X-%04X-%04x-%02X%02X-%02X%02X%02X%02X%02X%02X", 
				guid.Data1, 
				guid.Data2, 
				guid.Data3, 
				guid.Data4[0], 
				guid.Data4[1], 
				guid.Data4[2], 
				guid.Data4[3], 
				guid.Data4[4], 
				guid.Data4[5], 
				guid.Data4[6], 
				guid.Data4[7]
		);
	}

	return (const char*)message_id;
}

bool commonused::deleteFolder(string folder) 
{
	SHFILEOPSTRUCT FileOp; 
	ZeroMemory((void*)&FileOp, sizeof(SHFILEOPSTRUCT));

	FileOp.fFlags = FOF_NOCONFIRMATION | FOF_NOCONFIRMMKDIR | FOF_NOERRORUI;  
	FileOp.hNameMappings = NULL; 
	FileOp.hwnd = NULL; 
	FileOp.lpszProgressTitle = NULL; 

#ifdef UNICODE
	TCHAR szPath[522] = {0};
	MultiByteToWideChar(CP_ACP, 0, folder.c_str(), -1, szPath, 522);
	FileOp.pFrom = szPath; 
#else
	FileOp.pFrom = folder.c_str();
#endif
	FileOp.pTo = NULL; 
	FileOp.wFunc = FO_DELETE; 

	return SHFileOperation(&FileOp) == 0;
}

bool commonused::createFolder(string folder)
{
	return (bool)MakeSureDirectoryPathExists(folder.c_str());
}

void commonused::deleteFile(string file)
{
	TCHAR szFile[522] = {0};

#ifdef UNICODE
	MultiByteToWideChar(CP_ACP, 0, file.c_str(), -1, szFile, 522);
#else
	strcpy(szFile, file.c_str());;
#endif

	DeleteFile(szFile);
}

bool commonused::createFile(string file, bool overwrite)
{
	bool result = false;;
	FILE* fh = fopen(file.c_str(), overwrite ? "w+" : "a+");
	result = fh == NULL;
	fclose(fh);

	return result;
}

bool commonused::isFileExist(string file)
{
	return _access(file.c_str(), 0) != -1;
}

bool commonused::isFolderExist(string folder)
{
	TCHAR szPath[522] = {0};
#ifdef UNICODE
	MultiByteToWideChar(CP_ACP, 0, folder.c_str(), -1, szPath, 522);
#else
	strcpy(szPath, folder.c_str());
#endif

	return PathFileExists(szPath);
}

string commonused::findFirstFileWithSpecifiedExtension(string path, string ext)
{
	string result;
	string to_find = path + "*." + ext;
	TCHAR szToFind[522] = {0};

#ifdef UNICODE
	MultiByteToWideChar(CP_ACP, 0, to_find.c_str(), -1, szToFind, 522);
#else
	strcpy(szToFind, to_find.c_str());
#endif

	WIN32_FIND_DATA FindFileData;
	HANDLE hFind = FindFirstFile(szToFind, &FindFileData);

	if (hFind != INVALID_HANDLE_VALUE)
	{
		char file_name[522] = {0};

#ifdef UNICODE
		WideCharToMultiByte(CP_ACP, 0, FindFileData.cFileName, -1, file_name, 522, NULL, NULL);
#else
		strcpy(file_name, FindFileData.cFileName);
#endif
		result = path + file_name;
		
		FindClose(hFind);
	}

	return result;
}

bool commonused::moveFile(string src, string dest)
{
	TCHAR szSrc[522] = {0}, szDest[522] = {0};

#ifdef UNICODE
	MultiByteToWideChar(CP_ACP, 0, src.c_str(), -1, szSrc, 522);
	MultiByteToWideChar(CP_ACP, 0, dest.c_str(), -1, szDest, 522);
#else
	strcpy(szSrc, src.c_str());
	strcpy(szDest, dest.c_str());
#endif

	return (bool)MoveFile(szSrc, szDest);
}

time_t commonused::getFileModifiedTime(string file)
{
	struct stat buf;

	if(stat(file.c_str(), &buf) != 0)
		return 0;

	return buf.st_mtime;
}

bool commonused::isProcessWindowRunning(string window)
{
	TCHAR szWindow[1024] = {0};

#ifdef UNICODE
	MultiByteToWideChar(CP_ACP, 0, window.c_str(), -1, szWindow, 1024);
#else
	strcpy(szWindow, window.c_str());
#endif

	return ::FindWindow(NULL, szWindow) != NULL;
}

bool commonused::invokeProcess(string command_line)
{
	TCHAR szCommandLine[1024] = {0};

#ifdef UNICODE
	MultiByteToWideChar(CP_ACP, 0, command_line.c_str(), -1, szCommandLine, 1024);
#else
	strcpy(szCommandLine, command_line.c_str());
#endif

	STARTUPINFO si;
    PROCESS_INFORMATION pi;

    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);

    ZeroMemory(&pi, sizeof(pi));

    if(!CreateProcess(NULL, szCommandLine, NULL, NULL, FALSE, CREATE_NEW_CONSOLE, NULL, NULL, &si, &pi))
		return false;

    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);

	return true;
}

bool commonused::killProcess(string process)
{
    bool result = false;  
    SHFILEINFO shSmall;    
    PROCESSENTRY32 ProcessInfo;    
    string current_process;

    HANDLE SnapShot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);    
    if(SnapShot != NULL)     
    {  
        ProcessInfo.dwSize = sizeof(ProcessInfo); 
        bool found = Process32First(SnapShot, &ProcessInfo);    
  
        HANDLE hToken;  
        if(OpenProcessToken(GetCurrentProcess(), TOKEN_ALL_ACCESS, &hToken))  
        {  
            LUID luid;  
            if(LookupPrivilegeValue(NULL, SE_DEBUG_NAME, &luid))  
            {  
                TOKEN_PRIVILEGES TokenPrivileges;  
                TokenPrivileges.PrivilegeCount = 1;  
                TokenPrivileges.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;  
                TokenPrivileges.Privileges[0].Luid = luid;  
                AdjustTokenPrivileges(hToken, FALSE, &TokenPrivileges, 0, NULL, NULL);  
            }  
            CloseHandle(hToken);  
        }  
  
        while(found)    
        {    
            SHGetFileInfo(ProcessInfo.szExeFile, 0, &shSmall, sizeof(shSmall), SHGFI_ICON|SHGFI_SMALLICON);    
  
			char szExeFile[256] = {0};

#ifdef UNICODE
			WideCharToMultiByte(CP_ACP, 0, ProcessInfo.szExeFile, -1, szExeFile, 256, NULL, NULL);
#else
			strcpy(szExeFile, ProcessInfo.szExeFile);
#endif

			current_process = szExeFile;  
 
            if(current_process == process)  
            {    
                HANDLE ProcessHandle = OpenProcess(PROCESS_ALL_ACCESS, FALSE, ProcessInfo.th32ProcessID);  
                if (ProcessHandle != NULL)  
                {  
					result = TerminateProcess(ProcessHandle, 1);  
					CloseHandle(ProcessHandle);  
                 }  
                      
                break;    
            }    

			found = Process32Next(SnapShot, &ProcessInfo);    
        }     
    }  
  
    return  result;  
}

bool commonused::isProcessRunning(string process)
{
    bool result = false; 

    SHFILEINFO shSmall;
    PROCESSENTRY32 ProcessInfo;    
    string current_process;

    HANDLE hSnapShot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);    
    if(hSnapShot != NULL)
    {  
        ProcessInfo.dwSize = sizeof(ProcessInfo); 
        bool found = Process32First(hSnapShot, &ProcessInfo);    
  
        HANDLE hToken;  
        if(OpenProcessToken(GetCurrentProcess(), TOKEN_ALL_ACCESS, &hToken))  
        {  
            LUID luid;  
            if(LookupPrivilegeValue(NULL, SE_DEBUG_NAME, &luid))  
            {  
                TOKEN_PRIVILEGES TokenPrivileges;  
                TokenPrivileges.PrivilegeCount = 1;  
                TokenPrivileges.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;  
                TokenPrivileges.Privileges[0].Luid = luid;  
                AdjustTokenPrivileges(hToken, FALSE, &TokenPrivileges, 0, NULL, NULL);  
            }  
            CloseHandle(hToken);  
        }  
  
        while(found)    
        {    
            SHGetFileInfo(ProcessInfo.szExeFile, 0, &shSmall, sizeof(shSmall), SHGFI_ICON | SHGFI_SMALLICON);    
  
			char szExeFile[256] = {0};

#ifdef UNICODE
			WideCharToMultiByte(CP_ACP, 0, ProcessInfo.szExeFile, -1, szExeFile, 256, NULL, NULL);
#else
			strcpy(szExeFile, ProcessInfo.szExeFile);
#endif

			current_process = szExeFile;  
 
            if(current_process == process)  
            {    
				result = true;  
                break;    
            }    

			found = Process32Next(hSnapShot, &ProcessInfo);    
        }

		CloseHandle(hSnapShot);
    }
  
    return  result;  
}

time_t commonused::__get_time()
{
	time_t tm;
	time(&tm);

	return tm;
}

char* commonused::__format_time(time_t t, const char* format, char *& buf, int max_size)
{
	if(buf == NULL)
		return NULL;

	buf[0] = '\0';
	tm* p = localtime(&t);

	if (p == NULL)
		return NULL;

	TCHAR szBuf[256] = {0}, szFormat[256] = {0};

#ifdef UNICODE
	MultiByteToWideChar(CP_ACP, 0, format, -1, szFormat, 256);
#else
	strcpy(szFormat, format);
#endif

	if(!_tcsftime(szBuf, max_size, szFormat, p))
		return NULL;

#ifdef UNICODE
	WideCharToMultiByte(CP_ACP, 0, szBuf, -1, buf, max_size, NULL, NULL);
#else
	strcpy(buf, szBuf);
#endif

	return buf;
}

int commonused::__get_year()
{
	time_t tm = __get_time();

	char buf[20], *b = buf;
	__format_time(tm, "%Y", b, 20);

	int year = atoi(buf);

	return year;
}

int commonused::__get_month()
{
	time_t tm = __get_time();

	char buf[20], *b = buf;
	__format_time(tm, "%m", b, 20);

	int month = atoi(buf);

	return month;
}

int commonused::__get_day()
{
	time_t tm = __get_time();

	char buf[20], *b = buf;
	__format_time(tm, "%d", b, 20);

	int day = atoi(buf);

	return day;
}

int commonused::__get_hour()
{
	time_t tm = __get_time();

	char buf[20], *b = buf;
	__format_time(tm, "%H", b, 20);

	int hour = atoi(buf);

	return hour;
}

int commonused::__get_minute()
{
	time_t tm = __get_time();

	char buf[20], *b = buf;
	__format_time(tm, "%M", b, 20);

	int minute = atoi(buf);

	return minute;
}

int commonused::__get_second()
{
	time_t tm = __get_time();

	char buf[20], *b = buf;
	__format_time(tm, "%S", b, 20);

	int second = atoi(buf);

	return second;
}

time_t commonused::__to_datetime(char* buf)
{
	if(buf == NULL)
		return 0;

	char** pp = _strTokenize(buf, "-: ");

	if(_strStringsCount(pp) != 6)
	{
		_strFreeStrings(pp);
		return 0;
	}

	tm atm;

	atm.tm_year = atoi(pp[0]) - 1900;
	atm.tm_mon = atoi(pp[1]) - 1;
	atm.tm_mday = atoi(pp[2]);
	atm.tm_hour = atoi(pp[3]);
	atm.tm_min = atoi(pp[4]);
	atm.tm_sec = atoi(pp[5]);

	time_t t = mktime(&atm);

	_strFreeStrings(pp);

	return t;
}

int commonused::__get_timediff(time_t start, time_t end)
{
	return (int)difftime(start, end);
}

string commonused::__get_localtime_string(time_t t)
{
	tm *lmt = localtime(&t);
	return asctime(lmt);
}

string commonused::__get_gmtime_string(time_t t)
{
	tm *gmt = _gmtime64(&t);
	return asctime(gmt);
}

char* commonused::_strDup(char * pszString)
{
	int nLen = 0;
	char * pszBuffer = NULL;

	nLen = strlen(pszString);
	
	pszBuffer = (char*)malloc(nLen + 1);
	if(pszBuffer != NULL)
		strcpy(pszBuffer, pszString);

	return pszBuffer;
}

int commonused::_strGetCharCount(char *pszString, char c)
{
	int count = 0, counter = 0;

	while(true)
	{
		if(pszString[counter] == 0)
			break;

		if(pszString[counter] == c)
			count ++;

		counter ++;		
	}

	return count;
}

char** commonused::_strTokenize(char *pszString, char *pszTokenizer)
{
	if(pszString == NULL || pszTokenizer == NULL)
		return NULL;

	int nTokenCount = 0;
	char * pszToken = NULL;
	char **ppszTokens = NULL;

	char * pszBuffer = _strDup(pszString);
	if(pszBuffer == NULL)
		return NULL;

	pszToken = strtok(pszBuffer, pszTokenizer);

	while(pszToken != NULL)
	{
		++ nTokenCount;
	
		pszToken = strtok(NULL, pszTokenizer);
	}

	ppszTokens = (char **)malloc((nTokenCount + 1) * sizeof(char *));
	if(ppszTokens == NULL)
	{
		free(pszBuffer);
		return NULL;
	}

	strcpy(pszBuffer, pszString);

	nTokenCount = 0;
	pszToken = strtok(pszBuffer, pszTokenizer);

	while(pszToken != NULL)
	{
		ppszTokens[nTokenCount ++] = _strDup(pszToken);

		pszToken = strtok(NULL, pszTokenizer);
	}

	ppszTokens[nTokenCount] = NULL;

	free(pszBuffer);

	return ppszTokens;
}

int commonused::_strStringsCount(char **ppszStrings)
{
	int i = 0;

	for(i = 0; ppszStrings[i] != NULL; i ++);

	return i;
}

void commonused::_strFreeStrings(char **ppszStrings)
{
	int ii = 0;

	while(ppszStrings[ii] != NULL)
		free(ppszStrings[ii ++]);

	free(ppszStrings);
}

bool commonused::strTokenize(string source, string tokenizer, vector<string>& v)
{
	char** pp = _strTokenize((char*)source.c_str(), (char*)tokenizer.c_str());
	if(pp == NULL)
		return false;

	for(int ii = 0; ii < _strStringsCount(pp); ii ++)
		v.push_back(pp[ii]);

	_strFreeStrings(pp);

	return true;
}

string commonused::trim(string& source, char quot)
{
	if(source.empty()) return "";

	const char* s = source.c_str();	
	int length = source.length();

	if(s[0] == quot && s[length - 1] == quot)
	{
		char* v = new char[length - 1];
		strncpy(v, s + 1, length - 2);
		v[length - 2] = 0;

		source = v;

		delete[] v;
	}
	else if(s[0] == quot)
	{
		char* v = new char[length];
		strncpy(v, s + 1, length - 1);
		v[length - 1] = 0;

		source = v;

		delete[] v;
	}
	else if(s[length - 1] == quot)
	{
		char* v = new char[length];
		strncpy(v, s, length - 1);
		v[length - 1] = 0;

		source = v;

		delete[] v;
	}

	return source;
}

string commonused::trimLeft(string& source, char quot)
{
	if(source.empty()) return "";

	const char* s = source.c_str();	
	int length = source.length();

	if(*s == quot)
	{
		char* v = new char[length];
		strncpy(v, s + 1, length - 1);
		v[length - 1] = 0;

		source = v;

		delete[] v;
	}

	return source;
}

string commonused::trimRight(string& source, char quot)
{
	if(source.empty()) return "";

	const char* s = source.c_str();	
	int length = source.length();

	if(*(s + length - 1) == quot)
	{
		char* v = new char[length];
		strncpy(v, s, length - 1);
		v[length - 1] = 0;

		source = v;

		delete[] v;
	}

	return source;
}

bool commonused::parseJsonString(string json_string, JsonObject& json_object)
{
	string str = json_string;

	trim(str, ' ');
	trim(str, '\n');
	trimLeft(str, '{');
	trimRight(str, '}');

	const char* p = str.c_str();
	int length = str.length(), counter = 0;

	int left_quote_pos = 0, right_quote_pos = 0;
	string pair_name, pair_value;
	bool colon = false;

	while(counter < length)
	{
		if(*(p + counter) == '\'' || *(p + counter) == '\"')
		{
			if(left_quote_pos == 0)
				left_quote_pos = counter + 1;
			else
			{
				right_quote_pos = counter - 1;

				char* buf = new char[right_quote_pos - left_quote_pos + 2];
				memset(buf, 0, right_quote_pos - left_quote_pos + 2);

				strncpy(buf, p + left_quote_pos, right_quote_pos - left_quote_pos + 1);

				if(pair_name.empty())
					pair_name = buf;
				else
				{
					pair_value = buf;

					json_object.push_back(JsonPair(pair_name, pair_value));

					pair_name.clear();
					pair_value.clear();
				}

				delete[] buf;

				left_quote_pos = right_quote_pos = 0;
			}
		}

		counter ++;
	}

	return true;
}

string commonused::getJsonPairValue(JsonObject& json_object, string pair_name)
{
	string value;

	for(int ii = 0; ii < json_object.size(); ii ++)
	{
		if(json_object[ii].name == pair_name)
		{
			value = json_object[ii].value;
			break;
		}
	}

	return value;
}

string commonused::getJsonPairValue(string json_string, string pair_name)
{
	JsonObject json_object;

	if(!parseJsonString(json_string, json_object))
		return "";

	return getJsonPairValue(json_object, pair_name);
}

bool commonused::createJsonString(JsonObject& json_object, string& json_string)
{
	if(json_object.size() == 0)
		return false;

	json_string = "{";

	json_string += "\n";

	for(int ii = 0; ii < json_object.size(); ii ++)
	{
		json_string += "\'" + json_object[ii].name + "\'" + ":" + "\'" + json_object[ii].value + "\'";

		if(ii < json_object.size() - 1)
			json_string += ",";

		json_string += "\n";
	}

	json_string += "}";

	return true;
}

void commonused::eraseJsonPair(JsonObject& json_object, string pair_name)
{
	if(json_object.size() == 0)
		return;

	vector<JsonPair>::iterator it = json_object.begin();
			
	while(it != json_object.end())
	{
		if((*it).name != pair_name)
			it ++;
		else
		{
			json_object.erase(it);
			break;
		}
	}
}

bool commonused::parseKeyValueString(string source, vector<string>& keys, vector<string>& values)
{
	vector<string> kvs;
	if(!strTokenize(source, ";", kvs))
		return false;

	if(kvs.size() == 0)
		return false;

	for(int ii = 0; ii < kvs.size(); ii ++)
	{
		string kv = kvs[ii];
		vector<string> vs;
		
		if(!strTokenize(kv, "=", vs))
			return false;

		if(vs.size() != 2)
			return false;

		keys.push_back(vs[0]);
		values.push_back(vs[1]);
	}

	return true;
}

bool commonused::getKeyValue(string source, string key, string& value)
{
	vector<string> keys, values;

	if(!parseKeyValueString(source, keys, values))
		return false;

	for(int ii = 0; ii < keys.size(); ii ++)
	{
		if(keys[ii] == key)
		{
			value = values[ii];
			return true;
		}
	}

	return false;
}

void multiByteToUnicode(WCHAR* pOut, char *pIn)
{
    ::MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, pIn, 2, pOut, 1);
    return;
}

void unicodeToUTF8(char* pOut, WCHAR* pIn)
{
    // 注意 WCHAR高低字的顺序,低字节在前，高字节在后
    char* pchar = (char *)pIn;
	
	if(pchar[1] == 0x0)
	{
		pOut[0] = (0xC0 | ((pchar[0] >> 6) & 0x03));
		pOut[1] = (0x80 | (pchar[0] & 0x3F));
	}
	else
	{
		pOut[0] = (0xE0 | ((pchar[1] & 0xF0) >> 4));
		pOut[1] = (0x80 | ((pchar[1] & 0x0F) << 2)) + ((pchar[0] & 0xC0) >> 6);
		pOut[2] = (0x80 | (pchar[0] & 0x3F));
	}

    return;
}

void stringToUTF8(string& pOut, char *pText, int pLen)
{
    char buf[4];
    memset(buf,0,4);
	
    int i = 0;
    while(i < pLen)
    {
        if( pText[i] >= 0)
        {
            char asciistr[2]={0};
            asciistr[0] = (pText[i++]);
            pOut.append(asciistr);
        }
        else 
        {
            WCHAR pbuffer;
            multiByteToUnicode(&pbuffer, pText+i);
            unicodeToUTF8(buf, &pbuffer);
            pOut.append(buf);
			
            i += 2;
        }
    }
	
    return;
}

string commonused::encodeURLToUTF8(char * pIn)
{
    string tt;
    string dd;
    stringToUTF8(tt, pIn, (int)strlen(pIn));
	
    size_t len = tt.length();
    for (size_t i=0; i<len; i++)
    {
        if(isalnum((BYTE)tt.at(i)))
        {
            char tempbuff[2]={0};
            sprintf(tempbuff,"%c",(BYTE)tt.at(i));
            dd.append(tempbuff);
        }
        else if (isspace((BYTE)tt.at(i)))
        {
            dd.append("+");
        }
        else
        {
            char tempbuff[4];
            sprintf(tempbuff,"%%%X%X",((BYTE)tt.at(i)) >>4, ((BYTE)tt.at(i)) %16);
            dd.append(tempbuff);
        }
    }

    return dd;
}



