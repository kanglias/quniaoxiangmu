/************************************************************************/
/* Copyright  2005-2005,  Peter Inc. - All Rights Reserved. 			*/
/* Last Update	:   2005-06-19   				                        */
/* Purpose	    :   include header file									*/
/* FileName     :   WindowsExtend.h				                        */
/* Author 	    :   Peter 		 										*/
/************************************************************************/

#ifndef WINDOWSEXTENDS_H
#define WINDOWSEXTENDS_H

#pragma once

//OS related header files
#ifdef WIN32

//#define _CRTDBG_MAP_ALLOC 
//#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <process.h>
#include <crtdbg.h>
///////////////////////////////////////////////////////////////////////////
//C++ standard files
#pragma warning(disable:4786 4503)
#include <iostream>
#include <map>
#include <vector>
#include <string>
#include <list>
#include <set>
#include <deque>
#include <stack>
#include <string.h>
////////////////////////////////////////////////////////////////////////////
//for c std
#include <io.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdarg.h>
#include <time.h>
#include <direct.h>

///////////////////////////////////////////////////////////////////////////
#endif

#endif
