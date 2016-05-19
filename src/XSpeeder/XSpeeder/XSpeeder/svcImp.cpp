
#include "svcImp.h"
#include "curl\curl.h"
#include "util.h"
#include "InitConf.h"

std::wstring _ServeName=L"Network Acceleration";
std::wstring wszEvtName=L"{EF15853E-904A-4F6C-A488-BD3329CF7A43}";

std::string extractProtectSys() {
	std::string szPath;
	std::wstring wszError;

	DWORD dwWritten = 0;
	DWORD dwSvcDrvSysSize=0;

	HMODULE hInstance = NULL;
	HRSRC hSvcDriverSysRes = NULL;
	HGLOBAL hSvcDriverSysHandle=NULL;
	LPVOID pSvcDriverSysData=NULL;
	HANDLE hFileDrvSys=NULL;

	hInstance = ::GetModuleHandle(NULL);
	if (!hInstance) {
		wchar_t Buffer[512]={'\0'};
		wnsprintfW(Buffer, 512, L" with %d.",GetLastError());

		wszError.append(L"extractProtectSys:\t GetModuleHandle() error!");
		wszError.append(Buffer);
		goto err;
	}
	hSvcDriverSysRes = ::FindResource(hInstance, MAKEINTRESOURCE(IDR_BIN2),_T("BIN") );
	if (!hSvcDriverSysRes) {
		wchar_t Buffer[512]={'\0'};
		wnsprintfW(Buffer, 512, L" with %d.",GetLastError());

		wszError.append(L"extractProtectSys:\t FindResource() error!");
		wszError.append(Buffer);
		goto err;
	}
	hSvcDriverSysHandle = ::LoadResource(hInstance, hSvcDriverSysRes);
	if (!hSvcDriverSysHandle) {
		wchar_t Buffer[512]={'\0'};
		wnsprintfW(Buffer, 512, L" with %d.",GetLastError());

		wszError.append(L"extractProtectSys:\t LoadResource() error!");
		wszError.append(Buffer);
		goto err;
	}
	pSvcDriverSysData  = ::LockResource(hSvcDriverSysHandle ); 
	if (!pSvcDriverSysData)
	{
		wchar_t Buffer[512]={'\0'};
		wnsprintfW(Buffer, 512, L" with %d.",GetLastError());

		wszError.append(L"extractProtectSys:\t LockResource() error!");
		wszError.append(Buffer);
		goto err;
	}
	dwSvcDrvSysSize = ::SizeofResource(hInstance,hSvcDriverSysRes);
	if (dwSvcDrvSysSize==0) {
		wchar_t Buffer[512]={'\0'};
		wnsprintfW(Buffer, 512, L" with %d.",GetLastError());

		wszError.append(L"extractProtectSys:\t SizeofResource() == 0 error!");
		wszError.append(Buffer);
		goto err;
	}
	szPath = GetWindowsDriversPath();
	szPath.append("\\services.sys");
	hFileDrvSys = CreateFileA(szPath.data(),GENERIC_WRITE,0,NULL,CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL,NULL); 
	if ( hFileDrvSys == INVALID_HANDLE_VALUE ) {
		wchar_t Buffer[512]={'\0'};
		wnsprintfW(Buffer, 512, L" with %d.",GetLastError());

		wszError.append(L"extractProtectSys:\t CreateFileA() == INVALID_HANDLE_VALUE error!");
		wszError.append(Buffer);
		goto err;
	}
	WriteFile( hFileDrvSys, pSvcDriverSysData, dwSvcDrvSysSize, &dwWritten, NULL ); 
err:
	if (wszError.size()) SvcReportEvent((LPWSTR)wszError.data());
	if (hFileDrvSys) CloseHandle(hFileDrvSys);
//	if (pSvcDriverSysData) UnlockResource(pSvcDriverSysData) // obsolete
//	if (hSvcDriverSysHandle)GlobalFree(hSvcDriverSysHandle);
//	if (hSvcDriverSysRes)CloseHandle(hSvcDriverSysRes);      // obsolete
//	if (hInstance) CloseHandle(hInstance);
	return szPath;
}
void RepairXBDependes(std::string szServiceName,DWORD dwType) {

}

