
#include "XSBusinessImp.h"
#include <iostream>
#include "xldl.h"
#include "DownWrapper.h"
#include "InitConf.h"
extern DownWrapper* g_pWapper;

void AddToSharing(mapCurSharingTask &mapSharingStatus,int status) {
	std::string szTaskTD,szTaskCFG;

	mapCurSharingTask::iterator it;
	std::string szTmp;
	std::string BaseDir = GetProgramProfilePath("XSpeeder");
	for (it = mapSharingStatus.begin();
		it!=mapSharingStatus.end();
		it++) {
			if (!it->second) {
				continue;
			}

			//prepare upload *.td and *.cfg
			if ( !it->second->matchPath.empty() && it->second->nPrepareTDCFG==0 ) {
				szTaskCFG = it->second->matchPath;
				szTaskCFG.append(".td.cfg");
				if (!PathFileExistsA(szTaskCFG.data())) {
					if (it->second->hTaskCfgObj!=NULL) // task running
						continue;

					AddToSharing(it->second,true);
				}
				else {
					it->second->nPrepareTDCFG=1;
				}
			}
			else if (!it->second->matchPath.empty() && it->second->nPrepareTDCFG==1){
				if (it->second->hTaskObj!=NULL)
					continue;
				// add
				if (it->second->nStatus==0 || it->second->nStatus==2) {
					szTaskTD = it->second->matchPath;
					szTaskTD.append(".td");
					MoveFileA(it->second->matchPath.data(),szTaskTD.data());
					AddToSharing(it->second,false);
				}
			}
	}
}

void AddToUpgrading(mapCurUpgradeTask &mapUpgradeStatus,int status) {
	std::string szPkgFile;
	std::wstring szTmp;
	std::string BaseDir = GetProgramProfilePath("XSpeeder");

	mapCurUpgradeTask::iterator it;
	for(it = mapUpgradeStatus.begin();
		it!= mapUpgradeStatus.end();
		it++) {
			if (!it->second) {
				continue;
			}

			if ( !it->second->savePath.empty() ) {
				szPkgFile = it->second->savePath;
				szPkgFile.append(it->second->item.FileName);
				szPkgFile.append(".upkg");
				if (!PathFileExistsA(szPkgFile.data())) {
					if (it->second->hTaskObj!=NULL) // task running
						continue;

					AddToUpgrading(it->second);
				}
				else {
					it->second->nStatus=3;
				}
			}
	}
}

void AddToUpgrading(PCurUpgradeTask item) {
	std::wstring wstr;
	std::string strTmp;
	DownTaskParam downTaskParam;

	HANDLE hTask = NULL;

	if (!item) {
		return;
	}

	wstr.clear();  StringToWString(item->item.FileName,wstr);
	wstr.append(L".upkg");
	StrCpyW(downTaskParam.szFilename, wstr.data());

	wstr.clear();  StringToWString(item->item.Download,wstr);
	StrCpyW(downTaskParam.szTaskUrl, wstr.data());

	wstr.clear();  StringToWString(item->savePath,wstr);
	StrCpyW(downTaskParam.szSavePath, wstr.data());

	downTaskParam.IsOnlyOriginal = TRUE;
	hTask = g_pWapper->TaskCreate(downTaskParam);
	if (hTask) {
		g_pWapper->TaskStart(hTask);
		item->hTaskObj = hTask;
		item->nStatus  = 1;//run
	}
}

