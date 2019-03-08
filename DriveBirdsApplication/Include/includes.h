/////////////////////////////////////////////////////////////////////////////////////////

/*
	This file contains the header files name that 
	should be included in this project.
*/

/////////////////////////////////////////////////////////////////////////////////////////
#pragma once

#if !defined __INCLUDE_H__
#define __INCLUDE_H__y qt

#pragma comment(lib, "kernel32.lib")
#pragma comment(lib, "user32.lib")
#pragma comment(lib, "gdi32.lib")
#pragma comment(lib, "msimg32.lib")
#pragma comment(lib, "comdlg32.lib")
#pragma comment(lib, "winspool.lib")
#pragma comment(lib, "advapi32.lib")
#pragma comment(lib, "comctl32.lib")
#pragma comment(lib, "shlwapi.lib")

#include <time.h>
#include <atltime.h>
#include <Windows.h>
#include <intrin.h>
#include <winsvc.h>
#include <winioctl.h>

#pragma comment(lib, "advapi32.lib")
#include <process.h>
#include <tchar.h>

#pragma comment(lib, "ws2_32.lib")
#include <winsock2.h>

#include <fcntl.h>

#include <aclapi.h>

#pragma comment(lib, "winmm.lib")
#include <mmsystem.h>

#include <sys/stat.h>
#pragma comment(lib, "netapi32.lib")
#include <nb30.h>

#pragma comment(lib, "comsupp.lib")
#include <comutil.h>

#pragma comment(lib, "pdh.lib")
#include <pdh.h>
#include <pdhmsg.h>

#include <tlhelp32.h>

#pragma comment(lib, "ole32.lib")
#include <objbase.h>

#pragma comment(lib, "psapi.lib")
#include <psapi.h>

#include <shellapi.h>

#pragma comment(lib, "imagehlp.lib")
#include <imagehlp.h>

//////////////////////////////////////////////////////////////////////////////////////////
//C++ standard files
#pragma warning( disable :  4786 4503 )

#include <iostream>
#include <fstream>
#include <map>
#include <vector>
#include <string>
#include <list>
#include <set>
#include <stack>
#include <typeinfo>
#include <algorithm>
//////////////////////////////////////////////////////////////////////////////////////////

//for c std
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <time.h>
#include <io.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <direct.h>


//////////////////////////////////////////////////////////////////////////////////////////
#endif
