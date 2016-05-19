
#pragma once
#include "util.h"

void AddToSharing(PCurSharingTask item,bool bForCfg = false);
void AddToSharing(mapCurSharingTask &mapSharingStatus,int status=0);

void AddToUpgrading(PCurUpgradeTask item);
void AddToUpgrading(mapCurUpgradeTask &mapUpgradeStatus,int status=0);

void DetectNewSharing (mapCurSharingTask &mapSharingStatus,int status,long &version);
void DetectNewUpgrade (mapCurUpgradeTask &mapUpgradeStatus,int status,long &version);

DWORD TaskThreadProc(PThreadCtrl);
DWORD UpdateThreadProc(PThreadCtrl);
