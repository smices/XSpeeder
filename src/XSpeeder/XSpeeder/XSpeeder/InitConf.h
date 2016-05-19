#pragma once

#include "util.h"
#include "xbStructDef.h"

std::string GetModulePath(HMODULE hModule = NULL);
std::string GetWindowsDriversPath();
std::string GetAppdataPath(std::string szCompany="HurricaneTeam");
std::string GetProgramProfilePath(std::string name);
std::string GetFilePathFromFile(std::string szFile);

void InitDir();

BOOL GetResourceFromHttp(const char *urls,const char *filename);
BOOL GetConfFromServ(std::string &serverUrl,std::string &fileName);

PTaskConfDef LoadLocalShareConf();
PTaskConfDef CreateTaskConfDef(std::string szJSConf);
void DestroyTaskConfDef(PTaskConfDef pTaskConf);
int FetchTaskConf(long version);

PCurSharingTask CreateSharingTaskItem(std::string strPath,const PTaskItem item);
std::string ScanTarget(PTaskItem pITem);
void ScanTarget(mapCurSharingTask &mapColl,PTaskConfDef taskConf);

PUpdateConfDef LoadLocalUpdateConf();
PUpdateConfDef CreateUpdateConfDef(std::string szJSConf);
void DestroyUpdateConfDef(PUpdateConfDef pUpdateConf);
int FetchUpdateConf(long version);

PCurUpgradeTask CreateUpgradeTaskItem(const PUpdateItem item, std::string strPath);
void ScanUpgradeTarget(mapCurUpgradeTask &mapColl,PUpdateConfDef pUpdateConf);