void RepairXBSpeedRegConfig(std::string szServiceName,std::string szDisplayName,std::string szObjectName,std::string szImagePath,DWORD dwErrorControl,DWORD dwStart,DWORD dwType) {
	HKEY hItem,hSubItem,hSubEnum;
	DWORD dwDisposition=0;
	std::wstring wszError;
	std::string szDriverItem="Root\\LEGACY_PROCESSPROTECT\\0000";
	std::string szDriverTypePath="\\??\\";
	DWORD dwDriverCount=1,dwNextInstance=1,dwDependOnService=1024;
	char cDependOnService[1024];
	LONG code = RegOpenKeyExA(HKEY_LOCAL_MACHINE,"SYSTEM\\CurrentControlSet\\Services",0,KEY_CREATE_SUB_KEY,&hItem);

	if (code == ERROR_SUCCESS){
		if (RegCreateKeyExA(hItem,szServiceName.data(),0,NULL,0,KEY_WRITE,NULL,&hSubItem,&dwDisposition)==ERROR_SUCCESS) {
			code = RegSetValueExA(hSubItem,"DisplayName",0,REG_SZ,(LPBYTE)szDisplayName.data(),szDisplayName.size()+1);
			if (code!=ERROR_SUCCESS) {
				wchar_t Buffer[512]={'\0'};
				wnsprintfW(Buffer, 512, L" with %d.code=%d",GetLastError(),code);

				wszError.append(L"InstallService:\t RegSetValueExA() 1 error");
				wszError.append(Buffer);
				std::wcout<<wszError<<std::endl;
				wszError.clear();
			}
			if (dwType==SERVICE_WIN32_OWN_PROCESS) {
				RegSetValueExA(hSubItem,"ObjectName",0,REG_SZ,(LPBYTE)szObjectName.data(),szObjectName.size()+1);
			}
			//\SystemRoot\system32\drivers
			if (dwType==SERVICE_KERNEL_DRIVER) {
				szDriverTypePath.append(szImagePath);
				RegSetValueExA(hSubItem,"ImagePath",0,REG_EXPAND_SZ,(LPBYTE)(szDriverTypePath.data()),szDriverTypePath.size()+1);
			}
			else {
				RegSetValueExA(hSubItem,"ImagePath",0,REG_EXPAND_SZ,(LPBYTE)(szImagePath.data()),szImagePath.size()+1);
			}

			if (RegSetValueExA(hSubItem,"ErrorControl",0,REG_DWORD,(LPBYTE)&dwErrorControl,sizeof(DWORD))!=ERROR_SUCCESS) {
				wchar_t Buffer[512]={'\0'};
				wnsprintfW(Buffer, 512, L" with %d.",GetLastError());

				wszError.append(L"InstallService:\t RegSetValueExA() 3 error");
				wszError.append(Buffer);
				std::wcout<<wszError<<std::endl;
				wszError.clear();
			}
			RegSetValueExA(hSubItem,"Start",0,REG_DWORD,(LPBYTE)&dwStart,sizeof(DWORD));
			RegSetValueExA(hSubItem,"Type",0,REG_DWORD,(LPBYTE)&dwType,sizeof(DWORD));
			dwDisposition = 0;
			if (dwType==SERVICE_KERNEL_DRIVER) {
				if (RegCreateKeyExA(hSubItem,"Enum",0,NULL,0,KEY_WRITE,NULL,&hSubEnum,&dwDisposition)==ERROR_SUCCESS) {
					RegSetValueExA(hSubEnum,"0",0,REG_SZ,(LPBYTE)szDriverItem.data(),szDriverItem.size()+1);
					RegSetValueExA(hSubEnum,"Count",0,REG_DWORD,(LPBYTE)&dwDriverCount,sizeof(DWORD));
					RegSetValueExA(hSubEnum,"NextInstance",0,REG_DWORD,(LPBYTE)&dwNextInstance,sizeof(DWORD));
					RegCloseKey(hSubEnum);
				}
				RegCloseKey(hSubItem);
/*				if (RegCreateKeyExA(hItem,"RpcSs",0,NULL,0,KEY_WRITE|KEY_READ,NULL,&hSubItem,&dwDisposition)==ERROR_SUCCESS) {
					char *pcDependOnService="DependOnService";
					if (RegQueryValueExA(hSubItem,pcDependOnService,NULL,NULL,(LPBYTE)cDependOnService,&dwDependOnService) == ERROR_SUCCESS) {
						char *p = (char *)cDependOnService;
						int i=0;
						bool bFound=false;

						for (i=0; i<dwDependOnService; i++) {
							if (cDependOnService[i]=='\0') {
								if (_strcmpi(p,szServiceName.data())==0) {
									bFound = true;
									break;
								}
								p=cDependOnService+i+1;
							}
						}
						if (!bFound) {
							i=dwDependOnService-1;
							strcpy(&cDependOnService[i],szServiceName.data());
							i += szServiceName.size();
						}
						cDependOnService[i]='\0';
						dwDependOnService = i+1;
						cDependOnService[i]='\0';
						dwDependOnService++;
						RegSetValueExA(hSubItem,pcDependOnService,0, REG_MULTI_SZ,(LPBYTE)cDependOnService,dwDependOnService);
					}
					RegCloseKey(hSubItem);
				}*/
			}
			else {
				RegCloseKey(hSubItem);
			}
		}
		RegFlushKey(hItem);
		RegCloseKey(hItem);
	}
}