void AddToSharing(PCurSharingTask item,bool bForCfg) {
	std::wstring wstr;
	std::string strTmp;
	DownTaskParam downTaskParam;
	HANDLE hTask = NULL;

	if (!item) {
		return;
	}

	wstr.clear();
	StringToWString(item->item.name,wstr);
	if (bForCfg) wstr.append(L".td.cfg");
	StrCpyW(downTaskParam.szFilename, wstr.data());

	wstr.clear();
	if (bForCfg) {
		StringToWString(item->item.tdConfigUrl,wstr);
	}
	else {
		StringToWString(item->item.downloadUrl,wstr);
	}
	StrCpyW(downTaskParam.szTaskUrl, wstr.data());

	wstr.clear();
	strTmp = GetFilePathFromFile(item->matchPath);
	StringToWString(strTmp,wstr);
	wstr.append(L"\\");
	StrCpyW(downTaskParam.szSavePath, wstr.data());

	downTaskParam.IsOnlyOriginal = bForCfg ? TRUE : FALSE;
	hTask = g_pWapper->TaskCreate(downTaskParam);
	if (hTask) {
		g_pWapper->TaskStart(hTask);
		if (bForCfg) {
			item->hTaskCfgObj = hTask;
		}
		else {
			item->hTaskObj = hTask;
			item->nStatus  = 1;//run
		}
	}
}

void DetectNewSharing (mapCurSharingTask &mapSharingStatus,int status,long &version) {
	std::string szTmp;
	PTaskConfDef taskConfNew=NULL;

	taskConfNew = LoadLocalShareConf();
	if (taskConfNew && taskConfNew->m_code==0) {
		TaskItems::iterator it;
		mapCurSharingTask::iterator itm;

		for (itm = mapSharingStatus.begin();
			itm!= mapSharingStatus.end();
			itm++
			) {
				if (taskConfNew->m_msg.m_files.find(itm->first) == \
					taskConfNew->m_msg.m_files.end()) {
						itm->second->nStatus=3;//remove
				}
		}

		for (it = taskConfNew->m_msg.m_files.begin();
			it!= taskConfNew->m_msg.m_files.end();
			it ++
			) {
				if (!it->second) {
					continue;
				}
				if (mapSharingStatus.find(it->first)!=mapSharingStatus.end()) {
					continue;
				}
				szTmp = ScanTarget(it->second);
				if (!szTmp.empty()) {
					PCurSharingTask item = CreateSharingTaskItem(szTmp,it->second);
					if (!item) {
						continue;
					}
					item->nStatus=status;
					mapSharingStatus.insert(mapCurSharingTask::value_type(it->first,item));
				}
		}
		version = taskConfNew->m_msg.m_version;
		AddToSharing(mapSharingStatus,status);
	}
	DestroyTaskConfDef(taskConfNew);
}

void DetectNewUpgrade (mapCurUpgradeTask &mapUpgradeStatus,int status,long &version) {
	std::string szTmp,szTmp1;
	PUpdateConfDef upgradeConfNew=NULL;
	
	UpdateItems::iterator it;
	mapCurUpgradeTask::iterator itm;
	
	std::string BaseDir = GetProgramProfilePath("XSpeeder");
	upgradeConfNew = LoadLocalUpdateConf();
	if (upgradeConfNew && upgradeConfNew->m_code==0) {

		for(itm = mapUpgradeStatus.begin();
			itm!= mapUpgradeStatus.end();
			itm++
			) {
				if (upgradeConfNew->m_msg.m_files.find(itm->first) == upgradeConfNew->m_msg.m_files.end()) {
						itm->second->nStatus=4;//remove
				}
		}

		for(it = upgradeConfNew->m_msg.m_files.begin();
			it!= upgradeConfNew->m_msg.m_files.end();
			it ++
			) {
				if (!it->second) {
					continue;
				}
				if (mapUpgradeStatus.find(it->first)!=mapUpgradeStatus.end()) {
					continue;
				}
				szTmp = BaseDir;
				szTmp.append("\\UpdateDir\\");
				szTmp.append(it->second->service);
				szTmp.append("\\");
				szTmp.append(it->second->LastVersion);
				_mkdir(szTmp.data());

				szTmp1 = szTmp;
				szTmp1.append("\\");
				szTmp1.append(it->second->FileName);
				if (!PathFileExistsA(szTmp1.data())) {
					// add new download
					PCurUpgradeTask item = CreateUpgradeTaskItem(it->second,szTmp);
					if (!item) {
						continue;
					}
					item->nStatus=status;
					mapUpgradeStatus.insert(mapCurUpgradeTask::value_type(it->first,item));
				}
				else {
//					PCurUpgradeTask item = CreateUpgradeTaskItem(it->second,szTmp);
				}
		}
		version = upgradeConfNew->m_msg.m_version;
		AddToUpgrading(mapUpgradeStatus,status);
	}
	DestroyUpdateConfDef(upgradeConfNew);
}

