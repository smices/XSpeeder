#pragma once
#include "global.h"
#include <vector>
#include "resource.h"

extern std::wstring _ServeName;

// -----------------------------------------
std::string extractDrv();
void InstallServ();
void RunServ();
std::string extractProtectSys();
// -----------------------------------------
typedef enum {
	cmdQuit = 0,
	cmdStop,
	cmdShareStopped,
	cmdInstallStop,
	cmdProtectRegChange,
	cmdXbSpeedRegChange
} cmdType;

typedef struct _tagSrvInfo {
	SERVICE_STATUS_HANDLE   svcStatusHandle;
	SERVICE_STATUS          svcStatus;
	HANDLE                  vecCtrlEventHandle[10];
	DWORD                   cntCtrlEventHandle;
	DWORD                   dwCheckPoint;
	DWORD                   dwCtrlStop;
	HKEY                    hkProcessProtect;
	HKEY                    hkXBSpeed;
}SrvInfo,*PSrvInfo;

VOID ReportSvcStatus( PSrvInfo, DWORD, DWORD, DWORD );
VOID SvcReportEvent( LPWSTR );
VOID WINAPI xbServiceMain(DWORD dwArgc, LPTSTR* lpszArgv);
DWORD WINAPI xbSvcCtrlHandler(DWORD dwControl, DWORD dwEventType, LPVOID lpEventData, LPVOID lpContext);
// -----------------------------------------