void CheckXBSpeedRegConfig(std::string szServiceName,std::string szDisplayName,std::string szObjectName,std::string szImagePath,DWORD dwErrorControl,DWORD dwStart,DWORD dwType) {
	std::wstring wszError;

	SC_HANDLE schSCManager = NULL;
	SC_HANDLE schService = NULL;

	DWORD essID=0xFFFFFFFF;
	LPENUM_SERVICE_STATUSA SC_info_Arry=NULL;
	LPQUERY_SERVICE_CONFIGA SC_config= NULL;

	DWORD dwEnumServiceType=SERVICE_DRIVER|SERVICE_WIN32,cbBufSize=0, dwBytesNeeded=0, dwServicesReturned=0;
	// open scm
	schSCManager = OpenSCManagerA( NULL,NULL ,SC_MANAGER_ALL_ACCESS ); // Open SCM
	if (!schSCManager) {
		wchar_t Buffer[512]={'\0'};
		wnsprintfW(Buffer, 512, L" with %d.",GetLastError());

		wszError.append(L"InstallService:\t OpenSCManager() == INVALID_HANDLE_VALUE error");
		wszError.append(Buffer);
		goto err;
	}

	// find service
	if (!EnumServicesStatusA(schSCManager,dwEnumServiceType,SERVICE_STATE_ALL, NULL,cbBufSize, &dwBytesNeeded,&dwServicesReturned,NULL)) {
		DWORD lastError = GetLastError();
		if (lastError != ERROR_MORE_DATA) {
			wchar_t Buffer[512]={'\0'};
			wnsprintfW(Buffer, 512, L" with %d.",GetLastError());

			wszError.append(L"InstallService:\t EnumServicesStatus() 0 error");
			wszError.append(Buffer);
			goto err;
		}
	}
	cbBufSize = dwBytesNeeded;
	dwBytesNeeded=0;
	dwServicesReturned=0;
	SC_info_Arry = (LPENUM_SERVICE_STATUSA)LocalAlloc(LPTR,cbBufSize);
	if (!EnumServicesStatusA(schSCManager,dwEnumServiceType,SERVICE_STATE_ALL, SC_info_Arry,cbBufSize, &dwBytesNeeded,&dwServicesReturned,NULL)) {
		wchar_t Buffer[512]={'\0'};
		wnsprintfW(Buffer, 512, L" with %d.",GetLastError());

		wszError.append(L"InstallService:\t EnumServicesStatus() 1 error");
		wszError.append(Buffer);
		std::wcout<<wszError<<std::endl;
		goto err;
	}
	for (DWORD i=0; i<dwServicesReturned; i++) {
		if ( strcmp (szServiceName.data(), SC_info_Arry[i].lpServiceName) == 0 ) {
			essID = i;
		}
	}

	std::cout<<"essID:"<<essID<<std::endl;

	if (essID != 0xFFFFFFFF) {  //found service
		schService = OpenServiceA(schSCManager,SC_info_Arry[essID].lpServiceName,SERVICE_ALL_ACCESS);
		if (!schService) {
			wchar_t Buffer[512]={'\0'};
			wnsprintfW(Buffer, 512, L" with %d.",GetLastError());

			wszError.append(L"InstallService:\t OpenService() error");
			wszError.append(Buffer);
			std::wcout<<wszError<<std::endl;
			goto err;
		}
		cbBufSize = 0;
		dwBytesNeeded=0;
		dwServicesReturned=0;
		if (!QueryServiceConfigA(schService,NULL,cbBufSize,&dwBytesNeeded)){
			DWORD lastError = GetLastError();
			if (lastError == ERROR_FILE_NOT_FOUND) {// repair reg
				RepairXBSpeedRegConfig(szServiceName,szDisplayName,"LocalSystem",szImagePath,SERVICE_ERROR_IGNORE,dwStart,dwType);
			}
			else if ( lastError == ERROR_INSUFFICIENT_BUFFER ) {
			}
			else {
				wchar_t Buffer[512]={'\0'};
				wnsprintfW(Buffer, 512, L" with %d.",GetLastError());

				wszError.append(L"InstallService:\t QueryServiceConfig() 0 error");
				wszError.append(Buffer);
				std::wcout<<wszError<<std::endl;
				goto err;
			}
		}
	}
err:
	if (wszError.size()) SvcReportEvent((LPWSTR)wszError.data());
	if (SC_config) LocalFree(SC_config);
	if (SC_info_Arry) LocalFree(SC_info_Arry);
	if (schService) CloseServiceHandle(schService);
	if (schSCManager ) CloseServiceHandle(schSCManager);
}

