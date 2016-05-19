// generateCFG.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "HttpFunctions.h"

int test_func(){
	return 0;
}

BOOL StringToWString(const std::string &str,std::wstring &wstr) {
	int nLen = (int)str.length();    
	wstr.resize(nLen,L' ');

	int nResult = MultiByteToWideChar(CP_ACP,0,(LPCSTR)str.c_str(),nLen,(LPWSTR)wstr.c_str(),nLen);

	if (nResult == 0) {
		return FALSE;
	}

	return TRUE;
}
BOOL WStringToString(const std::wstring &wstr,std::string &str) {
	int nLen = (int)wstr.length();    
	str.resize(nLen,' ');

	int nResult = WideCharToMultiByte(CP_ACP,0,(LPCWSTR)wstr.c_str(),nLen,(LPSTR)str.c_str(),nLen,NULL,NULL);

	if (nResult == 0) {
		return FALSE;
	}

	return TRUE;
}

DownWrapper* LoadDll()
{
	WCHAR szDllpath[512] = {0};
	std::string szLoadPath;
	szLoadPath.append(".\\xldl.dll");

	std::wstring wstr;
	StringToWString(szLoadPath,wstr);
	StrCpyW(szDllpath,wstr.data());
	DownWrapper* pWapper = new DownWrapper(szDllpath);
	return pWapper;
}

void UnloadDll(DownWrapper** Wapper){
	if (!Wapper) {
		return ;
	}
	if ((*Wapper)!=NULL) {
		(*Wapper)->UnInit();
		delete (*Wapper);
		(*Wapper) = NULL;
	}
}
class CurlInitialize
{
public:
	CurlInitialize() {
		m_bStatus = FALSE;
		m_pWapper = LoadDll();
		if (!m_pWapper) {
			m_bStatus=FALSE;
			return;
		}
		if (::curl_global_init(CURL_GLOBAL_WIN32)==CURLE_OK)
			m_bStatus = TRUE;

	}
	~CurlInitialize() {
		if (m_pWapper) {
			UnloadDll(&m_pWapper);
		}
		if (m_bStatus) {
			::curl_global_cleanup();
		}
		m_bStatus = FALSE;

	}
protected:
	BOOL m_bStatus;
	DownWrapper* m_pWapper;
public:
	BOOL GetStatus() {
		return m_bStatus;
	}
	DownWrapper* GetWrapper() {
		return m_pWapper;
	}
};