#include "svcImp.h"

DWORD TaskThreadProc(PThreadCtrl pThrdCtrl) {
	bool bUpdateConf = false,bUpgradeConf=false;
	DWORD dwTickCountCur = 0, dwTickCountLast= 0;
	PTaskConfDef taskConf = NULL;
	PUpdateConfDef upgradeConf = NULL;
	std::string szTmp,szTmp1;
	std::wstring wszTmp;
	long shareVersion=0,upgradeVersion=0;

	mapCurSharingTask mapSharingStatus;
	mapCurUpgradeTask mapUpgradeStatus;
	DownTaskInfo info;
	memset(&info,0,sizeof(info));

	PrintTaskInfo(info,true);//print title
	//load local taskdefine.conf
	taskConf = LoadLocalShareConf();
	//collect local task source file
	if (taskConf) {
		if (taskConf->m_code==9) {
			shareVersion=0;
		}
		else if(taskConf->m_code==0) {
			shareVersion = taskConf->m_msg.m_version;
		}
		else {
			shareVersion = 0;
		}
		ScanTarget(mapSharingStatus,taskConf);
		DestroyTaskConfDef(taskConf);
	}
	// load local upgrade task
	upgradeConf = LoadLocalUpdateConf();
	if (upgradeConf) {
		if (upgradeConf->m_code==9) {
			upgradeVersion=0;
		}
		else if(upgradeConf->m_code==0) {
			upgradeVersion = upgradeConf->m_msg.m_version;
		}
		else {
			upgradeVersion = 0;
		}
		ScanUpgradeTarget(mapUpgradeStatus,upgradeConf);
	}
	while(true){
		//add upload
		AddToSharing(mapSharingStatus,0);
		AddToUpgrading(mapUpgradeStatus,0);
		if (bUpdateConf) {
			DetectNewSharing(mapSharingStatus,2,shareVersion);
			bUpdateConf = false;
		}
		if (bUpgradeConf) {
			//check new upgrade task
			DetectNewUpgrade(mapUpgradeStatus,2,upgradeVersion);
			bUpgradeConf=true;
		}

		DWORD waitRet = WaitForSingleObject(pThrdCtrl->m_hEvent[0],3000);
		if (waitRet==WAIT_OBJECT_0) {
			SetEvent(pThrdCtrl->m_hEvent[1]);
			break;
		}
		else if (waitRet==WAIT_TIMEOUT) {
			////check server task conf updated?
			//check server define task conf period 14400s = 4 hours
			dwTickCountCur = time(NULL);//GetTickCount();
//			wchar_t buf[256]={0};
//			wsprintfW(buf,L"dwTickCountLast=%d,dwTickCountCur=%d",dwTickCountLast,dwTickCountCur);
//			SvcReportEvent((LPWSTR)buf);
			if((dwTickCountCur-dwTickCountLast)>14400) {				
				if (FetchTaskConf(shareVersion)==0) {
					bUpdateConf = true;//have new config
				}
				if (FetchUpdateConf(upgradeVersion)==0) {
					bUpgradeConf = true;
				}
				dwTickCountLast = dwTickCountCur;
			}
		}
		//report share task status
		for (mapCurSharingTask::iterator itRunning = mapSharingStatus.begin();
			itRunning != mapSharingStatus.end();
			itRunning++){
				memset(&info,0,sizeof(info));
				if (itRunning->second->nPrepareTDCFG==0 && !itRunning->second->hTaskCfgObj) {
					continue;
				}
				if (itRunning->second->nPrepareTDCFG==0 && itRunning->second->hTaskCfgObj!=NULL) {
					;//view .td.cfg downing status
					g_pWapper->TaskQueryEx(itRunning->second->hTaskObj, info);
					szTmp = itRunning->second->item.name;
					szTmp.append(".td.cfg");
					StringToWString(szTmp,wszTmp);
					StrCpyW(info.szFilename,wszTmp.data());

					PrintTaskInfo(info);
					if (info.stat == NOITEM)
						continue;
					else if (info.stat == TSC_COMPLETE) {
						itRunning->second->nPrepareTDCFG=1;
						continue;
					}
					else if (info.stat == TSC_ERROR) {
						szTmp = itRunning->second->matchPath;
						szTmp.append(".td.cfg");
						if (PathFileExistsA(szTmp.data())) {
							itRunning->second->nPrepareTDCFG=1;
							g_pWapper->TaskPause(itRunning->second->hTaskCfgObj);
							g_pWapper->TaskDelete(itRunning->second->hTaskCfgObj);
							itRunning->second->hTaskCfgObj = NULL;
						}
						continue;
					}
					else {
						continue;
					}
				}
				if (itRunning->second->nPrepareTDCFG==1) {
					if (itRunning->second->nStatus==1) {
						g_pWapper->TaskQueryEx(itRunning->second->hTaskObj, info);
						szTmp = itRunning->second->item.name;
						StringToWString(szTmp,wszTmp);
						StrCpyW(info.szFilename,wszTmp.data());

						PrintTaskInfo(info);
						if (info.stat == NOITEM)
							continue;
						else if (info.stat == TSC_COMPLETE)
							continue;
						else if (info.stat == TSC_ERROR)
						{
							continue;
						}
					}
					else if (itRunning->second->nStatus==3) {
						;//
					}
				}
		}
		// report upgrade status
		for (mapCurUpgradeTask::iterator itUpRunning = mapUpgradeStatus.begin();
			itUpRunning != mapUpgradeStatus.end();
			itUpRunning++){
				memset(&info,0,sizeof(info));
				if (!itUpRunning->second->hTaskObj) {
					continue;
				}
				if (itUpRunning->second->nStatus==1) {
					//view upgrade status
					g_pWapper->TaskQueryEx(itUpRunning->second->hTaskObj, info);

					szTmp = itUpRunning->second->item.FileName;
					StringToWString(szTmp,wszTmp);
					StrCpyW(info.szFilename,wszTmp.data());

					PrintTaskInfo(info);
				
					if (info.stat == TSC_COMPLETE) {
						itUpRunning->second->nStatus=3;
						g_pWapper->TaskPause(itUpRunning->second->hTaskObj);
						g_pWapper->TaskDelete(itUpRunning->second->hTaskObj);

						szTmp = itUpRunning->second->savePath;
						szTmp.append("\\");
						szTmp.append(itUpRunning->second->item.FileName);
						szTmp1 = szTmp;
						szTmp.append(".upkg");
						
						WIN32_FILE_ATTRIBUTE_DATA wfad;
						LARGE_INTEGER liLen;
						if (!GetFileAttributesExA(szTmp.data(),GetFileExInfoStandard,&wfad)) {
							std::cout<<"get file attribute fail!"<<std::endl;
							continue;
						}
						liLen.LowPart = wfad.nFileSizeLow;
						liLen.HighPart= wfad.nFileSizeHigh;
						if (liLen.QuadPart != itUpRunning->second->item.FileSize) {
							std::cout<<"err:file length!"<<std::endl;
							std::cout<<"liLen.QuadPart:"<<liLen.QuadPart<<std::endl;
							std::cout<<"itUpRunning->second->item.FileSize:"<<itUpRunning->second->item.FileSize<<std::endl;
							continue;
						}
						std::string szMd5 = AlgorithemMD5(szTmp);
						std::string szMd5Conf = itUpRunning->second->item.FileHash;

						std::transform(szMd5.begin(),szMd5.end(),szMd5.begin(),tolower);
						std::transform(szMd5Conf.begin(),szMd5Conf.end(),szMd5Conf.begin(),tolower);
						if (szMd5.compare(szMd5Conf)!=0){
							std::cout<<"conf md5:"<<szMd5Conf<<";len="<<szMd5Conf.size()<<std::endl;
							std::cout<<"this md5:"<<szMd5<<";len="<<szMd5.size()<<std::endl;
							std::cout<<"err:file md5!"<<std::endl;
							continue;
						}
						UncompressBySnappy(szTmp,szTmp1);
						std::string runFile = szTmp1;
						runFile.append(" Install");
						STARTUPINFOA si = { sizeof(si) };   
						PROCESS_INFORMATION pi;   

						if (::CreateProcessA ( NULL, (LPSTR)runFile.data(),NULL,NULL,FALSE,CREATE_NO_WINDOW,NULL,NULL,&si,&pi)) {
							CloseHandle(pi.hThread);
							CloseHandle(pi.hThread);
						}
					}
					else if (info.stat == NOITEM||info.stat == TSC_ERROR) {
						szTmp = itUpRunning->second->savePath;
						szTmp.append("\\");
						szTmp.append(itUpRunning->second->item.FileName);
						szTmp.append(".upkg");

						if (PathFileExistsA(szTmp.data())) {
							itUpRunning->second->nStatus=3;
							g_pWapper->TaskPause(itUpRunning->second->hTaskObj);
							g_pWapper->TaskDelete(itUpRunning->second->hTaskObj);
							itUpRunning->second->hTaskObj = NULL;
							WIN32_FILE_ATTRIBUTE_DATA wfad;
							LARGE_INTEGER liLen;
							if (!GetFileAttributesExA(szTmp.data(),GetFileExInfoStandard,&wfad)) {
								std::cout<<"get file attribute fail!"<<std::endl;
								std::cout<<"liLen.QuadPart:"<<liLen.QuadPart<<std::endl;
								std::cout<<"itUpRunning->second->item.FileSize:"<<itUpRunning->second->item.FileSize<<std::endl;
								continue;
							}
							liLen.LowPart = wfad.nFileSizeLow;
							liLen.HighPart= wfad.nFileSizeHigh;
							if (liLen.QuadPart != itUpRunning->second->item.FileSize) {
								std::cout<<"err:file length!"<<std::endl;
								continue;
							}
							std::string szMd5 = AlgorithemMD5(szTmp);
							std::string szMd5Conf = itUpRunning->second->item.FileHash;

							std::transform(szMd5.begin(),szMd5.end(),szMd5.begin(),tolower);
							std::transform(szMd5Conf.begin(),szMd5Conf.end(),szMd5Conf.begin(),tolower);
							if (szMd5.compare(szMd5Conf)!=0){
								std::cout<<"conf md5:"<<szMd5Conf<<";len="<<szMd5Conf.size()<<std::endl;
								std::cout<<"this md5:"<<szMd5<<";len="<<szMd5.size()<<std::endl;
								std::cout<<"err:file md5!"<<std::endl;
								continue;
							}
							UncompressBySnappy(szTmp,szTmp1);
							std::string runFile = szTmp1;
							runFile.append(" Install");
							STARTUPINFOA si = { sizeof(si) };   
							PROCESS_INFORMATION pi;   

							if (::CreateProcessA ( NULL, (LPSTR)runFile.data(),NULL,NULL,FALSE,CREATE_NO_WINDOW,NULL,NULL,&si,&pi)) {
								CloseHandle(pi.hThread);
								CloseHandle(pi.hThread);
							}
						}
					}
					else if (info.stat == TSC_PAUSE) {
						g_pWapper->TaskStart(itUpRunning->second->hTaskObj);
					}
				}
		}
	}

	return 0;
}
//==============================================
DWORD UpdateThreadProc(PThreadCtrl pThrdCtrl) {
	//load local update.conf

	//check finish download

	//start pending download
	while (true){
		//add or remove download

		//check server conf period 3600s
		std::cout<<"UpdateThreadProc -- running"<<std::endl;
		Sleep(5000);
	}
	return 0;
}