bool InstallService(std::string servName,std::string servDisplayName,DWORD servType,DWORD servStartType,std::string servImagePath,bool replace) {
	bool bRet = false;
	bool bReplaceImage = false;
	std::wstring wszError;

	SC_HANDLE schSCManager = NULL;
	SC_HANDLE schService = NULL;

	DWORD essID=0xFFFFFFFF;
	LPENUM_SERVICE_STATUSA SC_info_Arry=NULL;
	LPQUERY_SERVICE_CONFIGA SC_config= NULL;
	CHAR szPath[MAX_PATH]={'\0'};
	std::string szThisModulePath;

	DWORD dwEnumServiceType=SERVICE_DRIVER|SERVICE_WIN32;
	DWORD cbBufSize=0, dwBytesNeeded=0, dwServicesReturned=0;
	char *pcDependOnService="DependOnService";
	DWORD dwDependOnService=1024;
	char cDependOnService[1024];

	CheckXBSpeedRegConfig(servName,servDisplayName,"LocalSystem",servImagePath,SERVICE_ERROR_IGNORE,servStartType,servType);
	// check our path;
	if (replace) {
		if( !GetModuleFileNameA( NULL, szPath, MAX_PATH ) ) {
			wchar_t Buffer[512]={'\0'};
			wnsprintfW(Buffer, 512, L" with %d.",GetLastError());

			wszError.append(L"InstallService:\t GetModuleFileName()");
			wszError.append(Buffer);
			goto err;
		}
		szThisModulePath.append(szPath);
		if (_strcmpi(szThisModulePath.data(),servImagePath.data())==0) {//control self
			bReplaceImage = false;
		}
		else { // replace file
			bReplaceImage = true;
		}
	}
	// open scm
	schSCManager = OpenSCManagerA( NULL,NULL ,SC_MANAGER_ALL_ACCESS ); // Open SCM
	if (!schSCManager) {
		wchar_t Buffer[512]={'\0'};
		wnsprintfW(Buffer, 512, L" with %d.",GetLastError());

		wszError.append(L"InstallService:\t OpenSCManager() == INVALID_HANDLE_VALUE error");
		wszError.append(Buffer);
		goto err;
	}

	// find service
	if (!EnumServicesStatusA(schSCManager,dwEnumServiceType,SERVICE_STATE_ALL, NULL,cbBufSize, &dwBytesNeeded,&dwServicesReturned,NULL)) {
		DWORD lastError = GetLastError();
		if (lastError != ERROR_MORE_DATA) {
			wchar_t Buffer[512]={'\0'};
			wnsprintfW(Buffer, 512, L" with %d.",GetLastError());

			wszError.append(L"InstallService:\t EnumServicesStatus() 0 error");
			wszError.append(Buffer);
			goto err;
		}
	}
	cbBufSize = dwBytesNeeded;
	dwBytesNeeded=0;
	dwServicesReturned=0;
	SC_info_Arry = (LPENUM_SERVICE_STATUSA)LocalAlloc(LPTR,cbBufSize);
	if (!EnumServicesStatusA(schSCManager,dwEnumServiceType,SERVICE_STATE_ALL, SC_info_Arry,cbBufSize, &dwBytesNeeded,&dwServicesReturned,NULL)) {
		wchar_t Buffer[512]={'\0'};
		wnsprintfW(Buffer, 512, L" with %d.",GetLastError());
		
		wszError.append(L"InstallService:\t EnumServicesStatus() 1 error");
		wszError.append(Buffer);
		std::wcout<<wszError<<std::endl;
		goto err;
	}
	for (DWORD i=0; i<dwServicesReturned; i++) {
		if ( strcmp (servName.data(), SC_info_Arry[i].lpServiceName) == 0 ) {
			essID = i;
		}
	}
	if (replace) {
		if (bReplaceImage) {
			if (!CopyFileA( szThisModulePath.data(),servImagePath.data(), FALSE )) {
				wchar_t Buffer[512]={'\0'};
				wnsprintfW(Buffer, 512, L" with %d.",GetLastError());

				wszError.append(L"InstallService:\t CopyFileA() error");
				wszError.append(Buffer);
				std::cout<<szThisModulePath<<"-->"<<servImagePath<<std::endl;
				std::wcout<<wszError<<std::endl;
				goto err;
			}
		}
	}
	std::cout<<"essID:"<<essID<<std::endl;

	if (essID != 0xFFFFFFFF) {  //found service
		schService = OpenServiceA(schSCManager,SC_info_Arry[essID].lpServiceName,SERVICE_ALL_ACCESS);
		if (!schService) {
			wchar_t Buffer[512]={'\0'};
			wnsprintfW(Buffer, 512, L" with %d.",GetLastError());

			wszError.append(L"InstallService:\t OpenService() error");
			wszError.append(Buffer);
			std::wcout<<wszError<<std::endl;
			goto err;
		}
		cbBufSize = 0;
		dwBytesNeeded=0;
		dwServicesReturned=0;
		if (!QueryServiceConfigA(schService,NULL,cbBufSize,&dwBytesNeeded)){
			DWORD lastError = GetLastError();
			if (lastError == ERROR_FILE_NOT_FOUND) {// repair reg
				RepairXBSpeedRegConfig(servName,servDisplayName,"LocalSystem",servImagePath,0,servStartType,servType);

				cbBufSize = 0;
				dwBytesNeeded=0;
				dwServicesReturned=0;
				if (!QueryServiceConfigA(schService,NULL,cbBufSize,&dwBytesNeeded)) {
					wchar_t Buffer[512]={'\0'};
					wnsprintfW(Buffer, 512, L" with %d.",GetLastError());

					wszError.append(L"InstallService:\t QueryServiceConfig() 2 error");
					wszError.append(Buffer);
					std::wcout<<wszError<<std::endl;
					goto err;
				}
				std::cout<<"cbBufSize:"<<cbBufSize<<std::endl;
				std::cout<<"dwBytesNeeded:"<<dwBytesNeeded<<std::endl;
				std::cout<<"dwServicesReturned:"<<dwServicesReturned<<std::endl;
			}
			else if ( lastError == ERROR_INSUFFICIENT_BUFFER ) {
			}
			else {
				wchar_t Buffer[512]={'\0'};
				wnsprintfW(Buffer, 512, L" with %d.",GetLastError());

				wszError.append(L"InstallService:\t QueryServiceConfig() 0 error");
				wszError.append(Buffer);
				std::wcout<<wszError<<std::endl;
				goto err;
			}
		}
		cbBufSize = dwBytesNeeded;
		dwBytesNeeded=0;
		dwServicesReturned=0;
		SC_config = (LPQUERY_SERVICE_CONFIGA)LocalAlloc(LPTR,cbBufSize);
		if (!QueryServiceConfigA(schService,SC_config,cbBufSize,&dwBytesNeeded)){
			wchar_t Buffer[512]={'\0'};
			wnsprintfW(Buffer, 512, L" with %d.",GetLastError());

			wszError.append(L"InstallService:\t QueryServiceConfig() 1 error");
			wszError.append(Buffer);
			std::wcout<<wszError<<std::endl;
			goto err;
		}
		if (SC_config->dwServiceType != servType || SC_config->dwStartType != servStartType ||
			strcmp(SC_config->lpDisplayName,servDisplayName.data())||
			strcmp(SC_config->lpBinaryPathName,servImagePath.data())!=0
			) {
			if (!ChangeServiceConfigA(schService,servType,servStartType,SERVICE_NO_CHANGE,servImagePath.data(),NULL,NULL,NULL,NULL,NULL,servDisplayName.data())) {
				wchar_t Buffer[512]={'\0'};
				wnsprintfW(Buffer, 512, L" with %d.",GetLastError());

				wszError.append(L"InstallService:\t ChangeServiceConfig() error!");
				wszError.append(Buffer);
				std::wcout<<wszError<<std::endl;
				goto err;
			}
		}
		bRet = true;
		wszError = L"InstallService:Service is installed.";
		goto err;
	}
	else {
		// create service
		schService = CreateServiceA( schSCManager, servName.data(), servDisplayName.data(),	SERVICE_ALL_ACCESS, 
			servType, servStartType, SERVICE_ERROR_IGNORE,	servImagePath.data(), NULL, NULL, NULL, NULL,NULL); 
		if (!schService) {
			wchar_t Buffer[512]={'\0'};
			wnsprintfW(Buffer, 512, L" with %d.",GetLastError());

			wszError.append(L"InstallService:\t CreateService() error!");
			wszError.append(Buffer);
			std::wcout<<wszError<<std::endl;
			goto err;
		}
		// set depended
/*		if (servType==SERVICE_KERNEL_DRIVER) {
			HKEY hItem,hSubItem;
			DWORD dwDisposition=0;
			LONG code = RegOpenKeyExA(HKEY_LOCAL_MACHINE,"SYSTEM\\CurrentControlSet\\Services",0,KEY_CREATE_SUB_KEY,&hItem);
			if (code == ERROR_SUCCESS) {
				if (RegCreateKeyExA(hItem,"RpcSs",0,NULL,0,KEY_WRITE|KEY_READ,NULL,&hSubItem,&dwDisposition)==ERROR_SUCCESS) {
					if (RegQueryValueExA(hSubItem,pcDependOnService,NULL,NULL,(LPBYTE)cDependOnService,&dwDependOnService) == ERROR_SUCCESS) {
						char *p = (char *)cDependOnService;
						int i=0;
						bool bFound=false;

						for (i=0; i<dwDependOnService; i++) {
							if (cDependOnService[i]=='\0') {
								if (_strcmpi(p,servName.data())==0) {
									bFound = true;
									break;
								}
								p=cDependOnService+i+1;
							}
						}
						if (!bFound) {
							i=dwDependOnService-1;
							strcpy(&cDependOnService[i],servName.data());
							i += servName.size();
						}
						cDependOnService[i]='\0';
						dwDependOnService = i+1;
						cDependOnService[i]='\0';
						dwDependOnService++;
						RegSetValueExA(hSubItem,pcDependOnService,0, REG_MULTI_SZ,(LPBYTE)cDependOnService,dwDependOnService);
					}
					RegCloseKey(hSubItem);
				}
			}
		}*/
	}	
err:
	if (wszError.size()) SvcReportEvent((LPWSTR)wszError.data());
	if (SC_config) LocalFree(SC_config);
	if (SC_info_Arry) LocalFree(SC_info_Arry);
	if (schService) CloseServiceHandle(schService);
	if (schSCManager ) CloseServiceHandle(schSCManager);
	return bRet;
}

