#pragma once

//================================================
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <map>
#include <algorithm>
//#include <locale>
//================================================

#include <tchar.h>
#include <direct.h>

#define _WIN32_WINNT 0x0500
#include <Windows.h>
#include <Shlwapi.h>
#pragma comment(lib,"shlwapi.lib") 
#include <Wincrypt.h>
#pragma comment(lib,"Advapi32.lib")
#include <SDKDDKVer.h>
//#include <strsafe.h>
#include <comdef.h>
#include <comutil.h>
#include <Wbemidl.h>
#pragma comment ( lib, "wbemuuid.lib" )