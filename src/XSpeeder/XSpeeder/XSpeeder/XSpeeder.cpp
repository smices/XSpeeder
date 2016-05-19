
#include "global.h"
#include <iostream>
#include "xbStructDef.h"

#include "curl\curl.h"

#include "util.h"
#include "InitConf.h"
#include "svcImp.h"

//--------------------------------------------------------

int main(int argc, char* argv[]) {
	int nInstallMode = 0;
	//花指令
	__asm
	{
		jz  GG  
			jnz GG  
			_emit 0xe8 
GG:
	} //L"花指令end" 

	if (argc==1) {
		std::cout<<"This is a service."<<std::endl;
		nInstallMode = 0;
	}
	else if (argc==2) {
		// cmd mode
		if ( _strcmpi(argv[1],"Install")==0 ) {
			std::cout<<"Will install service!"<<std::endl;
			nInstallMode = 1;
		}
		else if ( _strcmpi(argv[1],"XSpeeder")==0 ) {
			nInstallMode = 2;
		}
		else {
			std::cout<<"Not support command parameter!"<<std::endl;
			nInstallMode = 0;
			return 1;
		}
	}
	else {
		std::cout<<"Not support num of parameters!"<<std::endl;
		nInstallMode = 0;
		return 1;
	}

	if (nInstallMode == 0) {
		RunServ();//service
		return 0;
	}
	else if (nInstallMode == 1) {
		InstallServ();//install
//		extractDrv();
		return 0;
	}
	else if (nInstallMode == 2) {
		;//run debug;
	}
	else {
		return 0;
	}

	CurlInitialize curl;
	if (GlobalInitialize(curl)!=0)
		return 1;
	//start business thread
	PThreadCtrl pShardTaskThrdCtrl=CreateXbThread((PVOID)NULL,TaskThreadProc);
	if ( pShardTaskThrdCtrl ) {
		ResumeXbThread(pShardTaskThrdCtrl);
		while(true) {
			//input ?
			char cmd = std::cin.get();
			//-->send command to thread
			if (cmd=='q') {
				QuitXbThread(pShardTaskThrdCtrl);
				WaitForSingleObject(pShardTaskThrdCtrl->m_hEvent[1],INFINITE);
				break;
			}
			Sleep(1000);
		}
	}
	if (pShardTaskThrdCtrl) {
		DestoryXbThread(pShardTaskThrdCtrl);
	}
	std::wcout << "waiting quit" << std::endl;
	Sleep(3000);

	// Uninitialize Engine
	UnloadDll(&g_pWapper);
	return 0;
}