int main(int argc, char *argv[])
{
//	return test_func();
	std::string szSaveDir;
	std::string szCfgDir;
	char buf[1024] = {'\0'};
	::GetModuleFileNameA(NULL, buf, MAX_PATH);
	::PathRemoveFileSpecA(buf);
//	std::cout<<argv[0]<<std::endl;
	std::cout<<buf<<std::endl;

	szSaveDir.append(buf);
	szCfgDir = szSaveDir;
	szSaveDir.append("\\DownTdcfg");
	szCfgDir.append("\\GenCFG");

	_mkdir(szSaveDir.data());
	_mkdir(szCfgDir.data());

	std::wcout.imbue(std::locale("chs"));
	CurlInitialize _init_env_;
	if (!_init_env_.GetStatus()) {
		std::wcout<<L"初始化失败！！！"<<std::endl;
		return 1;
	}

	_init_env_.GetWrapper()->Init();

	// get less 1k server config list
	std::string szConfigUrl, szConfigLists;
	int nTimes=3;
	while((nTimes--)>0) {
//		szConfigUrl="127.0.0.1:8080";
		std::wcout<<std::endl<<std::endl<<L"请输入迅雷文件配置生成器的服务器地址:";
		std::cout<<szConfigUrl;
		std::cin>>szConfigUrl;
		if (szConfigUrl.size()>0){
			std::cout<<std::endl<<szConfigUrl<<std::endl;
			break;
		}
		else {
			std::wcout<<std::endl<<L"你没有输入任何内容！！！";
		}
		// get list from server
	}
	if (szConfigUrl.empty()) {
		std::wcout<<std::endl<<L"输入错误,程序将退出"<<std::endl;
		return 1;
	}
	std::wcout<<L"正在从服务器:";
	std::cout<<szConfigUrl;
	std::wcout<<L",获取配置文件"<<std::endl;
	if ( !GetResourceFromHttp(szConfigUrl,&szConfigLists) || szConfigLists.size()==0) {
		std::wcout<<std::endl<<L"指定的服务器地址:";
		std::cout<<szConfigUrl;
		std::wcout<<L",你没有任何内容！！！"<<std::endl;
		return 0;
	}
	std::cout<<szConfigLists<<std::endl;
	// parse config list
	std::vector<std::string> lstFileItems;
	if (!ParseConfigList(szConfigLists,lstFileItems)) {
		std::wcout<<L"从服务器获取的配置解析错误."<<std::endl;
	}
	else {
		for (std::vector<std::string>::iterator it = lstFileItems.begin();
			it != lstFileItems.end();
			it++) {
				std::cout<<*it<<std::endl;
		}
	}
	// prompt select operation code
	bool bWaitCmd=true;
	bool execbuild=false;
	while(bWaitCmd) {
		std::string cmd;
		std::wcout<<L"**请执行命令:"<<std::endl;
		std::wcout<<L"\t确认执行:(Y/y)"<<std::endl;
		std::wcout<<L"\t取消执行:(N/n)"<<std::endl;
		std::cin>>cmd;
		if (cmd.size()==1) {
			switch(cmd[0]){
			case 'Y':
			case 'y':
				std::wcout<<L"开始生成..."<<std::endl;
				bWaitCmd = false;
				execbuild = true;
				break;
			case 'N':
			case 'n':
				std::wcout<<L"取消执行..."<<std::endl;
				bWaitCmd = false;
				execbuild = false;
				break;
			default:
				break;
			}
		}
		else {
			std::wcout<<L"输入错误"<<std::endl;
		}
	}
	// execute command and display processing
	if (!execbuild) {
		return 0;
	}
	// begin generate cfg.
	std::wstring wszSaveDir;

	StringToWString(szSaveDir, wszSaveDir);
	time_t timeVar = time(NULL);
	std::cout<<"Begin time at:"<<ctime(&timeVar)<<std::endl;
	for (std::vector<std::string>::iterator it = lstFileItems.begin();
		it != lstFileItems.end();
		it++) {
			std::wstring wstrUrl, wstrFileName;
			
			StringToWString(*it,wstrUrl);
			wstrFileName = wstrUrl.substr(wstrUrl.find(L"df=")+3);
			std::wcout<<wstrUrl<<std::endl;//<<L"::"<<wstrFileName
			std::string szSrcCfg,szDestCfg,szTmp;

			szTmp.clear();

			WStringToString(wstrFileName,szTmp);
//-------------------------------------------------------
/*			std::string strFileName="http://";
			strFileName.append(szConfigUrl);
			strFileName.append("/iso/");
			strFileName.append(szTmp);
			wstrUrl.clear();
			StringToWString(strFileName,wstrUrl);*/
//-------------------------------------------------------

			szSrcCfg = szSaveDir;
			szSrcCfg.append("\\");

			szSrcCfg.append(szTmp);
			szSrcCfg.append(".td.cfg");

			szDestCfg = szCfgDir;
			szDestCfg.append("\\");
			szDestCfg.append(szTmp);
			szDestCfg.append(".td.cfg");

			DownTaskParam downTaskParam;
			HANDLE hTask = NULL;

			StrCpyW(downTaskParam.szFilename, wstrFileName.data());
			StrCpyW(downTaskParam.szTaskUrl,  wstrUrl.data());
			StrCpyW(downTaskParam.szSavePath, wszSaveDir.data());
			downTaskParam.IsOnlyOriginal=TRUE;
			hTask = _init_env_.GetWrapper()->TaskCreate(downTaskParam);
			_init_env_.GetWrapper()->TaskStart(hTask);
			
			DownTaskInfo info;
			char szPercent[10]={'\0'};
			nTimes=0;
			while(true) {
				memset(&info,0,sizeof(info));
				if (!_init_env_.GetWrapper()->TaskQueryEx(hTask, info)) {
					std::wcout<<std::endl<<L"文件:"<<wstrFileName<<L",生成失败"<<std::endl;
					break;
				}
				_snprintf(szPercent,10,"%02f",info.fPercent*100);
				std::wcout<<L'\r'<<wstrFileName<<L":"<<szPercent<<L'%'<<L"        ";
				for ( int i=0; i<nTimes; i++) {
					std::wcout<<L'.';
				}
				for ( int i=nTimes; i<10; i++) {
					std::wcout<<L' ';
				}

				nTimes++;
				nTimes = nTimes%10;
				if (info.stat == TSC_ERROR) {
					if (!PathFileExistsA(szSrcCfg.data())) {
						std::wcout<<std::endl<<L"文件:"<<wstrFileName<<L",生成失败"<<std::endl;
						break;
					}
					if (info.nTotalDownload>(info.nTotalSize-2*1024)&&info.nTotalDownload<info.nTotalSize) {
						std::wcout<<std::endl<<L"文件:"<<wstrFileName<<L",生成成功"<<std::endl;
						std::cout<<"src:"<<szSrcCfg<<std::endl;
						std::cout<<"src:"<<szDestCfg<<std::endl;
//						MoveFileExA(szSrcCfg.data(),szDestCfg.data(),MOVEFILE_REPLACE_EXISTING);
						if (PathFileExistsA(szDestCfg.data())) {
							DeleteFileA(szDestCfg.data());
						}
						CopyFileA(szSrcCfg.data(),szDestCfg.data(),FALSE);
					}
					else {
						std::wcout<<std::endl<<L"文件:"<<wstrFileName<<L",生成失败"<<std::endl;
					}
					break;
				}
				else if (info.stat == TSC_COMPLETE){
					std::wcout<<std::endl<<L"文件:"<<wstrFileName<<L",完整下载成功"<<std::endl;
					break;
				}
/*				else if (info.stat == TSC_DOWNLOAD){
					std::wcout<<std::endl<<L"Error! code="<<info.stat<<std::endl;
				}*/
				Sleep(1000);
			}
			DownTaskParam deleteParam;
			StrCpyW(deleteParam.szFilename, wstrFileName.data());
			StrCpyW(deleteParam.szSavePath, wszSaveDir.data());

			_init_env_.GetWrapper()->TaskDelete(hTask);
			_init_env_.GetWrapper()->DelTempFile(downTaskParam);
	}
	timeVar = time(NULL);
	std::cout<<"Finish time at:"<<ctime(&timeVar)<<std::endl;
	std::cout<<"*************************************"<<std::endl;
	// collect cfg to target directory

	// prompt result
	   // ex:which success or failed.

	return 0;
}

