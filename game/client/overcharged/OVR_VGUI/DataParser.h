#if !defined( DATAPARSER_H )
#define DATAPARSER_H
#ifdef _WIN32
#pragma once
#endif

#include "cbase.h"

#include "tier1/convar.h"
#include "tier1/KeyValues.h"
#include "filesystem.h"
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>
#include "vgui_controls/RadioButton.h"

using namespace std;

int GetData(IFileSystem* filesystem, const char *name);
string_t GetDataString(IFileSystem* filesystem, ConVar* pCvar, const char *name);
void SetData(IFileSystem* filesystem, vector<ConVar*> *gp_globalsCvarNames, int type = 0);
//сделать поддержку reshade и поддержку парсинга путей(строковых данных)
#endif