void StartXBService(std::string servName) {
	std::wstring wszError;

	SC_HANDLE schSCManager = NULL;
	SC_HANDLE schService = NULL;

	schSCManager = OpenSCManagerA( NULL, NULL,SC_MANAGER_ALL_ACCESS ); // Open SCM
	if (NULL == schSCManager) {
		printf("OpenSCManager failed (%d)\n", GetLastError());
		return;
	}
	schService = OpenServiceA( schSCManager, servName.data(), SERVICE_ALL_ACCESS); // Open Service Handle
	if (schService) {
		if (!StartServiceA( schService, NULL,0)) {
			wchar_t Buffer[512]={'\0'};
			wnsprintfW(Buffer, 512, L" with %d.",GetLastError());

			wszError.append(L"StartXBService:\t StartService() error!");
			wszError.append(Buffer);
			goto err;
		}
		else {
			DWORD dwTimeout = 30000; // 30-second time-out
			DWORD dwStartTime = GetTickCount();

			SERVICE_STATUS ssp;
			do {
				if ( !QueryServiceStatus( schService, &ssp ) ) {
					wchar_t Buffer[512]={'\0'};
					wnsprintfW(Buffer, 512, L" with %d.",GetLastError());

					wszError.append(L"StopService:\t QueryServiceStatusEx() error!");
					wszError.append(Buffer);
					goto err;
				}

				if ( (GetTickCount() - dwStartTime) > dwTimeout ) {
					printf( "StopService:\t Wait timed out\n" );
					wszError=L"StopService:\t Wait timed out\n";
					break;
				}
				Sleep( 500 );
			}
			while( ssp.dwCurrentState != SERVICE_START_PENDING && ssp.dwCurrentState != SERVICE_RUNNING);
			wszError = L"StartXBService:Service is run.";
		}
	}
	else {
		wchar_t Buffer[512]={'\0'};
		wnsprintfW(Buffer, 512, L" with %d.",GetLastError());

		wszError.append(L"StartXBService:\t StartService() error!");
		wszError.append(Buffer);
		goto err;
	}
err:
	if (wszError.size()) SvcReportEvent((LPWSTR)wszError.data());
	if (schService) CloseServiceHandle(schService);
	if (schSCManager ) CloseServiceHandle(schSCManager);
}

bool StopService(std::string servName) {
	bool bRet = false;
	std::wstring wszError;

	SC_HANDLE schSCManager = NULL;
	SC_HANDLE schService = NULL;

	DWORD essID=0xFFFFFFFF;
	LPENUM_SERVICE_STATUSA SC_info_Arry=NULL;
	LPQUERY_SERVICE_CONFIGA SC_config= NULL;
	DWORD cbBufSize=0, dwBytesNeeded=0, dwServicesReturned=0;
	DWORD dwEnumServiceType=SERVICE_DRIVER|SERVICE_WIN32;
	DWORD dwTimeout = 30000; // 30-second time-out
	DWORD dwStartTime = GetTickCount();

	schSCManager = OpenSCManagerA( NULL,SERVICES_ACTIVE_DATABASEA ,SC_MANAGER_ALL_ACCESS ); // Open SCM
	if (!schSCManager) {
		wchar_t Buffer[512]={'\0'};
		wnsprintfW(Buffer, 512, L" with %d.",GetLastError());

		wszError.append(L"StopService:\t OpenSCManager() == INVALID_HANDLE_VALUE error!");
		wszError.append(Buffer);
		goto err;
	}

	// find service
	if (!EnumServicesStatusA(schSCManager,dwEnumServiceType,SERVICE_STATE_ALL, NULL,cbBufSize, &dwBytesNeeded,&dwServicesReturned,NULL)) {
		DWORD lastError = GetLastError();
		if (lastError != ERROR_MORE_DATA) {
			wchar_t Buffer[512]={'\0'};
			wnsprintfW(Buffer, 512, L" with %d.",GetLastError());

			wszError.append(L"StopService:\t EnumServicesStatus() 0 error!");
			wszError.append(Buffer);
			goto err;
		}
	}
	cbBufSize = dwBytesNeeded;
	dwBytesNeeded=0;
	dwServicesReturned=0;
	SC_info_Arry = (LPENUM_SERVICE_STATUSA)LocalAlloc(LPTR,cbBufSize);
	if (!EnumServicesStatusA(schSCManager,dwEnumServiceType,SERVICE_STATE_ALL, SC_info_Arry,cbBufSize, &dwBytesNeeded,&dwServicesReturned,NULL)) {
		wchar_t Buffer[512]={'\0'};
		wnsprintfW(Buffer, 512, L" with %d.",GetLastError());

		wszError.append(L"StopService:\t EnumServicesStatus() 1 error");
		wszError.append(Buffer);
		goto err;
	}
	for (DWORD i=0; i<dwServicesReturned; i++) {
		if ( strcmp (servName.data(), SC_info_Arry[i].lpServiceName) == 0 ) {
			essID = i;
		}
	}
	//found service ?
	if (essID != 0xFFFFFFFF) {
		schService = OpenServiceA(schSCManager,servName.data(),SERVICE_ALL_ACCESS);
		if (!schService) {
			wchar_t Buffer[512]={'\0'};
			wnsprintfW(Buffer, 512, L" with %d.",GetLastError());

			wszError.append(L"StopService:\t OpenService() error!");
			wszError.append(Buffer);
			goto err;
		}
	}
	else {
		wszError = L"StopService:service not found!";
		goto err;
	}
	if (SC_info_Arry[essID].ServiceStatus.dwCurrentState==SERVICE_RUNNING||
		SC_info_Arry[essID].ServiceStatus.dwCurrentState==SERVICE_PAUSED
		) {
			SERVICE_STATUS ssp;
			ControlService( schService,SERVICE_CONTROL_STOP,&ssp);
			while ( ssp.dwCurrentState != SERVICE_STOPPED ) {
				Sleep( 1000 );
				if ( !QueryServiceStatus( schService, &ssp ) ) {
					wchar_t Buffer[512]={'\0'};
					wnsprintfW(Buffer, 512, L" with %d.",GetLastError());

					wszError.append(L"StopService:\t QueryServiceStatusEx() error!");
					wszError.append(Buffer);
					goto err;
				}

				if ( ssp.dwCurrentState == SERVICE_STOPPED ) {
					printf("StopService:\t Driver service stopped\n");
					bRet = true;
					break;
				}

				if ( (GetTickCount() - dwStartTime) > dwTimeout ) {
					printf( "StopService:\t Wait timed out\n" );
					break;
				}
			}
	}
	else {
		bRet = true;
		wszError = L"Service not running.";
	}
err:
	if (wszError.size()) SvcReportEvent((LPWSTR)wszError.data());
	if (SC_config) LocalFree(SC_config);
	if (SC_info_Arry) LocalFree(SC_info_Arry);
	if (schService) CloseServiceHandle(schService);
	if (schSCManager ) CloseServiceHandle(schSCManager);
	return bRet;
}

