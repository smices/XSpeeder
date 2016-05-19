
#pragma once

#include "stdafx.h"

bool GetResourceFromHttp(std::string urls,std::string *szbuf);

bool ParseConfigList(std::string &contents,std::vector<std::string> &lstItems);