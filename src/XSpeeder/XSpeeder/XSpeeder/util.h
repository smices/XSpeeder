
#pragma once
#include "global.h"
#include "xldl.h"
#include "DownWrapper.h"

#include "xbStructDef.h"

extern DownWrapper* g_pWapper;
extern std::string strHDSerial;
void PrintTaskInfo(DownTaskInfo &info,bool bTitle=false);

BOOL StringToWString(const std::string &str,std::wstring &wstr);
BOOL WStringToString(const std::wstring &wstr,std::string &str);

class CurlInitialize
{
public:
	CurlInitialize();
	~CurlInitialize();
protected:
	BOOL m_bStatus;
public:
	BOOL GetStatus();
};
//==========================

typedef struct _tagThreadCtrl {
	HANDLE m_hThread;
	HANDLE m_hEvent[2];//0:quit;1:resolve;
	HANDLE m_hPipe[2]; //communication to thread;

	PVOID  m_Parameter;
	DWORD (*ThreadProc)(_tagThreadCtrl*);
	DWORD m_dwThreadId;
	WORD  m_wStatus;
}ThreadCtrl,*PThreadCtrl;

int GlobalInitialize(CurlInitialize &curl);
DownWrapper* LoadDll();
void UnloadDll(DownWrapper** Wapper);
//----------------------------------------------
DWORD WINAPI CommonThreadProc(LPVOID);
PThreadCtrl CreateXbThread(PVOID,DWORD (*ThreadProc)(PThreadCtrl));
BOOL ResumeXbThread(PThreadCtrl);
VOID QuitXbThread(PThreadCtrl);
VOID DestoryXbThread(PThreadCtrl);
//==========================================
DWORD UpdateThreadProc(PThreadCtrl);
DWORD TaskThreadProc(PThreadCtrl);
//==========================================
int ScanLogicalDrive(char **disk);

int CompressBySnappy(std::string szSource,std::string szDestination);
int UncompressBySnappy(std::string szSource,std::string szDestination);

std::string AlgorithemMD5(std::string szFile);

std::string GetSystemRootHDSerialNumber();