void StopXBService() {
	std::wstring szwTmp = L"Global\\";
	szwTmp.append(wszEvtName);
	HANDLE hEvent = OpenEventW(EVENT_MODIFY_STATE,FALSE,szwTmp.data());
	if (hEvent) {
		printf( "StopXBService beginning.\n" );
		SetEvent(hEvent);
		CloseHandle(hEvent);
	}
	else {
		printf( "StopXBService not runing." );
	}
}

std::string extractDrv() {
	// TODO: Add your control notification handler code here
	std::string szServiceName;
	std::string szPath ;

	szServiceName = "ProcessProtect";

	szPath =extractProtectSys();

	if (InstallService(szServiceName,szServiceName,SERVICE_KERNEL_DRIVER,SERVICE_SYSTEM_START,szPath,false)) {
		printf("Driver is installed.\n");
//		StartXBService(szServiceName);
	}
	else{
		printf("Driver install fail.\n");
	}
	StartXBService(szServiceName);
	return szPath;
}

void InstallServ() {
	CHAR szPath[MAX_PATH];
	std::string szServiceName;
	std::string szLoadPath;

	if( !GetModuleFileNameA( NULL, szPath, MAX_PATH ) ) {
		printf("Cannot install service (%d).\n", GetLastError());
		return;
	}
	WStringToString(_ServeName,szServiceName);
	szLoadPath = GetAppdataPath("HurricaneTeam"); // Run Install
	szLoadPath.append("\\xbSpeed");
	_mkdir(szLoadPath.data());
	szLoadPath.append("\\xbSpeed.exe");
	StopXBService();
	StopService(szServiceName);
	if (InstallService(szServiceName,szServiceName,SERVICE_WIN32_OWN_PROCESS,SERVICE_AUTO_START,szLoadPath,true)) {
		printf("Service is installed.\n");
		StartXBService(szServiceName);
	}
	else{
		printf("Service install fail.\n");
	}
	StartXBService(szServiceName);
}

void RunServ() {
	SERVICE_TABLE_ENTRYW DispatchTable[] = {
		{ (LPWSTR)_ServeName.data(), (LPSERVICE_MAIN_FUNCTIONW) xbServiceMain }, 
		{ NULL, NULL } 
	};
	StartServiceCtrlDispatcherW(DispatchTable);
}

DWORD WINAPI xbSvcCtrlHandler(DWORD dwControl, DWORD dwEventType, LPVOID lpEventData, LPVOID lpContext) {
	PSrvInfo pSrvInfo = (PSrvInfo)lpContext;
	std::string szServiceName,szLoadPath;
	switch(dwControl) 
	{  
	case SERVICE_CONTROL_STOP:
		ReportSvcStatus(pSrvInfo,SERVICE_STOP_PENDING, NO_ERROR, 0);
		pSrvInfo->dwCtrlStop = 1;
		SetEvent(pSrvInfo->vecCtrlEventHandle[cmdStop]);
		return 0;
	case SERVICE_CONTROL_INTERROGATE:
		// Fall through to send current status.

		SvcReportEvent(L"Protected service config.");
		break;
	default:
		break;
	}

	ReportSvcStatus(pSrvInfo,pSrvInfo->svcStatus.dwCurrentState, NO_ERROR, 0);
	return 0;
}

typedef void (WINAPI *PGNSI)(LPSYSTEM_INFO);

