
#pragma once
#include "global.h"

// ===============================================
typedef struct _tagTaskItem {
	std::string name;
	std::string storage;
	long size;
	std::string hash;
	long uploadSpeed;
	long downloadSpeed;
	std::string downloadUrl;
	std::string tdConfigUrl;
} TaskItem,*PTaskItem;
typedef std::map<std::string,PTaskItem> TaskItems;
typedef struct _tagMsg {
	TaskItems m_files;
	long m_version;
} TaskMsg,*TaskPMsg;
typedef struct _tagTaskConfDef {
	long m_code;
	TaskMsg m_msg;
} TaskConfDef,*PTaskConfDef;
//================================================
typedef struct _tagUpdateItem {
	std::string service;
	std::string updateMode;
	std::string LastVersion;
	long LastVersionCode;
	std::string ReleaseTime;
	std::string LowCompatible;
	std::string Arch;
	std::string FileName;
	long FileSize;
	std::string FileHash;
	std::string Download;
}UpdateItem,*PUpdateItem;
typedef std::map<std::string,PUpdateItem> UpdateItems;
typedef struct _tagUpdateMsg{
	UpdateItems m_files;
	long m_version;
} UpdateMsg,*PUpdateMsg;
typedef struct _tagUpdateConfDef {
	long m_code;
	UpdateMsg m_msg;
} UpdateConfDef,*PUpdateConfDef;
// ===============================================

typedef struct _tagCurSharingTask {
	TaskItem item;
	std::string matchPath;
	int nStatus; //0:not init;1:run;2:new;3:delete;4:not found
	HANDLE hTaskObj;
//---------for .td.cfg---------------------
	HANDLE hTaskCfgObj;
	int nPrepareTDCFG;//0:no td.cfg; 1:ok td.cfg
} CurSharingTask,*PCurSharingTask;

typedef struct _tagCurUpgradeTask {
	UpdateItem item;
	std::string savePath;
	int nStatus;//0:not init;1:run;2:new;3:down finish;4:delete
	HANDLE hTaskObj;
}CurUpgradeTask,*PCurUpgradeTask;

typedef std::map<std::string,PCurSharingTask> mapCurSharingTask;

typedef std::map<std::string,PCurUpgradeTask> mapCurUpgradeTask;