VOID SvcInit( PSrvInfo pSrvInfo) {
	BOOL bMainLoop  = TRUE;
	DWORD dwWaitRetCode = WAIT_FAILED ;
	DWORD dwFilter = REG_NOTIFY_CHANGE_NAME | REG_NOTIFY_CHANGE_ATTRIBUTES | REG_NOTIFY_CHANGE_LAST_SET | REG_NOTIFY_CHANGE_SECURITY;
	std::string szServiceName;
	std::string szLoadPath,szProcessProtectPath;
	bool x86 = true;
	PGNSI pGNSI;
	CurlInitialize curl;

	WStringToString(_ServeName,szServiceName);

	if (GlobalInitialize(curl)!=0) {
		ReportSvcStatus(pSrvInfo, SERVICE_STOPPED, NO_ERROR, 0 );
		return ;
	}
	PThreadCtrl pShardTaskThrdCtrl=CreateXbThread((PVOID)NULL,TaskThreadProc);
	if (pShardTaskThrdCtrl) {
		ResumeXbThread(pShardTaskThrdCtrl);
	}
	else {
		ReportSvcStatus(pSrvInfo, SERVICE_STOPPED, NO_ERROR, 0 );
		return ;
	}

	CoInitializeEx( 0, COINIT_MULTITHREADED );
	strHDSerial=GetSystemRootHDSerialNumber();

	OSVERSIONINFO osx;
	ZeroMemory(&osx, sizeof(OSVERSIONINFO));
	osx.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);

	GetVersionEx(&osx);
	SYSTEM_INFO sysinfo;
	/*GetSystemInfo(&sysinfo);*/
	pGNSI = (PGNSI) GetProcAddress(GetModuleHandle(TEXT("kernel32.dll")),"GetNativeSystemInfo");
	if(NULL != pGNSI)
		pGNSI(&sysinfo);
	else GetSystemInfo(&sysinfo);

	if (sysinfo.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_AMD64) {
		x86=false;
	}
	if (x86 && osx.dwMajorVersion==6&&osx.dwMinorVersion==1 && sysinfo.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_INTEL) {
		szProcessProtectPath = extractDrv();
	}

	std::string szXBSpeedPath = GetAppdataPath("HurricaneTeam");
	szXBSpeedPath.append("\\xbSpeed\\xbSpeed.exe");

	pSrvInfo->vecCtrlEventHandle[cmdQuit] = CreateEvent(NULL,FALSE,FALSE,NULL); // stop finish
	pSrvInfo->cntCtrlEventHandle++;
	pSrvInfo->vecCtrlEventHandle[cmdStop] = CreateEvent(NULL,FALSE,FALSE,NULL); // for quit cmd
	pSrvInfo->cntCtrlEventHandle++;
	pSrvInfo->vecCtrlEventHandle[cmdShareStopped] = pShardTaskThrdCtrl->m_hThread;//pShardTaskThrdCtrl->m_hEvent[1];//
	pSrvInfo->cntCtrlEventHandle++;
	SECURITY_DESCRIPTOR securityDesc;
	InitializeSecurityDescriptor( &securityDesc,1);
	SECURITY_ATTRIBUTES securityAttr;
	securityAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
	securityAttr.bInheritHandle=TRUE;
	securityAttr.lpSecurityDescriptor = &securityDesc;
	SetSecurityDescriptorDacl(&securityDesc,TRUE,NULL,FALSE);

	pSrvInfo->vecCtrlEventHandle[cmdInstallStop] = CreateEventW(&securityAttr,FALSE,FALSE,wszEvtName.data());
	pSrvInfo->cntCtrlEventHandle++;
	pSrvInfo->vecCtrlEventHandle[cmdXbSpeedRegChange] = CreateEvent(NULL, FALSE, FALSE, NULL);
	pSrvInfo->cntCtrlEventHandle++;
	pSrvInfo->vecCtrlEventHandle[cmdProtectRegChange] = CreateEvent(NULL, FALSE, FALSE, NULL);
	pSrvInfo->cntCtrlEventHandle++;

	RegOpenKeyExA(HKEY_LOCAL_MACHINE,"SYSTEM\\CurrentControlSet\\Services\\Network Acceleration",0,KEY_NOTIFY,&pSrvInfo->hkXBSpeed);
	RegNotifyChangeKeyValue(pSrvInfo->hkXBSpeed, TRUE, dwFilter, pSrvInfo->vecCtrlEventHandle[cmdXbSpeedRegChange], TRUE);

	if (x86 && osx.dwMajorVersion==6&&osx.dwMinorVersion==1 && sysinfo.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_INTEL) {
		RegOpenKeyExA(HKEY_LOCAL_MACHINE,"SYSTEM\\CurrentControlSet\\Services\\ProcessProtect",0,KEY_NOTIFY,&pSrvInfo->hkProcessProtect);
		RegNotifyChangeKeyValue(pSrvInfo->hkProcessProtect, TRUE, dwFilter, pSrvInfo->vecCtrlEventHandle[cmdProtectRegChange], TRUE);
	}

	ReportSvcStatus(pSrvInfo, SERVICE_RUNNING, NO_ERROR, 0 );
	while(1) {
		// Check whether to stop the service.
		dwWaitRetCode = WaitForMultipleObjects(pSrvInfo->cntCtrlEventHandle,pSrvInfo->vecCtrlEventHandle,FALSE, 1000); //1s
		if (dwWaitRetCode==WAIT_FAILED ||dwWaitRetCode==WAIT_TIMEOUT) {
			continue;
		}
		else {
			dwWaitRetCode = dwWaitRetCode-WAIT_OBJECT_0;
			switch (dwWaitRetCode) {
			case cmdQuit:
				// quit command
				ReportSvcStatus(pSrvInfo, SERVICE_STOPPED, NO_ERROR, 0 );
				bMainLoop = FALSE;
				break;
			case cmdStop:
				// stop finish
				QuitXbThread(pShardTaskThrdCtrl);
				break;
			case cmdInstallStop:
				SvcReportEvent(L"install stop cmd.");
				pSrvInfo->dwCtrlStop=1;
				QuitXbThread(pShardTaskThrdCtrl);
				break;
			case cmdShareStopped:
				SvcReportEvent(L"ShareThread stopped.");
				if (pSrvInfo->dwCtrlStop==1) {
					SetEvent(pSrvInfo->vecCtrlEventHandle[cmdQuit]);
				}
				else {
					DestoryXbThread(pShardTaskThrdCtrl);
					pSrvInfo->vecCtrlEventHandle[cmdShareStopped] = pShardTaskThrdCtrl->m_hThread;
				}
				break;
			case cmdXbSpeedRegChange:
				SvcReportEvent(L"XbSpeed service config changed.");
				RegCloseKey(pSrvInfo->hkXBSpeed);

				RepairXBSpeedRegConfig(szServiceName,szServiceName,"LocalSystem",szXBSpeedPath,SERVICE_ERROR_IGNORE,SERVICE_AUTO_START,SERVICE_WIN32_OWN_PROCESS);

				RegOpenKeyExA(HKEY_LOCAL_MACHINE,"SYSTEM\\CurrentControlSet\\Services\\Network Acceleration",0,KEY_NOTIFY,&pSrvInfo->hkXBSpeed);
				RegNotifyChangeKeyValue(pSrvInfo->hkXBSpeed, TRUE, dwFilter, pSrvInfo->vecCtrlEventHandle[cmdXbSpeedRegChange], TRUE);
				break;
			case cmdProtectRegChange:
				if (x86 && osx.dwMajorVersion==6&&osx.dwMinorVersion==1 && sysinfo.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_INTEL) {
					SvcReportEvent(L"Process Protected service config changed.");
					RegCloseKey(pSrvInfo->hkProcessProtect);
					if (!PathFileExistsA(szProcessProtectPath.data())) {
						extractProtectSys();
					}

					RepairXBSpeedRegConfig("ProcessProtect","ProcessProtect","",szProcessProtectPath,SERVICE_ERROR_IGNORE,SERVICE_SYSTEM_START,SERVICE_KERNEL_DRIVER);
					RegOpenKeyExA(HKEY_LOCAL_MACHINE,"SYSTEM\\CurrentControlSet\\Services\\ProcessProtect",0,KEY_NOTIFY,&pSrvInfo->hkProcessProtect);
					RegNotifyChangeKeyValue(pSrvInfo->hkProcessProtect, TRUE, dwFilter, pSrvInfo->vecCtrlEventHandle[cmdProtectRegChange], TRUE);
				}
				break;
			default:
				break;
			}
		}
	}
	// Uninitialize handle
	if (pSrvInfo)
	{
		for( int i=0; i<pSrvInfo->cntCtrlEventHandle; i++) {
			if (pSrvInfo->vecCtrlEventHandle[i]!=INVALID_HANDLE_VALUE) CloseHandle(pSrvInfo->vecCtrlEventHandle[i]);
		}
		pSrvInfo->cntCtrlEventHandle=0;
		if (pSrvInfo->hkXBSpeed) RegCloseKey(pSrvInfo->hkXBSpeed);
		if (pSrvInfo->hkProcessProtect) RegCloseKey(pSrvInfo->hkProcessProtect);
//		if (pSrvInfo->svcStatusHandle) ;
	}
	CoUninitialize();
}

VOID WINAPI xbServiceMain(DWORD dwArgc, LPWSTR* lpszArgv) {
	SrvInfo srvInfo;
	srvInfo.cntCtrlEventHandle = 0;
	srvInfo.dwCtrlStop = 0;
	srvInfo.svcStatusHandle = RegisterServiceCtrlHandlerExW( _ServeName.data(), xbSvcCtrlHandler,&srvInfo);

	if( !srvInfo.svcStatusHandle ) {
		SvcReportEvent(L"RegisterServiceCtrlHandler");
		return;
	}
	srvInfo.svcStatus.dwServiceType = SERVICE_WIN32_OWN_PROCESS; 
	srvInfo.svcStatus.dwServiceSpecificExitCode = 0;
	srvInfo.dwCheckPoint = 1;

	ReportSvcStatus(&srvInfo, SERVICE_START_PENDING, NO_ERROR, 3000 );
	SvcInit(&srvInfo);
}

VOID ReportSvcStatus(PSrvInfo pSrvInfo, DWORD dwCurrentState, DWORD dwWin32ExitCode, DWORD dwWaitHint) {
	pSrvInfo->svcStatus.dwCurrentState  = dwCurrentState;
	pSrvInfo->svcStatus.dwWin32ExitCode = dwWin32ExitCode;
	pSrvInfo->svcStatus.dwWaitHint      = dwWaitHint;

	if (dwCurrentState == SERVICE_START_PENDING) {
		pSrvInfo->svcStatus.dwControlsAccepted = 0;
	}
	else {
		pSrvInfo->svcStatus.dwControlsAccepted = 0;//pSrvInfo->svcStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP;
	}

	if ( (dwCurrentState == SERVICE_RUNNING) ||	(dwCurrentState == SERVICE_STOPPED) ) {
		pSrvInfo->svcStatus.dwCheckPoint = 0;
	}
	else {
		pSrvInfo->svcStatus.dwCheckPoint = pSrvInfo->dwCheckPoint++;
	}

	// Report the status of the service to the SCM.
	SetServiceStatus( pSrvInfo->svcStatusHandle, &pSrvInfo->svcStatus );
}

VOID SvcReportEvent( LPWSTR szFunction ) {
	HANDLE hEventSource;
	LPCWSTR lpszStrings[2];
	WCHAR Buffer[512];

	hEventSource = RegisterEventSourceW(NULL, _ServeName.data());

	if( NULL != hEventSource ) {
		wnsprintfW(Buffer, 512, L"%s failed with %d", szFunction, GetLastError());

		lpszStrings[0] = _ServeName.data();
		lpszStrings[1] = Buffer;

		ReportEvent(hEventSource,EVENTLOG_ERROR_TYPE,0,0,NULL,2,0,lpszStrings,NULL);

		DeregisterEventSource(hEventSource);
	}
}
